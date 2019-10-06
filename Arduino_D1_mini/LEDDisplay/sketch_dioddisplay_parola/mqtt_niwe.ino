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
    prevTime = millis();      // starting point for next time
  }
}
