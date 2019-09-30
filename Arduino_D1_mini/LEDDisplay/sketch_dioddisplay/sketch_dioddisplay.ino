// Use the MD_MAX72XX library to scroll text on the display   ( skrivit av majicDesign  använd MD_PAROLA
//
// Demonstrates the use of the callback function to control what
// is scrolled on the display text.
//
// User can enter text on the serial monitor and this will display as a
// scrolling message on the display.

// PIR detector
// http://henrysbench.capnfatz.com/henrys-bench/arduino-sensors-and-input/arduino-hc-sr501-motion-sensor-tutorial/
//
// This version tries to used the PIR but does not work
//#include "Arduino.h"

#include <MD_MAX72xx.h>
#include <SPI.h>

// Added for MQTT by Niclas
// ****************************************************************
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "C:\Users\weile\OneDrive\Dokument\Arduino\personal_info.h"

// *****************************************************************

#define PRINT_CALLBACK  0
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }


// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
//#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8 // 4
#define CLK_PIN     D5 //   green
#define DATA_PIN    D7 //   orange
#define CS_PIN      D6 //   yellow
                 // VCC 5V, brown
                 // Ground, red
//#define SUMMER_PIN  D0
//#define PIR_PIN     D1
//#define IR_RECV_PIN D4
//#define IR_SEND_PIN D3
//#define TEMP_PIN    D2
#define HW_RST_PIN  D8   // Currently not taken out in the HW design

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Scrolling parameters

#define SCROLL_DELAY  40 //75  // in milliseconds
#define CHAR_SPACING  1 // pixels between characters

// Global message buffers shared by Serial and Scrolling functions
// Size of string that is displayed
#define BUF_SIZE  75
char curMessage[BUF_SIZE] = " \0";

char changeMessage[BUF_SIZE] = " \0";
int change = 1;
int topicNr = 0;

char display_1[BUF_SIZE] = " ";     // IOT message buffer 1
char display_2[BUF_SIZE] = " ";     // IOT message buffer 2
char display_3[BUF_SIZE] = " ";     // IOT message buffer 3
char alarm_display[BUF_SIZE] = " "; // Alarm messages
char WiFi_display[BUF_SIZE] = " ";  // Only internal error messages
char MQTT_display[BUF_SIZE] = " ";  // Only internal error messages

WiFiClient espClient;
int wiFiIndicator = 9;  // Indicates which WiFi in the wiFiList is active (9 indicates not known)
int mqttIndicator = -1;  // Indicates which mqtt server in the "mqttServerList" is active
PubSubClient client(espClient);

/* This function check if connected to wifi. If it is not connecte, it ties to connect.
 * If connection does not work A message is added to the display and loop is stopped.
 */
void connectToWiFi(void)
{
  if (WiFi.status() != WL_CONNECTED) 
  {
    if (wiFiIndicator == 9)  // If indicator undifined then set to 0.
    {
      wiFiIndicator = -1;
    }
    wiFiIndicator++;
    wiFiIndicator = wiFiIndicator % nrOfWiFi;
    Serial.println("Connecting to WiFi: " + String(wiFiIndicator+1) + " .........");
    strcpy(WiFi_display, "No WiFi connection           \0");
    WiFi.begin(wiFiList[wiFiIndicator].ssid, wiFiList[wiFiIndicator].password);
  }
  else
  {
    strcpy(WiFi_display, " ");
    Serial.println("Connected to WiFi: " + String(wiFiIndicator+1));
  }
}

void connectToClient(void)
{
  if (!client.connected()) 
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      mqttIndicator++;
      mqttIndicator = mqttIndicator % nrOfMqtt;
      Serial.println("Connecting to MQTT server: " + String(mqttIndicator+1));
      client.setServer(mqttServerList[mqttIndicator].serverAdress, mqttServerList[mqttIndicator].port);
      if (!client.connect("Niclas LED Display 1", mqttServerList[mqttIndicator].user, mqttServerList[mqttIndicator].password )) 
      {
        Serial.print("failed with state: ");
        Serial.println (client.state());
        strcpy(MQTT_display, "No connection to MQTT server            \0");
      }
      else
      {
        strcpy(MQTT_display, " ");
        client.subscribe("niwe/display_1");
        client.subscribe("niwe/display_2");
        client.subscribe("niwe/display_3");
        client.subscribe("niwe/alarm_display");
      }
    }
  }
  else
  {
      Serial.println("Connected to MQTT server: " + String(mqttIndicator+1));
  }
}

