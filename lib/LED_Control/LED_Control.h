#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>

#define RED_PIN 25
#define GREEN_PIN 26
#define BLUE_PIN 27

void initLED();
void setColor(String color);

#endif