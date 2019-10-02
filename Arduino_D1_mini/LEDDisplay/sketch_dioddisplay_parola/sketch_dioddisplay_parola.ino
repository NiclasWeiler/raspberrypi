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
char curMessage[BUF_SIZE] = " \0";

char changeMessage[BUF_SIZE] = " \0";
int change = 0;
int topicNr = 0;

int  nrOfDisplays = 6;
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

/* This function check if connected to wifi. If it is not connected, it tries to connect.
 * If connection does not work, A message is added to the display and next WiFi network is tried.
 */
void connectToWiFi(void)
{
  if (WiFi.status() != WL_CONNECTED) 
  {
    strcpy(WiFi_display, "No WiFi connection           \0");
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
  bool changed = false;
  for (int i = 0; i < nrOfDisplays ; i++)
  {
    //Serial.println("Checking text buffer " + String(change));
    switch(change)
    {
      case 0:
        if (!(display_1[0] == ' '))
        {
          strcpy(changeMessage, display_1);
          changed = true;
        }
        change++;
        break;
      case 1:  
        if (!(display_2[0] == ' '))
        {
          strcpy(changeMessage, display_2);
          changed = true;
        }
        change++;
        break;
      case 2:
        if (!(display_3[0] == ' '))
        {
          strcpy(changeMessage, display_3);
          changed = true;
        }
        change++;
        break;
      case 3:
        if (!(alarm_display[0] == ' '))
        {
          strcpy(changeMessage, alarm_display);
          changed = true;
        }
        change++;
        break;
      case 4:
        if (!(WiFi_display[0] == ' '))
        {
          strcpy(changeMessage, WiFi_display);
          changed = true;
        }
        change++;
        break;
      case 5:
        if (!(MQTT_display[0] == ' '))
        {
          strcpy(changeMessage, MQTT_display);
          changed = true;
        }
        change = 0;
        break;
    }
    if (changed)
    {
      Serial.println("Displaying text buffer " + String(change) + ", text: " + String(changeMessage));
      break;
    }
  }
  if (!changed) // If no messages found then display empty message
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
