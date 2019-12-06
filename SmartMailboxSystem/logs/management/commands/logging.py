from django.core.management.base import BaseCommand, CommandError
from django.utils import timezone
from logs.models import Log
from django.conf import settings

import paho.mqtt.client as mqtt

####### MQTT ########
def on_connect(client, userdata, rc):
    print("Connected with code" + str(rc))

def on_message(client, userdata, msg):
    print("Topic:", str(msg.topic))
    print("Message:", str(msg.payload.decode().lstrip().rstrip()))

    new_log = Log.objects.create(datetime=timezone.now, uid='00', action="Hatdog")
    new_log.save()
    #if str(msg.topic) == "/uid_from_rfid":
    #  uid_val = str(msg.payload.decode().lstrip().rstrip())

class Command(BaseCommand):
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(settings.SERVER_IP, 1883, 60)
    client.subscribe("/adding_logs")
    client.loop_forever()