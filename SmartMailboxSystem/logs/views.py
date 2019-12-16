from django.shortcuts import render
from logs.models import Log
from users.models import Profile
from django.conf import settings

from django.shortcuts import render, redirect
from django.contrib import messages
from django.contrib.auth.decorators import login_required
from django.http import HttpResponse
# Create your views here.
def home(request):
    return render(request, 'logs/home.html')

def logs(request):
    user = Profile.objects.get(user=request.user)
    user_logs = Log.objects.filter(uid=user.uid).values()
    
    return render(request, 'logs/user_logs.html', {'logs' : user_logs })