void changeMessages(void)
{
  switch(change)
  {
    case 0:
      strcpy(changeMessage, display_1);
      change++;
      break;
    case 1:  
      strcpy(changeMessage, display_2);
      change++;
      break;
    case 2:
      strcpy(changeMessage, display_3);
      change++;
      break;
    case 3:
      strcpy(changeMessage, alarm_display);
      change++;
      break;
    case 4:
      strcpy(changeMessage, WiFi_display);
      change++;
      break;
    case 5:
      strcpy(changeMessage, MQTT_display);
      change=0;
      break;
  }
}

void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col)
// Callback function for data that is being scrolled off the display
{
#if PRINT_CALLBACK
  Serial.print("\n cb ");
  Serial.print(dev);
  Serial.print(' ');
  Serial.print(t);
  Serial.print(' ');
  Serial.println(col);
#endif
}
uint8_t scrollDataSource(uint8_t dev, MD_MAX72XX::transformType_t t)
// Callback function for data that is required for scrolling into the display
{
  static char   *p = curMessage;
  static uint8_t  state = 0;
  static uint8_t  curLen, showLen;
  static uint8_t  cBuf[8];
  uint8_t colData;
  // finite state machine to control what we do on the callback
  switch(state)
  {
    case 0: // Load the next character from the font table
      showLen = mx.getChar(*p++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state++;

      // if we reached end of message, reset the message pointer
      if (*p == '\0')
      {
        p = curMessage;     // reset the pointer to start of message
        // Change display message  
        strcpy(curMessage, changeMessage);  // copy it in
        changeMessages();  // update to next message
      }
      // !! deliberately fall through to next state to start displaying
      
    case 1: // display the next part of the character
      colData = cBuf[curLen++];
      if (curLen == showLen)
      {
        showLen = CHAR_SPACING;
        curLen = 0;
        state = 2;
      }
      break;
    case 2: // display inter-character spacing (blank column)
      colData = 0;
      if (curLen == showLen)
        state = 0;
      curLen++;
      break;
    default:
      state = 0;
  }
  return(colData);
}

void scrollText(void)
{
  static uint32_t prevTime = 0;
  // Is it time to scroll the text?
  if (millis()-prevTime >= SCROLL_DELAY)
  {
    mx.transform(MD_MAX72XX::TSL);  // scroll along - the callback will load all the data
    prevTime = millis();      // starting point for next time
  }
}

void handleNewMessage(char* topic, byte* payload, unsigned int length)
// Callback function for subscribed data from MQTT server
{
  static bool emptyString = false;
  static char *cp;
  static char number = 0;

// Find out wich string is to be updated.
  if (String(topic) == String("niwe/display_1"))
  {
    cp = display_1;
    number = '1';
  }
  if (String(topic) == String("niwe/display_2"))
  {
    cp = display_2;
    number = '2';
  }
  if (String(topic) == String("niwe/display_3"))
  {
    cp = display_3;
    number = '3';
  }
    if (String(topic) == String("niwe/alarm"))
  {
    cp = alarm_display;
    number = '4';
  }

/*  If Text not starts with space character, then copy tring.
 *  Otherwise copy nothing to create empty string, to shut the display down.
 */
  if (!(String((char)payload[0]) == String(" ")))
  {
    // Check length of text and cut the lenght, if neccesary
    if (length > BUF_SIZE-5) // 5 Characters are added below for readability
    {
      length = BUF_SIZE-4;
    }
    // Add text number and copy text
    *cp++ = number;
    *cp++ = char(':');
    *cp++ = char(' ');
    for (int i = 0; i < length ; i++)  // Add string
    {
      *cp++ = (char)payload[i];
    }
  }
//  *cp++ = ' ';   
  *cp++ = '\0'; // Always end string with Null. Even empty string.
}

void setup()
{
  Serial.begin(9600);
  delay(2000);
  connectToWiFi();
  
  client.setCallback(handleNewMessage);
  connectToClient();

  mx.begin();
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);

  // Test server
  // client.publish("niwe/display_1", "Hello from display_1");

}
int count = 0;
void loop()
{
  client.loop();
  scrollText();
  count++;
  if (count == 500000) 
  {
    connectToWiFi();
    count = 0;
  }
  if (count == 250000)
  {
    connectToClient();
  }
}
