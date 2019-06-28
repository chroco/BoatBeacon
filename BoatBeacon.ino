#include <TinyGPS++.h> 
#include <stdio.h>
#include <ctype.h>

#define LED RED_LED
//#define LED GREEN_LED

#define OOK_PIN P5_0 
#define VCO_PIN P5_2
#define GPS_RX P3_2 
#define GPS_TX P3_3 
#define GPS_BAUD 9600 
#define gpsPort Serial1  
#define SerialMonitor Serial

TinyGPSPlus tinyGPS; 

const int kMessageLength = (1<<10);
const int kAsciiOffset = 49;
const int kDotLengthMS = 150;
const int kDeadSpaceMS = kDotLengthMS;
const int kDashLengthMS = 3 * kDotLengthMS;
bool gUseFSK = true;

char message[kMessageLength] = "KJ7FVI\0";

const char *pMorseCodeHash[] = 
{
  ".----\0",  // 0 
  "..---\0",  // 1
  "...--\0",  // 2 
  "....-\0",  // 3 
  ".....\0",  // 4
  "-....\0",  // 5
  "--...\0",  // 6
  "---..\0",  // 7
  "----.\0",  // 8
  "-----\0",  // 9
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  ".-\0",     // A
  "-...\0",   // B
  "-.-.\0",   // C
  "-..\0",    // D
  ".\0",      // E
  "..-.\0",   // F
  "--.\0",    // G
  "....\0",   // H
  "..\0",     // I
  ".---\0",   // J
  "-.-\0",    // K
  ".-..\0",   // L
  "--\0",     // M
  "-.\0",     // N
  "---\0",    // O
  ".--.\0",   // P
  "--.-\0",   // Q
  ".-.\0",    // R
  "...\0",    // S
  "-\0",      // T
  "..-\0",    // U
  "...-\0",   // V
  ".--\0",    // W
  "-..-\0",   // X
  "-.--\0",   // Y
  "--..\0",   // Z
  NULL
};

#define MORSE_CODE_HASH_LENGTH (sizeof(pMorseCodeHash)/sizeof(char *))

int i = 0;
int j = 0;
const char *pMorseCode = NULL;

const char *getMorseCode(char character)
{
  character = toupper(character);
  int index = character - kAsciiOffset;
  if(index < 0 || index > MORSE_CODE_HASH_LENGTH)
  {
    return NULL;
  }

  return pMorseCodeHash[index];
}

void setup()
{
  SerialMonitor.begin(9600);
  gpsPort.begin(GPS_BAUD);

  pinMode(LED, OUTPUT); 
  pinMode(OOK_PIN, OUTPUT); 
  pinMode(VCO_PIN, OUTPUT); 
  delay(500);
  digitalWrite(OOK_PIN, LOW);   
  digitalWrite(VCO_PIN, LOW);   
}

void loop()
{
  transmitGPSInfo();
  smartDelay(1000); 
}

void txSignal(int pin, int length)
{
  digitalWrite(pin, HIGH);   
  digitalWrite(LED, HIGH);   
  delay(length);               
  digitalWrite(pin, LOW);    
  digitalWrite(LED, LOW);    
  delay(kDeadSpaceMS);
}

int txPin;

void serialOutMorseCode() 
{
  if(gUseFSK)
  {
    digitalWrite(OOK_PIN, HIGH);
    txPin = VCO_PIN; 
    SerialMonitor.print("FSK TX\n");
  }
  else
  {
    digitalWrite(VCO_PIN, LOW);   
    txPin = OOK_PIN;
    SerialMonitor.print("OOK TX\n");
  }
  gUseFSK = !gUseFSK;

  for(i = 0; i < kMessageLength && message[i] != '\0';++i)
  {
    pMorseCode = getMorseCode(message[i]);
    if(pMorseCode)
    {
      SerialMonitor.print(message[i]);
      SerialMonitor.print(": ");
      
      for(j = 0; pMorseCode[j] && pMorseCode[j] != '\0';++j)
      {
        SerialMonitor.print(pMorseCode[j]); 
        if(pMorseCode[j] == '.')
        {
          txSignal(txPin, kDotLengthMS);
        }
        else if(pMorseCode[j] == '-')
        {
          txSignal(txPin, kDashLengthMS);
        }
      }
    }
    
    SerialMonitor.print('\n');
    delay(kDashLengthMS);
  }

  SerialMonitor.print("\n\r");
}


void transmitGPSInfo()
{
  /*
  String sLatitude = tinyGPS.location.lat();
  String sLongitude = tinyGPS.location.lng();
  String sAltitude = tinyGPS.altitude.feet();

  SerialMonitor.print("Lat: "); SerialMonitor.println(sLatitude);
  SerialMonitor.print("Long: "); SerialMonitor.println(sLongitude);
  SerialMonitor.print("Alt: "); SerialMonitor.println(sAltitude);
  SerialMonitor.println();
  //*/
  String gpsPosition = "";
  gpsPosition.concat("Lat "); gpsPosition.concat(tinyGPS.location.lat()); gpsPosition.concat(", "); 
  gpsPosition.concat("Lng "); gpsPosition.concat(tinyGPS.location.lng()); gpsPosition.concat(", "); 
  gpsPosition.concat("Alt "); gpsPosition.concat(tinyGPS.altitude.feet()); 
  SerialMonitor.println(gpsPosition);

  /*
  SerialMonitor.print("Lat: "); SerialMonitor.println(tinyGPS.location.lat(), 5);
  SerialMonitor.print("Long: "); SerialMonitor.println(tinyGPS.location.lng(), 5);
  SerialMonitor.print("Alt: "); SerialMonitor.println(tinyGPS.altitude.feet());
  SerialMonitor.print("Course: "); SerialMonitor.println(tinyGPS.course.deg());
  SerialMonitor.print("Speed: "); SerialMonitor.println(tinyGPS.speed.mph());
  SerialMonitor.print("Date: "); printDate();
  SerialMonitor.print("Time: "); printTime();
  SerialMonitor.print("Sats: "); SerialMonitor.println(tinyGPS.satellites.value());
  SerialMonitor.println();
  //*/ 
}

// "Smart delay" looks for GPS data while the Arduino's not doing anything else
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (gpsPort.available())
    {
      tinyGPS.encode(gpsPort.read()); // Send it to the encode function
    } 
  // tinyGPS.encode(char) continues to "load" the tinGPS object with new
  // data coming in from the GPS module. As full NMEA strings begin to come in
  // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}

// printDate() formats the date into dd/mm/yy.
void printDate()
{
  SerialMonitor.print(tinyGPS.date.day());
  SerialMonitor.print("/");
  SerialMonitor.print(tinyGPS.date.month());
  SerialMonitor.print("/");
  SerialMonitor.println(tinyGPS.date.year());
}

// printTime() formats the time into "hh:mm:ss", and prints leading -1's
// where they're called for.
void printTime()
{
  SerialMonitor.print(tinyGPS.time.hour());
  SerialMonitor.print(":");
  if (tinyGPS.time.minute() < 9) SerialMonitor.print('0');
  SerialMonitor.print(tinyGPS.time.minute());
  SerialMonitor.print(":");
  if (tinyGPS.time.second() < 9) SerialMonitor.print('0');
  SerialMonitor.println(tinyGPS.time.second());
}

