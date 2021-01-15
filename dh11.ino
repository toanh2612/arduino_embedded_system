#include <WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include "DHT.h"
const char ssid[] = "_toanh";
const char password[] = "_12345678";
const int DHTPIN = 34;
const int DHTTYPE = DHT11;
char roomId[] = "s-01";
float h = 0;
float t = 0;
char type[] = "automatic";
char deviceId[] = "s-01-6a451aef-4d78-49e7-a864-8cceddf2f3fe";
char ledId[] = "s-01-29c9889d-5b79-4df8-ba58-91e7d02a6aa7";

int led = 21;
DHT dht(DHTPIN, DHTTYPE);
WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;
void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  while (!client.connect("arduinoESP32")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
  client.subscribe(ledId);
  // client.unsubscribe("/clientSub");
}
void messageReceived(String &topic, String &payload) {
  if (topic == ledId) {
    Serial.println("topic == ledId");
    if (payload == "1") {
      digitalWrite(led, HIGH);
    } else {
      digitalWrite(led, LOW);
    }
  }
  Serial.println("incoming: " + topic + " - " + payload);
}
void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.begin("192.168.43.45", net);
  client.onMessage(messageReceived);
  connect();
}
void loop() {
  client.loop();
  delay(10); // <- fixes some issues with WiFi stability
  if (!client.connected()) {
    connect();
  }
  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    h = dht.readHumidity() || h;
    t = dht.readTemperature() || t;
    Serial.println(h);
    Serial.println(t);
    StaticJsonDocument<500> data;
    data["roomId"] = roomId;
    data["t"] = t;
    data["h"] = h;
    data["deviceId"] = deviceId;
    data["type"] = type;
    String output;
    serializeJson(data, output);
    client.publish(deviceId, output);
  }
  delay(5000);
}
