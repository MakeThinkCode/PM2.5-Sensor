 //******************************
 //*Abstract: Read value of PM1,PM2.5 and PM10 of air quality
 //
 //*Version：V3.1
 //*Author：Zuyang @ HUST
 //*Modified by Cain for Arduino Hardware Serial port compatibility
 //*Date：March.25.2016
 //* further modified by Bennett Battaile,  Philip Orlando, Meenakshi Rao
 //*  for real-time clock, SD card saving, using Teensy board. 7/2017
 //******************************
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>



String VERSIONSTRING = "plantower script version 14";


#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];
int bufIndex = -1;
const int chipSelect = 10;
  
// plantower code adapted from
//  https://www.dfrobot.com/wiki/index.php/PM2.5_laser_dust_sensor_SKU:SEN0177
int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module

int latestDataSet[12];


// from Blink example code:
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;

// use these directly as number of blinks and as status for log file
int blink_allfine = 1;
int blink_error2 = 2;
int blink_error3 = 3;
int blink_error4 = 4;
int blink_error5 = 5;
int blink_error6 = 6;
int maxnumblinks = 6;
int blinkstatus = blink_allfine;
unsigned long enteredthisblinkstatewhen;
unsigned long nextchangeblinkwhen;



void setup()
{
  // from Blink example code:
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);

  //initialize serial ports
  Serial.begin(9600);   //use serial0
  Serial.setTimeout(1500);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor

  //added the Serial1 lines -mr 2017-08-18
  Serial1.begin(9600);         //use serial0
  Serial1.setTimeout(1500);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor

  //digitalWrite(10,HIGH); // BB this was thought for a while to help with SD card initialization problems, but maybe not.  7/17

  if (Serial) Serial.println(VERSIONSTRING);

  initblinkstate();

  //lines through Serial.println("card initialized") from Teensy Datalogger example  BB 7/17
  //BB 7/17 if(Serial) in case usb isn't plugged in; if(Serial1) is always true per https://www.arduino.cc/en/Serial/IfSerial
  //to initialize the SD card, which is on the Teensy Audio board,
  //chipSelect should be set to 10
  if (Serial) Serial.print("Initializing SD card...");
  //UNCOMMENT THESE TWO LINES FOR TEENSY AUDIO BOARD:
  SPI.setMOSI(7);  // Audio shield has MOSI on pin 7
  SPI.setSCK(14);  // Audio shield has SCK on pin 14

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    if (Serial) Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  if (Serial) Serial.println("card initialized.");

  
  //dumpDataFile(); // handy to be able to see the file without pulling the card


  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(VERSIONSTRING + " starting up");
    dataFile.println("date-time, millis, status, PM1.0 ug/m3,  PM2.5 ug/m3,  PM10 ug/m3, ...");
    dataFile.close();
  }
}



void serialEvent1() {
    int b = 0;

    // when not already started a message, flush everything up to first 'B'
    while (bufIndex < 0 && (b = Serial1.read()) >= 0 && b != 'B');

    // if we got a start byte start a new message
    if ('B' == b) bufIndex = 0;

    // fill all available bytes into open message buffer
    while (0 <= bufIndex && bufIndex < sizeof(buf) && Serial1.available())
    {
        buf[bufIndex++] = Serial1.read();
    }

    if (bufIndex == sizeof(buf))
    {
        if(buf[0] == 0x4d && checkValue(buf,LENG))
        {
            fillLatestDataSet();
            //PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
            //PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
            //PM10Value=transmitPM10(buf); //count PM10 value of the air detector module
            //Serial.println("data updated"); // happens about 1x/sec
        }
        bufIndex = -1;  // mark as treated and ready for next message
    }
    //Serial.println("serialEvent1 called " + String(bufIndex) + " " + String(buf[0]));
}

void fillLatestDataSet()
{
    for ( int i = 0; i < 12; i++ ) 
    {
        int hi = buf[2*i+3];
        int lo = buf[2*i+4];
        latestDataSet[i] = (hi<<8) + lo;
    }
}


