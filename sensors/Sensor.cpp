/*
  Sensor.cpp - Library for flashing Sensor code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/

#include <Arduino.h>
#include "Sensor.h"
#include <Wire.h>

Sensor::Sensor()
{
  //pinMode(pin, OUTPUT);
  //_pin = pin;
}

char* Sensor::idTheType(int sensorValue)
{
	char* sensorType;
	if (sensorValue > 175 && sensorValue < 190) { // Distance
		sensorType = "DISTANCE";
		return sensorType;
	} else if (sensorValue > 505 && sensorValue < 516) { // Humidity
		sensorType = "HUMIDITY";
		return sensorType;
	} else if (sensorValue > 248 && sensorValue < 258) { // PIR
		sensorType = "PIR";
		return sensorType;
	} else if (sensorValue > 362 && sensorValue < 371) { // Light
		sensorType = "LIGHT";
		return sensorType;
	} else if (sensorValue > 780 && sensorValue < 790) { // Button
		sensorType = "BUTTON";
		return sensorType;
	} else {
		sensorType = "UNKNOWN";
		return sensorType;
	}
}


int Sensor::getSensorValue(int port, char* type)
{
	int aInPin;
	
	if(port == 1){
		aInPin = A0;
	}
	else if (port == 2)
	{
		aInPin = A1;
		
	}
	else if (port == 3)
	{
		aInPin = A2;
	}
	else{
		Serial.println("--> ERROR");
		Serial.print("--> Attempting to assign port: ");
		Serial.println(port);
		Serial.println("--> Port can only be 1, 2 or 3");
		//break;	
	}
	
	if(type == "LIGHT"){
		int sensorValue = analogRead(aInPin);
		return sensorValue;
	}
	else{
		//int sensorValue = analogRead(aInPin);
		return 0;
	}
}


float Sensor::getTemperature()
{
  Wire.begin();
  Wire.requestFrom(0x48,2); 
  byte MSB = Wire.read();
  byte LSB = Wire.read();

  int TemperatureSum = ((MSB << 8) | LSB) >> 4; //it's a 12bit int, using two's compliment for negative
  float celsius = TemperatureSum*0.0625;

  return celsius;
}

void Sensor::dot()
{
	Serial.println("dot");
  //digitalWrite(_pin, HIGH);
  //delay(250);
  //digitalWrite(_pin, LOW);
  //delay(250);  
}

void Sensor::dash()
{
	Serial.println("dash");  
	//digitalWrite(_pin, HIGH);
  //delay(1000);
  //digitalWrite(_pin, LOW);
  //delay(250);
}
