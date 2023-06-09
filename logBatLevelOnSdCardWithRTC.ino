#define DS1307_ADDRESS 0x68
// SPI       PINS
// MOSI       11
// MISO       12
// CLOCK      13
// CS         10
#define CS    7     // adjust this ChipSelect line if needed !

int currentTime[7];
int i = 0;
double bat_ADC;
double batLevel;

#include <Wire.h>
#include <SPI.h>
#include <SD.h>

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  while (!Serial); // wait for serial port to connect. Needed for native USB
  Serial.flush();
  
  // year (00-99) / month (1-12) / monthday (1-31) / weekday (1-7) / hour (0-23) /  minute (0-59) / second (always zero)
  //setDateTime(23, 6, 16, 6, 15, 45, 0);

  initSDcard();

  // wait for first measurement to be finished
  delay(5000);

  pinMode(A0, INPUT);
}

void loop()
{
  addValuesToSDCard();
  delay(10000);
}

// Sets date/time of the RTC module via the serial monitor
// year (00-99) / month (1-12) / monthday (1-31) / weekday (1-7) / hour (0-23) /  minute (0-59) / second (always zero)

void setDateTime(byte year, byte month, byte monthday, byte weekday, byte hour, byte minute, byte second)
{
  second = 0;

  // The following codes transmits the data to the RTC
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(byte(0));
  Wire.write(decToBcd(byte(second)));
  Wire.write(decToBcd(byte(minute)));
  Wire.write(decToBcd(byte(hour)));
  Wire.write(decToBcd(byte(hour)));
  Wire.write(decToBcd(byte(monthday)));
  Wire.write(decToBcd(byte(month)));
  Wire.write(decToBcd(byte(year)));
  Wire.write(byte(0));
  Wire.endTransmission();
}

// Prints the current date/time set in the RTC module to the serial monitor

void printDateTime(int nowTime[7])
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);

  while(Wire.available()){
    nowTime[0] = bcdToDec(Wire.read());              // seconds
    nowTime[1] = bcdToDec(Wire.read());              // minute
    nowTime[2] = bcdToDec(Wire.read() & 0b111111);   // hour
    nowTime[3] = bcdToDec(Wire.read());              // weekday
    nowTime[4] = bcdToDec(Wire.read());              // monthday
    nowTime[5] = bcdToDec(Wire.read());              // month
    nowTime[6] = bcdToDec(Wire.read());              // year
  }
}

// Converts a decimal (Base-10) integer to BCD (Binary-coded decimal)
int decToBcd(int value)
{
  return ((value/10*16) + (value%10));
}

// Converts a BCD (Binary-coded decimal) to decimal (Base-10) integer
int bcdToDec(int value)
{
  return ((value/16*10) + (value%16));
}

void initSDcard(){
  // initialize the SD card
  if (!SD.begin(CS))
  {
    Serial.println("init1 Error: SD card failure");
  }
  /*
  // remove file for proper timing
  SD.remove("data.xml");
  delay(1000);
  */

  File logfile = SD.open("data.txt", FILE_WRITE);
  if (!logfile)
  {
    Serial.println("init2Error: SD card failure");
  }

  logfile.println("hour minute  day month year  temp  humi  co  co2 batLevel");

  logfile.close();

  Serial.println("Setup done done...");
}

void addValuesToSDCard(){
  if (!SD.begin(CS))
  {
    Serial.println("addVal Error: SD card failure");
  }

  File logfile = SD.open("data.txt", FILE_WRITE);
  if (!logfile)
  {
    Serial.println("addVal2 Error: SD card failure");
  }

  printDateTime(currentTime);

  Serial.print(currentTime[2]);
  Serial.print(" - ");
  Serial.print(currentTime[1]);
  Serial.print(" - ");
  Serial.print(currentTime[0]);

  Serial.print(" \t ");
  Serial.print(currentTime[4]);
  Serial.print(" / ");
  Serial.print(currentTime[5]);
  Serial.print(" / ");
  Serial.println(currentTime[6]);
  

  char buffer[200];
  // hour minute  day month year  temp  humi  co  co2 batLevel

  bat_ADC = 0;
  batLevel = 0;

  for(int z = 1; z <= 200; z++){
    bat_ADC += analogRead(A0);
  }

  bat_ADC = bat_ADC / 200;
  batLevel = bat_ADC * 5.0 / 1023;
  
  sprintf(buffer, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t", i, currentTime[2], currentTime[1], currentTime[4], currentTime[5], currentTime[6], bat_ADC, bat_ADC, bat_ADC, bat_ADC);
  
  //Serial.println(buffer);

  logfile.print(buffer);

  logfile.println(batLevel);

  Serial.println( batLevel);

  logfile.close();
  i++;
}
