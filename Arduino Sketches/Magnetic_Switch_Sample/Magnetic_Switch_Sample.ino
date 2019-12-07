/* Arduino Tutorial: How to use a magnetic contact switch 
   Dev: Michalis Vasilakis // www.ardumotive.com // Date: 4/8/2016 */

const int buzzer = 3; 
const int sensor = 4;

int state; // 0 close - 1 open wwitch

void setup()
{
  pinMode(sensor, INPUT_PULLUP);
}

void loop()
{
  state = digitalRead(sensor);
  
  if (state == HIGH){
    tone(buzzer, 400);
  }
  else{
    noTone(buzzer);
  }
  delay(200);
}
