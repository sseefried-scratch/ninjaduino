#include <DHT22.h>
#include <I2C.h>
#include <Wire.h>
#include <Sensor.h>
#include <aJSON.h>
#include <stdio.h>
#include <avr/sleep.h>

#define DHT22_PIN 15



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

const byte REG_STATUS = 0x00; //(R) Real time status
const byte REG_OUT_X_MSB = 0x01; //(R) [7:0] are 8 MSBs of 10-bit sample
const byte REG_OUT_X_LSB = 0x02; //(R) [7:6] are 2 LSBs of 10-bit sample
const byte REG_OUT_Y_MSB = 0x03; //(R) [7:0] are 8 MSBs of 10-bit sample
const byte REG_OUT_Y_LSB = 0x04; //(R) [7:6] are 2 LSBs of 10-bit sample
const byte REG_OUT_Z_MSB = 0x05; //(R) [7:0] are 8 MSBs of 10-bit sample
const byte REG_OUT_Z_LSB = 0x06; //(R) [7:6] are 2 LSBs of 10-bit sample
const byte REG_SYSMOD = 0x0b; //(R) Current system mode
const byte REG_INT_SOURCE = 0x0c; //(R) Interrupt status
const byte REG_WHO_AM_I = 0x0d; //(R) Device ID (0x3A)
const byte REG_XYZ_DATA_CFG = 0xe; //(R/W) Dynamic range settings
const byte REG_HP_FILTER_CUTOFF = 0x0f; //(R/W) cut-off frequency is set to 16Hz @ 800Hz
const byte REG_PL_STATUS = 0x10; //(R) Landscape/Portrait orientation status
const byte REG_PL_CFG = 0x11; //(R/W) Landscape/Portrait configuration
const byte REG_PL_COUNT = 0x12; //(R) Landscape/Portrait debounce counter
const byte REG_PL_BF_ZCOMP = 0x13; //(R) Back-Front, Z-Lock trip threshold
const byte REG_P_L_THS_REG = 0x14; //(R/W) Portrait to Landscape trip angle is 29 degree
const byte REG_FF_MT_CFG = 0x15; //(R/W) Freefall/motion functional block configuration
const byte REG_FF_MT_SRC = 0x16; //(R) Freefall/motion event source register
const byte REG_FF_MT_THS = 0x17; //(R/W) Freefall/motion threshold register
const byte REG_FF_MT_COUNT = 0x18; //(R/W) Freefall/motion debounce counter
const byte REG_TRANSIENT_CFG = 0x1d; //(R/W) Transient functional block configuration
const byte REG_TRANSIENT_SRC = 0x1e; //(R) Transient event status register
const byte REG_TRANSIENT_THS = 0x1f; //(R/W) Transient event threshold
const byte REG_TRANSIENT_COUNT = 0x20; //(R/W) Transient debounce counter
const byte REG_PULSE_CFG = 0x21; //(R/W) ELE, Double_XYZ or Single_XYZ
const byte REG_PULSE_SRC = 0x22; //(R) EA, Double_XYZ or Single_XYZ
const byte REG_PULSE_THSX = 0x23; //(R/W) X pulse threshold
const byte REG_PULSE_THSY = 0x24; //(R/W) Y pulse threshold
const byte REG_PULSE_THSZ = 0x25; //(R/W) Z pulse thresholdf
const byte REG_PULSE_TMLT = 0x26; //(R/W) Time limit for pulse
const byte REG_PULSE_LTCY = 0x27; //(R/W) Latency time for 2nd pulse
const byte REG_PULSE_WIND = 0x28; //(R/W) Window time for 2nd pulse
const byte REG_ASLP_COUNT = 0x29; //(R/W) Counter setting for auto-sleep
const byte REG_CTRL_REG1 = 0x2a; //(R/W) ODR = 800 Hz, STANDBY mode
const byte REG_CTRL_REG2 = 0x2b; //(R/W) Sleep enable, OS Modes, RST, ST
const byte REG_CTRL_REG3 = 0x2c; //(R/W) Wake from sleep, IPOL, PP_OD
const byte REG_CTRL_REG4 = 0x2d; //(R/W) Interrupt enable register
const byte REG_CTRL_REG5 = 0x2e; //(R/W) Interrupt pin (INT1/INT2) map
const byte REG_OFF_X = 0x2f; //(R/W) X-axis offset adjust
const byte REG_OFF_Y = 0x30; //(R/W) Y-axis offset adjust
const byte REG_OFF_Z = 0x31; //(R/W) Z-axis offset adjust
 
