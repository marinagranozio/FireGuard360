#include <ArduinoBLE.h>
#include "DHT.h"

// ----------------------- CONFIG -----------------------
#define DHT_PIN 2
#define MQ2_PIN 3
#define MQ2_A0  A7
#define LEDR    4
#define DHTTYPE DHT22
const char* DEVICE_NAME = "server1";

// UUIDs
const char* UUID_SERVICE         = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* UUID_DATA            = "19b10001-e8f2-537e-4f6c-d104768a1215";

// ------------------ BLE Characteristics ------------------
BLEService dangerService(UUID_SERVICE);

// Caratteristica per inviare il JSON con tutti i dati
BLEStringCharacteristic dataCharacteristic(UUID_DATA, BLERead | BLENotify, 100);

// ----------------------- VARIABILI -----------------------
DHT dht(DHT_PIN, DHTTYPE);
float t = 0, h = 0, f = 0, hic = 0, hif = 0;
int   gasState   = 0;
float gasLevel = 0;

int dangerValue = 0;
int dangerValueLimit = 30;
bool dangerValueChanged = false;
bool resetFlag = false;

// ----------------------- SETUP -----------------------
void setup() {
  Serial.begin(9600);

  dht.begin();
  pinMode(MQ2_A0, INPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(LEDR, OUTPUT);

  setupBLE();

  delay(2000);
}

// ----------------------- LOOP -----------------------
void loop() {
  getSensorData();
  logValues();
  ledBlink();
  handleBLEConnection();
}

// ---------------------- SENSOR -----------------------
void getSensorData() {
  delay(1000);

  h = dht.readHumidity();
  t = dht.readTemperature();
  f = dht.readTemperature(true);
  gasState = digitalRead(MQ2_PIN);
  gasLevel = (float)analogRead(MQ2_A0) / 1024.0 * 5.0;

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("DHT read failed!");
    return;
  }

  hif = dht.computeHeatIndex(f, h);
  hic = dht.computeHeatIndex(t, h, false);
}

// ------------------------ LOG ------------------------
void logValues() {
  Serial.print("Temp: "); Serial.print(t); Serial.print("°C ");
  Serial.print("Hum: "); Serial.print(h); Serial.print("% ");
  Serial.print("Gas: "); Serial.print(gasLevel); Serial.print(" V ");
  Serial.println(gasState == HIGH ? "(No gas)" : "(Gas detected)");
}

// ------------------------ BLE ------------------------
void setupBLE() {
  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1);
  }

  BLE.setDeviceName(DEVICE_NAME);
  BLE.setLocalName(DEVICE_NAME);
  BLE.setAdvertisedService(dangerService);

  // Aggiunta della caratteristica per i dati
  dangerService.addCharacteristic(dataCharacteristic);

  BLE.addService(dangerService);

  dataCharacteristic.writeValue(""); // Inizializzazione della caratteristica

  BLE.advertise();
  Serial.println("BLE advertising started");
}

void handleBLEConnection() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      checkAndResetDangerLevel();
      updateSensorData();  // Invia i dati in JSON
      delay(1000);
    }

    Serial.println("Central disconnected");
  }
}

void checkAndResetDangerLevel() {
  // Se il flag reset è attivo, resetta il valore di pericolo
  if (resetFlag) {
    dangerValue = 0;
    dangerValueChanged = false;
    resetFlag = false;
    Serial.println("Danger reset from central");
  } else if (!dangerValueChanged) {
    // Calcola e aggiorna il livello di pericolo
    dangerValue = computeDangerValue();
    dangerValueChanged = true;
  }
}

void updateSensorData() {
  // Creazione del JSON con i dati dei sensori
  String jsonData = buildSensorJSON();

  // Invia il JSON alla caratteristica BLE
  dataCharacteristic.writeValue(jsonData);
  Serial.println("Sent JSON to central: " + jsonData);
}

// ------------------------ UTILS ------------------------
int computeDangerValue() {
  // Da valutare: somma pesata con soglie arbitrarie
  float tempFactor = constrain(t / 50.0, 0, 1);
  float humFactor = constrain(h / 100.0, 0, 1);
  float gasFactor = constrain(gasLevel / 5.0, 0, 1);
  float combined = (tempFactor + humFactor + gasFactor) / 3.0;
  return (int)(combined * 100);
}

void ledBlink() {
  digitalWrite(LEDR, LOW);
  delay(100);
  digitalWrite(LEDR, HIGH);
}

String buildSensorJSON() {

  String json = "{";
  json += "\"t\":" + String(t, 1) + ",";
  json += "\"h\":" + String(h, 1) + ",";
  json += "\"gas\":" + String(gasLevel, 2) + ",";
  json += "\"danger\":" + String(dangerValue);
  json += "}";

  Serial.println("JSON created: " + json);

  return json;
}
