#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "secrets.h"

// ==========================================
// 1. MODULES (ĐÃ ĐƯỢC BÓC TÁCH)
// ==========================================
#include <LED_Control.h>  // Phần Output của Khang
#include <DHT_Sensor.h>   // Phần Input của Trang

// ==========================================
// 2. CẤU HÌNH MẠNG VÀ MQTT CHUNG
// ==========================================
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

const char* topic_rgb = "smartroom/rgb";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// ==========================================
// 3. LOGIC KẾT NỐI & XỬ LÝ
// ==========================================
// Hàm lắng nghe lệnh từ Web (Của Khang)
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
    setColor(msg); // Lệnh này giờ sẽ chạy an toàn qua module LED_Control
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

    String clientId = "ESP32-Real-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("MQTT connected!");

      // Khang đăng ký nhận lệnh điều khiển LED
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

  // --- Khởi tạo các module phần cứng ---
  initLED(); // Khang
  initDHT(); // Trang 

  // --- Khởi tạo kết nối mạng ---
  connectWiFi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  
  // Duy trì kết nối MQTT và lắng nghe luồng Output của Khang
  client.loop(); 

  // Thực thi luồng Input đọc cảm biến của Trang
  handleDHTInput(client); 
}