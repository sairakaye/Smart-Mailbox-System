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

/** BYTEEE **/
byte boxToOpen1 = LOW;
byte boxToOpen2 = LOW;

boolean canAccess = false;
boolean isAccessingReg = false;

int accessStatus = 0;
int ctrTime = 0;

/** For RFID **/
#define SS_PIN 4
#define RST_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);
String id = "";

/** FOR BUTTONS **/
#define BTN_PIN_1 0
#define BTN_PIN_2 2

#define REG_BTN_PIN 17

/** FOR REG LED **/
#define REG_LED_PIN 15

/** For MQTT **/
const char* ssid = "Esp8266";
const char* password = "animo0920";
const char* mqtt_server = "192.168.43.75";

const char *uid_topic = "/uid_from_rfid";
const char *stat_topic = "/status_request";
const char *operate_topic = "/nodemcu_only";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
char payloadReceiver[50];
int value = 0;

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
    payloadReceiver[i] = payload[i];
  }

  payloadReceiver[i] = '\0';

  if (String(topic).equalsIgnoreCase("/status_request")) {
    if (payloadReceiver[0] == 'G') {
      accessStatus = 1;
    } else if (payloadReceiver[0] == 'D') {
      accessStatus = -1;
    }
  }

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
      client.subscribe(stat_topic);
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
  // Setup for RFID
  SPI.begin(); // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();

  // Setup for WiFi
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Setup for Buttons
  pinMode(BTN_PIN_1, INPUT);
  pinMode(BTN_PIN_2, INPUT);

  //pinMode(REG_BTN_PIN, INPUT);

  pinMode(REG_LED_PIN, OUTPUT);
  digitalWrite(REG_LED_PIN, LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  byte readButton1 = digitalRead(BTN_PIN_1);
  byte readButton2 = digitalRead(BTN_PIN_2);

  byte readRegButton = analogRead(REG_BTN_PIN);

  if (accessStatus == -1) {
    canAccess = false;
    accessStatus = 0;
  } else if (accessStatus == 1) {
    canAccess = true;
    accessStatus = 0;

    String sendAccess = "A";
    sendAccess.concat(String(boxToOpen1));
    sendAccess.concat(String(boxToOpen2));
    sendAccess.concat(" ");
    sendAccess.concat(id);

    sendAccess.toCharArray(msg, sendAccess.length() + 1);
    client.publish(operate_topic, msg);
    id = "";
  }

  if (isAccessingReg) {
    String regMessage = "REGISTER ";
    regMessage.concat(id);

    regMessage.toCharArray(msg, regMessage.length() + 1);

    client.publish(uid_topic, msg);
    isAccessingReg = false;
    id = "";
  }

  if (canAccess) {
    if (ctrTime <= 1000) {
      ctrTime++;
    } else {
      ctrTime = 0;
      canAccess = false;
      String closeMsg = "X";
      closeMsg.toCharArray(msg, closeMsg.length() + 1);
      client.publish(operate_topic, msg);
    }
  }

  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      id = "";
      ctrTime = 0;
      boxToOpen1 = LOW;
      boxToOpen2 = LOW;

      Serial.print("Tag UID: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
        id.concat(String(mfrc522.uid.uidByte[i], HEX));
        id.toUpperCase();
      }

      Serial.println(id);

      if (readRegButton == LOW) {
        // publish here for readRegButton
        analogWrite(REG_LED_PIN, HIGH);
        Serial.println("Here for registration!");
        delay(100);
        analogWrite(REG_LED_PIN, LOW);
        isAccessingReg = true;
      } else {
        if (readButton1 == HIGH) {
          boxToOpen1 = HIGH;
        }

        if (readButton2 == HIGH) {
          boxToOpen2 = HIGH;
        }

        String reqMsg = "REQ ";
        reqMsg.concat(id);
        reqMsg.toCharArray(msg, reqMsg.length() + 1);
        client.publish(uid_topic, msg);
      }

      mfrc522.PICC_HaltA();
    }
  }

  delay(10);
}
