from django.shortcuts import render, redirect
from django.contrib import messages
from django.contrib.auth.decorators import login_required
from .forms import UserRegisterForm, UserUpdateForm
from django.http import HttpResponse

import json
import paho.mqtt.client as mqtt
#import serial

uid_val = None

####### MQTT ########
def on_connect(client, userdata, rc):
    print("Connected with code" + str(rc))

def on_disconnect(client, userdata,rc):
    print("Disconnected with code " + str(rc))
    client.loop_stop()

def on_message(client, userdata, msg):
    global uid_val

    print("Topic:", str(msg.topic))
    print("Message:", str(msg.payload.decode().lstrip().rstrip()))

    if str(msg.topic) == "/uid_from_rfid":
      uid_val = str(msg.payload.decode().lstrip().rstrip())

client = mqtt.Client()
client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_message = on_message

client.connect("192.168.1.6", 1883, 60)
client.subscribe("/uid_from_rfid")
######## END ########

def get_uid_value(request):
    global uid_val
    
    return HttpResponse(json.dumps({
        'uid_value' : uid_val
    }))


# Create your views here.
def pre_register(request):
    global uid_val

    uid_val = None
    client.loop_start()
    return render(request, 'users/preregister.html', { 'startuid' : 'true'})

def register(request):
    global uid_val

    if request.method == 'POST':
        form = UserRegisterForm(request.POST)
        if form.is_valid():
            form.save()
            username = form.cleaned_data.get('username')
            messages.success(request, f'Your account has been created! You are now able to login')
            return redirect('login')

    else:
        form = UserRegisterForm(initial={'uid': uid_val})
    return render(request, 'users/register.html', { 'form': form })

@login_required
def profile(request):
    if request.method == 'POST':
        u_form = UserUpdateForm(request.POST, instance=request.user)

        if u_form.is_valid():
            u_form.save()
            messages.success(request, f'Your account has been updated!')
            return redirect('profile')
    else:
        u_form = UserUpdateForm(instance=request.user)

    context = {
        'u_form' : u_form
    }

    return render(request, 'users/profile.html', context)