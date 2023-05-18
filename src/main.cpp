#include <Arduino.h>
#include <map>

/*********
	  Based on Rui Santos work : https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
	  File mqtt_full/mqtt_full.ino
	  Modified by GM
*********/

/*===== ARDUINO LIBS ========*/
#include "WiFi.h"
#include <PubSubClient.h>
#include <Wire.h>
#include "classic_setup.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"

/*===== UTILS LIBS ========*/
#include "generate_json.h"
#include "utils.h"

/*===== MQTT broker/server ========*/
//const char* mqtt_server = "192.168.1.101"; 
//const char* mqtt_server = "public.cloud.shiftr.io"; // Failed in 2021
// need login and passwd (public,public) mqtt://public:public@public.cloud.shiftr.io
//const char* mqtt_server = "broker.hivemq.com"; // anynomous Ok in 2021 
// const char* mqtt_server = "test.mosquitto.org"; // anynomous Ok in 2021
const char* mqtt_server = "mqtt.eclipseprojects.io"; // anynomous Ok in 2021

/*===== MQTT TOPICS ===============*/
#define TOPIC_TEMP "uca/iot/piscine"
#define TOPIC_LED1  "uca/M1/iot/led1"
#define TOPIC_LED2  "uca/M1/iot/led2"
#define TOPIC_LED3  "uca/M1/iot/led3"

/*===== ESP is MQTT Client =======*/
WiFiClient espClient;           // Wifi 
PubSubClient client(espClient); // MQTT client

HTTPClient http;

/*============= GPIO ==============*/
const int lightPin = A5;
const int GREEN_LED_PIN = 19;
const int ONBOARD_LED=2;
const int RED_LED_PIN=21;
const int ledPin = 19; // LED Pin

/* ---- TEMP ---- */
OneWire oneWire(23); // Pour utiliser une entite oneWire sur le port 23
DallasTemperature tempSensor(&oneWire) ; // Cette entite est utilisee par le capteur de temperature

String USER_NAME = "Maxime BELLET";
float temperature = 0;
float light = 0;
boolean heater_on = false;
boolean cooler_on = false;
boolean is_fire = false;
unsigned long upTime = 0;
String ssid, mac, ip, presence;
String ledColor = "Blue";

float SH = 30;
float STH = 30.0;
float SB = 29;
float SL = 500;

const String name = "Maxime pool";
const String descr = "Piscine de fou";
const float lat = 43.58190010070465;
const float lon = 7.0953758788333765;

std::map<String, float> tempMap;

/*============= TO COMPLETE ===================*/
void set_LED(int ledPin,int v){
  digitalWrite(ledPin, v);
}

/*============== CALLBACK ===================*/
void mqtt_pubcallback(char* topic, 
                      byte* message, 
                      unsigned int length) {
  /* 
   * Callback if a message is published on this topic.
   */
  Serial.print("Message arrived on topic : ");
  Serial.println(topic);
  Serial.print("=> ");

  // Byte list to String and print to Serial
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Feel free to add more if statements to control more GPIOs with MQTT

  if (String(topic) == TOPIC_TEMP) {
    String espName;
    Serial.println(messageTemp);

    StaticJsonDocument<1024> messageDoc;
    DeserializationError messageError = deserializeJson(messageDoc, messageTemp);
    if (!messageError) {
      const float espLat = messageDoc["info"]["loc"]["lat"];
      const float estLon = messageDoc["info"]["loc"]["lon"];
      const String espName = messageDoc["info"]["ident"];
      const float espTemperature = messageDoc["status"]["temperature"];

      if (espName != name) {
        String url = 
          "https://api.distancematrix.ai/maps/api/distancematrix/json?origins=" + 
          String(lat) + 
          "," +
          String(lon) + 
          "&destinations=" + 
          String(espLat) + 
          "," + 
          String(estLon) + 
          "&key=YymMJQnXeBwQt3g9DmNie8F0yuPG4";

        http.begin(url);
        int httpCode = http.GET();
        String response = http.getString();
        http.end();

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error) {
          const int distance = doc["rows"][0]["elements"][0]["distance"]["value"];
          Serial.println("Distance =>");
          Serial.println(String(distance));
          if (distance <= 25000) {
            tempMap[espName] = espTemperature;
            // if (tempMap.count(espName.c_str())) {
            //   tempMap[espName.c_str()] = espTemperature;
            // } else {
            //   tempMap.insert({espName.c_str(), espTemperature});
            // }
          }
        } else {
          Serial.println("Error parsing JSON");
        }
      }
    } else   {
      Serial.println("Error parsing JSON");
    }
  }

  // If a message is received on the topic,
  // you check if the message is either "on" or "off".
  // Changes the output state according to the message

  if (String(topic) == TOPIC_LED1) {
    Serial.print("so ... changing output to ");
    Serial.println(messageTemp);
    if (messageTemp == "true") {
      set_LED(ONBOARD_LED, HIGH);
    }
    else if (messageTemp == "false") {
      set_LED(ONBOARD_LED ,LOW);
    }
  }
  if (String(topic) == TOPIC_LED2) {
    Serial.print("so ... changing output to ");
    Serial.println(messageTemp);
    if (messageTemp == "true") {
      set_LED(RED_LED_PIN, HIGH);
    }
    else if (messageTemp == "false") {
      set_LED(RED_LED_PIN ,LOW);
    }
  }
  if (String(topic) == TOPIC_LED3) {
    Serial.print("so ... changing output to ");
    Serial.println(messageTemp);
    if (messageTemp == "true") {
      set_LED(GREEN_LED_PIN, HIGH);
    }
    else if (messageTemp == "false") {
      set_LED(GREEN_LED_PIN ,LOW);
    }
  }
}

