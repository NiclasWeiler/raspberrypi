#-------------------------------------------------------------------------------
# Name:
# Purpose: Test cases to test python code on raspberry Pi
#
# Author:      Niclas
#
# Created:     20-03-2018
# Copyright:   (c) Niclas 2018
# Licence:     <your licence>
#-------------------------------------------------------------------------------

#!/usr/bin/python
# Import required libraries

import mail_utility as mail
import fill_data_utility as fill

def test_prepareGmail():

    filename = "pi_temperature_test.txt"
    toAdress = "weiler.niclas@gmail.com"
    fromAdress = "sodrahoka.weiler@gmail.com"
    subject = "test sommarstuga"
    mail.prepareGmail(filename, toAdress, fromAdress, subject)

def test_sendGmail():

    filename = "pi_temperature_test.txt"
    toAdress = "weiler.niclas@gmail.com"
    mail.sendGmail(filename, toAdress)

def test_getPiTempToFile():

    filename = "pi_temperature_test.txt"
    fill.getPiTempToFile(filename)

def test_sendAlarm():

    alarmText = "Huga Huga Huuga"
    mail.sendAlarm(alarmText, 'noAttachment')

def test_sendDataMessage():

    filename = "pi_data_Message_test.txt"
    toAdress = "weiler.niclas@gmail.com"
    fromAdress = "sodrahoka.weiler@gmail.com"
    subject = "test sommarstuga"

    mail.prepareGmail(filename, toAdress, fromAdress, subject)
    fill.getPiTempToFile(filename)
    mail.sendDataMessage(filename)


# Test cases

#test_prepareGmail()
#test_getPiTempToFile()
#test_sendGmail()
test_sendAlarm()
#test_sendDataMessage()



