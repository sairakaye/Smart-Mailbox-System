from django.db import models
from django.utils import timezone

from django.contrib.auth.models import User

# Create your models here.
class Log(models.Model):
    datetime = models.DateTimeField(default=timezone.now)
    user = models.ForeignKey(User, on_delete=models.CASCADE)
    action = models.TextField()