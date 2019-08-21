#-------------------------------------------------------------------------------
# Name:        mail_utility
# Purpose:     Utilities for mail handling.
#              Only valid for sodrahoka.weiler@gmail.com as sender
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

def prepareGmail(filename, toAdress, fromAdress, subject):

  # Create new message
  p = subprocess.call(["touch", filename])

  # Prepare header and text
  f = open(filename, 'a')
  f.write("To: " + toAdress + "\n")
  f.write("From: " + fromAdress + "\n")
  f.write("Subject: " + subject + "\n\n")
  f.close()

def sendGmail(filename, toAdress):

  #send email
  os.system("ssmtp " + toAdress + " < " + filename)
  #subprocess.call(["ssmtp", "weiler.niclas@gmail.com", "< " + filename])
  #p = subprocess.Popen(["ssmtp", "weiler.niclas@gmail.com", "<", filename], stdout=subprocess.PIPE)

def sendGmailAttach(attachment, toAdress, subject):

  #send email with attachment
  subject = ("\'" + subject + "\'")
  os.system ("mpack -s " + subject + " " + attachment + " " + toAdress)


def sendAlarm(alarmText, attachment):

  filename = "alarm_message.txt"
  toAdress = "weiler.niclas@gmail.com, sodrahoka.weiler@gmail.com, \
              niclas.weiler@ericsson.com, melvin.sillens@gmail.com, \
              ann.karlsson@yahoo.se"
  fromAdress = "sodrahoka.weiler@gmail.com"
  prepareGmail(filename, toAdress, fromAdress, "Alarm sommarstuga!!!")

  # Write alarm text into file
  f = open(filename, 'a')
  f.write(alarmText)
  f.close()
  #send email with alarmText
  sendGmail(filename, toAdress)
  # remove sent email message
  subprocess.call(["rm", filename])

  # If attachment, send email with attachment
  if attachment != 'noAttach':
    sendGmailAttach(attachment, toAdress, "Alarm attachment")



def sendDataMessage(filename):

#  filename = "data_message.txt"
  toAdress = "weiler.niclas@gmail.com"
  fromAdress = "sodrahoka.weiler@gmail.com"

  #send email
  sendGmail(filename, toAdress)

  # remove sent email message
  subprocess.call(["rm", filename])

  # Prepare next data message
  prepareGmail(filename, toAdress, fromAdress, "Sommarstuga data")


# This function will replace all space characters in "str" with a "_"
def replaceSpace( str ):
  st = list(str)
  res = str.find(" ")
  while res != -1:
    st[res] = "_"
    res = str.find(" ")
    str = ''.join(st)
  return str


