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

#define WINDOW_1_FEEDBACK_PIN_OUT 8
#define WINDOW_2_FEEDBACK_PIN_OUT 11
#define GAS_VALVE_FEEDBACK_PIN_OUT A0
#define WATER_VALVE_FEEDBACK_PIN_OUT A1


// ----------------------- VARIABLES ---------------------
const int numero_server = 2;
int DangerLevel = 0;
long int token = 0;
 
BLEDevice server_device[numero_server];
BLEDevice server_corrente;

bool toggle_state_windows = false;
bool toggle_state_gas = false;
bool toggle_state_water = false;
 
// ----------------------- SETUP -------------------------
void setup() {
  Serial.begin(9600);
 
  pinMode(DANGER_PIN_1, OUTPUT);
  pinMode(DANGER_PIN_2, OUTPUT);

  pinMode(WINDOW_1_FEEDBACK_PIN, INPUT);
  pinMode(WINDOW_2_FEEDBACK_PIN, INPUT);
  pinMode(GAS_VALVE_FEEDBACK_PIN, INPUT);
  pinMode(WATER_VALVE_FEEDBACK_PIN, INPUT);

  pinMode(WINDOW_1_FEEDBACK_PIN_OUT, OUTPUT);
  pinMode(WINDOW_2_FEEDBACK_PIN_OUT, OUTPUT);
  pinMode(GAS_VALVE_FEEDBACK_PIN_OUT, OUTPUT);
  pinMode(WATER_VALVE_FEEDBACK_PIN_OUT, OUTPUT);

  digitalWrite(WINDOW_1_FEEDBACK_PIN_OUT, LOW); 
  digitalWrite(WINDOW_2_FEEDBACK_PIN_OUT, LOW);
  digitalWrite(GAS_VALVE_FEEDBACK_PIN_OUT, LOW);
  digitalWrite(WATER_VALVE_FEEDBACK_PIN_OUT, LOW);
 
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
  Serial.println(json_data);
}
 
// ----------------------- LOOP --------------------------
void loop() {

  // Gestione BLE
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
          //log_system_info("[OK] Disconnect...", String(nome_device[i]));
          log_system_info("---------------------------------");
          delay(300);
          server_device[i].disconnect();
        }else{
            //log_system_info(String(nome_device[i]));
            server_device[i].connect();
        }
        tentativi++;
      }

      get_danger_Level();
      danger_to_Central();

      connesso = false;
      delay(500);
    } else {
      get_danger_Level();
      danger_to_Central();
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
 
    //log_system_info("[OK] Servizio e caratteristica trovati!", nomedev);
 
    char buffer[51];
    int len = caratteristica.readValue(buffer, sizeof(buffer) - 1);
    //log_system_info("[INFO] lunghezza: " + String(len), nomedev);
    if (len > 0) {
      buffer[len] = '\0';
      //log_system_info("[OK] Dati ottenuti: " + String(buffer), nomedev);
 
      /*Avendo ottenuto i dati invio tutto al PC*/
      send_sensor_data(String(buffer), nomedev);
      delay(100);
    }else {
      log_system_info("[ERROR] Lettura dei dati fallita.", nomedev);
    }
  }
}

//---------------- UTILITY FUNCTIONS ---------------------

String make_data_json(String inputString, String nomeDevice) {
  // Inizializza i valori
  float temperature = 0.0;
  float humidity = 0.0;
  float gasLevel = 0.0;
  int dangerValue = 0;

  // Estrazione delle posizioni delle lettere chiave
  int tIndex = inputString.indexOf("T");
  int hIndex = inputString.indexOf("H");
  int gIndex = inputString.indexOf("G");
  int dIndex = inputString.indexOf("D");

  // Parsing dei valori se gli indici sono validi
  if (tIndex != -1 && hIndex != -1) { temperature = inputString.substring(tIndex + 1, hIndex).toFloat(); }
  if (hIndex != -1 && gIndex != -1) { humidity = inputString.substring(hIndex + 1, gIndex).toFloat(); }
  if (gIndex != -1 && dIndex != -1) { gasLevel = inputString.substring(gIndex + 1, dIndex).toFloat(); }
  if (dIndex != -1) { dangerValue = inputString.substring(dIndex + 1).toInt(); }

  // Crea il JSON
  DynamicJsonDocument doc(256);
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["gas"] = gasLevel;
  doc["sensor_id"] = nomeDevice;
  doc["danger_value"] = dangerValue;

  // Serializza in stringa
  String output;
  serializeJson(doc, output);
  return output;
}

void get_danger_Level() {
  if (Serial.available() > 0) {
    String rawInput = Serial.readStringUntil('\n'); // Legge l'intera riga, inclusi "54D", "1D", ecc.
    rawInput.trim(); // Rimuove spazi bianchi e \r

    log_system_info("Ricevuto dal PC: " + rawInput);

    // Verifica che finisca in 'D' (es. "0D", "5D" ecc.)
    if (rawInput.endsWith("D") && rawInput.length() >= 2) {
      String numPart = rawInput.substring(0, rawInput.length() - 1);  // estrae "5" da "5D"
      int receivedLevel = numPart.toInt();

      log_system_info("Interpretato DangerLevel: " + String(receivedLevel));

      if (receivedLevel >= 0 && receivedLevel < 4) {
        DangerLevel = receivedLevel;
        log_system_info("✅ Nuovo livello di Danger: " + String(DangerLevel));
      }
      if (receivedLevel == 4) {
        DangerLevel = 3;
        toggle_feedback_pulse(WINDOW_1_FEEDBACK_PIN_OUT, WINDOW_1_FEEDBACK_PIN, toggle_state_windows, "FINESTRE");
        toggle_feedback_pulse(WINDOW_2_FEEDBACK_PIN_OUT, WINDOW_2_FEEDBACK_PIN, toggle_state_windows, "FINESTRE");
      }
      else if (receivedLevel == 5) {
        DangerLevel = 3;
        toggle_feedback_pulse(GAS_VALVE_FEEDBACK_PIN_OUT, GAS_VALVE_FEEDBACK_PIN, toggle_state_gas, "GAS");
      }
      else if (receivedLevel == 6) {
        DangerLevel = 3;
        toggle_feedback_pulse(WATER_VALVE_FEEDBACK_PIN_OUT, WATER_VALVE_FEEDBACK_PIN, toggle_state_water, "ACQUA");
      }
    } else {
      log_system_info("⚠️ Formato ricevuto non valido o assente 'D'");
    }
  }
}

void toggle_feedback_pulse(int outPin, int readPin, bool& toggle_state, String label) {
  toggle_state = !toggle_state;

  digitalWrite(outPin, LOW);
  delay(10);

  digitalWrite(outPin, HIGH);
  delay(200);
  digitalWrite(outPin, LOW);
}

void danger_to_Central() {
  // PIN 5 e 4 per inviare il danger level binario
  if (DangerLevel >= 0 && DangerLevel <= 3) {
    digitalWrite(DANGER_PIN_1, (DangerLevel & 0b10) ? HIGH : LOW);
    digitalWrite(DANGER_PIN_2, (DangerLevel & 0b01) ? HIGH : LOW);

    log_system_info("To central: " + String(DangerLevel));
  } else {
    log_system_info("DangerLevel out of bounds, non comunico il nuovo livello al centrale");
  }
}
