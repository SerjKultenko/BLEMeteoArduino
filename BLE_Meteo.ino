#include <AltSoftSerial.h>

AltSoftSerial BTserial;
// https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html

// BT Serial Constants
char c = ' ';
boolean NL = true;


// DHT Sensor Stuff
#include "DHT.h"

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;
const long interval = 2000;           // interval at which to read and send sensors (milliseconds)

#include <MQ135.h>

MQ135 coSensor(0);


// BMP280 Support
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10

Adafruit_BMP280 bmp; // I2C

void setup()
{
  Serial.begin(9600);
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");

  if (!bmp.begin(0x76)) { 
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  BTserial.begin(9600);
  Serial.println("BTserial started at 9600");
  dht.begin();
}

void loop()
{
  // Read from the Bluetooth module and send to the Arduino Serial Monitor
  if (BTserial.available())
  {
    c = BTserial.read();
    Serial.write(c);
  }


  // Read from the Serial Monitor and send to the Bluetooth module
  if (Serial.available())
  {
    c = Serial.read();

    // do not send line end characters to the HM-10
    if (c != 10 & c != 13 )
    {
      BTserial.write(c);
    }

    // Echo the user input to the main window.
    // If there is a new line print the ">" character.
    if (NL) {
      Serial.print("\r\n>");
      NL = false;
    }
    Serial.write(c);
    if (c == 10) {
      NL = true;
    }
  }


  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you worked with sensors
    previousMillis = currentMillis;

    readAndSendSensors();
  }

}

void readAndSendSensors() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  String hStr = String("HU") + String(h, 2) + String(" ");
  String tStr = String("TE") + String(t, 2) + String(" ");

  BTserial.write(hStr.c_str());
  BTserial.write(tStr.c_str());

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  float co = coSensor.getPPM();
  float rzero = coSensor.getRZero();

  // Send CO2
  String co2Str = String("CO") + String(co, 2) + String(" ");
  BTserial.write(co2Str.c_str());
  
  // Send CO2 resistence
  String cRzStr = String("CR") + String(rzero, 2) + String(" ");
  BTserial.write(cRzStr.c_str());
  Serial.print("CR: ");
  Serial.println(rzero);

  float tBMP280 = bmp.readTemperature();
  Serial.print(F("BMP280 Temperature = "));
  Serial.print(tBMP280);
  Serial.println(" *C");

  // Send Temperature obtained from BMP280 sensor
  String tBMP280Str = String("TB") + String(tBMP280, 2) + String(" ");
  BTserial.write(tBMP280Str.c_str());

  float pressure = bmp.readPressure();  
  Serial.print(F("BMP280 Pressure = "));
  Serial.print(pressure);
  Serial.println(" Pa");
  
  // Send Pressure obtained from BMP280 sensor
  String pressureStr = String("PR") + String(pressure, 2) + String(" ");
  BTserial.write(pressureStr.c_str());

  Serial.print(F("BMP280  Approx altitude = "));
  Serial.print(bmp.readAltitude(1013.25)); // this should be adjusted to your local forcase
  Serial.println(" m");
}
