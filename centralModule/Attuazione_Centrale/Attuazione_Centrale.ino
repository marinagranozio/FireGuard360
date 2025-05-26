#include <Servo.h>

//--------- COSTANTI
#define PIN_DANGER_A 4
#define PIN_DANGER_B 5
 
#define RELAY1 13        // Relè per il primo motore

//---------  VARIABILI
Servo myservo1;  // create servo object to control a servo
Servo myservo2;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position

int dangerLevel = 0;  //Danger Level
int new_dangerLevel = 0;  //Danger Level ricevuto dai pin
bool changed = false;   //è cambiato il valore del danger value?

//------------- SETUP

void setup() {
  Serial.begin(9600);
  myservo1.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo1.write(180);
  myservo2.attach(10);  // attaches the servo on pin 9 to the servo object
  myservo2.write(180);
  pinMode(RELAY1, OUTPUT);

  pinMode(PIN_DANGER_A, INPUT);
  pinMode(PIN_DANGER_B, INPUT);
 
  digitalWrite(RELAY1, LOW);

}

//--------------- LOOP
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
}

//------------------ UTILS

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

/*Funzione che avvia attuazioni diverse in base al dangerLevel*/
void attuazione()
{
    //TODO: COMPLETA CON LE ATTUAZIONI NEI VARI CASI
  if(dangerLevel == 0){ 
    /* ***VECCHIA ATTUAZIONE DI PROVA***
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo1.write(pos);              // tell servo to go to position in variable 'pos'
      myservo2.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo1.write(pos);              // tell servo to go to position in variable 'pos'
      myservo2.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    digitalWrite(RELAY1, HIGH);
    Serial.println(digitalRead(RELAY1));
    delay(10000);
    digitalWrite(RELAY1, LOW);
    Serial.println(digitalRead(RELAY1));

    getDangerValue(); */}
  else if(dangerLevel == 1){ }
  else if(dangerLevel == 2){ }
  else if(dangerLevel == 3){ }
}
 