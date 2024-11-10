#include <Arduino.h>
#include "Wire.h"
#include <stdio.h>

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
int16_t x_accel [2], y_accel [2], z_accel[2];

// Declaring the digital GPIO pins that we will communicate for the LEDs
size_t red_lef = 10;
size_t yellow_led = 9;
size_t green_lef = 8;

// Declaring the digital GPIO pins that we will communicate for the relay switch
size_t relay_switch = 7;

// Takes the reading from that is stored in the 16-bit integer and converts it into a string (array of chars) 
char temp_str[7];
char * convert_int16_to_str(int16_t i) {
  sprintf(temp_str, "%6d", i);
  return temp_str;
}

void setup () {
  Serial.begin(9600);

  Wire.begin();
  Wire.beginTransmission(MPU_neck_address); // Begins the I2C slave
  Wire.beginTransmission(MPU_spine_address); // Begins the I2C slave
  Wire.write(0x6B); // configures the chip in both the MPU to not to be in sleep mode
  Wire.write(0x00); // set to zero; wakes up both the MPU
  Wire.endTransmission(true);
}

void loop () {
  // DATA FROM THE SPIN MPUE
  Wire.beginTransmission(MPU_spine_address);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_spine_address, 6, true); // requesting a total of 6 registers 

  x_accel[1] = Wire.read()<<8 | Wire.read();
  y_accel[1] = Wire.read()<<8 | Wire.read();
  z_accel[1] = Wire.read()<<8 | Wire.read();

  // Printing the spine
  Serial.print("spineX = ");
  Serial.print(convert_int16_to_str(x_accel[1]));
  Serial.print(" | spineY = ");
  Serial.print(convert_int16_to_str(y_accel[1]));
  Serial.print(" | spineZ = ");
  Serial.print(convert_int16_to_str(z_accel[1]));


  // DATA FROM THE NECK MPU
  Wire.beginTransmission(MPU_neck_address);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_neck_address, 6, true); // requesting a total of 6 registers

  x_accel[0] = Wire.read()<<8 | Wire.read();
  y_accel[0] = Wire.read()<<8 | Wire.read();
  z_accel[0] = Wire.read()<<8 | Wire.read();

  // Printing the neck 
  Serial.print("\t\tneckX = ");
  Serial.print(convert_int16_to_str(x_accel[0]));
  Serial.print(" | neckY = ");
  Serial.print(convert_int16_to_str(y_accel[0]));
  Serial.print(" | neckZ = ");
  Serial.print(convert_int16_to_str(z_accel[0]));


  Serial.println();
}