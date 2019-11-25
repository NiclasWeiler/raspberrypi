void connectToClient(void)
{
  static uint32_t prevTime = 0;
  if (millis()-prevTime >= MQTT_CHECK_INTERVAL)
  {   
    if (!client.connected()) 
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        mqttIndicator++;
        mqttIndicator = mqttIndicator % nrOfMqtt;
        Serial.println("Connecting to MQTT server: " + String(mqttIndicator+1));
        client.setServer(mqttServerList[mqttIndicator].serverAdress, mqttServerList[mqttIndicator].port);
        if (!client.connect("Niclas LED Blue Display", mqttServerList[mqttIndicator].user, mqttServerList[mqttIndicator].password )) 
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
          client.subscribe("niwe/blue_display");
          client.subscribe("niwe/alarm_display");
        }
      }
    }
    else
    {
        Serial.println("Connected to MQTT server: " + String(mqttIndicator+1));
    }
    prevTime = millis();      // starting point for next time
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
  else if (String(topic) == String("niwe/display_2"))
  {
    cp = LED_chan[text_2];;
    number = '2';
  }
  else if (String(topic) == String("niwe/display_3"))
  {
    cp = LED_chan[text_3];;
    number = '3';
  }
  else if (String(topic) == String("niwe/blue_display"))
  {
    cp = LED_chan[text_4];;
    number = '4';
  }
  else if (String(topic) == String("niwe/alarm_display"))
  {
    cp = LED_chan[alarms];;
    number = '5';
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
      //Serial.println("payload: " + String(payload[i]));
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
