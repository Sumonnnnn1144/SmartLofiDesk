#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "secrets.h"

#define DHTPIN 15
#define DHTTYPE DHT11

#define RED_PIN 25
#define GREEN_PIN 26
#define BLUE_PIN 27

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

const char* topic_temp = "smartroom/temperature";
const char* topic_humi = "smartroom/humidity";
const char* topic_rgb = "smartroom/rgb";

WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastSend = 0;

void setColor(String color) {
  color.trim();

  Serial.print("Set color command: ");
  Serial.println(color);

  digitalWrite(RED_PIN, color == "RED" ? HIGH : LOW);
  digitalWrite(GREEN_PIN, color == "GREEN" ? HIGH : LOW);
  digitalWrite(BLUE_PIN, color == "BLUE" ? HIGH : LOW);

  if (color == "OFF") {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  if (String(topic) == topic_rgb) {
    setColor(msg);
  }
}

void connectWiFi() {
  Serial.println();
  Serial.println("===== WIFI CONNECTING =====");
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int retry = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retry++;

    if (retry > 40) {
      Serial.println();
      Serial.println("WiFi connection timeout. Restarting...");
      ESP.restart();
    }
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.println();
    Serial.println("===== MQTT CONNECTING =====");
    Serial.print("Broker: ");
    Serial.println(mqtt_server);
    Serial.print("Port: ");
    Serial.println(mqtt_port);

    String clientId = "ESP32-Real-";
    clientId += String(random(0xffff), HEX);

    Serial.print("Client ID: ");
    Serial.println(clientId);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("MQTT connected!");

      client.subscribe(topic_rgb);

      Serial.print("Subscribed to topic: ");
      Serial.println(topic_rgb);
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.println(client.state());
      Serial.println("Retry in 3 seconds...");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 SMARTROOM PROJECT STARTED");
  Serial.println("================================");

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  setColor("OFF");

  dht.begin();
  Serial.println("DHT11 initialized on GPIO15");

  connectWiFi();

  espClient.setInsecure();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("Setup completed.");
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();

  if (millis() - lastSend >= 5000) {
    lastSend = millis();

    float temp = dht.readTemperature();
    float humi = dht.readHumidity();

    if (!isnan(temp) && !isnan(humi)) {
      Serial.print("Temperature: ");
      Serial.println(temp);

      Serial.print("Humidity: ");
      Serial.println(humi);

      client.publish(topic_temp, String(temp).c_str());
      client.publish(topic_humi, String(humi).c_str());

      Serial.println("Published temperature and humidity to HiveMQ.");
    } else {
      Serial.println("Failed to read from DHT11.");
    }
  }
}