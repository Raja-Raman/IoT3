// Microwave radar RCWL 0516 and PIR State machine.  simplified state management- 3 states now.  
#include <SoftwareSerial.h>
#include "Timer.h"

/*
// home
const int pir1 = 8;
const int pirled1 = 4;
const int radar = 11;
const int radarled = 5;
const int roomstatusled = 7;
const int buzzer = 13; //  @office: buzzer is active high
const int relay1 = ??;
*/
// office
const int pir1 = 12;
const int pirled1 = 3;
const int radar = 11;  //A6;  originally pir2 = 11 -> has been given to uW radar ! 
const int radarled = 5;
const int roomstatusled = 6;
const int buzzer = 10; // buzzer is active high
const int relay1 = 7;

// Software serial port for BT
/*
// home
const int softTx = 3;  // TX pin for Arduino; connect the Rx of BT shield to this pin
const int softRx = 2;   // RX pin for Arduino; Tx of the BT shield
*/
// office
const int softTx = A3;  // TX pin for Arduino; connect the Rx of BT shield to this pin
const int softRx = A2;   // RX pin for Arduino; Tx of the BT shield
SoftwareSerial softSerial(softRx, softTx); // The order is RX, TX
#define ser Serial
#define baud 9600
/*
#define ser softSerial
//#define baud 38400 // in case 9800 is not working with BT
#define baud 9600
*/

// *** Timer durations SHOULD be unsigned long int, if they are > 16 bit! ***
unsigned int statusinterval = 5*1000; // 5 seconds
unsigned int tickinterval = 100;      // in milliSec; 10 ticks=1 second 
unsigned int buzzerticks = 50*10;     // n*10 ticks=n seconds
unsigned int releaseticks = 60*10;    // n*10 ticks=n seconds  
Timer T;

void setup() {
  pinMode(radar, INPUT);
  pinMode(pir1, INPUT);
  pinMode(radarled, OUTPUT); 
  pinMode(pirled1, OUTPUT);  
  pinMode(roomstatusled, OUTPUT); 
  pinMode(buzzer, OUTPUT); 
  pinMode(relay1, OUTPUT); 
  digitalWrite(relay1, LOW);
  digitalWrite(radarled, LOW);
  digitalWrite(pirled1, LOW);
  digitalWrite(roomstatusled, LOW);
  digitalWrite(buzzer, LOW); // active high @ office
  ser.begin(baud);
  ser.println("0"); // status = 'Just Reset'  
  blinker();    
  occupyRoom(); // program starts in occupied status
  int timerid1 = T.every(tickinterval, tick);
  T.every(statusinterval ,sendStatus);
}

void loop() {
  T.update();
}

boolean occupied = 1;  
boolean radarstatus = 0;
boolean pirstatus1 = 0;
unsigned int tickcounter = 0;

void tick() {
    radarstatus = digitalRead(radar);
    digitalWrite(radarled, radarstatus); 
    pirstatus1 = digitalRead(pir1);
    digitalWrite(pirled1, pirstatus1);  

    if (!occupied)
        if (radarstatus & pirstatus1) 
            occupyRoom();
            
    if (radarstatus | pirstatus1) 
          tickcounter = 0;  // keep resetting it, if there is motion
             
    tickcounter++;
    
    if (tickcounter == buzzerticks) {
        if (occupied)
            buzz();
    }
    else        
    if (tickcounter == releaseticks){
         tickcounter = 0;
         if (occupied)
               releaseRoom();  
    }  
}

// Status:
// 1 - occupied state
// 2 - pre-release warning event
// 3 - vacant state
int statusCode = 0;

void sendStatus() {
  ser.print(statusCode);
}

void occupyRoom() {
  occupied = 1;
  statusCode = "1";
  digitalWrite(roomstatusled, HIGH);
  digitalWrite(relay1, HIGH);
  sendStatus(); 
}

void releaseRoom() {
  occupied = 0;
  statusCode = "3";  
  digitalWrite(roomstatusled, LOW);
  digitalWrite(relay1, LOW);
  sendStatus(); 
}

// pre-release warning 
void buzz() {
  statusCode = "2"; 
  sendStatus();   
  T.oscillate (buzzer,75, LOW, 6);  
  T.oscillate (roomstatusled, 100, HIGH, 4);
}

void blinker() {
  for (int i=0; i<6; i++) {
    digitalWrite(radarled, HIGH);
    digitalWrite(pirled1, HIGH);
    digitalWrite(roomstatusled, HIGH);
    delay(200);  
    digitalWrite(radarled, LOW);
    digitalWrite(pirled1, LOW);
    digitalWrite(roomstatusled, LOW);
    delay(200);   
  }
}

