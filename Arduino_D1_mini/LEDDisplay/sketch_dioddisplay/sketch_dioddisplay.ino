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

const char* ssid = "AdamsFamily";
const char* password =  "karlssonmelvin07";

const char* mqttServer = "weilerhoka.hopto.org";
const int   mqttPort = 1883;
const char* mqttUser = "pi";
const char* mqttPassword = "Willeberry3";

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
#define BUF_SIZE  75
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;

char changeMessage[BUF_SIZE];
bool changeMessageAvailable = false;
int change = 1;
int topicNr = 0;

char display_1[BUF_SIZE];
char display_2[BUF_SIZE];
char display_3[BUF_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);

void changeMessages(void)
{
  switch(change)
  {
    case 0:
      strcpy(changeMessage, display_1);
      changeMessageAvailable = true;
      change++;
      break;
    case 1:  
      strcpy(changeMessage, display_2);
      changeMessageAvailable = true;
      change++;
      break;
    case 2:
      strcpy(changeMessage, display_3);
      changeMessageAvailable = true;
      change=0;
      break;
/*      
    case 3:
      strcpy(changeMessage, "From sensor to data on your phone in 24 hours      ");
      changeMessageAvailable = true;
      change++;
      break;
    case 4:
      strcpy(changeMessage, "Everyone is welcome - You teach what you know and learn from others    ");
      changeMessageAvailable = true;
      change=0;
      break;
*/
  }
}

void readSerial(void)
{
  static uint8_t  putIndex = 0;
  
  while (Serial.available())
  {
    newMessage[putIndex] = (char)Serial.read();
    if ((newMessage[putIndex] == '\n') || (putIndex >= BUF_SIZE-3)) // end of message character or full buffer
    {
      // put in a message separator and end the string
      newMessage[putIndex++] = ' ';
      newMessage[putIndex] = '\0';
      // restart the index for next filling spree and flag we have a message waiting
      putIndex = 0;
      newMessageAvailable = true;
    }
    else if (newMessage[putIndex] != '\r')
      // Just save the next char in next location
      putIndex++;
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
        if (changeMessageAvailable)  // there is a new message waiting
        {
          strcpy(curMessage, changeMessage);  // copy it in
          changeMessages();  // update to next message
        }
        if (newMessageAvailable)  // there is a new message waiting
        {
          strcpy(curMessage, newMessage);      // copy it in
          newMessageAvailable = false;
        }
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
  static uint32_t                        prevTime = 0;
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

  bool emptyString = false;
  
  if (String((char)payload[0]) == String(" ")) // If Text starts with space character
  {                                           // then create an Empty string
    emptyString = true;
  }

  if (String(topic) == String("niwe/display_1"))
  {
    topicNr = 1;
  }
  if (String(topic) == String("niwe/display_2"))
  {
    topicNr = 2;
  }
  if (String(topic) == String("niwe/display_3"))
  {
    topicNr = 3;
  }
  
  switch(topicNr)
  {

    case 1: // Display 1
      if (!emptyString)
      {
        strcpy(display_1, "1: ");  // Add display number to beginning of string
        for (int i = 0; i < length ; i++)  // Add string
        {
          display_1[i + 3] = (char)payload[i];
        }
        display_1[length + 3] = ' ';
        display_1[length + 4] = '\0';
      }
      else
      {
        display_1[0] = ' ';
        display_1[1] = '\0';
      }
      break;
      
    case 2:  //Display 2
      if (!emptyString)
      {
        strcpy(display_2, "2: ");  // Add display number to beginning of string
        for (int i = 0; i < length ; i++)  // Add string
        {
          display_2[i + 3] = (char)payload[i];
        }
        display_2[length + 3] = ' ';
        display_2[length + 4] = '\0';
      }
      else
      {
        display_2[0] = ' ';
        display_2[1] = '\0';
      }
      break;

    case 3:  //  Display 3
      if (!emptyString)
      {
        strcpy(display_3, "3: ");  // Add display number to beginning of string
        for (int i = 0; i < length ; i++)  // Add string
        {
          display_3[i + 3] = (char)payload[i];
        }
        display_3[length + 3] = ' ';
        display_3[length + 4] = '\0';
      }
      else
      {
        display_3[0] = ' ';
        display_3[1] = '\0';
      }
      break;

    default:
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
//    Serial.print("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(handleNewMessage);

  while (!client.connected()) 
  {
//    Serial.println("Connecting to MQTT...");
    if (client.connect("Niclas LED Display 1", mqttUser, mqttPassword )) 
    {
      Serial.println("connected");  
    } 
    else
    {
      Serial.println("failed with state ");
//      Serial.print(client.state());
      delay(2000);
    }
  }
 
  client.subscribe("niwe/display_1");
  client.subscribe("niwe/display_2");
  client.subscribe("niwe/display_3");

  strcpy(display_1, "No messages           ");
  strcpy(display_2, " ");
  strcpy(display_3, " ");
  strcpy(curMessage, display_1);
  changeMessages(); // Initiate first message
  newMessage[0] = '\0';

  mx.begin();
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);

  // Test server
  // client.publish("niwe/display_1", "Hello from display_1");

}
int count = 0;
void loop()
{
//  changeMessages();
  client.loop();
  readSerial();
  scrollText();
  count++;
  //Serial.println(count);
  if (count == 10000) 
  {
    Serial.print("count\n");
    count = 0;
  }

}
