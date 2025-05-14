#include <ArduinoBLE.h>

// ------------------ CONFIG --------------------------

// UUIDs
const char* nomeDevice = "server1";
const char* UuidService = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* UuidCharacteristic = "19b10001-e8f2-537e-4f6c-d104768a1215";

 
// --------- BLE Service and Characteristics ----------
BLEService bluetoothService(UuidService);  // UUID del servizio
BLEStringCharacteristic bluetoothCharacteristic(UuidCharacteristic, BLERead, 32);

// ----------------------- SETUP ----------------------
void setup() {
  Serial.begin(9600);
  setup_BLE();
}

// ----------------------- LOOP -----------------------
void loop() {

  BLE_Update_Values();
  
}

// ------------------------ BLE -----------------------
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
     
      bluetoothCharacteristic.writeValue("{'t': 80, 'h': 40 }");

    }
  }else{
    Serial.print("Il centrale ");
    Serial.print(central.address());
    Serial.println(" non è connesso");
  }
  delay(1000);
}