/*===== Arduino IDE paradigm : setup+loop =====*/
void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for a serial connection. Needed for native USB port only
   
  connect_wifi(); // Connexion Wifi  
  print_network_status();
  
  // Initialize the output variables as outputs
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);// Set outputs to LOW
  digitalWrite(GREEN_LED_PIN, LOW);// Set outputs to LOW
  digitalWrite(RED_LED_PIN, LOW);// Set outputs to LOW
  
  // Init temperature sensor 
  tempSensor.begin();

  // set server of our client
  client.setServer(mqtt_server, 1883);
  // set callback when publishes arrive for the subscribed topic
  client.setCallback(mqtt_pubcallback); 
}

/*============= SUBSCRIBE =====================*/
void mqtt_mysubscribe(char *topic) {
  /*
   * Subscribe to a MQTT topic 
   */
  while (!client.connected()) { // Loop until we're reconnected

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect => https://pubsubclient.knolleary.net/api

    String mqttId = "DeathStar-";
    const int mqttPort = 1883;
    const char* mqttUser = "darkvador";
    const char* mqttPassword = "6poD2R2";
    mqttId += String(random(0xffff), HEX);

    if (client.connect("Lil ESP", /* Client Id when connecting to the server */
		       NULL,    /* No credential */ 
		       NULL)) {
      Serial.println("connected");
      // then Subscribe topic
      Serial.println(topic);
      client.subscribe(topic);
      client.subscribe(TOPIC_LED1);
      client.subscribe(TOPIC_LED2);
      client.subscribe(TOPIC_LED3);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      
      Serial.println(" try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

void publish_json(int delay) {
  static uint32_t tick = 0;
  Serial.println(millis());
  Serial.println(tick);
  Serial.println(millis() - tick);
  Serial.println(delay);
  Serial.println(millis() - tick < delay);

  if (millis() - tick < delay) {
    return;
  }
  tick = millis();
  String serializeData = Serialize_ESPstatus(name, descr, lat, lon);
  char piscineJSONString[serializeData.length()];
  strcpy(piscineJSONString, serializeData.c_str());

  sprintf(piscineJSONString, "%s", piscineJSONString);

  client.setBufferSize(2048);
  client.publish(TOPIC_TEMP, piscineJSONString);
}

/*================= LOOP ======================*/
void loop() {
  /*--- subscribe to TOPIC_LED if not yet ! */
  if (!client.connected()) {
    mqtt_mysubscribe((char *)(TOPIC_TEMP));
  }

  client.loop();

  temperature = get_Temperature(tempSensor);
  light = get_light(lightPin);
  heater_on = get_heater_status(temperature, SB);
  cooler_on = get_cooler_status(temperature, SH);
  is_fire = get_fire_status(light);
  ssid = String(WiFi.SSID());
  ip = WiFi.localIP().toString();
  presence = get_presence(light, SL);
  ledColor = get_led_color(tempMap, temperature);

  upTime = millis() / 1000;

  publish_json(10000);

  Serial.println("------------------------------------------");
  for (auto const& pair: tempMap) {
    Serial.print(pair.first);
    Serial.print(": ");
    Serial.println(pair.second);
  }
  Serial.println("------------------------------------------");
  Serial.println(temperature);
  Serial.println(ledColor);
  Serial.println("------------------------------------------");

  /* Process MQTT ... une fois par loop() ! */
}
