from django import forms
from django.contrib.auth.models import User
from django.contrib.auth.forms import UserCreationForm
from .models import Profile

class UserRegisterForm(UserCreationForm):
    value = ""
    email = forms.EmailField()
    uid = forms.CharField(widget = forms.HiddenInput(), max_length=71, required=True)

    def set_value(self, value):
        self.value = value
        print(self.value)

    class Meta:
        model = User
        fields = ['username', 'email', 'password1', 'password2', 'uid']
    
    def save(self, commit=True):
        user = super(UserRegisterForm, self).save(commit=False)
        user.uid = self.cleaned_data["uid"]
        if commit:
            user.save()
        return user

class UserUpdateForm(forms.ModelForm):
    email = forms.EmailField()

    class Meta:
        model = User
        fields = ['username', 'email']