const byte FULL_SCALE_RANGE_2g = 0x0;
const byte FULL_SCALE_RANGE_4g = 0x1;
const byte FULL_SCALE_RANGE_8g = 0x2;
 
const byte I2C_ADDR = 0x1D; //SA0=0
 

const prog_char PROGMEM RETRIEVING_NAME[] ="Retrieving name\n";
const prog_char PROGMEM ERROR_RETRIEVING_NAME[] ="Error retrieving name\n";
const prog_char PROGMEM SUCCESSFULLY_RETRIEVED_NAME[] ="Successfully retrieved Name:";
const prog_char PROGMEM PARSING_OBJECT[] ="Parsing String\n";
const prog_char PROGMEM ERROR_PARSING_OBJECT[] ="Error parsing Object\n";
const prog_char PROGMEM SUCCESSFULLY_PARSED_OBJECT[] ="Successfully parsed Object\n";
const prog_char PROGMEM DELETING_OBJECT_STRING[] = "Deleting the object\n";
const prog_char PROGMEM FORMAT_FAILED_STRING[] = "Failed to create Format Object\n";
const prog_char PROGMEM OUTPUT_STRING_ERROR[] = "Error creating output String\n";
const prog_char PROGMEM RESULT_PRINTING_STRING[] = "Printing the result:\n";
const prog_char PROGMEM ADDING_FRAMERATE_STRING[] = "Adding frame rate to the format\n";
const prog_char PROGMEM ADDING_INTERLACE_STRING[] = "Adding interlace to the format\n";
const prog_char PROGMEM ADDING_HEIGHT_STRING[] = "Adding height to the format\n";
const prog_char PROGMEM ADDING_WIDTH_STRING[] = "Adding width to the format\n";
const prog_char PROGMEM ADDING_TYPE_STRING[] = "Adding type to the format\n";
const prog_char PROGMEM ADDING_FORMAT_STRING[] = "Adding format to the object\n";
const prog_char PROGMEM ADDING_LENGTH_STRING[] = "Adding length to the object\n";
const prog_char PROGMEM CREATING_FROMAT_STRING[] = "Creating format object\n";
const prog_char PROGMEM ADDING_NAME_STRING[] = "Adding name to the object\n";
const prog_char PROGMEM OBJECT_CREATION_FAILED_STRING[] = "Failed to create the object\n";
const prog_char PROGMEM OBJECT_CREATE_STRING[] = "Created a Object\n";
const prog_char PROGMEM HELLO_STRING[] = "********************\nTesting aJson\n*****************\n";

/*
  Read register content into buffer.
  The default count is 1 byte.
 
  The buffer needs to be pre-allocated
  if count > 1
*/
void regRead(byte reg, byte *buf, byte count = 1)
{
  I2c.write(I2C_ADDR, reg);
  I2c.read(I2C_ADDR, reg, count);
 
  for (int i = 0; i < count; i++)
    *(buf+i) = I2c.receive();
}
 
/*
  Write a byte value into a register
*/
void regWrite(byte reg, byte val)
{
  I2c.write(I2C_ADDR, reg, val);
}
 
/*
  Put MMA8453Q into standby mode
*/
void standbyMode()
{
  byte reg;
  byte activeMask = 0x01;
 
  regRead(REG_CTRL_REG1, &reg);
  regWrite(REG_CTRL_REG1, reg & ~activeMask);
}
 