// 2-second activity cycle
void loop()
{
  String dataString = "";
  tmElements_t tm;
  static unsigned long OldTimer=millis();

  if ( bufIndex != -1 ) return;

  unsigned long timenow = millis();
  if ( timenow > nextchangeblinkwhen ) {
        transitionblink();
  }
  if ( timenow - OldTimer < 2000 ) {
    asm volatile("wfi"); // Magic from Paul.
    return;
  }
  OldTimer=timenow; 
  
  dataString =  " ";
  for ( int i = 0; i < 12; i++ )
  {
    dataString += String(latestDataSet[i]);
    if ( i+1 < 12 )
      dataString += ", ";
  }
    
  String dateTime = "0000-0-0 00:00:00"; // overwritten unless RTC fails
  if (RTC.read(tm)) {
    dateTime = String(tmYearToCalendar(tm.Year)) + "-" + 
               String(tm.Month) + "-" +
               String(tm.Day) + " " +
               as2digits(tm.Hour) + ":" +
               as2digits(tm.Minute) + ":" +
               as2digits(tm.Second);
  } else {
    if (RTC.chipPresent()) {
      blinkstatus = blink_error2;   // bad data from RTC
      if ( Serial ) {
        Serial.println("The DS1307 is stopped.  Please run the SetTime");
        Serial.println("example to initialize the time and begin running.");
        Serial.println();
      }
    } else {
      blinkstatus = blink_error3;  // no RTC -- low voltage can cause this
      if (Serial) {
        Serial.println("DS1307 read error!  Please check the circuitry.  (low voltage?)");
        Serial.println();
      }
    }
  }


  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if ( !dataFile ) {
    // see if we can shake things into place by reading the file  instead of writing
    File tmp = SD.open("datalog.txt", FILE_READ);
    if ( tmp ) tmp.close();
    
    if ( SD.begin(chipSelect) ) {  // desperation: reinitialize SD connection
      dataFile = SD.open("datalog.txt", FILE_WRITE);
      if ( dataFile ) {
        blinkstatus = blink_error4;  // open succeeded but only after SD re-init
      } else {
        blinkstatus = blink_error5;  // reinitialized SD, still couldn't open file
      }
    } else {
      blinkstatus = blink_error6;  // couldn't open file and couldn't reinitialize SD
    }
  }

  // if the file is available, write to it:
  String toprint = dateTime + ", " + millis() + ", " + blinkstatus + ", " + dataString;
  if (dataFile) {
    dataFile.println(toprint);
    dataFile.close();
  }
  if(Serial) Serial.println(toprint);
  
}



char checkValue(unsigned char *thebuf, char leng)
{
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
    receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;

  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
}

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module
  return PM10Val;
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

String as2digits(int number) {
  String answer = "";
  if (number >= 0 && number < 10) {
    answer += ('0');
  }
  answer += String(number);
  return(answer);
}



//int numblinkstoshow;

int currentblinknumber; // on-off are same number
int ison;  // to differentiate states within a blink number
int onduration = 200;
int offduration = 200;
int pauseduration = 400; // = onduration+offduration is nice
//int currentstate; // 

void initblinkstate() {

    enteredthisblinkstatewhen = millis();
    digitalWrite(led, HIGH);
    currentblinknumber = 1;
    ison = 1;
    setnextblinkchange();
}

void setnextblinkchange() {
    if ( currentblinknumber == blinkstatus && !ison ) {
        // the off after the blink merges with the between-cycle pause
        nextchangeblinkwhen = enteredthisblinkstatewhen + offduration + pauseduration;
    } else if ( ison ) {
        nextchangeblinkwhen = enteredthisblinkstatewhen + onduration;
    } else  { // !ison
        nextchangeblinkwhen = enteredthisblinkstatewhen + offduration;
    }
}


void transitionblink() {
    enteredthisblinkstatewhen = millis();
    if ( ison ) {
        digitalWrite(led, LOW);
    } else {
        digitalWrite(led, HIGH);
    }
    ison  = !ison;
    if ( ison) {
        currentblinknumber ++;
        if ( currentblinknumber > blinkstatus ) {
            currentblinknumber = 1;
        }
    }
    setnextblinkchange();
}





void dumpDataFile()
{
  File myFile = SD.open("datalog.txt", FILE_READ);
  if (myFile) {
    // from https://www.arduino.cc/en/Tutorial/ReadWrite
    Serial.println("datalog.txt contains:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
    Serial.println("---- end datalog.txt -----");
  }
}


