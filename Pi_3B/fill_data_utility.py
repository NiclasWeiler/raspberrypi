#-------------------------------------------------------------------------------
# Name:        fill_data_utility
# Purpose:     Functions to fill variuos datameasurements
#              of raspberry pi into file
#
# Author:      Niclas
#
# Created:     13-08-2018
# Copyright:   (c) Niclas 2018
# Licence:     <your licence>
#-------------------------------------------------------------------------------

#!/usr/bin/python
import subprocess
import time
import mail_utility as mail
import Adafruit_DHT
import mqtt_utility as mqtt

def getTempHumToFile(filename):

  # get local time
  localtime = time.asctime( time.localtime(time.time()) )

  #get humidity temperature
  humidity, temperature = Adafruit_DHT.read_retry(Adafruit_DHT.DHT11, 17)
  if (type(1.2) != type(humidity)): # If no values received set values to 999
    humidity = 999
    temperature = 999
  else: # Check if temperature is abnormal
    if (checkAlarm(temperature, 40, 0)):
      mail.sendAlarm("Air temperatur Larm !!!!!!!!!!!!!!!!!!!!\n"
                      + "Air Temp = " + format(temperature, '.1f'), 'noAttach')
      # Send Alarm to both local MQTT server and External MQTT server
      mqtt.sendMessage("niwe/alarm_display", "Air Temperature alarm, sommarstuga!!!!", True)
      mqtt.sendMessage("niwe/alarm_display", "Air Temperature alarm, sommarstuga!!!!", False)

  # Write data into file
  f = open(filename, 'a')
  f.write(localtime + ' Temp=' + format(temperature, '.1f') + "* Humidity="  + \
           format(humidity, '.1f') + "%\n")
  f.close()


def getPiTempToFile(filename):

  # get local time
  localtime = time.asctime( time.localtime(time.time()) )

  #get GPU temperature
  p = subprocess.Popen(["vcgencmd", "measure_temp"], stdout=subprocess.PIPE)
  gpuTemp_str, err = p.communicate()
  gpuTempExt_str = "GPU " + gpuTemp_str  # for printing to file
  # Check GPU Temperature
  startP = gpuTemp_str.find("=") + 1 # Temperature value starts after "=" sign
  gpuTemp_str = gpuTemp_str[startP:startP + 4]
  gpuTempValue = float(gpuTemp_str)
  # Send alarm if temp is not within max and min
  if (checkAlarm(gpuTempValue, 60, 30)):
    mail.sendAlarm("Pi temperatur Larm !!!!!!!!!!!!!!!!!!!!\n"
                    + "GPU Temp = " + gpuTemp_str, 'noAttach')
    # Send Alarm to both local MQTT server and External MQTT server
    mqtt.sendMessage("niwe/alarm_display", "Pi Temperature alarm, sommarstuga!!!!", True)
    mqtt.sendMessage("niwe/alarm_display", "Pi Temperature alarm, sommarstuga!!!!", False)


  #get CPU temperature
  p = subprocess.Popen(["cat", "/sys/class/thermal/thermal_zone0/temp"], \
                       stdout=subprocess.PIPE)
  cpuTemp, err = p.communicate()
  cpuTempValue = float(cpuTemp)/1000
  cpuTemp_str = "CPU temp =" + str(cpuTempValue) + " C"
  # Send alarm if temp is not within max and min
  if (checkAlarm(cpuTempValue, 60, 30)):
    mail.sendAlarm("Pi temperatur Larm !!!!!!!!!!!!!!!!!!!!\n"
               + "CPU Temp = " + str(cpuTempValue) + " C", 'noAttach')
    mqtt.sendMessage("niwe/alarm_display", "Pi Temperature alarm, sommarstuga!!!!", True)
    mqtt.sendMessage("niwe/alarm_display", "Pi Temperature alarm, sommarstuga!!!!", False)

  # Write data into file
  f = open(filename, 'a')
  f.write(localtime + " " + cpuTemp_str + ", " + gpuTempExt_str)
  f.close()

def checkAlarm(value,maximum,minimum):
  alarm = False
  if value > maximum:
    alarm = True
  if value < minimum:
    alarm = True
  return alarm

def takePicture(picName):
  subprocess.call(["raspistill", "-o", picName])
