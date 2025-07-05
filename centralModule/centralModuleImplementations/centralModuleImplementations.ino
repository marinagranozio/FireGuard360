#include <Servo.h>

// --------- COSTANTI -----------------
#define PIN_DANGER_A 4
#define PIN_DANGER_B 5

#define WINDOW_1_FEEDBACK_PIN 6
#define WINDOW_2_FEEDBACK_PIN 7
#define GAS_VALVE_FEEDBACK_PIN 2
#define WATER_VALVE_FEEDBACK_PIN 3

#define WINDOW_1_FEEDBACK_PIN_IN 8
#define WINDOW_2_FEEDBACK_PIN_IN 11
#define GAS_VALVE_FEEDBACK_PIN_IN A0
#define WATER_VALVE_FEEDBACK_PIN_IN A1

#define RELAY_WATERVALVE 13
#define RELAY_GASVALVE 12

// --------- OGGETTI E VARIABILI -------
Servo myservo1;
Servo myservo2;

int dangerLevel = -1;             // valore attuale
int lastStableLevel = -1;         // ultimo livello confermato
int stableCount = 0;              // contatore di conferme stabili
const int requiredStability = 3;  // letture consecutive richieste

bool windows_open = true;
bool gas_valve_open = true;
bool water_valve_open = false;

// ---------- SETUP --------------------
void setup() {
  Serial.begin(9600);

  myservo1.attach(9); myservo2.attach(10);
  myservo1.write(180); myservo2.write(180);

  pinMode(RELAY_WATERVALVE, OUTPUT);
  pinMode(RELAY_GASVALVE, OUTPUT);

  pinMode(PIN_DANGER_A, INPUT);
  pinMode(PIN_DANGER_B, INPUT);

  pinMode(WINDOW_1_FEEDBACK_PIN_IN, INPUT);
  pinMode(WINDOW_2_FEEDBACK_PIN_IN, INPUT);
  pinMode(GAS_VALVE_FEEDBACK_PIN_IN, INPUT);
  pinMode(WATER_VALVE_FEEDBACK_PIN_IN, INPUT);

  pinMode(WINDOW_1_FEEDBACK_PIN, OUTPUT);
  pinMode(WINDOW_2_FEEDBACK_PIN, OUTPUT);
  pinMode(GAS_VALVE_FEEDBACK_PIN, OUTPUT);
  pinMode(WATER_VALVE_FEEDBACK_PIN, OUTPUT);

  init_actuators();
  Serial.println("Sistema inizializzato. In attesa di DangerLevel stabile...");
}

// ---------- LOOP ---------------------
void loop() {
  int currentLevel = getDangerValue();

  if (currentLevel == lastStableLevel) {
    stableCount++;
  } else {
    stableCount = 0;
    lastStableLevel = currentLevel;
  }

  if (stableCount >= requiredStability && currentLevel != dangerLevel) {
    dangerLevel = currentLevel;
    Serial.println("🟡 Nuovo DangerLevel STABILE rilevato: " + String(dangerLevel));
    esegui_attuazione(dangerLevel);
  }

  // Se livello di sicurezza, verifica feedback ogni ciclo
  if (dangerLevel == 0) {
    monitor_feedback();
  }

  delay(500);
}

// ---------- FUNZIONI ------------------

// Inizializza tutti gli attuatori in stato sicuro all'avvio
void init_actuators() {
  windows_open = true;
  gas_valve_open = true;
  water_valve_open = false;

  digitalWrite(RELAY_WATERVALVE, LOW);
  digitalWrite(RELAY_GASVALVE, HIGH);

  digitalWrite(WINDOW_1_FEEDBACK_PIN, HIGH);
  digitalWrite(WINDOW_2_FEEDBACK_PIN, HIGH);
  digitalWrite(GAS_VALVE_FEEDBACK_PIN, HIGH);
  digitalWrite(WATER_VALVE_FEEDBACK_PIN, LOW);
}

// Legge i due bit di input per determinare il dangerLevel
int getDangerValue() {
  int a = digitalRead(PIN_DANGER_A);
  int b = digitalRead(PIN_DANGER_B);
  return (a << 1) | b; // a*2 + b*1
}

// Controlla feedback da sensori e riapplica attuazione se serve
void monitor_feedback() {
  bool f1 = digitalRead(WINDOW_1_FEEDBACK_PIN_IN);
  bool f2 = digitalRead(WINDOW_2_FEEDBACK_PIN_IN);
  bool gas = digitalRead(GAS_VALVE_FEEDBACK_PIN_IN);
  bool water = digitalRead(WATER_VALVE_FEEDBACK_PIN_IN);

  if (f1 != windows_open || f2 != windows_open) actuate_Windows(f1);
  if (gas != gas_valve_open) actuate_GasValve(gas);
  if (water != water_valve_open) actuate_WaterValve(water);
}

// Gestisce le attuazioni principali in base al livello
void esegui_attuazione(int level) {
  switch (level) {
    case 0:
      actuate_Windows(true);
      actuate_GasValve(true);
      actuate_WaterValve(false);
      break;
    case 1:
      actuate_Windows(false);
      actuate_GasValve(true);
      actuate_WaterValve(false);
      break;
    case 2:
      actuate_Windows(false);
      actuate_GasValve(false);
      actuate_WaterValve(true);
      break;
    default:
      Serial.println("DangerLevel non gestito: " + String(level));
      break;
  }
}

// -------- ATTUATORI -------------------

void actuate_Windows(bool open) {
  if (open == windows_open) return;

  Serial.println(open ? "🪟 Apro finestre..." : "🪟 Chiudo finestre...");
  for (int pos = open ? 0 : 90; open ? pos <= 90 : pos >= 0; pos += open ? 1 : -1) {
    myservo1.write(pos);
    myservo2.write(pos);
    delay(10);
  }

  windows_open = open;
  digitalWrite(WINDOW_1_FEEDBACK_PIN, open ? HIGH : LOW);
  digitalWrite(WINDOW_2_FEEDBACK_PIN, open ? HIGH : LOW);
}

void actuate_GasValve(bool open) {
  if (open == gas_valve_open) return;

  Serial.println(open ? "🟢 Apro valvola GAS" : "🔴 Chiudo valvola GAS");
  digitalWrite(RELAY_GASVALVE, open ? HIGH : LOW);
  gas_valve_open = open;
  digitalWrite(GAS_VALVE_FEEDBACK_PIN, open ? HIGH : LOW);
}

void actuate_WaterValve(bool open) {
  if (open == water_valve_open) return;

  Serial.println(open ? "💧 Apro valvola ACQUA" : "💧 Chiudo valvola ACQUA");
  digitalWrite(RELAY_WATERVALVE, open ? HIGH : LOW);
  water_valve_open = open;
  digitalWrite(WATER_VALVE_FEEDBACK_PIN, open ? HIGH : LOW);
}
