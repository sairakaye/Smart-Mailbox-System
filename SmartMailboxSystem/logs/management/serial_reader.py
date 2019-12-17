#from django.core.management.base import BaseCommand, CommandError
#from django.utils import timezone
#from logs.models import Log
#from users.models import Profile
#from django.conf import settings
import serial
import paho.mqtt.client as mqtt

class Command(BaseCommand):
    arduino = serial.Serial('COM8',9600)
    mqttc = mqtt.Client("python pub") 

    mqttc.connect("localhost",1883)

    while True: 
        data = arduino.readline().decode().lstrip().rstrip()

        if data[0] == 'A':
            mqttc.publish("/nodemcu_only", str(data))