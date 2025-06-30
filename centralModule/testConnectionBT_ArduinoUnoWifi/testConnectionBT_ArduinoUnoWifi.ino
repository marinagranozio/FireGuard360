#include <ArduinoBLE.h>
#include <ArduinoJson.h>
 
// ----------------------- CONFIG -----------------------
 
// UUIDs
const char* nome_device[] = {"server1", "server2"};
const char* UuidService = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* UuidCharacteristic = "19b10001-e8f2-537e-4f6c-d104768a1215";
 
#define DANGER_PIN_1  5
#define DANGER_PIN_2  4
// Pin per il controllo delle finestre e delle valvole
#define WINDOW_1_FEEDBACK_PIN 6
#define WINDOW_2_FEEDBACK_PIN 7
#define GAS_VALVE_FEEDBACK_PIN 2
#define WATER_VALVE_FEEDBACK_PIN 3

// ----------------------- VARIABLES ---------------------
const int numero_server = 2;
int DangerLevel = 0;
long int token = 0;
 
BLEDevice server_device[numero_server];
BLEDevice server_corrente;
 
// ----------------------- SETUP -------------------------
void setup() {
  Serial.begin(9600);
 
  pinMode(DANGER_PIN_1, OUTPUT);
  pinMode(DANGER_PIN_2, OUTPUT);

  pinMode(WINDOW_1_FEEDBACK_PIN, INPUT);
  pinMode(WINDOW_2_FEEDBACK_PIN, INPUT);
  pinMode(GAS_VALVE_FEEDBACK_PIN, INPUT);
  pinMode(WATER_VALVE_FEEDBACK_PIN, INPUT);
 
  digitalWrite(DANGER_PIN_1, LOW);
  digitalWrite(DANGER_PIN_2, LOW);
 
  bluetooth_setup();
}
 
//-------------------------- LOG & SERIAL COM -----------------------------
void log_system_info(String info, String nomeDevice = "Centrale"){
  Serial.println("LOG: [" + nomeDevice + "] - " + info);
}
 
void send_sensor_data(String info, String nomeDevice = ""){
  String json_data = make_data_json(info, nomeDevice);
  Serial.println("DATA:" + json_data);
}

void send_actuation_status(){
  // Controlla lo stato delle finestre e delle valvole
  String actuation_json = make_actuation_json();
  Serial.println("ACTUATION:" + actuation_json);
}
 
// ----------------------- LOOP --------------------------
void loop() {
  for(int i = 0; i < numero_server; i++){
 
    if(server_device[i]){
      int maxtentativi = 5;
      int tentativi = 0;
      bool connesso = false;

      while(!connesso && tentativi < maxtentativi){
        if(server_device[i].connected()){
          connesso = true;
        log_system_info("[OK] Connesso!", String(nome_device[i]));
        handle_device(server_device[i], nome_device[i]);
        delay(100);
        log_system_info("[OK] Disconnect...", String(nome_device[i]));
        log_system_info("---------------------------------");
        delay(300);
        server_device[i].disconnect();
      }else{
        log_system_info("Provo a connettermi a: " + String(nome_device[i]));
        server_device[i].connect();
      }
      tentativi++;
      }

      connesso = false;
 
      send_actuation_status();
      delay(3000);
 
    }
  }  
}
 
// ------------------------ BLE --------------------------
 
void bluetooth_setup(){
 
  while (!BLE.begin()) {
    log_system_info("Errore nell'inizializzazione del BLE");
    delay(500);
  }
 
  log_system_info("BLE inizializzato");
 
  BLE.setDeviceName("Centrale");
  BLE.setLocalName("Centrale");
 
  for(int i = 0; i < numero_server; i++){
    do{
      log_system_info("Searching..." + String(nome_device[i]));
 
      delay(10);
 
      BLE.scanForName(nome_device[i]);
      server_corrente = BLE.available();
    } while(!server_corrente);
 
    if(server_corrente) {
      log_system_info("Trovato", String(server_corrente.localName()));
      server_device[i] = server_corrente;  
    }
  }
 
  BLE.stopScan();
}
 
void handle_device(BLEDevice device, String nomedev){
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
      log_system_info("[ERROR] Servizio o caratteristica non trovati.", nomedev);
      return;
    }
 
    log_system_info("[OK] Servizio e caratteristica trovati!", nomedev);
 
    char buffer[51];
    int len = caratteristica.readValue(buffer, sizeof(buffer) - 1);
    log_system_info("[INFO] lunghezza: " + String(len), nomedev);
    if (len > 0) {
      buffer[len] = '\0';
      log_system_info("[OK] Dati ottenuti: " + String(buffer), nomedev);
 
      /*Avendo ottenuto i dati invio tutto al PC e recupero il nuovo DangerLevel*/
      send_sensor_data(String(buffer), nomedev);
      delay(100);
      get_danger_Level();
      danger_to_Central();
 
    } else {
      log_system_info("[ERROR] Lettura dei dati fallita.", nomedev);
    }
  }
}


