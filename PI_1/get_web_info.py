#-------------------------------------------------------------------------------
# Name:        get_web_info
# Purpose:     Get various web information and present to LED display
#
# Author:      weile
#
# Created:     15-12-2019
# Copyright:   (c) weiler 2019
# Licence:     <your licence>
#-------------------------------------------------------------------------------

import sys
import requests
from bs4 import BeautifulSoup
import time
import mqtt_utility as mqtt

def getStockValue():
  urlEricsson = "https://www.svd.se/bors/detail.php?insref=772"
  req = requests.get(urlEricsson)
  #print("Request to ", urlEricsson)
  #print("Recieved status code: ", req.status_code)
  soup = BeautifulSoup(req.text, 'html.parser')
  senast = soup.find("span", string="Senast:")
  temp = senast.find_next(string=True)
  kurs = temp.find_next(string=True)
  #print(kurs)
  return kurs

def presentStockValue():
  eriValue = getStockValue()
  message = ("Ericsson:" + eriValue + " :-")
  #print(message)
  mqtt.sendMessage("niwe/display_3", message, True)
  mqtt.sendMessage("niwe/display_3", message, False)
  time.sleep(10)
  mqtt.clearMessage("niwe/display_3", True)
  mqtt.clearMessage("niwe/display_3", False)


def main(argv):
  useCase = argv[0]
  if useCase == 'presentEriStock':
    presentStockValue()
  elif useCase == 'getEriStock':
    getStockValue()
  else:
    print ('erroneous input argument')
    #presentStockValue()


if __name__ == '__main__':
    main(sys.argv[1:])


