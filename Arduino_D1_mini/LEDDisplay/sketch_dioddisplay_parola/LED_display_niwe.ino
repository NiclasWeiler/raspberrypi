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
