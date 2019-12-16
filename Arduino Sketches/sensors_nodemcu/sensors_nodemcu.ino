/** Typical pin layout used:
   ----------------------------------
               MFRC522      Node
               Reader/PCD   MCU
   Signal      Pin          Pin
   ----------------------------------
   RST/Reset   RST          D1 (GPIO5)
   SPI SS      SDA(SS)      D2 (GPIO4)
   SPI MOSI    MOSI         D7 (GPIO13)
   SPI MISO    MISO         D6 (GPIO12)
   SPI SCK     SCK          D5 (GPIO14)
   3.3V        3.3V         3.3V
   GND         GND          GND
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define BUZZER D0

#define GREEN_LED1 D3
#define RED_LED1 D1
#define LOCK1 D2

#define GREEN_LED2 D5
#define RED_LED2 D4
#define LOCK2 D6


byte boxToOpen1 = LOW;
byte boxToOpen2 = LOW;

boolean canAccess = true;

int accessStatus = 0;
int ctrTime = 0;

/** For MQTT **/
const char* ssid = "Esp8266";
const char* password = "animo0920";
const char* mqtt_server = "192.168.43.75";

const char *uid_topic = "/uid_from_rfid";
const char *logs_topic = "/adding_logs";
const char *sub_topic = "/request_access";
const char *operate_topic = "/nodemcu_only";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

char data[50];
String id = "";
String temp;

/** MELODY **/
// The melody array
//int melody[] = {
//  NOTE_FS5, NOTE_FS5, NOTE_D5, NOTE_B4, NOTE_B4, NOTE_E5,
//  NOTE_E5, NOTE_E5, NOTE_GS5, NOTE_GS5, NOTE_A5, NOTE_B5,
//  NOTE_A5, NOTE_A5, NOTE_A5, NOTE_E5, NOTE_D5, NOTE_FS5,
//  NOTE_FS5, NOTE_FS5, NOTE_E5, NOTE_E5, NOTE_FS5, NOTE_E5
//};
//
//// The note duration, 8 = 8th note, 4 = quarter note, etc.
//int durations[] = {
//  8, 8, 8, 4, 4, 4,
//  4, 5, 8, 8, 8, 8,
//  8, 8, 8, 4, 4, 4,
//  4, 5, 8, 8, 8, 8
//};
//
//int songLength = sizeof(melody)/sizeof(melody[0]);

/** END MELODY **/


/** Functions under MQTT **/
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  int i;
  for (i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data[i] = payload[i];
  }

  if (String(topic).equalsIgnoreCase("/nodemcu_only")) {
    if (data[0] == 'A') {
      if (data[1] == '0') {
        boxToOpen1 = LOW;
        digitalWrite(RED_LED1, HIGH);
        digitalWrite(GREEN_LED1, LOW);
      } else if (data[1] == '1') {
        boxToOpen1 = HIGH;
        digitalWrite(RED_LED1, LOW);
        digitalWrite(GREEN_LED1, HIGH);
      }

      if (data[2] == '0') {
        boxToOpen2 = LOW;
        digitalWrite(RED_LED2, HIGH);
        digitalWrite(GREEN_LED2, LOW);
      } else if (data[2] == '1') {
        boxToOpen2 = HIGH;
        digitalWrite(RED_LED1, LOW);
        digitalWrite(GREEN_LED1, HIGH);
      }

      int index = 4;

      while (index < strlen(data)) {
        id.concat(data[index]);
        index++;
      }

      Serial.println(id);
    } else if (data[0] == 'X') {
      boxToOpen1 = LOW;
      boxToOpen2 = LOW;
      digitalWrite(RED_LED1, HIGH);
      digitalWrite(GREEN_LED1, LOW);
      digitalWrite(RED_LED2, HIGH);
      digitalWrite(GREEN_LED2, LOW);
      id = "";
    }
  }

  /*
    if (String(topic).equalsIgnoreCase("/status_request")) {
    if (payloadReceiver[0] == 'G') {
      accessStatus = 1;
    } else if (payloadReceiver[0] == 'D') {
      accessStatus = -1;
    }
    }
  */
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(pub_topic, "hello world");
      // ... and resubscribe
      client.subscribe(sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Setup for WiFi
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(RED_LED1, OUTPUT);
  pinMode(LOCK1, INPUT_PULLUP);
  pinMode(GREEN_LED1, OUTPUT);

  pinMode(RED_LED2, OUTPUT);
  pinMode(LOCK2, INPUT_PULLUP);
  pinMode(GREEN_LED2, OUTPUT);

  digitalWrite(RED_LED1, HIGH);
  digitalWrite(GREEN_LED1, LOW);

  digitalWrite(RED_LED2, HIGH);
  digitalWrite(GREEN_LED2, LOW);

  client.subscribe(operate_topic);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  byte readLock1 = digitalRead(LOCK1);
  byte readLock2 = digitalRead(LOCK2);

  if ((readLock1 == HIGH && boxToOpen1 == LOW) ||
      (readLock2 == HIGH && boxToOpen2 == LOW)) {
    tone(BUZZER, 400);
  } else {
    noTone(BUZZER);
  }

  if (id.length() > 0) {
    if (readLock1 == HIGH && boxToOpen1 == HIGH) {
      String sendLog = "OPEN 1 ";
      sendLog.concat(id);
      sendLog.toCharArray(msg, sendLog.length() + 1);
      client.publish(logs_topic, msg);
      // publish that the box opened...
    } else if (readLock1 == HIGH && boxToOpen1 == LOW) {
      String sendLog = "FORCED 1 ";
      sendLog.concat(id);
      sendLog.toCharArray(msg, sendLog.length() + 1);
      client.publish(logs_topic, msg);
    }

    if (readLock2 == HIGH && boxToOpen2 == HIGH) {
      String sendLog = "OPEN 2 ";
      sendLog.concat(id);
      sendLog.toCharArray(msg, sendLog.length() + 1);
      client.publish(logs_topic, msg);
    } else if (readLock1 == HIGH && boxToOpen1 == LOW) {
      String sendLog = "OPEN 2 ";
      sendLog.concat(id);
      sendLog.toCharArray(msg, sendLog.length() + 1);
      client.publish(logs_topic, msg);
    }
  }
  
  delay(10);
}
