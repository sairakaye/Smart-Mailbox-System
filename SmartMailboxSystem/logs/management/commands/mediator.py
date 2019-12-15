from django.core.management.base import BaseCommand, CommandError
from django.utils import timezone
from logs.models import Log
from users.models import Profile
from django.conf import settings

import paho.mqtt.client as mqtt

####### MQTT ########
def on_connect(client, userdata, rc):
    print("Connected with code" + str(rc))

def on_message(client, userdata, msg):
    print("Topic:", str(msg.topic))
    print("Message:", str(msg.payload.decode().lstrip().rstrip()))
    
    message = str(msg.payload.decode().lstrip().rstrip())
    split_msg = message.split()

    if str(msg.topic) == "/adding_logs":
        if split_msg[0] == "OPEN" or split_msg[0] == "FORCED":
            try:
                user = Profile.objects.get(uid=split_msg[1])
            except Profile.DoesNotExist:
                user = None

            if user != None:        
                act = ""

                if split_msg[0] == "OPEN":
                    act = "The user opened the box."
                elif split_msg[0] == "FORCED":
                    act = "The user forced the box to open."
                
                new_log = Log.objects.create(uid=user, action=act)
                new_log.save()

                print("Log added for " + user.user.username + "!")
    elif str(msg.topic) == "/uid_from_rfid":
        if split_msg[0] == "REQ":
            try:
                user = Profile.objects.get(uid=split_msg[1])
            except Profile.DoesNotExist:
                user = None
            
            if user != None:
                client.publish("/status_request", str("G " + split_msg[1]))
            else:
                client.publish("/status_request", str("D " + split_msg[1]))


class Command(BaseCommand):
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(settings.SERVER_IP, 1883, 60)
    client.subscribe("/adding_logs")
    client.subscribe("/uid_from_rfid")
    
    print("Logging started...")

    client.loop_forever()