#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <PubSubClient.h>

/** FOR MQTT **/

// Do not change.
byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x02};

// IP address of the microcontroller.
IPAddress ip(169, 254, 78, 243);
// IP address of the computer.
IPAddress server(169, 254, 78, 241);

const char *mqtt_topic = "/people_count";
const char *mqtt_topic2 = "/button_trigger";

char message[50];
char receiver[11];

/** ALLOW ACCESS **/
boolean canAccess = true; // temporarily without MQTT.

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    receiver[i] = payload[i];

  }

  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until connection is completed
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("Connected");
      //client.publish("outTopic", "Hello world!");
      client.subscribe("/button_trigger");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}

void print_local_IP() {
  Serial.print("My IP address: ");
  for (byte nCtr = 0; nCtr < 4; nCtr++) {
    Serial.print(Ethernet.localIP()[nCtr], DEC);
    Serial.print(".");
  }
  Serial.println();
}

/** For RFID **/
#define SS_PIN 10
#define RST_PIN 9

/** FOR MAGNETIC SENSORS **/
#define MDS_PIN_1 2
#define MDS_PIN_2 3

/** FOR BUTTONS **/
#define BTN_PIN_1 A2
#define BTN_PIN_2 A3

#define REG_BTN_PIN A0

/** LED **/
#define REG_LED_PIN A1

#define RLED_PIN_1 4
#define RLED_PIN_2 5

#define GLED_PIN_1 6
#define GLED_PIN_2 7

/** BUZZER **/
#define BUZZER_PIN 8

MFRC522 mfrc522(SS_PIN, RST_PIN);
String id = "";

/** FOR TIME **/
int ctrTime = 0;

/** STATES **/
byte boxToOpen1 = LOW;
byte boxToOpen2 = LOW;

void setup() {
  Serial.begin(9600);
  // Setup for Ethernet
  client.setServer(server, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);
  print_local_IP();

  // Setup for RFID
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();

  // Setup for Sensors
  pinMode(MDS_PIN_1, INPUT_PULLUP);
  pinMode(MDS_PIN_2, INPUT_PULLUP);

  // Setup for LED
  pinMode(GLED_PIN_1, OUTPUT);
  pinMode(GLED_PIN_2, OUTPUT);
  digitalWrite(GLED_PIN_1, LOW);
  digitalWrite(GLED_PIN_2, LOW);

  pinMode(REG_LED_PIN, OUTPUT);
  digitalWrite(REG_LED_PIN, LOW);

  pinMode(RLED_PIN_1, OUTPUT);
  pinMode(RLED_PIN_2, OUTPUT);
  digitalWrite(RLED_PIN_1, HIGH);
  digitalWrite(RLED_PIN_2, HIGH);

  // Setup for Buttons
  pinMode(BTN_PIN_1, INPUT);
  pinMode(BTN_PIN_2, INPUT);
  pinMode(REG_BTN_PIN, INPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  
  byte readButton1 = digitalRead(BTN_PIN_1);
  byte readButton2 = digitalRead(BTN_PIN_2);
  
  byte readRegButton = digitalRead(REG_BTN_PIN);
  
  byte readSensor1 = digitalRead(MDS_PIN_1);
  byte readSensor2 = digitalRead(MDS_PIN_2);

  if (mfrc522.PICC_IsNewCardPresent()) {
    if ( mfrc522.PICC_ReadCardSerial()) {

      /** INITIALIZE IT BACK TO EVERYTHING **/
      id = "";
      ctrTime = 0;
      digitalWrite(GLED_PIN_1, LOW);
      digitalWrite(GLED_PIN_2, LOW);
      digitalWrite(RLED_PIN_1, HIGH);
      digitalWrite(RLED_PIN_2, HIGH);
      boxToOpen1 = LOW;
      boxToOpen2 = LOW;

      Serial.print("Tag UID: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        id.concat(String(mfrc522.uid.uidByte[i], HEX));
        id.toUpperCase();
      }
      
      Serial.println(id);

      if (readRegButton == HIGH) {
        // publish here for readRegButton
        digitalWrite(REG_LED_PIN, HIGH);
        Serial.println("Here for registration!");
        delay(100);
        digitalWrite(REG_LED_PIN, LOW);
        id = "";
      } else if(canAccess) {
        if (readButton1 == HIGH) {
          boxToOpen1 = HIGH;
        }
        
        if (readButton2 == HIGH) {
          boxToOpen2 = HIGH;
        }
      } else {
        id = "";
      }
      
      mfrc522.PICC_HaltA();
    }
  }
  
  if (id.length() > 0) {
    if (ctrTime <= 2250) {
      if (boxToOpen1 == HIGH) {
        digitalWrite(GLED_PIN_1, HIGH);
        digitalWrite(RLED_PIN_1, LOW);
      }

      if (boxToOpen2 == HIGH) {
        digitalWrite(GLED_PIN_2, HIGH);
        digitalWrite(RLED_PIN_2, LOW);
      }

      if ((boxToOpen1 == LOW && readSensor1 == HIGH) ||
        (boxToOpen2 == LOW && readSensor2 == HIGH)) {
        tone(BUZZER_PIN, 400);
      } else {
        noTone(BUZZER_PIN);
      }
      
      ctrTime++;
    } else {
      digitalWrite(GLED_PIN_1, LOW);
      digitalWrite(GLED_PIN_2, LOW);
      digitalWrite(RLED_PIN_1, HIGH);
      digitalWrite(RLED_PIN_2, HIGH);

      boxToOpen1 = LOW;
      boxToOpen2 = LOW;
      ctrTime = 0;
      id = "";
      // canAccess = false;
    }
  } else {
    if (readSensor1 == HIGH || readSensor2 == HIGH) {
      tone(BUZZER_PIN, 400);
    } else {
      noTone(BUZZER_PIN);
    }
  }

  delay(10);
}
