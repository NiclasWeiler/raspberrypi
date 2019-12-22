#-------------------------------------------------------------------------------
# Name:        Sound detector LM393
# Purpose:     Checks if continuous sound is present for more than 3 seconds
#              This function will check sound for 3500 seconds
#              (approx 58,5 minutes) and then exit.
#
# Author:      Niclas
#
# Created:     08-09-2018
# Copyright:   (c) Niclas 2018
# Licence:     <your licence>
#-------------------------------------------------------------------------------

#Monitors GPIO pin 4 for input. A sound module is set up on physical pin 4.

import RPi.GPIO as GPIO
import time
import datetime
import os
import mail_utility as mail
import fill_data_utility as fill
import mqtt_utility as mqtt

count = 0
totalCount = 0
lastSoundDetectTime = time.time() - 1
dbg = False
nrOfAlarms = 0
logFile = 0

def DETECTED(SOUND_PIN):
  global count
  global lastSoundDetectTime
  global totalCount
  global dbg
  global nrOfAlarms

  totalCount += 1
  nowtime = datetime.datetime.now()
  nowInSec = time.time()
  if dbg:
    print("sound detected")
  if nowInSec - lastSoundDetectTime < 0.51:
    count += 1
  else:
    count = 1
  # Print to soundcheck.log if continous sound
  if (count > 1):
    alarmMessage = (str(nowtime) + ", Sound detected! Total: " + \
                    str(totalCount) + ", Sound count: " + str(count) + \
                    ", Alarm count: " + str(nrOfAlarms) + \
                    ", Time diff: " + \
                    str(nowInSec - lastSoundDetectTime) + "\n")

    logFile = open("soundcheck.log", 'a')
    logFile.write (alarmMessage)
    if count == 7:
        alarmDetectStr = (str(nowtime) + " Alarm detected! \n")
        logFile.write(alarmDetectStr)
        #print (alarmDetectStr)
    logFile.close()

    if dbg:
      print (alarmMessage)

  lastSoundDetectTime = nowInSec

  if count == 7: # (7-1)*0,5 = 3 sec of contineous sound
    count = 0
    nrOfAlarms += 1
    if nrOfAlarms == 1: # If first alarm, send picture
      nowtime_str = mail.replaceSpace(str(nowtime))
      picName = "//home//pi//Pictures//" + nowtime_str + ".jpeg"
      fill.takePicture(picName)
      mail.sendAlarm(alarmMessage, picName)
      mqtt.sendMessage("niwe/alarm_display", "Sound alarm, sommarstuga!!!!", True)
      mqtt.sendMessage("niwe/alarm_display", "Sound alarm, sommarstuga!!!!", False)
    elif nrOfAlarms < 6:     # Send only alarm message, without attachment
      mail.sendAlarm(alarmMessage, 'noAttach')

  return nowtime

def soundcheck(debug):
  global dbg
  dbg = debug
  GPIO.setmode(GPIO.BCM)
  SOUND_PIN = 4
  GPIO.setup(SOUND_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
  #GPIO.setup(SOUND_PIN, GPIO.IN)

  # Ignore edges for 500 ms
  GPIO.add_event_detect(SOUND_PIN, GPIO.FALLING, callback=DETECTED, bouncetime=500)

  startTime = time.time()
  now = startTime

  while (now - startTime < 3500):  # 3500 Time period for checking sound
    time.sleep(60)
    now = time.time()
  GPIO.remove_event_detect(SOUND_PIN)
  GPIO.cleanup()
