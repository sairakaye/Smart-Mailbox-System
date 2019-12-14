#include <SPI.h>
#include <MFRC522.h>

/** For RFID **/
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

String id = "";

/** FOR MAGNETIC SENSORS **/
int ms1 = 4;
int ms2 = 5;
int ms3 = 6;

/** FOR BUTTONS **/
int b1 = A1;
int b2 = A2;
int b3 = A3;

/** LED **/
int led1 = A0;
int led2 = 8;
int led3 = 7;

/** BUZZER **/
int buzzer = A4;

/** FOR TIME **/
int ctrTime = 0;

/** STATES **/
byte boxToOpen1 = LOW;
byte boxToOpen2 = LOW;
byte boxToOpen3 = LOW;

/** ALLOW ACCESS **/
boolean canAccess = true; // temporarily without MQTT.

void setup() {
  Serial.begin(9600);
  // Setup for RFID
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();

  // Setup for Sensors
  pinMode(ms1, INPUT_PULLUP);
  pinMode(ms2, INPUT_PULLUP);
  pinMode(ms3, INPUT_PULLUP);

  // Setup for LED
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);

  // Setup for Buttons
  pinMode(b1, INPUT);
  pinMode(b2, INPUT);
  pinMode(b3, INPUT);
}

void loop() {
  byte readButton1 = digitalRead(b1);
  byte readButton2 = digitalRead(b2);
  byte readButton3 = digitalRead(b3);

  byte readSensor1 = digitalRead(ms1);
  byte readSensor2 = digitalRead(ms2);
  byte readSensor3 = digitalRead(ms3);  

  if (mfrc522.PICC_IsNewCardPresent()) {
    if ( mfrc522.PICC_ReadCardSerial()) {

      /** INITIALIZE IT BACK TO EVERYTHING **/
      id = "";
      ctrTime = 0;
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
      boxToOpen1 = LOW;
      boxToOpen2 = LOW;
      boxToOpen3 = LOW;

      Serial.print("Tag UID: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        //Serial.print(mfrc522.uid.uidByte[i], HEX);
        id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        id.concat(String(mfrc522.uid.uidByte[i], HEX));
        id.toUpperCase();
      }
      
      Serial.println(id);

      if (canAccess) {
        if (readButton1 == HIGH) {
          boxToOpen1 = HIGH;
        }
  
  
        if (readButton2 == HIGH) {
          boxToOpen2 = HIGH;
        }
  
  
        if (readButton3 == HIGH) {
          boxToOpen3 = HIGH;
        }
      } else {
        id = "";
      }

      mfrc522.PICC_HaltA();
    }
  }

  if (id.length() > 0) {
    if (ctrTime <= 1500) {
      if (boxToOpen1 == HIGH) {
        digitalWrite(led1, HIGH);
      }

      if (boxToOpen2 == HIGH) {
        digitalWrite(led2, HIGH);
      }

      if (boxToOpen3 == HIGH) {
        digitalWrite(led3, HIGH);
      }

      if ((boxToOpen1 == LOW && readSensor1 == HIGH) ||
        (boxToOpen2 == LOW && readSensor2 == HIGH) ||
        (boxToOpen3 == LOW && readSensor3 == HIGH)) {
        tone(buzzer, 400);
      } else {
        noTone(buzzer);
      }
      
      ctrTime++;
    } else {
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);

      boxToOpen1 = LOW;
      boxToOpen2 = LOW;
      boxToOpen3 = LOW;
      ctrTime = 0;
      id = "";
      // canAccess = false;
    }
  } else {
    if (readSensor1 == HIGH || readSensor2 == HIGH || readSensor3 == HIGH) {
      tone(buzzer, 400);
    } else {
      noTone(buzzer);
    }
  }

  delay(10);
}
