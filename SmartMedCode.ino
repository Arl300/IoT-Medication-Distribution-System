#include <WiFi.h>
#include <ESP32Servo.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);
String wifiStat = "OFF", mqttStat = "OFF", slotStat = "S-", medStat = "IDLE";

void updateLCD() {
  lcd.setCursor(0,0); 
  lcd.print("                ");
  lcd.setCursor(0,0); 
  lcd.print("W:");
  lcd.print(wifiStat);
  lcd.print(" M:");
  lcd.print(mqttStat);

  lcd.setCursor(0,1); 
  lcd.print("                ");
  lcd.setCursor(0,1); 
  lcd.print(slotStat + ":" + medStat);
}

// ================= PIN DEFINITIONS =================
#define IR1 34
#define IR2 35
#define IR3 39

#define SERVO1_PIN 25
#define SERVO2_PIN 26
#define SERVO3_PIN 27

#define R1 13
#define G1 14
#define B1 18

#define R2 19
#define G2 21
#define B2 22

#define R3 23
#define G3 32
#define B3 33

#define buzzer 17

// ================= OBJECTS =================
Servo servo1, servo2, servo3;
WiFiClientSecure espClient;
PubSubClient client(espClient);

// ================= MQTT =================
const char* mqtt_server = "640a930f6421476fbfc9f85b8a42b601.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "SmartMed";
const char* mqtt_pass = "prepapreforSmart2026!";

// ================= STATE =================
const int STEP = 17;
const unsigned long TIME_LIMIT = 30000;  // 30 sekonda

int idx1 = 0, max1 = 0;
int idx2 = 0, max2 = 0;
int idx3 = 0, max3 = 0;

bool empty1 = true, empty2 = true, empty3 = true;
bool active1 = false, active2 = false, active3 = false;

unsigned long timer1 = 0, timer2 = 0, timer3 = 0;
unsigned long buzTimer = 0;

// ================= LED FUNCTIONS =================
void ledReady1() { digitalWrite(R1,LOW); digitalWrite(G1,LOW); digitalWrite(B1,HIGH); }
void ledTaken1() { digitalWrite(R1,LOW); digitalWrite(G1,HIGH); digitalWrite(B1,LOW); }
void ledEmpty1() { digitalWrite(R1,HIGH); digitalWrite(G1,LOW); digitalWrite(B1,LOW); }

void ledReady2() { digitalWrite(R2,LOW); digitalWrite(G2,LOW); digitalWrite(B2,HIGH); }
void ledTaken2() { digitalWrite(R2,LOW); digitalWrite(G2,HIGH); digitalWrite(B2,LOW); }
void ledEmpty2() { digitalWrite(R2,HIGH); digitalWrite(G2,LOW); digitalWrite(B2,LOW); }

void ledReady3() { digitalWrite(R3,LOW); digitalWrite(G3,LOW); digitalWrite(B3,HIGH); }
void ledTaken3() { digitalWrite(R3,LOW); digitalWrite(G3,HIGH); digitalWrite(B3,LOW); }
void ledEmpty3() { digitalWrite(R3,HIGH); digitalWrite(G3,LOW); digitalWrite(B3,LOW); }

// ================= SERVO LOGIC =================
void dispense(Servo &s, int &idx, int max, bool &empty) {
  idx++;

  int angle = 180 - (idx * STEP);
  if (angle < 0) angle = 0;

  s.write(angle);

  Serial.print("Servo moved to angle: ");
  Serial.println(angle);

  if (idx >= max) {
    s.write(180);
    idx = 0;
    empty = true;

    Serial.println("Slot reached max pills. Refill needed.");
  }
}

