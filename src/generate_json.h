#include <Arduino.h>

String Serialize_ESPstatus(String name, String descr, float lat, float lon);
String getJSONString_fromlocation(float lat, float lon);