/* This function checks if connected to wifi. If it is not connected, it tries to connect to network 0 (first time) 
 *  or the last connected network.
 * If connection still does not work, the next WiFi network is tried.
 */
void connectToWiFi(void)
{
  static uint32_t prevTime = 0;
  static int nrOfFailed = 0;
  if (millis()-prevTime >= WIFI_CHECK_INTERVAL)
  {   
    if (WiFi.status() != WL_CONNECTED) 
    {
      strcpy(LED_chan[WiFi_status], "No WiFi connection           \0");
      if (wiFiIndicator == 9)  // If indicator undifined then set to 0.
      {
        wiFiIndicator = 0;
      }
      if (nrOfFailed != 0)
      {
        wiFiIndicator++;
        wiFiIndicator = wiFiIndicator % nrOfWiFi;
      }
      Serial.println("Connecting to WiFi: " + String(wiFiIndicator+1) + " .........");
      WiFi.begin(wiFiList[wiFiIndicator].ssid, wiFiList[wiFiIndicator].password);
      nrOfFailed++;
    }
    else
    {
      strcpy(LED_chan[WiFi_status], " ");
      Serial.println("Connected to WiFi: " + String(wiFiIndicator+1));
      nrOfFailed = 0;
    }
    prevTime = millis();      // starting point for next time
  }
}
