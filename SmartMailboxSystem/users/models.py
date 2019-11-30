from django.db import models
from django.contrib.auth.models import User

# Create your models here.
class Profile(models.Model):
    username = models.OneToOneField(User, on_delete=models.CASCADE)
    uid = models.CharField(max_length=70, blank=False)

    def __str__(self):
        return f'{self.user.username} Profile'
