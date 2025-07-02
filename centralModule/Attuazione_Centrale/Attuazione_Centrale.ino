#include <Servo.h>

//--------- COSTANTI -----------------

#define PIN_DANGER_A 4
#define PIN_DANGER_B 5

// Pin per il controllo delle finestre e delle valvole
#define WINDOW_1_FEEDBACK_PIN 6
#define WINDOW_2_FEEDBACK_PIN 7
#define GAS_VALVE_FEEDBACK_PIN 2
#define WATER_VALVE_FEEDBACK_PIN 3

#define WINDOW_1_FEEDBACK_PIN_IN 8
#define WINDOW_2_FEEDBACK_PIN_IN 11
#define GAS_VALVE_FEEDBACK_PIN_IN A0
#define WATER_VALVE_FEEDBACK_PIN_IN A1

#define RELAY_WATERVALVE 13        // Relè per la pompa dell'acqua
#define RELAY_GASVALVE 12        // Relè per la valvola del gas


//---------  VARIABILI ---------------- 

Servo myservo1;  // create servo object to control a servo
Servo myservo2;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position

int dangerLevel = 0;  //Danger Level
int new_dangerLevel = 0;  //Danger Level ricevuto dai pin
bool changed = false;   //è cambiato il valore del danger value?

bool windows_open = true; // Stato delle finestre
bool gas_valve_open = true; // Stato della valvola del gas
bool water_valve_open = false; // Stato della valvola dell'acqua

//------------- SETUP ------------------

void setup() {
  Serial.begin(9600);
  myservo1.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo1.write(180);
  myservo2.attach(10);  // attaches the servo on pin 10 to the servo object
  myservo2.write(180);
  
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

  digitalWrite(WINDOW_1_FEEDBACK_PIN, HIGH); 
  digitalWrite(WINDOW_2_FEEDBACK_PIN, HIGH);
  digitalWrite(GAS_VALVE_FEEDBACK_PIN, HIGH);
  digitalWrite(WATER_VALVE_FEEDBACK_PIN, LOW);
 
  digitalWrite(RELAY_WATERVALVE, LOW);
  digitalWrite(RELAY_GASVALVE, HIGH);
}

//--------------- LOOP ----------------

void loop() {
 //Controllo che il dangerLevel sia cambiato 
 new_dangerLevel = getDangerValue();
 changed = (dangerLevel != new_dangerLevel);

 if(changed)  //Se è cambiato il dangerLevel
 {
    dangerLevel = new_dangerLevel;
    Serial.println("Il DangerLevel è cambiato! Nuovo: " + String(dangerLevel));
    changed = false;
    attuazione();
 }
   delay(1000);
  
  if(dangerLevel==0) {
    bool feedbackW1=(digitalRead(WINDOW_1_FEEDBACK_PIN_IN)==HIGH);
    bool feedbackW2=(digitalRead(WINDOW_2_FEEDBACK_PIN_IN)==HIGH);  
    bool feedbackGas=(digitalRead(GAS_VALVE_FEEDBACK_PIN_IN)==HIGH); 
    bool feedbackWater=(digitalRead(WATER_VALVE_FEEDBACK_PIN_IN)==HIGH); 
    if(feedbackW1!=windows_open||feedbackW2!=windows_open) {
      actuate_Windows(feedbackW1); 
    }
    if(feedbackGas!=gas_valve_open) {
      actuate_GasValve(feedbackGas); 
    }
    if(feedbackWater!=water_valve_open) {
      actuate_WaterValve(feedbackWater); 
    }
  }
}

//------------------ UTILS -----------------

//Leggo i pin per ottenere il dangerValue
int getDangerValue()
{
  int a;
  int b;

  a = digitalRead(PIN_DANGER_A);
  b = digitalRead(PIN_DANGER_B);
  
  int ndangerLevel = a*2+b*1;
  Serial.print("Nuovo DangerLevel: ");
  Serial.println(ndangerLevel);
  return ndangerLevel;
  
}

void actuate_Windows(bool open_windows)
{

  if(open_windows && !windows_open)
  {
    for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo1.write(pos);              
      myservo2.write(pos);             
      delay(15);                       
    }
    windows_open = true; // Aggiorno lo stato delle finestre
  }

  else 
  if(!open_windows && windows_open)
  {
    for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo1.write(pos);              // tell servo to go to position in variable 'pos'
      myservo2.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }

    windows_open = false; // Aggiorno lo stato delle finestre
  }

  // Chiude le finestre
  digitalWrite(WINDOW_1_FEEDBACK_PIN, windows_open ? HIGH : LOW);
  digitalWrite(WINDOW_2_FEEDBACK_PIN, windows_open ? HIGH : LOW);
}

void actuate_GasValve(bool open_valve)
{
  // Chiude la valvola del gas
  if(open_valve && !gas_valve_open)
  {
    digitalWrite(RELAY_GASVALVE, HIGH); // Attiva il relè per aprire la valvola del gas
    gas_valve_open = true;
    Serial.println("Valvola del gas aperta");
  }
  else if(!open_valve && gas_valve_open)
  {
    digitalWrite(RELAY_GASVALVE, LOW); // Disattiva il relè per chiudere la valvola del gas
    gas_valve_open = false;
    Serial.println("Valvola del gas chiusa");
  }

  digitalWrite(GAS_VALVE_FEEDBACK_PIN, gas_valve_open ? HIGH : LOW);
}

void actuate_WaterValve(bool open_valve)
{
  if(open_valve && !water_valve_open)
  {
    digitalWrite(RELAY_WATERVALVE, HIGH); // Attiva il relè per aprire la valvola dell'acqua
    water_valve_open = true;
    Serial.println("Valvola dell'acqua aperta");
  }
  else if(!open_valve && water_valve_open)
  {
    digitalWrite(RELAY_WATERVALVE, LOW); // Disattiva il relè per chiudere la valvola dell'acqua
    water_valve_open = false;
    Serial.println("Valvola dell'acqua chiusa");
  }
  digitalWrite(WATER_VALVE_FEEDBACK_PIN, water_valve_open ? HIGH : LOW);
  
}

/*Funzione che avvia attuazioni diverse in base al dangerLevel*/
void attuazione()
{
    //TODO: COMPLETA CON LE ATTUAZIONI NEI VARI CASI
  if(dangerLevel == 0){actuate_Windows(true); actuate_GasValve(true); actuate_WaterValve(false); } //Tengo aperte finestre, gas, acqua - dangerLevel 0
  else if(dangerLevel == 1){ actuate_Windows(false); actuate_GasValve(true); actuate_WaterValve(false); } //Chiudo finestre, tengo gas e acqua aperti - dangerLevel 1
  else if(dangerLevel == 2){ actuate_Windows(false); actuate_GasValve(false); actuate_WaterValve(true); } //Chiudo finestre e gas, tengo acqua aperta - dangerLevel 2
}
 
