import configparser
import requests
import json
import time
import datetime

# read configuration file:
config = configparser.ConfigParser()
config.read("AIS.conf")

# get configuration parameters
delay = int(config.get("config", "delay"))
ip = config.get("config", "ip")


# testing read ip:
message = requests.get("http://" + ip + "/")

# checking if ESP is present todo: check if it is the esp
if message.ok:
    print("ip correct! Receiving meeasurements")
else:
    print("ESP not found on given ip! Are you on the same network? (to set different ip, edit AIS.conf)")

# read the data as long as eternity
while True:
    # request datapack from esp and deserialize
    message = requests.get("http://" + ip + "/measure")
    data = json.loads(message.text)

    # write to log file
    with open("data.log", "a+") as data_log:
        data_log.write(datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S") +
                       " pulses: " + str(data["pulses"]) +
                       " light: " + str(data["light"]) +
                       " temperature: " + str(data["temperature"]) +
                       "\n")

    # wait delay amount of sec
    time.sleep(delay)
