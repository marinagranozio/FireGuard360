#include <Servo.h>

// --- PIN DEFINITI ---
#define SERVO_1_PIN 9        // D9
#define SERVO_2_PIN 10       // D10
#define GAS_PUMP_PIN 13      // D13
#define WATER_PUMP_PIN 12    // D12

Servo myservo1;
Servo myservo2;

bool firstCycleDone = false;

void setup() {
  Serial.begin(9600);

  // Inizializzazione servo
  myservo1.attach(SERVO_1_PIN);
  myservo2.attach(SERVO_2_PIN);

  // Inizializzazione pompe
  pinMode(GAS_PUMP_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);

  Serial.println("✅ Sistema inizializzato");

  // CENTRA I SERVO PRIMA DI TUTTO
  Serial.println("Centraggio dei servo a 90°...");
  myservo1.write(90);
  myservo2.write(90);
  delay(2000); // aspetta 2 secondi per assestamento
}

void loop() {


    // --- Step 1: Apri finestre (senso orario: 90 → 0) ---
    Serial.println("🪟 Apro finestre...");
    for (int pos = 90; pos >= 0; pos--) {
      myservo1.write(pos);
      myservo2.write(pos);
      delay(10);
    }
    delay(2000);

    // --- Step 2: Chiudi finestre (senso orario: 0 → 90) ---
    Serial.println("🪟 Chiudo finestre...");
    for (int pos = 0; pos <= 90; pos++) {
      myservo1.write(pos);
      myservo2.write(pos);
      delay(10);
    }
    delay(2000);

    // --- Step 3: Attiva pompa GAS ---
    Serial.println("🟢 Attivo pompa GAS...");
    digitalWrite(GAS_PUMP_PIN, HIGH);
    delay(2000);

    Serial.println("🔴 Disattivo pompa GAS...");
    digitalWrite(GAS_PUMP_PIN, LOW);
    delay(2000);

    // --- Step 4: Attiva pompa ACQUA ---
    Serial.println("💧 Attivo pompa ACQUA...");
    digitalWrite(WATER_PUMP_PIN, HIGH);
    delay(2000);

    Serial.println("💧 Disattivo pompa ACQUA...");
    digitalWrite(WATER_PUMP_PIN, LOW);
    delay(2000);

    Serial.println("✅ TEST COMPLETATO\n\n");

}
