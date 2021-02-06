// mode COM5 BAUD=115200 PARITY=N DATA=8 STOP=1 xon=off odsr=off octs=off dtr=off rts=off
// echo %date% %time% >> COM6:
// type COM6: >> test.txt

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>

#include <Wire.h>
#include <SparkFunBME280.h> //Click here to get the library: http://librarymanager/All#SparkFun_BME280
#include <SparkFunCCS811.h> //Click here to get the library: http://librarymanager/All#SparkFun_CCS811

#define WIFI_SSID   "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define WIFI_SSID   "Indian"
#define WIFI_PASSWORD "r5s57koaqpczc"
#define  JST   3600*9

// Unique name of individual device.
#define MYNAME F("MW-01 01")

//#define CCS811_ADDR 0x5B //Default CCS811 I2C Address
#define CCS811_ADDR 0x5A //Alternate CCS811 I2C Address

// WAKE pin(not connected/direct connect to GND)
#define PIN_NOT_WAKE 5

#define BME280_ADDR 0x76  // BME280 I2C Address

//Global sensor objects
CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;

// RTC initialized flag
bool rtcinit = false;

// CCS811 calibrating flag
bool isCCS811Caribrating = true;
// CCS811 caribration time(20min)
#define CCS811_CARIB_TIME 1200000

// Header output flag
bool isHeaderWritten = false;

// Check all charactor is numeric
boolean isNumeric(String str) {
  unsigned int stringLength = str.length();
  if (stringLength == 0) {
    return false;
  }
  boolean seenDecimal = false;
  for (unsigned int i = 0; i < stringLength; ++i) {
    if (isDigit(str.charAt(i))) {
      continue;
    }
    if (str.charAt(i) == '.') {
      if (seenDecimal) {
        return false;
      }
      seenDecimal = true;
      continue;
    }
    return false;
  }
  return true;
}

// Print 2 digits
void print2digits(int number) {
  if (number < 10) {
    Serial.print("0"); // print a 0 before if the number is < than 10
  }
  Serial.print(number);
}

// Setup the RTC Date and time
bool setRtcDateTime(String dateTimeStr) {
  return true;
}

void printRtcDateTimeSerial() {
  time_t t;
  struct tm *tm;
  
  t = time(NULL);
  tm = localtime(&t);
  Serial.printf(" %04d/%02d/%02d %02d:%02d:%02d",
    tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
    tm->tm_hour, tm->tm_min, tm->tm_sec);
}

// Print measurement data to serial
void printInfoSerial()
{
  //getCO2() gets the previously read data from the library
  Serial.println("CCS811 data:");
  Serial.print(" CO2 concentration : ");
  Serial.print(myCCS811.getCO2());
  Serial.println(" ppm");

  //getTVOC() gets the previously read data from the library
  Serial.print(" TVOC concentration : ");
  Serial.print(myCCS811.getTVOC());
  Serial.println(" ppb");

  Serial.println("BME280 data:");
  Serial.print(" Temperature: ");
  Serial.print(myBME280.readTempC(), 2);
  Serial.println(" degrees C");

  Serial.print(" Temperature: ");
  Serial.print(myBME280.readTempF(), 2);
  Serial.println(" degrees F");

  Serial.print(" Pressure: ");
  Serial.print(myBME280.readFloatPressure(), 2);
  Serial.println(" Pa");

  Serial.print(" Pressure: ");
  Serial.print((myBME280.readFloatPressure() * 0.0002953), 2);
  Serial.println(" InHg");

  Serial.print(" Altitude: ");
  Serial.print(myBME280.readFloatAltitudeMeters(), 2);
  Serial.println("m");

  Serial.print(" Altitude: ");
  Serial.print(myBME280.readFloatAltitudeFeet(), 2);
  Serial.println("ft");

  Serial.print(" %RH: ");
  Serial.print(myBME280.readFloatHumidity(), 2);
  Serial.println(" %");

  Serial.println();
}

