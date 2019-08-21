#-------------------------------------------------------------------------------
# Name:
# Purpose: Use cases to be used for crontab jobs
#
# Author:      Niclas
#
# Created:     20-03-2018
# Copyright:   (c) Niclas 2018
# Licence:     <your licence>
#-------------------------------------------------------------------------------

#!/usr/bin/python
# Import required libraries

import sys
import time
import mail_utility as mail
import fill_data_utility as fill
import subprocess
import pi_sound_detector as sound
import datetime
import pi_record_video as rec
import pi_stepmotor as stepMotor


def preparePiTemperatureMessage():

    filename = "pi_temperature.txt"
    toAdress = "sodrahoka.weiler@gmail.com"
    fromAdress = "sodrahoka.weiler@gmail.com"
    subject = "PI Temperature"
    mail.prepareGmail(filename, toAdress, fromAdress, subject)

def prepareTempHumMessage():

    filename = "air_temperature.txt"
    toAdress = "sodrahoka.weiler@gmail.com"
    fromAdress = "sodrahoka.weiler@gmail.com"
    subject = "Air Temperature and Humidity"
    mail.prepareGmail(filename, toAdress, fromAdress, subject)

def preparePiSoundLogMessage():

    filename = "soundcheck.log"
    toAdress = "sodrahoka.weiler@gmail.com"
    fromAdress = "sodrahoka.weiler@gmail.com"
    subject = "soundcheck.log"
    mail.prepareGmail(filename, toAdress, fromAdress, subject)

def getPiTempToFile():

    filename = "pi_temperature.txt"
    fill.getPiTempToFile(filename)

def sendPiTemperatureMessage():

    filename = "pi_temperature.txt"
    toAdress = "sodrahoka.weiler@gmail.com"
    fromAdress = "sodrahoka.weiler@gmail.com"
    subject = "PI Temperature"

    # Send Gmail with pi temp data
    mail.sendGmail(filename, toAdress)
    # remove sent email message
    subprocess.call(["rm", filename])
    # Prepare new data message
    mail.prepareGmail(filename, toAdress, fromAdress, subject)

def getTempHumToFile():

    filename = "air_temperature.txt"
    fill.getTempHumToFile(filename)


def sendTempHumMessage():

    filename = "air_temperature.txt"
    toAdress = "sodrahoka.weiler@gmail.com"
    fromAdress = "sodrahoka.weiler@gmail.com"
    subject = "Air Temperature and Humidity"

    # Send Gmail with pi temp data
    mail.sendGmail(filename, toAdress)
    # remove sent email message
    subprocess.call(["rm", filename])
    # Prepare new data message
    mail.prepareGmail(filename, toAdress, fromAdress, subject)


def sendAttachment(attachment):
    toAdress = "sodrahoka.weiler@gmail.com"
    subject = "Test Attachment"

    # Send Gmail with pi temp data
    mail.sendGmailAttach(attachment, toAdress, subject)

def doSoundCheck(debug):
    sound.soundcheck(debug)

def doTakePicture():
    subject = "Bild, sommarstuga"
    toAdress = "sodrahoka.weiler@gmail.com"
    nowtime = datetime.datetime.now()
    nowtime_str = mail.replaceSpace(str(nowtime))
    picName = "//home//pi//Pictures//" + nowtime_str + ".jpeg"
    fill.takePicture(picName)
    # Send Gmail with picture
    mail.sendGmailAttach(picName, toAdress, subject)

def sendSoundCheckLog():

    filename = "soundcheck.log"
    toAdress = "sodrahoka.weiler@gmail.com"
    fromAdress = "sodrahoka.weiler@gmail.com"
    subject = "soundcheck.log"

    # Send Gmail with pi temp data
    mail.sendGmail(filename, toAdress)
    # remove sent email message
    subprocess.call(["rm", filename])
    # Prepare new data message
    mail.prepareGmail(filename, toAdress, fromAdress, subject)

def record_video():

  localtime = time.asctime( time.localtime(time.time()) )
  timestring = rec.replaceSpace(localtime)
  fileName = 'movie_' + timestring + '.h264'
  rec.recordMovie(fileName)
  print ("Movie recorded local time: " + localtime)

def move_camera(distance, direction):

  stepMotor.moveCamera(distance, direction)
  print ("Camera rotated  " + str(distance) + " sequencies to the " + direction)

def main(argv):

  useCase = argv[0]

  if useCase == 'getPiTempToFile':
    getPiTempToFile()
  elif useCase == 'sendPiTemperatureMessage':
    sendPiTemperatureMessage()
  elif useCase == 'preparePiTemperatureMessage':
    preparePiTemperatureMessage()
  elif useCase == 'preparePiSoundLogMessage':
    preparePiSoundLogMessage()
  elif useCase == 'soundcheck':
    doSoundCheck(False)
  elif useCase == 'soundcheckDbg':
    doSoundCheck(True)
  elif useCase == 'sendSoundCheckLog':
    sendSoundCheckLog()
  elif useCase == 'sendAttachment':
    #print (argv[1])
    sendAttachment(argv[1])
  elif useCase == 'prepareTempHumMessage':
    prepareTempHumMessage()
  elif useCase == 'getTempHumToFile':
    getTempHumToFile()
  elif useCase == 'sendTempHumMessage':
    sendTempHumMessage()
  elif useCase == 'takePicture':
    doTakePicture()
  elif useCase == 'recordVideo':
    record_video()
  elif useCase == 'moveCamera':
    move_camera(int(argv[1]), argv[2])
  else:
    print ('erroneous input argument')

if __name__ == "__main__":
    main(sys.argv[1:])

