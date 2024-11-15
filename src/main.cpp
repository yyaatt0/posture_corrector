#include <Arduino.h>
#include "Wire.h"
#include <stdio.h>

#define WINDOWSIZE 5

// Index 0 is for ROLL
// Index 1 is for PITCH
float bad_neck_posture_th [2]; 
float good_neck_posutre_th [2];

// Index 0 is for ROLL
// Index 1 is for PITCH

float bad_spine_posture_th [2] = {44.0, 55.0}; 
float good_spine_posutre_th [2] = {55.0, 30.0};

using namespace std;

// Hexadecimal adresses for I2C protocal
const int MPU_spine_address = 0x69; // Logic High; (110 1001)
const int MPU_neck_address = 0x68; // Logic Low; (110 1000) <-- default address 

/*
  16 bit integer to store the reading of the the accelerometer from the two MPU 6050
  Index 1 from the arrays will refer to the reading from the SPINE MPU
  Index 0 from the arrays will refer to the reading from the NECK MPU
*/

/*
  ACCEL_XOUT_H: 0x3B ; upper 2-bytes
  ACCEL_XOUT_L: 0x3C ; lower 2-bytes

  ACCEL_YOUT_H: 0x3D
  ACCEL_YOUT_L: 0x3E

  ACCEL_ZOUT_H: 0x3F
  ACCEL_ZOUT_L: 0x40

  Each accelerometer output for x, y, z are 16-bit 2's Complement binary number 
*/
float x_accel [2], y_accel [2], z_accel[2];
float x_accel_error[2], y_accel_error [2];

// Acting as our sliding window to store a set of data 
float roll_neck [WINDOWSIZE], pitch_neck [WINDOWSIZE];
float roll_spine [WINDOWSIZE], pitch_spine[WINDOWSIZE];
int8_t index; // Keep track where we are in the index in the array 

// Declaring the digital GPIO pins that we will communicate for the relay switch
size_t relay_switch = 7;

// FUNCTION DECLARATION 
char * convert_int16_to_str(int16_t i);
void calculate_MPU_error(const int & mpu_address, const size_t index);
void read_profile();


void setup () {
  Serial.begin(9600);

  pinMode(relay_switch, OUTPUT);
  digitalWrite(relay_switch, LOW);

  // Initialize the array to have some dummy value so when the loop function runs, there is some time for the array to fill up before anything happens 
  for(int i = 0; i < WINDOWSIZE; ++i)
  {
    roll_neck[i] = 17000.0;
    pitch_neck[i] = 17000.0;
    roll_spine[i] = 17000.0;
    pitch_spine[i] = 17000.0;
  }

  // Reading profile 

  Wire.begin();
  Wire.beginTransmission(MPU_neck_address); // Begins the I2C slave
  Wire.write(0x6B); // configures the chip in the neck MPU to not to be in sleep mode
  Wire.write(0x00); // set to zero; wakes up the MPU
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_spine_address); // Begins the I2C slave
  Wire.write(0x6B); // configures the chip in the sping MPU to not to be in sleep mode
  Wire.write(0x10); // set to zero; wakes up the MPU
  Wire.endTransmission(true);

  // // CALCULATING THE ERROR FROM THE MPUs
  // calculate_MPU_error(MPU_neck_address, 0);
  // calculate_MPU_error(MPU_spine_address, 1);

  // Serial.println("ERROR MPU MEASUREMENT");
  // Serial.print("spineErrorX = ");
  // Serial.print(x_accel_error[1]);
  // Serial.print(" | spineErrorY : ");
  // Serial.print(y_accel_error[1]);

  // Serial.print("\t\tneckErrorX = ");
  // Serial.print(x_accel_error[0]);
  // Serial.print(" | neckErrorY: ");
  // Serial.print(y_accel_error[0]);
  // Serial.println();
}

