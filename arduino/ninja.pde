int IDPinCon1 = A3;
int IDPinCon2 = A6;
int IDPinCon3 = A7;

int IDPinCon1Status = 0;
int IDPinCon2Status = 0;
int IDPinCon3Status = 0;
int connectionNumber = 0;
int red = 7;
int green = 8;
int blue = 9;

int valueID1 = 0;
int valueID2 = 0;
int valueID3 = 0;

int i[] = {0,0,0,0};



void setup() {
    // declare the ledPin as an OUTPUT:
    pinMode(red, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(blue, OUTPUT);

    digitalWrite(red, HIGH);
    digitalWrite(green, HIGH);
    digitalWrite(blue, HIGH);
    Serial.begin(9600);
}

void loop() {
    valueID1 = analogRead(IDPinCon1);
    valueID2 = analogRead(IDPinCon2);
    valueID3 = analogRead(IDPinCon3);

    //Serial.print("Connection 1: ");
    //Serial.println(valueID1);
    checkStatus(1, valueID1);
    checkStatus(2, valueID2);
    checkStatus(3, valueID3);

    //Serial.print("Connection 2: ");
    //Serial.println(valueID2);
    //Serial.print("Connection 3: ");
    //Serial.println(valueID3);
    delay(50);
}

int printStatus(int connectionNumber, char* sensor){

            Serial.print("Connection ");
            Serial.print(connectionNumber);
            Serial.print(": ");
            Serial.println(sensor);
            i[connectionNumber] = 3;
            
            Serial.println("{"accelerometer":{"x":0,"y":0,"z":0,"orientation":"up"},"temperature":0,"rgb":{"r":0,"g":0,"b":0},"ports":{"port1":{"accessory":"temperature sensor","value":0},"port2":{"accessory":"temperature sensor","value":0},"port3":{"accessory":"temperature sensor","value":0}}}"
}

int checkStatus(int connectionNumber, int sensorValue) {

  

  
    if (sensorValue > 175 && sensorValue < 190) { // Distance
        i[connectionNumber]++;
        if (i[connectionNumber] > 2) {
           printStatus(connectionNumber, "Distance Sensor");
        } else {
            Serial.print("Connection ");
            Serial.print(connectionNumber);
            Serial.println(": Unknown");
        }
    } else if (sensorValue > 505 && sensorValue < 516) { // Humidity
        i[connectionNumber]++;
        if (i[connectionNumber] > 2) {
             printStatus(connectionNumber, "Humidity Sensor");
        } else {
            Serial.print("Connection ");
            Serial.print(connectionNumber);
            Serial.println(": Unknown");
        }
    } else if (sensorValue > 248 && sensorValue < 258) { // PIR
        i[connectionNumber]++;
        if (i[connectionNumber] > 2) {
           printStatus(connectionNumber, "PIR Sensor");
        } else {
            Serial.print("Connection ");
            Serial.print(connectionNumber);
            Serial.println(": Unknown");
        }
    } else if (sensorValue > 362 && sensorValue < 371) { // Light
        i[connectionNumber]++;
        if (i[connectionNumber] > 2) {
            printStatus(connectionNumber, "Light Sensor");
        } else {
            Serial.print("Connection ");
            Serial.print(connectionNumber);

            Serial.println(": Unknown");
        }

    } else if (sensorValue > 780 && sensorValue < 790) { // Button
        i[connectionNumber]++;
        if (i[connectionNumber] > 2) {
          printStatus(connectionNumber, "Button Sensor");
        } else {
            Serial.print("Connection ");
            Serial.print(connectionNumber);
            Serial.println(": Unknown");
        }

    } else {
        i[connectionNumber] = 0;
        Serial.print("Connection: ");
        Serial.print(connectionNumber);
        Serial.println(": Unknown");
    }

    delay(100);


}