// ================= MQTT CALLBACK =================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  msg.trim();

  Serial.print("MQTT message received: ");
  Serial.println(msg);

  // REFILL1:5, REFILL2:3, REFILL3:10
  if (msg.startsWith("REFILL") && msg.indexOf(':') > 0) {
    int colon = msg.indexOf(':');
    int slot = msg.substring(6, colon).toInt();
    int count = msg.substring(colon + 1).toInt();

    if (count < 1) count = 1;
    if (count > 10) count = 10;

    if (slot == 1) {
      idx1 = 0;
      max1 = count;
      empty1 = false;
      active1 = false;

      servo1.write(180);
      ledEmpty1();

      slotStat = "S1";
      medStat = "REFILL";
      updateLCD();

      client.publish("smartmed/device1/status", "MED1_REFILLED");

      Serial.print("MED1_REFILLED with count: ");
      Serial.println(count);
    }

    else if (slot == 2) {
      idx2 = 0;
      max2 = count;
      empty2 = false;
      active2 = false;

      servo2.write(180);
      ledEmpty2();

      slotStat = "S2";
      medStat = "REFILL";
      updateLCD();

      client.publish("smartmed/device1/status", "MED2_REFILLED");

      Serial.print("MED2_REFILLED with count: ");
      Serial.println(count);
    }

    else if (slot == 3) {
      idx3 = 0;
      max3 = count;
      empty3 = false;
      active3 = false;

      servo3.write(180);
      ledEmpty3();

      slotStat = "S3";
      medStat = "REFILL";
      updateLCD();

      client.publish("smartmed/device1/status", "MED3_REFILLED");

      Serial.print("MED3_REFILLED with count: ");
      Serial.println(count);
    }

    return;
  }

  // ================= DISPENSE 1 =================
  if (msg == "DISPENSE1") {
    Serial.println("DISPENSE1 command received");

    slotStat = "S1";

    if (empty1) {
      medStat = "EMPTY";
      updateLCD();

      client.publish("smartmed/device1/status", "MED1_DISPENSE_EMPTY_REFILL");
      ledEmpty1();

      Serial.println("MED1_DISPENSE_EMPTY_REFILL");
    }

    else {
      dispense(servo1, idx1, max1, empty1);

      medStat = "READY";
      updateLCD();

      ledReady1();

      delay(1000);

      active1 = true;
      timer1 = millis();
      buzTimer = millis();

      Serial.println("MED1_READY - waiting 30 seconds");

      if (empty1) {
        client.publish("smartmed/device1/status", "MED1_REFILL_NEEDED");
        Serial.println("MED1_REFILL_NEEDED");
      }
    }
  }

  // ================= DISPENSE 2 =================
  if (msg == "DISPENSE2") {
    Serial.println("DISPENSE2 command received");

    slotStat = "S2";

    if (empty2) {
      medStat = "EMPTY";
      updateLCD();

      client.publish("smartmed/device1/status", "MED2_DISPENSE_EMPTY_REFILL");
      ledEmpty2();

      Serial.println("MED2_DISPENSE_EMPTY_REFILL");
    }

    else {
      dispense(servo2, idx2, max2, empty2);

      medStat = "READY";
      updateLCD();

      ledReady2();

      delay(1000);

      active2 = true;
      timer2 = millis();
      buzTimer = millis();

      Serial.println("MED2_READY - waiting 30 seconds");

      if (empty2) {
        client.publish("smartmed/device1/status", "MED2_REFILL_NEEDED");
        Serial.println("MED2_REFILL_NEEDED");
      }
    }
  }

  // ================= DISPENSE 3 =================
  if (msg == "DISPENSE3") {
    Serial.println("DISPENSE3 command received");

    slotStat = "S3";

    if (empty3) {
      medStat = "EMPTY";
      updateLCD();

      client.publish("smartmed/device1/status", "MED3_DISPENSE_EMPTY_REFILL");
      ledEmpty3();

      Serial.println("MED3_DISPENSE_EMPTY_REFILL");
    }

    else {
      dispense(servo3, idx3, max3, empty3);

      medStat = "READY";
      updateLCD();

      ledReady3();

      delay(1000);

      active3 = true;
      timer3 = millis();
      buzTimer = millis();

      Serial.println("MED3_READY - waiting 30 seconds");

      if (empty3) {
        client.publish("smartmed/device1/status", "MED3_REFILL_NEEDED");
        Serial.println("MED3_REFILL_NEEDED");
      }
    }
  }
}

