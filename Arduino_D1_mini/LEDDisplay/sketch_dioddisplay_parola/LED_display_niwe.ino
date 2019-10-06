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
    if (millis()-prevTime >= MESSAGE_SCROLL_INTERVAL)
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
