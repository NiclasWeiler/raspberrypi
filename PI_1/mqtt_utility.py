#-------------------------------------------------------------------------------
# Name:        mqtt_utility
# Purpose:     Utilities for communicating with MQTT server.
#
#
# Author:      Niclas
#
#
#
# Created:     13-08-2018
# Copyright:   (c) Niclas 2018
# Licence:     <your licence>
#-------------------------------------------------------------------------------

#!/usr/bin/python
# Import required libraries
import subprocess
import os
import paho.mqtt.client as mqtt

localBroker_address = "192.168.1.200"
extBroker_address = "weilerhoka.hopto.org"
clientName = "Lund_Pi_1"

def sendMessage(channelStr, messageStr, localMessage):

  mqttClient = mqtt.Client(clientName) #create new instance

  if (localMessage):
    mqttClient.connect(localBroker_address)
  else:
    mqttClient.connect(extBroker_address, port=1883)

  mqttClient.publish(channelStr, messageStr)
  mqttClient.disconnect()

def clearMessage(channelStr, localMessage):

  mqttClient = mqtt.Client(clientName) #create new instance

  if (localMessage):
    mqttClient.connect(localBroker_address)
  else:
    mqttClient.connect(extBroker_address, port=1883)

  mqttClient.publish(channelStr, " ")
  mqttClient.disconnect()
