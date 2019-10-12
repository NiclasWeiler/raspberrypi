void changeMessages(void)
{
  bool changed = false;
  for (int i = 0; i < NR_OF_CHAN-1 ; i++)    // Go through all text channels
  {
    Serial.println("Checking text channel " + String(ledChannelInd +1));
    if (!(LED_chan[ledChannelInd][0] == ' ' ))     // if not empty string found
    {
      strcpy(changeMessage, LED_chan[ledChannelInd]);
      changed = true;
      Serial.println("Displaying text channel " + String(ledChannelInd + 1) + ", text: " + String(changeMessage));
      ledChannelInd++;
      ledChannelInd = ledChannelInd % (NR_OF_CHAN-1);
      break;
    }
    ledChannelInd++;
    ledChannelInd = ledChannelInd % (NR_OF_CHAN-1);
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

  Serial.println("Change in channel " + String(topic) + " received");

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
    if (String(topic) == String("niwe/alarm_display"))
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
      Serial.println("payload: " + String(payload[i]));
      if (payload[i] == 195) // Incomming Swedish letters consists of two chars, first one is == 196 (decimal)
      {
        //Check second char and add the corresponding ascii (for the parola ascii table) to the out string
        switch(payload[i+1])
        {
          case 132:  // Ä
            *cp++ = char(196);
            i++;
            break;
          case 133:  // Å
            *cp++ = char(197);
            i++;
            break;
          case 150:  // Ö
            *cp++ = char(214);
            i++;
            break;
          case 164:  // ä
            *cp++ = char(228);
            i++;
            break;
          case 165:  // å
            *cp++ = char(229);
            i++;
            break;
          case 182:  // ö
            *cp++ = char(246);
            i++;
            break;
        }    
      }
      else
      {
        *cp++ = payload[i];  
      }
    }
  }
  *cp++ = ' ';   
  *cp++ = '\0'; // Always end string with Null. Even empty string.
}
