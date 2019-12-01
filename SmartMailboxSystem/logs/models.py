from django.db import models
from django.utils import timezone

from users.models import Profile

# Create your models here.
class Log(models.Model):
    datetime = models.DateTimeField(default=timezone.now)
    uid = models.ForeignKey(Profile, to_field='uid', on_delete=models.CASCADE)
    #uid = models.ForeignKey(Profile, on_delete=models.CASCADE)
    action = models.TextField()