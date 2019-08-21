// Use the MD_MAX72XX library to scroll text on the display   ( skrivit av majicDesign  använd MD_PAROLA
//
// Demonstrates the use of the callback function to control what
// is scrolled on the display text.
//
// User can enter text on the serial monitor and this will display as a
// scrolling message on the display.
// Speed for the display is controlled by a pot on SPEED_IN analog in.

// PIR detector
// http://henrysbench.capnfatz.com/henrys-bench/arduino-sensors-and-input/arduino-hc-sr501-motion-sensor-tutorial/
//
// This version tries to used the PIR but does not work
//#include "Arduino.h"

#include <MD_MAX72xx.h>
#include <SPI.h>
#define USE_POT_CONTROL 0
#define PRINT_CALLBACK  0
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
//#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8 // 4
#define CLK_PIN     D5 // 13  // or SCK  // Blå
#define DATA_PIN    D7 // 11  // or MOSI // Grå
#define CS_PIN      D6 // 10  // or SS   // Lila
#define SUMMER_PIN  D0   // 
#define PIR_PIN     D1 //D3   //
#define IR_RECV_PIN D4   //
#define IR_SEND_PIN D3 //D2   //
#define TEMP_PIN    D2 // D1   //
#define HW_RST_PIN  D8   // Currently not taken out in the HW design

//#define CLK_PIN   7  // or SCK  // Brun
//#define DATA_PIN  5  // or MOSI // Röd
//#define CS_PIN    4  // or SS   // Orange

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Scrolling parameters
#if USE_POT_CONTROL
#define SPEED_IN  A0
#else
#define SCROLL_DELAY  75  // in milliseconds
#endif // USE_POT_CONTROL

#define CHAR_SPACING  1 // pixels between characters

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE  75
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;

char changeMessage[BUF_SIZE];
bool changeMessageAvailable = false;
int change = 0;

uint16_t  scrollDelay;  // in milliseconds

/*
char oldMessage[BUF_SIZE];
bool presens = false;
bool rundisplay = false;

bool pirState = false;
*/

/*
void clearMessages(void)
{

  // Check if someone is present to continue, or stay in "case 0" until someone shows up
  if (digitalRead(PIR_PIN) == HIGH) 
  {
    presens = true;
  }
  else
  {
    presens = false;
  }
  Serial.print(presens);
  Serial.print(rundisplay);
  Serial.print("\n");
  if ((presens == true) && (rundisplay == false)) 
  {
    // start running as someone is there
    Serial.print("Start --- \n");
    strcpy(changeMessage, oldMessage);
    changeMessageAvailable = true;
    rundisplay = true;
  }  
  else {   
    if ((presens == false) && (rundisplay == true))
    {
      // stop running the display and store the old message for waking up
      Serial.print("--- Stop \n");
      strcpy(oldMessage, curMessage);
      //strcpy(changeMessage, "                                                                           ");// 75 blanks
      strcpy(changeMessage, " ");
      changeMessageAvailable = true;
      rundisplay = false;
    }
  }
}
*/
void changeMessages(void)
{
  switch(change)
  {
    case 0:
      //strcpy(changeMessage, "Join the Ericsson Hackathon - March 28th!     ");
      strcpy(changeMessage, "Join the Ericsson Hackathon    ");
      changeMessageAvailable = true;
      change++;
      break;
    case 1:  
      strcpy(changeMessage, "Build your first Internet of Things    ");
      changeMessageAvailable = true;
      change++;
      break;
    case 2:
      strcpy(changeMessage, "Get your Boat, Greenhouse, Caravan or anything else connected!     ");
      changeMessageAvailable = true;
      change++;
      break;
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
        // Daniel added motion detection with PIR here
        // clearMessages();
        if (changeMessageAvailable)  // there is a new message waiting
        {
          strcpy(curMessage, changeMessage);  // copy it in
          changeMessageAvailable = false;
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
  if (millis()-prevTime >= scrollDelay)
  {
    mx.transform(MD_MAX72XX::TSL);  // scroll along - the callback will load all the data
    prevTime = millis();      // starting point for next time
  }
}

uint16_t getScrollDelay(void)
{
#if USE_POT_CONTROL
  uint16_t  t;
  t = analogRead(SPEED_IN);
 //Serial.println(t);
  t = map(t, 0, 1023, 25, 250);
  return(t);
#else
  return(SCROLL_DELAY);
#endif

}

void setup()
{
  mx.begin();
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);
#if USE_POT_CONTROL
  pinMode(SPEED_IN, INPUT);
#else
  scrollDelay = SCROLL_DELAY;
#endif
  pinMode(PIR_PIN, INPUT);    // initialize sensor as an input
//  strcpy(curMessage, "MQTT Display - subscribes to topic edallam/MQTT_Display/text              ");
  strcpy(curMessage, "Welcome to Ericsson Lund         Enjoy your day here!              ");
  //strcpy(oldMessage, "                                                                           ");// 75 blanks
  newMessage[0] = '\0';
  //Serial.begin(57600);
  Serial.begin(115200);
// Serial.print("\n[MD_MAX72XX Message Display]\nType a message for the scrolling display\nEnd message line with a newline\n");
  Serial.print("End message line with a newline\n");

  // Test the led
  pinMode(IR_SEND_PIN, OUTPUT);
  digitalWrite(IR_SEND_PIN,HIGH);
  delay(200);
  digitalWrite(IR_SEND_PIN,LOW);
  delay(200);
  digitalWrite(IR_SEND_PIN,HIGH);
  delay(200);
  digitalWrite(IR_SEND_PIN,LOW);
  //pinMode(HW_RST_PIN, OUTPUT);
  //digitalWrite(HW_RST_PIN,HIGH);
}
int count = 0;
void loop()
{
  scrollDelay = getScrollDelay();
  changeMessages();
  //clearMessages();
  readSerial();
  scrollText();
  count++;
  //Serial.println(count);
  if (count == 10000) {
    Serial.print("count\n");
    count = 0;
  }

  /*
  // Check if someone is present to continue, or stay in "case 0" until someone shows up
  if (digitalRead(PIR_PIN) == LOW)
  {
    Serial.println("No one there!");
  }   
  else {   
    scrollText(); 
  }
  */

}
