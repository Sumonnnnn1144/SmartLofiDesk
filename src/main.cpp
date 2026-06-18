#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 15
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// Thông tin MQTT Broker miễn phí
const char* mqtt_server = "broker.hivemq.com";
const char* topic = "hcmus/iot/room_temp_abc123"; // Đổi tên topic này cho khỏi đụng hàng

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // 1. Kết nối Wi-Fi ảo của Wokwi
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // 2. Cấu hình MQTT Broker
  client.setServer(mqtt_server, 1883);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client_RandomID")) {
      Serial.println("MQTT connected!");
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // 3. Đọc nhiệt độ và Publish
  float t = dht.readTemperature();
  
  if (!isnan(t)) {
    char tempString[8];
    dtostrf(t, 1, 2, tempString); // Ép kiểu float sang string để gửi MQTT
    
    client.publish(topic, tempString); // Bắn lên server
    Serial.print("Da gui nhiet do: ");
    Serial.println(tempString);
  }
  
  delay(3000); // 3 giây gửi 1 lần
}