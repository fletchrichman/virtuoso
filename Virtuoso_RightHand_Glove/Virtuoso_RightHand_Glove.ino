#include <FreeSixIMU.h>
#include <FIMU_ADXL345.h>
#include <FIMU_ITG3200.h>
#include <SoftwareSerial.h>  

#include <Wire.h>

///// Finger Stuff ////
int fing1 = A0;
int fing2 = A1;
int fing3 = A2;
int fing4 = A3;
float POWER  = 4.1; // Voltage used to power Arduino

// Schmitt trigger voltage values for each finger
int v11=1024*2.2/POWER; //pinky
int v12=1024*1.9/POWER; //pinky

int v21=1024*1.55/POWER; //ring
int v22=1024*1.35/POWER; //ring

int v31=1024*1.3/POWER; //middle
int v32=1024*1.1/POWER; //middle

int v41=1024*2.2/POWER; //pointer
int v42=1024*1.9/POWER; //pointer

int fingState[] = {0,0,0,0};

int led = 13;          // Pin 13 has an LED connected on most Arduino boards.


///// IMU Stuff ////
float values[6];

float yaw;
float pitch;
float roll;

int avSamp = 25; // number of samples to be averaged

// Set the FreeSixIMU object
FreeSixIMU sixDOF = FreeSixIMU();

float threshRot = 350; // Threshold value for rotation
int rotWaitVal = 10; // How many times to wait for rotation
int rotWait = 0;

//// Bluetooth Stuff ////
int bluetoothTx = 0;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 1;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);


void setup() { 
  pinMode(led, OUTPUT);  // initialize the digital LED pin as an output.
  Wire.begin();
  Serial.begin(115200);  // Begin the serial monitor at 115200bps
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  delay(5);
  sixDOF.init(); //begin the IMU
  delay(5);
}

void loop() { 
    
  for(int j=0; j<avSamp; j++) {
    sixDOF.getValues(values);

    roll = roll + values[3];     //Sum of Yaw rotations
    pitch = pitch + values[4]; //Sum of Pitch rotations
    yaw = yaw + values[5];   //Sum of Roll rotations
  }
 
  yaw = yaw/avSamp;      //Average of Yaw Rotation
  pitch = pitch/avSamp;  //Average of Pitch Rotation  
  roll = roll/avSamp;    //Average of Roll Rotation

/// Gyro Stuff ///   
  if(rotWait == 0){
    if(abs(roll) > abs(pitch) && abs(roll) > abs(yaw)){ //Roll
      if(abs(roll) > threshRot){
        Serial.println(50);
        rotWait = rotWaitVal;
      }
    }
    else if(abs(pitch) > abs(yaw) && abs(pitch) > abs(roll)){ //Pitch
      if(abs(pitch) > threshRot){
        Serial.println(60);
        rotWait = rotWaitVal;
      }
    }
  }
  else{
    rotWait = rotWait - 1; 
   }
   
  pitch = 0;
  roll = 0;
  yaw = 0;
    
/// Finger Stuff ///
 // Finger 1 
  if(fingState[0]) { // If finger 1 is ON
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
 
} // End loop()