/*
  Put MMA8453Q into active mode
*/
void activeMode()
{
  byte reg;
  byte activeMask = 0x01;
 
  regRead(REG_CTRL_REG1, &reg);
  regWrite(REG_CTRL_REG1, reg | activeMask);
}
 
/*
  Use fast mode (low resolution mode)
  The acceleration readings will be
  limited to 8 bits in this mode.
*/
void lowResMode()
{
  byte reg;
  byte fastModeMask = 0x02;
 
  regRead(REG_CTRL_REG1, &reg);
  regWrite(REG_CTRL_REG1, reg | fastModeMask);
}
 
/*
  Use default mode (high resolution mode)
  The acceleration readings will be
  10 bits in this mode.
*/
void hiResMode()
{
  byte reg;
  byte fastModeMask = 0x02;
 
  regRead(REG_CTRL_REG1, &reg);
  regWrite(REG_CTRL_REG1,  reg & ~fastModeMask);
}
 
/*
  Get accelerometer readings (x, y, z)
  by default, standard 10 bits mode is used.
 
  This function also convers 2's complement number to
  signed integer result.
 
  If accelerometer is initialized to use low res mode,
  isHighRes must be passed in as false.
*/
void getAccXYZ(int *x, int *y, int *z, bool isHighRes=true)
{
  byte buf[6];
 
  if (isHighRes) {
    regRead(REG_OUT_X_MSB, buf, 6);
    *x = buf[0] << 2 | buf[1] >> 6 & 0x3;
    *y = buf[2] << 2 | buf[3] >> 6 & 0x3;
    *z = buf[4] << 2 | buf[5] >> 6 & 0x3;
  }
  else {
    regRead(REG_OUT_X_MSB, buf, 3);
    *x = buf[0] << 2;
    *y = buf[1] << 2;
    *z = buf[2] << 2;
  }
 
  if (*x > 511) *x = *x - 1024;
  if (*y > 511) *y = *y - 1024 ;
  if (*z > 511) *z = *z - 1024;
}
 
Sensor sen;

// Setup a DHT22 instance
DHT22 myDHT22(DHT22_PIN);

void setup()
{
  I2c.begin();
  Serial.begin(9600);
  Serial.println("beginning...");
  standbyMode(); //register settings must be made in standby mode
  regWrite(REG_XYZ_DATA_CFG, FULL_SCALE_RANGE_2g);
  hiResMode(); //this is the default setting and can be omitted.
  //lowResMode(); //set to low res (fast mode), must use getAccXYZ(,,,false) to retrieve readings.
  activeMode();
 
  byte b;
  regRead(REG_WHO_AM_I, &b);
  //Serial.print("WHO_AM_I=");
  ////Serial.println(b,HEX);
}




