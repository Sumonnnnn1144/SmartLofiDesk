#include "LED_Control.h"

void initLED() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setColor("OFF"); 
}

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