void loop () {
  // DATA FROM THE SPIN MPU
  Wire.beginTransmission(MPU_spine_address);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_spine_address, 6, true); // requesting a total of 6 registers 

  // READS RAW DATA 
  x_accel[1] = Wire.read()<<8 | Wire.read();
  y_accel[1] = Wire.read()<<8 | Wire.read();
  z_accel[1] = Wire.read()<<8 | Wire.read();

  // PPRINTING RAW SPINE DATA
  // Serial.print("spineX = ");
  // Serial.print(convert_int16_to_str(x_accel[1]));
  // Serial.print(" | spineY = ");
  // Serial.print(convert_int16_to_str(y_accel[1]));
  // Serial.print(" | spineZ = ");
  // Serial.print(convert_int16_to_str(z_accel[1]));

  roll_spine[index] = (atan(y_accel[1] / sqrt(pow(x_accel[1], 2) + pow(z_accel[1], 2))) * 180) / PI; 
  pitch_spine[index] = (atan((-1 * x_accel[1]) / sqrt(pow(y_accel[1], 2) + pow(z_accel[1], 2))) * 180) / PI;

  Serial.print("spineRoll : ");
  Serial.print(roll_spine[index]);
  Serial.print(" | spinePitch : ");
  Serial.print(pitch_spine[index]);

  // DATA FROM THE NECK MPU
  Wire.beginTransmission(MPU_neck_address);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_neck_address, 6, true); // requesting a total of 6 registers

  // READS RAW DATA
  x_accel[0] = Wire.read()<<8 | Wire.read();
  y_accel[0] = Wire.read()<<8 | Wire.read();
  z_accel[0] = Wire.read()<<8 | Wire.read();

  // PRINTING RAW NECK DATA
  // Serial.print("\t\tneckX = ");
  // Serial.print(convert_int16_to_str(x_accel[0]));
  // Serial.print(" | neckY = ");
  // Serial.print(convert_int16_to_str(y_accel[0]));
  // Serial.print(" | neckZ = ");
  // Serial.print(convert_int16_to_str(z_accel[0]));

  roll_neck[index] = (atan(y_accel[0] / sqrt(pow(x_accel[0], 2) + pow(z_accel[0], 2))) * 180) / PI; 
  pitch_neck[index] = (atan((-1 * x_accel[0]) / sqrt(pow(y_accel[0], 2) + pow(z_accel[0], 2))) * 180) / PI;
  
  Serial.print("\t\tneckRoll : ");
  Serial.print(roll_neck[index]);
  Serial.print(" | neckPitch : ");
  Serial.print(pitch_neck[index]);

  // This checks if the array has been filled up initially, this boolean only matters in like the first WINDOWSIZE iterations of the loop function
  if(pitch_neck[WINDOWSIZE - 1 ] != 17000.00) {

    float avg_roll_neck = 0, avg_pitch_neck = 0, avg_roll_spine = 0, avg_pitch_spine = 0;
    for(int i = 0; i < WINDOWSIZE; ++i) {
      avg_roll_neck += roll_neck[i];
      avg_pitch_neck += pitch_neck[i];
      avg_roll_spine += roll_spine[i];
      avg_pitch_spine += pitch_spine[i];
    }

    avg_roll_neck /= WINDOWSIZE;
    avg_pitch_neck /= WINDOWSIZE;
    avg_roll_spine /= WINDOWSIZE;
    avg_pitch_spine /= WINDOWSIZE;

    // Compares the threshold
    if((avg_pitch_spine >= good_spine_posutre_th[1])) {   // Also include if the neck posture is also in good threshold (AND STATEMENT); for now it's using the pitch
      Serial.println("GOOD Posture");
      digitalWrite(relay_switch, LOW);

    }
    else if (avg_pitch_spine < good_spine_posutre_th[1] && avg_pitch_spine > bad_neck_posture_th[1]) // Also include if the neck posture is in okay threshold; statement is OR between neck and spine for the yellow light to flash
    {
      Serial.println("OKAY Posture");
      digitalWrite(relay_switch, LOW);
    }
    else if(avg_pitch_spine <= bad_spine_posture_th[1]) { // Also include if the neck posutre is also in bad threshold; statement is OR between neck and spine for the red light to flash
      Serial.println("BAD Posture ");
      
      // Turns on the other circuit that is controlled by the relay switch 
      digitalWrite(relay_switch, HIGH);
      delay(1000);
      digitalWrite(relay_switch, LOW);
    }
  }

  // Increments the index in the range from 0 to WINDOWSIZE - 1; does this in a loop
  index = (index + 1) % WINDOWSIZE;

  Serial.println();
}


// FUNCTION DEFINITION

// Takes the reading from that is stored in the 16-bit integer and converts it into a string (array of chars) 
char temp_str[7];
char * convert_int16_to_str(int16_t i) {
  sprintf(temp_str, "%6d", i);
  return temp_str;
}

// Parameters: Takes either of the spine or neck mpu address
// The refers to the where in the accel_error arrays
// Errors are small on the MPUs, so we can ignore them 
void calculate_MPU_error(const int & mpu_address, const size_t index) {
  size_t i = 0, n = 50;
  float x, y, z;

  while(i < n) {
    Wire.beginTransmission(mpu_address);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu_address, 6, true);
    
    // Using accel arrays to store some temp values
    x = (Wire.read()<<8 | Wire.read()) / 16384.0;
    y = (Wire.read()<<8 | Wire.read()) / 16384.0;
    z = (Wire.read()<<8 | Wire.read()) / 16384.0;

    x_accel_error[index] = x_accel_error[index] + atan(y / sqrt( pow(x, 2) + pow(z, 2)));
    y_accel_error[index] = y_accel_error[index] + atan((-1 * x) / sqrt(pow(y, 2) + pow(z, 2)));
    ++i;
  }  

  x_accel_error[index] /= n;
  y_accel_error[index] /= n;
}