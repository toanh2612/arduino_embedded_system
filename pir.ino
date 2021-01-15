#include <WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>
const char ssid[] = "_toanh";
const char password[] = "_12345678";
char roomId[] = "s-01";
int d = 0;
char type[] = "automatic";
char deviceId[] = "s-01-22a810ca-bef3-41cf-9247-3b725a9c926d";
char ledId[] = "s-01-7a4e04cf-e449-41aa-b401-8f51aae57d00";
int pirPin = 26;
int led = 32;
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
  pinMode(pirPin, INPUT);
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
    d = digitalRead(pirPin);
    Serial.println(d);
    StaticJsonDocument<500> data;
    data["roomId"] = roomId;
    data["mode"] = d;
    data["deviceId"] = deviceId;
    data["type"] = type;
    String output;
    serializeJson(data, output);
    client.publish(deviceId, output);
  }
  delay(3000);
}
