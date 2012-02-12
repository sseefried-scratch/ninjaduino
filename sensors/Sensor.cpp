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
	if (sensorValue > 177 && sensorValue < 190) { // Distance (ideal )
		sensorType = "DISTANCE";
		return sensorType;
	} else if (sensorValue > 496 && sensorValue < 527) { // Humidity
		sensorType = "HUMIDITY";
		return sensorType;
	} else if (sensorValue > 246 && sensorValue < 261) { // PIR
		sensorType = "PIR";
		return sensorType;
	} else if (sensorValue > 355 && sensorValue < 378) { // Light
		sensorType = "LIGHT";
		return sensorType;
	} else if (sensorValue > 761 && sensorValue < 808) { // Button
		sensorType = "BUTTON";
		return sensorType;
	} else {
		sensorType = "UNKNOWN";
		Serial.print("bad reading: ");
		Serial.println(sensorValue);
		
		return sensorType;
	}
}


int Sensor::getSensorValue(int port, char* type)
{
	int aInPin;
	int dInPin;
	
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
	
	if(port == 1){
		dInPin = 14;
	}
	else if (port == 2)
	{
		dInPin = 15;
		
	}
	else if (port == 3)
	{
		dInPin = 16;
	}
	
	
	else{
		//Serial.println("--> ERROR");
		Serial.print("--> Attempting to assign port: ");
		//Serial.println(port);
		//Serial.println("--> Port can only be 1, 2 or 3");
		//break;	
	}
	
	if(type == "LIGHT"){
		int sensorValue = analogRead(aInPin);
		return sensorValue;
	}
	else if(type == "BUTTON"){
		int sensorValue = analogRead(aInPin);
		return sensorValue;
	}
	else if(type == "DISTANCE"){
		int sensorValue = analogRead(aInPin);
		return sensorValue;
	}
	else if(type == "HUMIDITY"){
		 
		return 0;
	
		
	}
	else if(type == "BUTTON"){
		pinMode(dInPin, INPUT);
		int sensorValue = digitalRead(dInPin);
		return sensorValue;
	}
	else{
		//int sensorValue = analogRead(aInPin);
		//Serial.print("returning 0 because type is ->");
		//Serial.print(type);
		//Serial.println("<-");
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
	//Serial.println("dot");
  //digitalWrite(_pin, HIGH);
  //delay(250);
  //digitalWrite(_pin, LOW);
  //delay(250);  
}

void Sensor::dash()
{
	//Serial.println("dash");  
	//digitalWrite(_pin, HIGH);
  //delay(1000);
  //digitalWrite(_pin, LOW);
  //delay(250);
}