int x = 0, y = 0, z = 0;
void testObjects() {

  aJsonObject* root = aJson.createObject();
  if (root != NULL) {
    //printProgStr( OBJECT_CREATE_STRING);
  } 
  else {
    //printProgStr( OBJECT_CREATION_FAILED_STRING);
    return;
  }
  //printProgStr( ADDING_NAME_STRING);
  //aJson.addItemToObject(root, "temperature", aJson.createItem(sen.getTemperature()));
  aJson.addNumberToObject(root, "temperature", sen.getTemperature());
  //printProgStr( CREATING_FROMAT_STRING);
  aJsonObject* accelerometer = aJson.createObject();
  if (accelerometer != NULL){
	int x = 0, y = 0, z = 0;

	getAccXYZ(&x, &y, &z); //get accelerometer readings in normal mode (hi res).
	aJson.addItemToObject(root, "accelerometer", accelerometer);
	aJson.addNumberToObject(accelerometer, "x", x);
	aJson.addNumberToObject(accelerometer, "y", y);
	aJson.addNumberToObject(accelerometer, "z", z);
  }
 else {
   printProgStr( FORMAT_FAILED_STRING);
   return;
 }
 aJsonObject* ports = aJson.createObject();
if (ports != NULL){
	
	//aJson.createArray(root, "ports", ports);
	aJsonObject* ports = aJson.createArray();
	aJson.addItemToObject(root, "ports", ports);
	
	aJsonObject* port1 = aJson.createObject();
	aJsonObject* port2 = aJson.createObject();
	aJsonObject* port3 = aJson.createObject();
	
	aJson.addItemToArray(ports, port1);
	aJson.addItemToObject(port1, "port", aJson.createItem(1));
	aJson.addItemToObject(port1, "type", aJson.createItem(sen.idTheType(analogRead(IDPinCon1), false)));
	aJson.addItemToObject(port1, "value", aJson.createItem(sen.getSensorValue(1, sen.idTheType(analogRead(IDPinCon1), false))));
	aJson.addItemToArray(ports, port2);
	aJson.addItemToObject(port2, "port", aJson.createItem(2));
	aJson.addItemToObject(port2, "type", aJson.createItem(sen.idTheType(analogRead(IDPinCon2), false)));
	aJson.addItemToObject(port2, "value", aJson.createItem(sen.getSensorValue(2, sen.idTheType(analogRead(IDPinCon2), true))));
	aJson.addItemToArray(ports, port3);
	aJson.addItemToObject(port3, "port", aJson.createItem(3));
	aJson.addItemToObject(port3, "type", aJson.createItem(sen.idTheType(analogRead(IDPinCon3), false)));
	aJson.addItemToObject(port3, "value", aJson.createItem(sen.getSensorValue(3, sen.idTheType(analogRead(IDPinCon3), false))));
	

  }
 else {
   printProgStr( FORMAT_FAILED_STRING);
   return;
 }
  //if (fmt != NULL) {
  //  printProgStr( ADDING_FORMAT_STRING);
  //  aJson.addItemToObject(root, "format", fmt);
  //  printProgStr( ADDING_TYPE_STRING);
  //  aJson.addStringToObject(fmt, "type", "rect");
  //  printProgStr( ADDING_WIDTH_STRING);
  //  aJson.addNumberToObject(fmt, "width", 1920);
  //  printProgStr( ADDING_HEIGHT_STRING);
  //  aJson.addNumberToObject(fmt, "height", 1080);
  //  printProgStr( ADDING_INTERLACE_STRING);
  //  aJson.addFalseToObject(fmt, "interlace");
  //  printProgStr( ADDING_FRAMERATE_STRING);
  //  aJson.addNumberToObject(fmt, "frame rate", 24);
  //  printProgStr( ADDING_LENGTH_STRING);
  //  aJson.addNumberToObject(fmt, "length", 1.29);
  //} 
  //else {
  //  printProgStr( FORMAT_FAILED_STRING);
  //  return;
  //}

  freeMem("with object");
  //printProgStr( RESULT_PRINTING_STRING);
  char* string = aJson.print(root);
  if (string != NULL) {
    Serial.println(string);
  } 
  else {
    //printProgStr( OUTPUT_STRING_ERROR);
  }

  //printProgStr( DELETING_OBJECT_STRING);
  aJson.deleteItem(ports);

  aJson.deleteItem(root);
  free(string);
  //freeMem("after deletion");
}

