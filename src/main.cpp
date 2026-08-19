#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "secrets.h"
#include <WiFiManager.h>

#include <LED_Control.h>  // Phần Output của Khang
#include <DHT_Sensor.h>   // Phần Input của Trang

const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

const char* topic_rgb = "smartroom/rgb";

WiFiClientSecure espClient;
PubSubClient client(espClient);


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
  WiFiManager wm;
  if(!wm.autoConnect("SmartRoom_Config")){
    Serial.println("Error connection, restarting...");
    ESP.restart();
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED) { break; }
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
  
  connectWiFi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Mất kết nối WiFi! Đang thử kết nối lại...");
    WiFi.reconnect(); 
    delay(5000);      
    return;           
  }
  if (!client.connected()) {
    connectMQTT();
  }
  
  // Duy trì kết nối MQTT và lắng nghe luồng Output của Khang
  client.loop(); 

  // Thực thi luồng Input đọc cảm biến của Trang
  handleDHTInput(client); 
}