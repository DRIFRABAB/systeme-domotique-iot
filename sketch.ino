#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ========== CONFIGURATION WOKWI ==========
// WiFi et MQTT
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

// ========== PINS ==========
// Capteurs
const int lightPin = 35;       // Pin du capteur LDR (luminosité)
const int pirPin = 32;         // Capteur PIR (Présence)

// Actionneurs
const int fanPin = 26;         // LED Magenta = Ventilateur
const int light1Pin = 25;      // LED Orange = Lampe Salon
const int light2Pin = 33;      // LED Bleue = Lampe Présence
const int devicePin = 27;      // LED Jaune = Appareil

// ========== CONFIGURATION DHT22 ==========
#define DHT_PIN 15
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// ========== VARIABLES ==========

WiFiClient espClient;
PubSubClient client(espClient);

float temperature = 20.0;
float humidity = 50.0;
int lightLevel = 2000;
bool presenceDetected = false;
bool lastPresenceState = false;
unsigned long lastMsg = 0;

// ========== SETUP ==========

void setup() {
  Serial.begin(115200);
  Serial.println("\n🔌 SYSTÈME DOMOTIQUE - RABAB DRIF 🔌");

  // Configuration des pins OUTPUT uniquement pour les LEDs
  pinMode(fanPin, OUTPUT);
  pinMode(light1Pin, OUTPUT);
  pinMode(light2Pin, OUTPUT);
  pinMode(devicePin, OUTPUT);
  
  // Pins INPUT pour les capteurs
  pinMode(pirPin, INPUT);
  pinMode(lightPin, INPUT);

  // Initialisation LEDs (éteintes)
  digitalWrite(fanPin, LOW);
  digitalWrite(light1Pin, LOW);
  digitalWrite(light2Pin, LOW);
  digitalWrite(devicePin, LOW);

  // Initialisation du capteur DHT22
  Serial.print(" Initialisation DHT22 sur pin ");
  Serial.println(DHT_PIN);
  dht.begin();
  delay(2000); // Attente plus longue pour stabilisation

  // Connexion WiFi
  connectWiFi();

  // Configuration MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);

  Serial.println("🎯 Système prêt !");
}

void connectWiFi() {
  Serial.print("📶 Connexion à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connecté !");
    Serial.print("🌐 Adresse IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n⚠️ Mode simulation - WiFi non connecté");
  }
}

// ========== CALLBACK MQTT ==========

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("📨 Topic: ");
  Serial.print(topic);
  Serial.print(" | Message: ");
  Serial.println(message);

  String topicStr = String(topic);

  if (topicStr == "domotique/control/ventilateur") {
    digitalWrite(fanPin, (message == "ON") ? HIGH : LOW);
    Serial.println(message == "ON" ? "🌀 Ventilateur ACTIVÉ" : "🌀 Ventilateur DÉSACTIVÉ");
  }
  else if (topicStr == "domotique/control/lampe1") {
    digitalWrite(light1Pin, (message == "ON") ? HIGH : LOW);
    Serial.println(message == "ON" ? "💡 Lampe Salon ACTIVÉE" : "💡 Lampe Salon DÉSACTIVÉE");
  }
  else if (topicStr == "domotique/control/lampe2") {
    digitalWrite(light2Pin, (message == "ON") ? HIGH : LOW);
    Serial.println(message == "ON" ? "🔵 Lampe Présence ACTIVÉE" : "🔵 Lampe Présence DÉSACTIVÉE");
  }
  else if (topicStr == "domotique/control/appareil") {
    digitalWrite(devicePin, (message == "ON") ? HIGH : LOW);
    Serial.println(message == "ON" ? "📱 Appareil ACTIVÉ" : "📱 Appareil DÉSACTIVÉ");
  }
}

// ========== RECONNEXION MQTT ==========

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("🔄 Tentative connexion MQTT...");

    String clientId = "ESP32-Domotique-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("✅ Connecté au broker MQTT!");

      // Souscription aux topics de contrôle
      client.subscribe("domotique/control/ventilateur");
      client.subscribe("domotique/control/lampe1");
      client.subscribe("domotique/control/lampe2");
      client.subscribe("domotique/control/appareil");

      Serial.println("📡 Topics souscrits avec succès");

    } else {
      Serial.print("❌ Échec, code: ");
      Serial.print(client.state());
      Serial.println(" - Nouvel essai dans 5s");
      delay(5000);
    }
  }
}

// ========== LECTURE CAPTEURS ==========

void readSensors() {
  // Lecture DHT22 avec plusieurs tentatives
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  if (!isnan(t) && !isnan(h)) {
    temperature = t;
    humidity = h;
    Serial.print("🌡️ DHT22 | Temp: ");
    Serial.print(temperature, 1);
    Serial.print("°C | Hum: ");
    Serial.print(humidity, 1);
    Serial.println("%");
  } else {
    Serial.println("⚠️ DHT22: Lecture échouée - Vérifier connexions");
    // Garder les dernières valeurs connues
  }

  // Lecture luminosité avec LDR
  int rawLight = analogRead(lightPin);
  
  // Sur Wokwi, le simulateur donne déjà des valeurs cohérentes
  lightLevel = rawLight;
  

  // Lecture PIR (HIGH = mouvement détecté)
  int pirValue = digitalRead(pirPin);
  presenceDetected = (pirValue == HIGH);

  // Détecter les changements pour un log clair
  if (presenceDetected != lastPresenceState) {
    if (presenceDetected) {
      Serial.println("👤 ═══ MOUVEMENT DÉTECTÉ ! ═══");
    } else {
      Serial.println("❌ ═══ Plus de mouvement ═══");
    }
    lastPresenceState = presenceDetected;
  }

  // Affichage standard
  Serial.print("📊 Capteurs | Temp: ");
  Serial.print(temperature, 1);
  Serial.print("°C | Lum: ");
  Serial.print(lightLevel);
  Serial.print(" (raw: ");
  Serial.print(rawLight);
  Serial.print(") | PIR: ");
  Serial.print(pirValue);
  Serial.print(" | Présence: ");
  Serial.println(presenceDetected ? "OUI 👤" : "NON ❌");
}

// ========== PUBLICATION DONNÉES ==========

void publishSensorData() {
  // Création objet JSON
  StaticJsonDocument<256> doc;
  doc["temperature"] = temperature;
  doc["humidite"] = humidity;
  doc["luminosite"] = lightLevel;
  doc["presence"] = presenceDetected;
  doc["timestamp"] = millis();

  String jsonStr;
  serializeJson(doc, jsonStr);

  // Publication sur MQTT
  bool published = client.publish("domotique/sensors/data", jsonStr.c_str());

  if (published) {
    Serial.print("📤 Données publiées: ");
    Serial.println(jsonStr);
  } else {
    Serial.println("⚠️ Échec publication MQTT");
  }
}

// ========== LOOP PRINCIPAL ==========

void loop() {
  // Gestion connexion MQTT
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Lecture et publication toutes les 2 secondes
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    readSensors();
    publishSensorData();

    Serial.println("────────────────────────────────────");
  }

  delay(10);
}