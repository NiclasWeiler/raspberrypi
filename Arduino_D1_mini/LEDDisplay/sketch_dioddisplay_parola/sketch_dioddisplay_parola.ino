// Use the MD_MAX72XX library to scroll text on the display   ( skrivit av majicDesign  använd MD_PAROLA

#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>

// ****************************************************************
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "C:\Users\weile\OneDrive\Dokument\Arduino\personal_info.h"

// *****************************************************************

#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8 // 4
#define CLK_PIN     D5 //   green
#define DATA_PIN    D7 //   orange
#define CS_PIN      D6 //   yellow
#define HW_RST_PIN  D8   // Currently not taken out in the HW design

// SPI hardware interface
MD_Parola parola = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Scrolling parameters
#define MESSAGE_SCROLL_DELAY  2000 // in milliseconds

// Used for Parola
textPosition_t scrollAlign = PA_LEFT;
uint8_t scrollSpeed = 25;
uint16_t scrollPause = 0; // in milliseconds
textEffect_t scrollEffect = PA_SCROLL_LEFT;

// Global message buffers shared by Serial and Scrolling functions
// Size of string that is displayed
#define BUF_SIZE  75
#define NR_OF_CHAN 6   // This is the number of different channels to be presented on the Display

/*
 * LED_chan[0]: Used for messages to/from family members
 * LED_chan[1]: Used for messages to/from family members
 * LED_chan[2]: Used for messages to/from family members
 * LED_chan[3]: Used for alarms
 * LED_chan[4]: Used for WiFi problems
 * LED_chan[5]: Used for MQTT status
 * 
*/

enum display_channel
{
  text_1,
  text_2,
  text_3,
  alarms,
  WiFi_status,
  MQTT_status,    
};
char curMessage[BUF_SIZE] = " \0";
char changeMessage[BUF_SIZE] = " \0";
int displayInd = 0;
int topicNr = 0;
char LED_chan [NR_OF_CHAN][BUF_SIZE];

WiFiClient espClient;
int wiFiIndicator = 9;  // Indicates which WiFi in the wiFiList is active (9 indicates not known)
int mqttIndicator = -1;  // Indicates which mqtt server in the "mqttServerList" is active
PubSubClient client(espClient);

/* This function check if connected to wifi. If it is not connected, it tries to connect.
 * If connection does not work, A message is added to the display and next WiFi network is tried.
 */
void connectToWiFi(void)
{
  if (WiFi.status() != WL_CONNECTED) 
  {
    strcpy(LED_chan[WiFi_status], "No WiFi connection           \0");
    if (wiFiIndicator == 9)  // If indicator undifined then set to 0.
    {
      wiFiIndicator = -1;
    }
    wiFiIndicator++;
    wiFiIndicator = wiFiIndicator % nrOfWiFi;
    Serial.println("Connecting to WiFi: " + String(wiFiIndicator+1) + " .........");
    WiFi.begin(wiFiList[wiFiIndicator].ssid, wiFiList[wiFiIndicator].password);
  }
  else
  {
    strcpy(LED_chan[WiFi_status], " ");
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
        strcpy(LED_chan[MQTT_status], "No connection to MQTT server            \0");
      }
      else
      {
        strcpy(LED_chan[MQTT_status], " ");
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
  bool changed = false;
  for (int i = 0; i < NR_OF_CHAN-1 ; i++)    // Go through all text channels
  {
    Serial.println("Checking text channel " + String(displayInd +1));
    if (!(LED_chan[displayInd][0] == ' ' ))     // if not empty string found
    {
      strcpy(changeMessage, LED_chan[displayInd]);
      changed = true;
      Serial.println("Displaying text channel " + String(displayInd + 1) + ", text: " + String(changeMessage));
      displayInd++;
      displayInd = displayInd % (NR_OF_CHAN-1);
      break;
    }
    displayInd++;
    displayInd = displayInd % (NR_OF_CHAN-1);
  }
  if (!changed)  // No empty string found then show empty string
  {
    strcpy(changeMessage, " "); 
  }
}

/* This function will find next not empty message.
 * The search for next message will only be done every MESSAGE_SCROLL_DELAY ms
 * If all messages empty, then don't change message
 */

void scrollMessage(void)
{
  static uint32_t prevTime = 0;
  if (parola.displayAnimate())
  {
    // Is it time to scroll the text?
    if (millis()-prevTime >= MESSAGE_SCROLL_DELAY)
    {
      strcpy(curMessage, changeMessage);
      changeMessages();
      prevTime = millis();      // starting point for next time
    }
    parola.displayReset();
  }
}

void handleNewMessage(char* topic, byte* payload, unsigned int length)
// Callback function for subscribed data from MQTT server
{
  char *cp;
  char number = 0;
  int strLength;

  Serial.println(topic);

// Find out wich string is to be updated.
  if (String(topic) == String("niwe/display_1"))
  {
    cp = LED_chan[text_1];
    number = '1';
  }
  if (String(topic) == String("niwe/display_2"))
  {
    cp = LED_chan[text_2];;
    number = '2';
  }
  if (String(topic) == String("niwe/display_3"))
  {
    cp = LED_chan[text_3];;
    number = '3';
  }
    if (String(topic) == String("niwe/alarm"))
  {
    cp = LED_chan[alarms];;
    number = '4';
  }

/*  If Text not starts with space character, then copy string.
 *  Otherwise  add a space character (" ") to indicate empty string, to shut the display down.
 */
  if (!(String((char)payload[0]) == String(" ")))
  {
    // Check length of text and cut the lenght, if neccesary
    if (length > BUF_SIZE-5) // 5 Characters are added below for readability
    {
      strLength = BUF_SIZE-5;
    }
    else
    {
      strLength = length;
    }
    // Add text number and copy text
    *cp++ = number;
    *cp++ = char(':');
    *cp++ = char(' ');
    for (int i = 0; i < strLength ; i++)  // Add string
    {
      *cp++ = (char)payload[i];
    }
  }
  *cp++ = ' ';   
  *cp++ = '\0'; // Always end string with Null. Even empty string.
}

void setup()
{
  Serial.begin(9600);
  delay(2000);
  connectToWiFi();
  
  for (int i = 0; i < NR_OF_CHAN ; i++)    // Go through all text channels
  {
    LED_chan[i][0] = ' ';
  }
  
  client.setCallback(handleNewMessage);
  connectToClient();
  //  parola.setIntensity(1);  // 0-15
  //  parola.setSpeed(100); // speed in millisecs
  //  parola.setCharSpacing();
  parola.begin();
  parola.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

  // Test server
  // client.publish("niwe/display_1", "Hello from display_1");
}

int count = 0;
void loop()
{
  client.loop();
  scrollMessage();
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
