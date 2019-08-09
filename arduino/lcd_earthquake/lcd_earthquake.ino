#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Definisi SSID dan password WiFi
const char* ssid = "SI4007";
const char* password = "Qud4terbang@";

// MQTT server
const char* mqtt_server = "soldier.cloudmqtt.com";
const char* mqtt_user = "mizrsriv";
const char* mqtt_password = "vKGsX3BGaJqa";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  char message[length];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  Serial.print("Magnitude = ");
  Serial.print(message);
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    const char* clientId = "LCDClient";

    if (client.connect(clientId,mqtt_user,mqtt_password)) {
      Serial.println("connected");
      client.subscribe("tugas-akhir");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 18162);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
