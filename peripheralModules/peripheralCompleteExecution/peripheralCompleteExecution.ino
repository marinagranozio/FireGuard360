#include <ArduinoBLE.h>
#include "DHT.h"
 
// ----------------------- CONFIG -----------------------
#define DHT_PIN 2
#define MQ2_PIN 3
#define MQ2_A7  A7
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
float t = 0, h = 0, f = 0;
int gasState = 0;
int gasLevel = 0;
 
int dangerValue = 0;
int dangerValueLimit = 30;
bool dangerValueChanged = false;
bool resetFlag = false;
 
int zero_millis = 0; // Per il reset del dangerValue
int millis_limit = 40000; // 40 secondi per il reset
bool watchdogEnabled = false; // Abilita il watchdog per il reset del dangerValue
 
 
// ----------------------- SETUP -----------------------
void setup() {
  Serial.begin(9600);
 
  dht.begin();
  pinMode(MQ2_A7, INPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(LEDR, OUTPUT);
 
  setup_BLE();
 
  delay(10000);
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
  gasLevel = analogRead(MQ2_A7);
 
  if (isnan(h) || isnan(t) || isnan(f)) {
    log_system_info("DHT read failed!");
    return;
  }
 
  dangerValue = computeDangerValue();
}
 
// ------------------------ LOG ------------------------
void logValues() {
    log_system_info(String("Temp: ")+ String(t) + String("°C "));
    log_system_info(String("Hum: ") + String(h) + String("% "));
    log_system_info(String("Gas: ") + String(gasLevel) + String("ppm "));
    log_system_info(gasState == HIGH ? String("No gas") : String("Gas detected"));
    log_system_info(String("Danger Value: ") + String(dangerValue));
    }
         
 
// ------------------------ BLE ------------------------
void setup_BLE(){
  if (!BLE.begin()) {
    log_system_info("Errore nell'inizializzare BLE");
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
 
  log_system_info("Advertising started");
}
 
void BLE_Update_Values(){
  BLEDevice central = BLE.central();
 
  if(central){
    if(watchdogEnabled) {
      // Se il watchdog è abilitato, lo disabilita
      log_system_info("Watchdog disabilitato, centrale connesso");
      watchdogEnabled = false; // Disabilita il watchdog quando connesso
      zero_millis = 0;          // Resetta il timer del watchdog
    }
   
 
    log_system_info("Connesso al dispositivo centrale: " + central.address());
   
    while(central.connected()){
      updateSensorData();
      delay(1000);
    }
 
  }else{
   
    if(!watchdogEnabled) {
      // Se il watchdog non è abilitato, lo attiva
      watchdogEnabled = true;
      log_system_info("Watchdog abilitato, comincio il countdown");
      zero_millis = millis();
    }
    else{
      if(millis() - zero_millis > millis_limit) {
      // Se il tempo limite è superato, resetta il bluetooth
      log_system_info("Bluetooth resettato");
      watchdogEnabled = false; // Disabilita il watchdog
      setup_BLE(); // Riprova a configurare il BLE
    }
  }
 
    log_system_info(String("Il centrale") + String(central.address()) + " non è connesso");
  }
  delay(1000);
}
 
void updateSensorData() {
  // Creazione del JSON con i dati dei sensori
  String sensorData = buildSensorCompact();
 
  // Invia il JSON alla caratteristica BLE
  bluetoothCharacteristic.writeValue((const uint8_t*)sensorData.c_str(), sensorData.length());
  log_sensor_data(sensorData);
}
 
// ------------------------ UTILS ------------------------
float normalizeGas(int gasValue, float minVal = 0, float maxVal = 5) {
  float norm = (float)(gasValue - minVal) / (maxVal - minVal);
  norm = constrain(norm, 0.0, 1.0);
  return norm;
}
 
float normalizeTemp(float tempC) {
  if (tempC <= 25) return 0.0;
  if (tempC >= 60) return 1.0;
  return (tempC - 25.0) / 35.0;  
}
 
float normalizeHumidity(float hum) {
  if (hum >= 60) return 0.0;
  if (hum <= 20) return 1.0;
  return (60.0 - hum) / 40.0;
}
 
int computeDangerValue() {
  // Da valutare: somma pesata con soglie arbitrarie
  float tempFactor = normalizeTemp(t);
  float temp_weight = 0.3;  //Peso della temperatura
  float humFactor = normalizeHumidity(h);
  float hum_weight = 0.2;  //Peso dell'umidità
  float gasFactor = normalizeGas(gasLevel);
  float gas_weight = 0.5;  //Peso dell'umidità
  float combined = (temp_weight*tempFactor + hum_weight*humFactor + gas_weight*gasFactor);  //formula a1 * T + a2 * H + a3 * G
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
  compact += "D" + String(dangerValue);
 
  return compact;
}
 
void log_system_info(String info){
  Serial.println("LOG: [" + String(nomeDevice) + "] - " + info);
}

void log_sensor_data(String info){
  Serial.println("DATA: [" + String(nomeDevice) + "] - " + info);
}