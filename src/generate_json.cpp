#include <Arduino.h>

#include "generate_json.h"
#include "ArduinoJson.h"

extern float temperature, light, SH, SB;
extern boolean heater_on, cooler_on, is_fire;
extern unsigned long upTime;
extern String USER_NAME, ssid, ip, ledColor, presence;

String Serialize_ESPstatus(String name, String descr, float lat, float lon) {
    StaticJsonDocument<1024> jsondoc;
    jsondoc["status"]["temperature"] = temperature;
    jsondoc["status"]["light"] = light;
    jsondoc["status"]["heat"] = heater_on;
    jsondoc["status"]["cold"] = cooler_on;
    jsondoc["status"]["running"] = true;
    jsondoc["status"]["fire"] = is_fire;

    jsondoc["info"]["ident"] = name;
    jsondoc["info"]["user"] = USER_NAME;
    jsondoc["info"]["description"] = descr;
    jsondoc["info"]["uptime"] = upTime;
    jsondoc["info"]["ssid"] = ssid;
    jsondoc["info"]["ip"] = ip;
    // jsondoc["info"]["loc"] = getJSONString_fromlocation(lat, lon);
    jsondoc["info"]["loc"]["lat"] = lat;
    jsondoc["info"]["loc"]["lon"] = lon;

    jsondoc["regul"]["sh"] = SH;
    jsondoc["regul"]["sb"] = SB;

    // jsondoc["reporthost"]["target_ip"] = String(2);
    // jsondoc["reporthost"]["target_port"] = String(2);
    // jsondoc["reporthost"]["sp"] = String(2);

    jsondoc["piscine"]["led"] = ledColor;
    jsondoc["piscine"]["presence"] = presence;

    String data = "";
    serializeJson(jsondoc, data);
    return data;
}

String getJSONString_fromlocation(float lat, float lon) {
    StaticJsonDocument<100> jsondoc;
    jsondoc["lat"] = lat;
    jsondoc["lon"] = lon;
    String data = "";
    serializeJson(jsondoc, data);
    return data;
}