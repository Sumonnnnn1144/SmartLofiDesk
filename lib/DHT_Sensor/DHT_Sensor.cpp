#include "DHT_Sensor.h"

#define DHTPIN 15
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastDHTSend = 0; //tg gui du lieu

//MQTT Topic
const char* topic_temp = "smartroom/temperature";
const char* topic_humi = "smartroom/humidity";

void initDHT() {
    dht.begin();
    Serial.println("DHT11 initialized on GPIO15");
}

void handleDHTInput(PubSubClient& mqttClient){
    //use millis avoid frozing
    if(millis() - lastDHTSend >= 5000){
        lastDHTSend = millis();
        float temp = dht.readTemperature();
        float humi = dht.readHumidity();

        //checking valid data
        if(!isnan(temp) && !isnan(humi)){
            Serial.print("Temperature: ");
            Serial.println(temp);
            Serial.print("Humidity: ");
            Serial.println(humi);
            mqttClient.publish(topic_temp, String(temp).c_str());
            mqttClient.publish(topic_humi, String(humi).c_str());
            Serial.println("Published temperature and humidity to HiveMQ.");
        }else {
            Serial.println("Failed to read from DHT11.");
        }
    }
}
