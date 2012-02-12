/*
  Sensor.h - Library for flashing Sensor code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef Sensor_h
#define Sensor_h

#include "Arduino.h"
#include <Wire.h>

class Sensor
{
  public:
    Sensor();
	char* idTheType(int pin);
	int getSensorValue(int port, char* type);
	float getTemperature();
    void dot();
    void dash();
  private:
    int _pin;
};

#endif