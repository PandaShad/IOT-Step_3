#include <Arduino.h>
#include <map>

#include "DallasTemperature.h"

String getLED_fromTempDelta();
float get_Temperature(DallasTemperature tempSensor);
float get_light(int lightPin);
boolean get_heater_status(float temperature, float SB);
boolean get_cooler_status(float temperature, float SH);
boolean get_fire_status(float light);
String get_presence(float light, float SL);
String get_led_color(std::map<String, float> tempMap, float temp);