//---------------- UTILITY FUNCTIONS ---------------------

String make_actuation_json()
{
  // Crea un oggetto JSON
  DynamicJsonDocument doc(1024);
 
  // Aggiungi i dati all'interno dell'oggetto "data"
  JsonObject actuation = doc.createNestedObject("actuation");
 
  // Controlla lo stato delle finestre e delle valvole
  bool window1_status = digitalRead(WINDOW_1_FEEDBACK_PIN);
  bool window2_status = digitalRead(WINDOW_2_FEEDBACK_PIN);
  bool gas_valve_status = digitalRead(GAS_VALVE_FEEDBACK_PIN);
  bool water_valve_status = digitalRead(WATER_VALVE_FEEDBACK_PIN);
 
  actuation["window1"] = window1_status ? "open" : "closed";
  actuation["window2"] = window2_status ? "open" : "closed";
  actuation["gasValve"] = gas_valve_status ? "open" : "closed";
  actuation["waterValve"] = water_valve_status ? "open" : "closed";
 
  // Converto l'oggetto JSON in una stringa
  String output;
  serializeJson(doc, output);
 
  return output;
}

String make_data_json(String inputString, String nomeDevice)
{
  // Estraiamo i valori dalla stringa
  float t = 0.0;
  float h = 0.0;
  float gasLevel = 0.0;
  int dangerValue = 0;

  //---------PARSING---------
  // Prendo i valori numerici dopo ogni lettera
  if (inputString.indexOf("T") != -1) {
    t = inputString.substring(inputString.indexOf("T") + 1, inputString.indexOf("H")).toFloat();
  }
  if (inputString.indexOf("H") != -1) {
    h = inputString.substring(inputString.indexOf("H") + 1, inputString.indexOf("G")).toFloat();
  }
  if (inputString.indexOf("G") != -1) {
    gasLevel = inputString.substring(inputString.indexOf("G") + 1, inputString.indexOf("D")).toFloat() / 100.0; // Convertiamo il valore in percentuale
  }
  if (inputString.indexOf("D") != -1) {
    dangerValue = inputString.substring(inputString.indexOf("D") + 1).toInt();
  }
 
//----------CREO IL JSON----------
// Crea un oggetto JSON
  DynamicJsonDocument doc(1024);
 
  doc["token"] = token++;
  // Aggiungi i dati all'interno dell'oggetto "data"
  JsonObject data = doc.createNestedObject("data");
  data["name"] = nomeDevice;
  data["dangerValue"] = dangerValue;
  data["temperature"] = t;
  data["humidity"] = h;
  data["gas"] = gasLevel;
 
  // Converto l'oggetto JSON in una stringa
  String output;
  serializeJson(doc, output);
 
  return output;
 
//{
  //"token": "<token_generated>",
  //"data" :
  //  {
  //    "id": number,
  //    "name": string,
  //    "dangerValue": number,
  //    "temperature": number,
  //    "humidity": number,
  //    "gas": number,
  //  }
//}
}


void get_danger_Level()
{
  //Recupera dal seriale del PC il livello di Danger
  if(Serial.available()>0){
    String DangerLevelString = Serial.readStringUntil('D');
    log_system_info("Ricevuto dal PC livello di Danger: " + DangerLevelString);
    int string_to_int_danger = DangerLevelString.toInt();
 
    if(string_to_int_danger < 4 && string_to_int_danger >= 0){
      DangerLevel = string_to_int_danger;
      log_system_info("Nuovo livello di Danger: " + String(DangerLevel));
    }
    else
      log_system_info("Il PC non ha inviato un danger level valido, DangerLevel non modificato");
 
  }
  else
    log_system_info("Il PC non ha comunicato nulla, DangerLevel non modificato.");
 
 
}
 
void danger_to_Central()
{
  //UTILIZZA I PIN 5 e 4 per inviare il danger level binario
  if(DangerLevel < 4 && DangerLevel >= 0)
  {
    if(DangerLevel == 0){ digitalWrite(DANGER_PIN_1, LOW); digitalWrite(DANGER_PIN_2, LOW); }
    else if(DangerLevel == 1){digitalWrite(DANGER_PIN_1, LOW); digitalWrite(DANGER_PIN_2, HIGH);}
    else if(DangerLevel == 2){digitalWrite(DANGER_PIN_1, HIGH); digitalWrite(DANGER_PIN_2, LOW);}
    else if(DangerLevel == 3){digitalWrite(DANGER_PIN_1, HIGH); digitalWrite(DANGER_PIN_2, HIGH);}
    log_system_info("Comunico al centrale il livello di pericolo: " + String(DangerLevel));
  }
  else
    log_system_info("DangerLevel out of bounds, non comunico il nuovo livello al centrale");
}
 
