/*
 * version 14 was used for the calibration run at DEQ in June 2018.
 * changes for version 15:
 *  - time/date stamp on file
 *  - report 1-minute averages in the file (rather than the current reading every 2 seconds)
 the output file name format should be
 AAAA_000.csv -- the name can be alphanum and not necessarily just 4 chars, so
 underscore for the separator from the numbers
  TO DO:
  - display page
  - multi-file upload
  - save file after uploading
  - file name format to match between this and data upload page
*/
 
 
 
 
 
 
 
 //******************************
 //*Abstract: Read value of PM1,PM2.5 and PM10 of air quality
 //
 //*Version：V3.1
 //*Author：Zuyang @ HUST
 //*Modified by Cain for Arduino Hardware Serial port compatibility
 //*Date：March.25.2016
 //* further modified by Bennett Battaile, Philip Orlando, Meenakshi Rao
 //*  for real-time clock, SD card saving, using Teensy board, etc.,
 //*  beginning 7/2017
 //******************************
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>


#define SENSOR_NAME      "ROBB"  //  up to 4 characters used for file name; file name is AAC_003.CSV
#define FNAME_USED_CHARS 4
#define FILE_SUFFIX      ".CSV" // must be upper case!
#define SOFTWARE_VERSION "15"
#define DELETE_OTHER_CSV_FILES 1
#define COLS_HDR "date-time, millis, status, navg, PM1.0std ug/m3,  PM2.5std ug/m3,  PM10std ug/m3, " \
                 "PM1.0atm ug/m3,  PM2.5atm ug/m3,  PM10atm ug/m3, " \
                 "pcnt/dl .3um, pcnt/dl .5um,  pcnt/dl 1um, pcnt/dl 2.5um, pcnt/dl 5um, pcnt/dl 10um"
#define NEW_LOGFILE_EVERY_TIME 1  // whether each restart begins a new logfile or reuses the same one

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];
int bufIndex = -1;
const int chipSelect = 10;
  
// plantower code adapted from
//  https://www.dfrobot.com/wiki/index.php/PM2.5_laser_dust_sensor_SKU:SEN0177
int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module


// each buffer full of readings from the sensor goes into latestDataSet.
int latestDataSet[12];

// latestDataSet is added in to an accumulator for doing 1-minute averaging.
int numAccumulatedDataSets;
int accumulatedDataSets[12];
String datadatetime = "0000-00-00 00:00:00"; // time of latest data point
int currentMinute; // for doing 1-minute averaging



String datalogfilename; // limited to 8.3 filename


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

String sensornameprefix()
{
  String answer(SENSOR_NAME);
  answer = answer.substring(0,FNAME_USED_CHARS);
  return answer;
}


// call back for file timestamps - https://forum.arduino.cc/index.php?topic=348562.0
void dateTimeCB(uint16_t* date, uint16_t* tim)
{
  tmElements_t tm;  
  if (RTC.read(tm)) {
    *date = FAT_DATE(tmYearToCalendar(tm.Year), tm.Month, tm.Day);
    *tim  = FAT_TIME(tm.Hour, tm.Minute, tm.Second);
  }  
}



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

  if (Serial) Serial.println(String("plantower_") + SOFTWARE_VERSION);

  initblinkstate();

  //lines through Serial.println("card initialized") from Teensy Datalogger example  BB 7/17
  //BB 7/17 if(Serial) in case usb isn't plugged in; if(Serial1) is always true per https://www.arduino.cc/en/Serial/IfSerial
  //to initialize the SD card, which is on the Teensy Audio board,
  //chipSelect should be set to 10
  if (Serial) Serial.print("Initializing SD card... ");
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

  cleanoutirrelevantfiles();
  makedatalogfilename();


  
  //dumpDataFile(); // handy to be able to see the file without pulling the card out

  String hdr1 = String("monitor starting up,")
                     + "name=" + SENSOR_NAME
                     + ", swversion=" + SOFTWARE_VERSION
                     + ", datetime=" + getdatetime() + "";
  String hdr2 = COLS_HDR;
  
  SdFile::dateTimeCallback(dateTimeCB); // https://forum.arduino.cc/index.php?topic=348562.0

  File dataFile = SD.open(datalogfilename.c_str(), FILE_WRITE);
  if (dataFile) {
    dataFile.println(hdr1);
    dataFile.println(hdr2);
    dataFile.close(); // to be reopened for every data write
  } else {
    Serial.println("could not write header to data file");  // unlikely error
  }
  Serial.println(hdr1);
  Serial.println(hdr2);
}


//  delete any .csv files which aren't from this sensor, to reduce confusion
//  if the card was used in a different monitor
void cleanoutirrelevantfiles()
{
  if ( !DELETE_OTHER_CSV_FILES )
    return;
    
  File root;
  root = SD.open("/");
  while (true) {
    File entry =  root.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    if ( String(entry.name()).startsWith(sensornameprefix()) ) {
      Serial.println( String("keeping ") + entry.name());
    } else if ( String(entry.name()).endsWith(FILE_SUFFIX) ) {
      Serial.println( String("deleting file: ") + entry.name());
      String fullpath = String("/")+entry.name();
      SD.remove(fullpath.c_str());      
    } else {
      Serial.println( String("ignoring ") + entry.name());      
    }
    entry.close();
  }
  root.close();
}


