#include <ArduinoBLE.h>
#include "DHT.h"

// ----------------------- CONFIG -----------------------
#define DHT_PIN 2
#define MQ2_PIN 3
#define MQ2_A0  A7
#define LEDR    4
#define DHTTYPE DHT22

// UUIDs
const char* nomeDevice = "server1";
const char* UuidService = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* UuidCharacteristic = "19b10001-e8f2-537e-4f6c-d104768a1215";

// --------- BLE Service and Characteristics ----------
BLEService bluetoothService(UuidService);  // UUID del servizio
BLECharacteristic bluetoothCharacteristic(UuidCharacteristic, BLERead, 50);

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

  setup_BLE();

  delay(2000);
}

// ----------------------- LOOP -----------------------
void loop() {
  getSensorData();
  logValues();
  ledBlink();
  BLE_Update_Values();
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
void setup_BLE(){
  if (!BLE.begin()) {
    Serial.println("Errore nell'inizializzare BLE");
    while (1);
  }

  BLE.setDeviceName(nomeDevice);
  BLE.setLocalName(nomeDevice);

  bluetoothService.addCharacteristic(bluetoothCharacteristic);
  
  BLE.addService(bluetoothService);
  BLE.setAdvertisedService(bluetoothService);
  delay(100);
 
  bluetoothCharacteristic.writeValue("Init");
  delay(100);

  BLE.advertise();
 
  Serial.println("Advertising started");
}

void BLE_Update_Values(){
  BLEDevice central = BLE.central();
 
  if(central){

    Serial.print("Connesso al dispositivo centrale: ");
    Serial.println(central.address());
    
    while(central.connected()){ 
      updateSensorData();
      delay(1000);
    }

  }else{
    Serial.print("Il centrale ");
    Serial.print(central.address());
    Serial.println(" non è connesso");
  }
  delay(1000);
}

void updateSensorData() {
  // Creazione del JSON con i dati dei sensori
  String sensorData = buildSensorCompact();

  // Invia il JSON alla caratteristica BLE
  bluetoothCharacteristic.writeValue((const uint8_t*)sensorData.c_str(), sensorData.length());
  Serial.println("Sent JSON to central: " + sensorData);
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

String buildSensorCompact() {
  String compact = "T" + String((int)t);
  compact += "H" + String((int)h);
  compact += "G" + String((int)(gasLevel * 100));
  compact += "D" + String(computeDangerValue());

  return compact;
}

