// Use the MD_MAX72XX library to scroll text on the display   ( skrivit av majicDesign  använd MD_PAROLA

// *********** Serial Peripheral Interface **************************
#include <SPI.h>
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }
//*******************************************************************

// Personal file with information of MQTT server and WiFi, including adresses and passwords
#include "C:\Users\weile\OneDrive\Dokument\Arduino\personal_info.h"

//  ********************* Used for WiFi ****************************
#include <ESP8266WiFi.h>
WiFiClient espClient;
#define WIFI_CHECK_INTERVAL  10000 // in milliseconds
int wiFiIndicator = 9;  // Indicates which WiFi in the wiFiList is active (9 indicates not known)

//******************************************************************

// ***************** Used for MQTT server **************************
#include <PubSubClient.h>
PubSubClient client(espClient);
#define MQTT_CHECK_INTERVAL  11000 // in milliseconds
int mqttIndicator = -1;  // Indicates which mqtt server in the "mqttServerList" is active
//*******************************************************************


//******************** Used for Parola and Display  ***************
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8 // 4
#define CLK_PIN     D5 //   green
#define DATA_PIN    D7 //   orange
#define CS_PIN      D6 //   yellow
#define HW_RST_PIN  D8   // Currently not taken out in the HW design

MD_Parola parola = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
textPosition_t scrollAlign = PA_LEFT;
uint8_t scrollSpeed = 25;
uint16_t scrollPause = 0; // in milliseconds
textEffect_t scrollEffect = PA_SCROLL_LEFT;
#define MESSAGE_SCROLL_INTERVAL  2000 // in milliseconds

// Global message buffers shared by Serial and Scrolling functions
// Size of string that is displayed
#define BUF_SIZE  75
//#define NR_OF_CHAN 7   // This is the number of different channels to be presented on the Display

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
  text_4,
  alarms,
  WiFi_status,
  MQTT_status,
  NR_OF_CHAN,    // This is the number of different channels to be presented on the Display
};

char curMessage[BUF_SIZE] = " \0";
char changeMessage[BUF_SIZE] = " \0";
int ledChannelInd = 0;
int topicNr = 0;
char LED_chan[NR_OF_CHAN][BUF_SIZE];

//******************************************************************

void setup()
{
  Serial.begin(9600);
  delay(2000);

  connectToWiFi();
  
  for (int i = 0; i < NR_OF_CHAN ; i++)    // Go through all text channels
  {
    LED_chan[i][0] = ' '; //Add space character which indicates "No text to Display"
  }

/*  Initiate messages for Demo/fake text
  strcpy(LED_chan[0], "Niclas will arrive late. Please put the wine in fridge!!!! ");
  strcpy(LED_chan[1], "Please peel the shrimps !!!!!!!!!");
  strcpy(LED_chan[2], "I am at Selmas, Can we get take away suchi??");
  strcpy(LED_chan[3], "The cottage alarm was triggered!");
  strcpy(LED_chan[4], "Cottage Temp: 90 deg");
*/

  client.setCallback(handleNewMessage);
  connectToClient();

  //  parola.setSpeed(100); // speed in millisecs
  //  parola.setCharSpacing();
  parola.begin();
  parola.setIntensity(0);  // 0-15
  parola.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

  // Test server
  // client.publish("niwe/display_1", "Hello from display_1");
}

void loop()
{
  client.loop();
  scrollMessage();
  connectToWiFi();
  connectToClient();
}