// Print CSV Header to serial
void printCsvHeaderSerial()
{
  //getCO2() gets the previously read data from the library
  Serial.print("Date Time,");
  Serial.print("Device name,");
  Serial.print("Caribrating,");
  Serial.print("CO2 concentration(ppm),");
  Serial.print("TVOC concentration(ppb),");
  Serial.print("Temperature(degrees C),");
  Serial.print("Pressure(Pa),");
  Serial.print("Altitude(m),");
  Serial.print("Humidity(%),");
  Serial.println();
}

// Print measurement csv data to serial
void printInfoCsvSerial()
{
  printRtcDateTimeSerial();
  Serial.print(",");

  Serial.print(MYNAME);
  Serial.print(",");

  Serial.print(isCCS811Caribrating ? "1" : "0");
  Serial.print(",");

  //getCO2() gets the previously read data from the library
  Serial.print(myCCS811.getCO2());
  Serial.print(",");

  //getTVOC() gets the previously read data from the library
  Serial.print(myCCS811.getTVOC());
  Serial.print(",");

  Serial.print(myBME280.readTempC(), 2);
  Serial.print(",");

  Serial.print(myBME280.readFloatPressure(), 2);
  Serial.print(",");

  Serial.print(myBME280.readFloatAltitudeMeters(), 2);
  Serial.print(",");

  Serial.print(myBME280.readFloatHumidity(), 2);

  Serial.println();
}

//printSensorError gets, clears, then prints the errors
//saved within the error register.
void printSensorError()
{
  uint8_t error = myCCS811.getErrorRegister();

  if (error == 0xFF) //comm error
  {
    Serial.println("Failed to get ERROR_ID register.");
  }
  else
  {
    Serial.print("Error: ");
    if (error & 1 << 5)
      Serial.print("HeaterSupply");
    if (error & 1 << 4)
      Serial.print("HeaterFault");
    if (error & 1 << 3)
      Serial.print("MaxResistance");
    if (error & 1 << 2)
      Serial.print("MeasModeInvalid");
    if (error & 1 << 1)
      Serial.print("ReadRegInvalid");
    if (error & 1 << 0)
      Serial.print("MsgInvalid");
    Serial.println();
  }
}

void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("Apply BME280 data to CCS811 for compensation.");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println();
  Serial.printf("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  Wire.begin(12,14);

  //This begins the CCS811 sensor and prints error status of .beginWithStatus()
  CCS811Core::CCS811_Status_e returnCode = myCCS811.beginWithStatus();
  Serial.print("CCS811 begin exited with: ");
  Serial.println(myCCS811.statusString(returnCode));

  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = BME280_ADDR;

  //Initialize BME280
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = BME280_ADDR;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;

  //Calling .begin() causes the settings to be loaded
  delay(10); //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  myBME280.begin();
}

void loop() {
  if ( millis() > CCS811_CARIB_TIME ) isCCS811Caribrating = false;

  if ( Serial.available() > 0) {
    String buf = Serial.readStringUntil('\n');
    if ( setRtcDateTime(buf) ) {
      rtcinit = true;
    }
  }
  if (rtcinit) {
    // print csv header
    if (!isHeaderWritten) {
      printCsvHeaderSerial();
      isHeaderWritten = true;
    }

    // get date and time from RTC and print to serial
    //printRtcDateTimeSerial();
    //Serial.println();

    //Calling this function updates the global tVOC and eCO2 variables
    myCCS811.readAlgorithmResults();
    //printInfoSerial fetches the values of tVOC and eCO2
    //printInfoSerial();
    printInfoCsvSerial();

    float BMEtempC = myBME280.readTempC();
    float BMEhumid = myBME280.readFloatHumidity();

    //Serial.print("Applying new values (deg C, %): ");
    //Serial.print(BMEtempC);
    //Serial.print(",");
    //Serial.println(BMEhumid);
    //Serial.println();

    //This sends the temperature data to the CCS811
    myCCS811.setEnvironmentalData(BMEhumid, BMEtempC);
  }
  else if (myCCS811.checkForStatusError())
  {
    //If the CCS811 found an internal error, print it.
    printSensorError();
  }
  delay(3000);
}