void loop()
{

 
  
	//aJson.addItemToObject(root, "temperature", fmt = aJson.createObject());
	//aJson.addStringToObject(fmt,"type",		"rect");
	//aJson.addNumberToObject(fmt,"width",		1920);
	//aJson.addNumberToObject(fmt,"height",		1080);
	//aJson.addFalseToObject (fmt,"interlace");
	//aJson.addNumberToObject(fmt,"frame rate",	24);
	////Serial.println(aJson.print(root));
	//aJson.deleteItem(root);
	////freeMem("the memory: ");
	delay(10);
	
  testObjects();


 //Serial.print("{\"i2c\":[{\"accelerometer\":{\"x\":");
 //Serial.print(x);
 //Serial.print(",\"y\":");
 //Serial.print(y);
 //Serial.print(",\"z\":");
 //Serial.print(z);
 //Serial.print("}},{\"temperature\":");
 
 //Serial.print(sen.getTemperature());
 ////Serial.println("0}]}");

 //char* sensorType = sen.idTheType(analogRead(IDPinCon1));
 //   int senval1 = sen.getSensorValue(1, sensorType);
 //   Serial.print("-------------->");
 //   //Serial.println(senval1);
 //DHT22_ERROR_t errorCode;
 //
 // // The sensor can only be read from every 1-2s, and requires a minimum
 // // 2s warm-up after power-on.
 // delay(2000);
 //
 // Serial.print("Requesting data...");
 // errorCode = myDHT22.readData();
 // switch(errorCode)
 // {
 //   case DHT_ERROR_NONE:
 //     Serial.print("Got Data ");
 //     Serial.print(myDHT22.getTemperatureC());
 //     Serial.print("C ");
 //     Serial.print(myDHT22.getHumidity());
 //     Serial.println("%");
 //     // Alternately, with integer formatting which is clumsier but more compact to store and
 //     // can be compared reliably for equality:
 //     //	  
 //     char buf[128];
 //     sprintf(buf, "Integer-only reading: Temperature %hi.%01hi C, Humidity %i.%01i %% RH",
 //                  myDHT22.getTemperatureCInt()/10, abs(myDHT22.getTemperatureCInt()%10),
 //                  myDHT22.getHumidityInt()/10, myDHT22.getHumidityInt()%10);
 //     Serial.println(buf);
 //     break;
 //   case DHT_ERROR_CHECKSUM:
 //     Serial.print("check sum error ");
 //     Serial.print(myDHT22.getTemperatureC());
 //     Serial.print("C ");
 //     Serial.print(myDHT22.getHumidity());
 //     Serial.println("%");
 //     break;
 //   case DHT_BUS_HUNG:
 //     Serial.println("BUS Hung ");
 //     break;
 //   case DHT_ERROR_NOT_PRESENT:
 //     Serial.println("Not Present ");
 //     break;
 //   case DHT_ERROR_ACK_TOO_LONG:
 //     Serial.println("ACK time out ");
 //     break;
 //   case DHT_ERROR_SYNC_TIMEOUT:
 //     Serial.println("Sync Timeout ");
 //     break;
 //   case DHT_ERROR_DATA_TIMEOUT:
 //     Serial.println("Data Timeout ");
 //     break;
 //   case DHT_ERROR_TOOQUICK:
 //     Serial.println("Polled to quick ");
 //     break;
 // }
}

//Code to print out the free memory

struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

extern char * const __brkval;
extern struct __freelist *__flp;

uint16_t freeMem(uint16_t *biggest)
{
  char *brkval;
  char *cp;
  unsigned freeSpace;
  struct __freelist *fp1, *fp2;

  brkval = __brkval;
  if (brkval == 0) {
    brkval = __malloc_heap_start;
  }
  cp = __malloc_heap_end;
  if (cp == 0) {
    cp = ((char *)AVR_STACK_POINTER_REG) - __malloc_margin;
  }
  if (cp <= brkval) return 0;

  freeSpace = cp - brkval;

  for (*biggest = 0, fp1 = __flp, fp2 = 0;
     fp1;
     fp2 = fp1, fp1 = fp1->nx) {
      if (fp1->sz > *biggest) *biggest = fp1->sz;
    freeSpace += fp1->sz;
  }

  return freeSpace;
}

uint16_t biggest;

void freeMem(char* message) {
  Serial.print(message);
  Serial.print(":\t");
  Serial.println(freeMem(&biggest));
}

// given a PROGMEM string, use Serial.print() to send it out
// this is needed to save precious memory
//htanks to todbot for this http://todbot.com/blog/category/programming/
void printProgStr(const prog_char* str) {
  char c;
  if (!str) {
    return;
  }
  while ((c = pgm_read_byte(str))) {
    Serial.print(c, byte(0));
    str++;
  }
}
