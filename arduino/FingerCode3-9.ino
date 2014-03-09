#include <SoftwareSerial.h>  
// Pins to read from
int fing1 = A0;
int fing2 = A1;
int fing3 = A2;



int fing4 = A3;

// Schmitt trigger voltage values for each finger(Assume 5V Power)
int v11=1024*(2.7/5);
int v12=1024*(2.1/5);

int v21=1024*(2.7/5);
int v22=1024*(2.1/5);

int v31=1024*(2.7/5);
int v32=1024*(2.1/5);

int v41=1024*(2.7/5);
int v42=1024*(2.1/5);


int fingState[] = {0,0,0,0};

int led = 13;          // Pin 13 has an LED connected on most Arduino boards.

int bluetoothTx = 0;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 1;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

void setup() {                
  pinMode(led, OUTPUT);  // initialize the digital LED pin as an output.
  
  Serial.begin(115200);  // Begin the serial monitor at 115200bps
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
}

void loop(){
  
 // Finger 1 
 if (fingState[0]) { // If finger 1 is ON
   if(analogRead(fing1) > v11){ // Finger 1 is now OFF
      digitalWrite(led, LOW); // For debugging
      Serial.print(-1); // Let computer know finger 1 is now OFF
      fingState[0] = 0; // Set as OFF for next time
   }
 }
 else{ // If finger 1 is OFF
   if (analogRead(fing1) < v12) { // Finger 1 is now ON
      digitalWrite(led, HIGH); // For debugging
      Serial.print(10); // Let computer know finger 1 is now ON
      fingState[0] = 1; // Set as ON for next time
   }
 }

 // Finger 2 
 if (fingState[1]) { // If finger 2 is ON
   if(analogRead(fing2) > v21){ // Finger 2 is now OFF
      Serial.print(-2); // Let computer know finger 2 is now OFF
      fingState[1] = 0; // Set as OFF for next time
   }
 }
 else{ // If finger 2 is OFF
   if (analogRead(fing2) < v22) { // Finger 2 is now ON
      Serial.print(20); // Let computer know finger 2 is now ON
      fingState[1] = 1; // Set as ON for next time
   }
 }

 // Finger 3
 if (fingState[2]) { // If finger 3 is ON
   if(analogRead(fing3) > v31){ // Finger 3 is now OFF
      Serial.print(-3); // Let computer know finger 3 is now OFF
      fingState[2] = 0; // Set as OFF for next time
   }
 }
 else{ // If finger 3 is OFF
   if (analogRead(fing3) < v32) { // Finger 3 is now ON
      Serial.print(30); // Let computer know finger 3 is now ON
      fingState[2] = 1; // Set as ON for next time
   }
 }

 // Finger 4
 if (fingState[3]) { // If finger 4 is ON
   if(analogRead(fing4) > v41){ // Finger 4 is now OFF
      Serial.print(-4); // Let computer know finger 4 is now OFF
      fingState[3] = 0; // Set as OFF for next time
   }
 }
 else{ // If finger 4 is OFF
   if (analogRead(fing4) < v42) { // Finger 4 is now ON
      Serial.print(40); // Let computer know finger 4 is now ON
      fingState[3] = 1; // Set as ON for next time
   }
 } 

 
}
