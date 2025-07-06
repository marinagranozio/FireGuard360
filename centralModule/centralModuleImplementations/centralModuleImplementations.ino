#include <Servo.h>

// --------- PIN DEFINITI -----------------
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

#define RELAY_WATERVALVE 12
#define RELAY_GASVALVE 13

#define SERVO_1_PIN 9
#define SERVO_2_PIN 10

// --------- OGGETTI E VARIABILI -------
Servo myservo1;
Servo myservo2;

int dangerLevel = -1;
int lastStableLevel = -1;
int stableCount = 0;
const int requiredStability = 3;

bool windows_open = false;
bool gas_valve_open = false;
bool water_valve_open = false;

// ---------- SETUP --------------------
void setup() {
  Serial.begin(9600);

  myservo1.attach(SERVO_1_PIN);
  myservo2.attach(SERVO_2_PIN);

  myservo1.write(90);  // finestre chiuse
  myservo2.write(90);
  delay(1000);

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
  Serial.println("✅ Sistema inizializzato. In attesa di DangerLevel stabile...");
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
    Serial.println("\n🟡 Nuovo DangerLevel STABILE rilevato: " + String(dangerLevel));
    esegui_attuazione(dangerLevel);
  }

  monitor_feedback(); // Sempre attiva, legge comandi 4D, 5D, 6D
  delay(20);
}

// ---------- FUNZIONI ------------------

void init_actuators() {
  windows_open = false;
  gas_valve_open = false;
  water_valve_open = false;

  digitalWrite(RELAY_GASVALVE, LOW);
  digitalWrite(RELAY_WATERVALVE, LOW);

  digitalWrite(WINDOW_1_FEEDBACK_PIN, LOW);
  digitalWrite(WINDOW_2_FEEDBACK_PIN, LOW);
  digitalWrite(GAS_VALVE_FEEDBACK_PIN, LOW);
  digitalWrite(WATER_VALVE_FEEDBACK_PIN, LOW);
}

int getDangerValue() {
  int b = digitalRead(PIN_DANGER_A);
  int a = digitalRead(PIN_DANGER_B);
  return (a << 1) | b;
}

void monitor_feedback() {
  static bool prev_window_state = LOW;
  static bool prev_gas_state = LOW;
  static bool prev_water_state = LOW;

  bool window_signal = digitalRead(WINDOW_1_FEEDBACK_PIN_IN);
  bool gas_signal = digitalRead(GAS_VALVE_FEEDBACK_PIN_IN);
  bool water_signal = digitalRead(WATER_VALVE_FEEDBACK_PIN_IN);

  if (window_signal == HIGH && prev_window_state == LOW) {
    Serial.println("🔁 Feedback FINESTRE ricevuto → toggle");
    actuate_Windows(!windows_open);
  }

  if (gas_signal == HIGH && prev_gas_state == LOW) {
    Serial.println("🔁 Feedback GAS ricevuto → toggle");
    actuate_GasValve(!gas_valve_open);
  }

  if (water_signal == HIGH && prev_water_state == LOW) {
    Serial.println("🔁 Feedback ACQUA ricevuto → toggle");
    actuate_WaterValve(!water_valve_open);
  }

  prev_window_state = window_signal;
  prev_gas_state = gas_signal;
  prev_water_state = water_signal;
}

void esegui_attuazione(int level) {
  switch (level) {
    case 0:
      actuate_Windows(true);      // finestre aperte
      actuate_GasValve(true);     // gas aperto
      actuate_WaterValve(false);  // acqua chiusa
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
    case 3:
      Serial.println("📟 Modalità comandi manuali attiva");
      break;
    default:
      Serial.println("⚠️ DangerLevel non gestito: " + String(level));
      break;
  }
}

// -------- ATTUATORI -------------------

void actuate_Windows(bool open) {
  if (open == windows_open) return;

  Serial.println(open ? "🪟 Apro finestre..." : "🪟 Chiudo finestre...");
  for (int pos = (open ? 90 : 0); (open ? pos >= 0 : pos <= 90); pos += (open ? -1 : 1)) {
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