// format:  SENSOR_NAME + _ + 4-digit number + FILE_SUFFIX
// we'll go through all such, extract the numbers, and
// return a new name with the next higher number so as
// to not collide with existing.
void makedatalogfilename()
{
  if ( ! NEW_LOGFILE_EVERY_TIME )
  {
    // we'll reuse the same log file
    datalogfilename = String(sensornameprefix()) + "_001" + String(FILE_SUFFIX);
    Serial.println( String("output file name:  " ) + datalogfilename);
    return;
  }
  Serial.println("figuring out next data file name");
  int highwater = 0; // highest number seen (so we'll start by generating 00001)
  File root;
  root = SD.open("/");
  while (true) {
    File entry =  root.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    if ( String(entry.name()).startsWith(sensornameprefix()+"_") 
      && String(entry.name()).endsWith(FILE_SUFFIX) ) {
      Serial.println( String("considering ") + entry.name());
      String mycopy = entry.name();
      mycopy.replace(sensornameprefix()+"_", "");
      mycopy.replace(FILE_SUFFIX, "");
      // now it should be a pure number.
      int seen = mycopy.toInt();
      if ( seen > highwater )
      {
        highwater = seen;
      }
    } else {
      Serial.println( String("ignoring ") + entry.name());      
    }
    entry.close();
  }
  root.close();
  char tmpbuf[55];
  sprintf(tmpbuf, "%03d", highwater+1);
  datalogfilename = String(sensornameprefix()) + "_" + String(tmpbuf) + String(FILE_SUFFIX);
  Serial.println( String("output file name:  " ) + datalogfilename);
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
    if ( isItANewMinute() ) 
    {
        writeDataToLogFile();
        clearDataAccumulator();
    }
    datadatetime = getdatetime();
    for ( int i = 0; i < 12; i++ ) 
    {
        int hi = buf[2*i+3];
        int lo = buf[2*i+4];
        latestDataSet[i] = (hi<<8) + lo;
    }

  String dataString =  " ";
  for ( int i = 0; i < 12; i++ )
  {
    dataString += String(latestDataSet[i]);
    if ( i+1 < 12 )
      dataString += ", ";
  }
  Serial.println("** - " + dataString);
  
    
    
    addToDataAccumulator();
}

int isItANewMinute()
{
    tmElements_t tm;
    if (RTC.read(tm))
    {
        if ( tm.Minute != currentMinute )
        {
            currentMinute = tm.Minute;
            return 1;
        }
    }
    return 0;
}

void clearDataAccumulator()
{
    numAccumulatedDataSets = 0;
    for ( int i = 0; i < 12; i++ ) 
    {
        accumulatedDataSets[i] = 0;
    }
}

void addToDataAccumulator()
{
    numAccumulatedDataSets ++;
    for ( int i = 0; i < 12; i++ ) 
    {
        accumulatedDataSets[i]  += latestDataSet[i];
    }
}

String reportDataAccumulator()
{
    String answer  =  "";

    answer += String(numAccumulatedDataSets) + ", ";
  
    for ( int i = 0; i < 12; i++ )
    {
        float avg = (numAccumulatedDataSets < 1)
                  ? 0
                  : ((float)accumulatedDataSets[i]/(float)numAccumulatedDataSets);
        answer += String(avg,3);  // how many decimal places?  Say 3.
        if ( i+1 < 12 )
            answer += ", ";
    }
    return answer;
}


// 2-second activity cycle
void loop()
{
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

//  writeDataToLogFile();  do this in the serialEvent function instead
}



void writeDataToLogFile()
{
  String dataString = "";
  /*
  dataString =  " ";
  for ( int i = 0; i < 12; i++ )
  {
    dataString += String(latestDataSet[i]);
    if ( i+1 < 12 )
      dataString += ", ";
  }
  */
  dataString = reportDataAccumulator();

  String dateTime = datadatetime; // want the last data point time, not now (which will be later)

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(datalogfilename.c_str(), FILE_WRITE);
  if ( !dataFile ) {
    // see if we can shake things into place by reading the file  instead of writing
    File tmp = SD.open(datalogfilename.c_str(), FILE_READ);
    if ( tmp ) tmp.close();
    
    if ( SD.begin(chipSelect) ) {  // desperation: reinitialize SD connection
      dataFile = SD.open(datalogfilename.c_str(), FILE_WRITE);
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




String getdatetime()
{
  String dateTime = "0000-0-0 00:00:00"; // overwritten unless RTC fails
  tmElements_t tm;

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
  
  return dateTime;
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
  String filetodump("datalog.txt");
  File myFile = SD.open(filetodump.c_str(), FILE_READ);
  if (myFile) {
    // from https://www.arduino.cc/en/Tutorial/ReadWrite
    Serial.println(filetodump + " contains:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
    Serial.println("---- end  -----");
  } else {
    Serial.println(filetodump + ": couldn't open data file");
  }
}


