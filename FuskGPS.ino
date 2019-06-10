#include <TinyGPS++.h> // Include the TinyGPS++ library

#define ARDUINO_GPS_RX P3_2 //9 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX P3_3 // 8 // GPS RX, Arduino TX pin
#define GPS_BAUD 9600 // GPS module baud rate. GP3906 defaults to 9600.
#define gpsPort Serial1  // Alternatively, use Serial1 on the Leonardo
#define SerialMonitor Serial

TinyGPSPlus tinyGPS; // Create a TinyGPSPlus object

void setupGPS()
{
  //SerialMonitor.begin(9600);
  gpsPort.begin(GPS_BAUD);
}

void loopGPS()
{
  printGPSInfo();
  smartDelay(1000); 
}

void printGPSInfo()
{
  // Print latitude, longitude, altitude in feet, course, speed, date, time,
  // and the number of visible satellites.
  //SerialMonitor.print("Lat: "); 
  //*
  SerialMonitor.print("Lat: "); SerialMonitor.println(tinyGPS.location.lat(), 5);
  SerialMonitor.print("Long: "); SerialMonitor.println(tinyGPS.location.lng(), 5);
  SerialMonitor.print("Alt: "); SerialMonitor.println(tinyGPS.altitude.feet());
  SerialMonitor.print("Course: "); SerialMonitor.println(tinyGPS.course.deg());
  SerialMonitor.print("Speed: "); SerialMonitor.println(tinyGPS.speed.mph());
  SerialMonitor.print("Date: "); printDate();
  SerialMonitor.print("Time: "); printTime();
  SerialMonitor.print("Sats: "); SerialMonitor.println(tinyGPS.satellites.value());
  //*/ 
  SerialMonitor.println();
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

