#include <ArduinoBLE.h>

// ----------------------- CONFIG -----------------------

// UUIDs
const char* nome_device[] = {"server1"};
const char* UuidService = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* UuidCharacteristic = "19b10001-e8f2-537e-4f6c-d104768a1215";

// ----------------------- VARIABLES ---------------------
const int numero_server = 1;
 
BLEDevice server_device[numero_server];
BLEDevice server_corrente;

// ----------------------- SETUP -------------------------
void setup() {
  Serial.begin(9600);
  bluetooth_setup();
}

// ----------------------- LOOP --------------------------
void loop() {
  for(int i = 0; i < numero_server; i++){

    if(server_device[i]){

      if(server_device[i].connected()){
        Serial.println("[OK] Connesso!");
        handle_device(server_device[i]);
        delay(100);
        Serial.println("[OK] Disconnect...");
        Serial.println("---------------------------------");
        delay(300);
        server_device[i].disconnect();
      }else{
        Serial.print("Provo a connettermi a: ");
        Serial.println(nome_device[i]);
        server_device[i].connect();
      }

      delay(5000);

    }
  }  
}

// ------------------------ BLE --------------------------

void bluetooth_setup(){

  while (!BLE.begin()) {
    Serial.println("Errore nell'inizializzazione del BLE");
    delay(500);
  }

  Serial.println("BLE inizializzato");

  BLE.setDeviceName("Centrale");
  BLE.setLocalName("Centrale");

  for(int i = 0; i < numero_server; i++){
    do{
      Serial.print("Searching...");
      Serial.println(nome_device[i]);
  
      delay(1000);

      BLE.scanForName(nome_device[i]);
      server_corrente = BLE.available();
    } while(!server_corrente);

    if(server_corrente) {
      Serial.println(server_corrente.localName());
      server_device[i] = server_corrente;  
    }
  }

  BLE.stopScan();
}
 
 

void handle_device(BLEDevice device){
  if(device.connected()){

    const int maxTentativi = 25;
    int tentativi = 0;

    while (!device.discoverAttributes() and (tentativi < maxTentativi)) {
      delay(200);
      tentativi++;
    }

    BLEService servizio;
    BLECharacteristic caratteristica;
    tentativi = 0;
    
    while (tentativi < maxTentativi) {
      servizio = device.service(UuidService);
      if (servizio) {
        caratteristica = servizio.characteristic(UuidCharacteristic);
        if (caratteristica) break;
      }
      delay(200);
      tentativi++;
    }

    if (!servizio || !caratteristica) {
      Serial.println("[ERROR] Servizio o caratteristica non trovati.");
      return;
    }

    Serial.println("[OK] Servizio e caratteristica trovati!");

    char buffer[51];
    int len = caratteristica.readValue(buffer, sizeof(buffer) - 1);
    Serial.print("[INFO] lunghezza: ");
    Serial.println(len);
    if (len > 0) {
      buffer[len] = '\0';
      Serial.print("[OK] Dati ottenuti: ");
      Serial.println(buffer);
    } else {
      Serial.println("[ERROR] Lettura dei dati fallita.");
    }
  }
}
 