// ================= MQTT RECONNECT =================
void reconnectMQTT() {
  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      wifiStat = "OFF";
      mqttStat = "OFF";
      updateLCD();

      Serial.println("WiFi disconnected. MQTT reconnect stopped.");

      delay(2000);
      return;
    }

    Serial.println("Trying MQTT connection...");

    if (client.connect("ESP32_SmartMed", mqtt_user, mqtt_pass)) {
      mqttStat = "OK";
      updateLCD();

      client.subscribe("smartmed/device1/command");

      Serial.println("MQTT connected");
      Serial.println("Subscribed to: smartmed/device1/command");
    }

    else {
      mqttStat = "ERR";
      updateLCD();

      Serial.print("MQTT failed, rc=");
      Serial.println(client.state());

      delay(3000);
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  Serial.println("System starting...");

  Wire.begin(4, 5);

  lcd.init();
  lcd.backlight();

  wifiStat = "OFF";
  mqttStat = "OFF";
  slotStat = "S-";
  medStat = "INIT";
  updateLCD();

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);

  servo1.write(180);
  servo2.write(180);
  servo3.write(180);

  Serial.println("Servos initialized at 180 degrees");

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);

  pinMode(buzzer, OUTPUT);

  pinMode(R1, OUTPUT);
  pinMode(G1, OUTPUT);
  pinMode(B1, OUTPUT);

  pinMode(R2, OUTPUT);
  pinMode(G2, OUTPUT);
  pinMode(B2, OUTPUT);

  pinMode(R3, OUTPUT);
  pinMode(G3, OUTPUT);
  pinMode(B3, OUTPUT);

  ledEmpty1();
  ledEmpty2();
  ledEmpty3();

  digitalWrite(buzzer, LOW);

  Serial.println("Pins initialized");

  WiFiManager wm;
  wm.autoConnect("SmartMed_Setup");

  wifiStat = "OK";
  updateLCD();

  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());

  espClient.setInsecure();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  Serial.println("MQTT settings configured");
}

// ================= LOOP =================
void loop() {
  static bool wifiPrev = false;

  bool wifiNow = (WiFi.status() == WL_CONNECTED);

  if (wifiNow && !wifiPrev) {
    wifiStat = "OK";
    updateLCD();

    Serial.println("WiFi connected");
  }

  if (!wifiNow && wifiPrev) {
    wifiStat = "OFF";
    mqttStat = "OFF";
    updateLCD();

    Serial.println("WiFi lost");
  }

  wifiPrev = wifiNow;

  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  // ================= SLOT 1 =================
  if (active1) {
    if (digitalRead(IR1) == HIGH) {
      client.publish("smartmed/device1/status", "MED1_TAKEN");

      digitalWrite(buzzer, LOW);

      ledTaken1();
      active1 = false;

      slotStat = "S1";
      medStat = "TAKEN";
      updateLCD();

      Serial.println("MED1_TAKEN");
    }

    else if (millis() - timer1 > TIME_LIMIT) {
      client.publish("smartmed/device1/status", "MED1_NOT_TAKEN");

      digitalWrite(buzzer, LOW);

      ledEmpty1();
      active1 = false;

      slotStat = "S1";
      medStat = "MISSED";
      updateLCD();

      Serial.println("MED1_NOT_TAKEN - 30 seconds passed");
    }
  }

  // ================= SLOT 2 =================
  if (active2) {
    if (digitalRead(IR2) == HIGH) {
      client.publish("smartmed/device1/status", "MED2_TAKEN");

      digitalWrite(buzzer, LOW);

      ledTaken2();
      active2 = false;

      slotStat = "S2";
      medStat = "TAKEN";
      updateLCD();

      Serial.println("MED2_TAKEN");
    }

    else if (millis() - timer2 > TIME_LIMIT) {
      client.publish("smartmed/device1/status", "MED2_NOT_TAKEN");

      digitalWrite(buzzer, LOW);

      ledEmpty2();
      active2 = false;

      slotStat = "S2";
      medStat = "MISSED";
      updateLCD();

      Serial.println("MED2_NOT_TAKEN - 30 seconds passed");
    }
  }

  // ================= SLOT 3 =================
  if (active3) {
    if (digitalRead(IR3) == HIGH) {
      client.publish("smartmed/device1/status", "MED3_TAKEN");

      digitalWrite(buzzer, LOW);

      ledTaken3();
      active3 = false;

      slotStat = "S3";
      medStat = "TAKEN";
      updateLCD();

      Serial.println("MED3_TAKEN");
    }

    else if (millis() - timer3 > TIME_LIMIT) {
      client.publish("smartmed/device1/status", "MED3_NOT_TAKEN");

      digitalWrite(buzzer, LOW);

      ledEmpty3();
      active3 = false;

      slotStat = "S3";
      medStat = "MISSED";
      updateLCD();

      Serial.println("MED3_NOT_TAKEN - 30 seconds passed");
    }
  }

  // ================= BUZZER COMMON =================
  bool alarmActive = active1 || active2 || active3;

  if (alarmActive) {
    unsigned long t = millis() - buzTimer;

    if (t < 500) {
      digitalWrite(buzzer, HIGH);
    }

    else if (t < 1000) {
      digitalWrite(buzzer, LOW);
    }

    else {
      buzTimer = millis();
    }
  }

  else {
    digitalWrite(buzzer, LOW);
  }

  delay(10);
}
