#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <DHT.h>
#include <PubSubClient.h>

//khoi tao cau hinh phan cung cho DHT11
void initDHT();
//doc va publish data lem MQTT Broker theo chu ky
void handleDHTInput(PubSubClient& mqttClient);

#endif