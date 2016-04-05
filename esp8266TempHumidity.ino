#include <OneWire.h>
#include <DallasTemperature.h>
#include <elapsedMillis.h>

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <QuickStats.h>

#include "Battery.h"
#include "Dht22.h"
#include "Led.h"
#include "Utils.h"
#include "Ds18b20.h"

const char* ssid     = "McKearneys";
const char* password = "";
const char* hostName = "hoopy-outside"; // TODO: CHANGE FOR EACH DEVICE!
#define HAS_SOIL_TEMP 1
 
const char* host = "192.168.0.106";
const int httpPort = 8080;

#define DHTPIN  12
#define MICROSECONDS_PER_MINUTE 60*1000000  // 60 seconds * 1 million 

const int minutesBetweenMeasurements = 1;
const int numTimesPerReading = 5; // times to read sensor per reading
const int numReadingsUntilSend = 30;

const uint8_t CODE_VERSION = 0x07; // Change/Increment this each version

struct Readings {
  uint8_t marker; // indicator that we're reading proper data
  uint8_t readingsFinished;

  int temperature[numReadingsUntilSend];
  int humidity[numReadingsUntilSend];
  int batteryLevel[numReadingsUntilSend];
  int soilTemperature[numReadingsUntilSend];
};

// forward declarations
void InitializeStoredData(Readings& data);
int normalizeReadings(float* readings, int length);
String arrayToJson(int* arr, int length);
void SendData(Readings& data);
void ReadValues(int index, Readings& storedData);

#define NO_SLEEP 1

// temp/humidity + battery
Dht22 dht22(DHTPIN);
Battery battery(A0);

#ifdef HAS_SOIL_TEMP
Ds18b20 soilTemp(4);
#endif

// Using Blue LED for debugging
Led led(2, true); // pin 2 needs inverting

void loop() {
#ifdef NO_SLEEP
  Serial.println("Next Batch");
  Readings data;
  InitializeStoredData(data);
  for(int i=0; i < numReadingsUntilSend; ++i) {
    elapsedMillis sinceStart;
    led.OnOff(true);
    ReadValues(i, data);
    led.OnOff(false);
    
    Serial.print(F("Took this many millis: "));
    Serial.println(sinceStart);

    // sleep for proper minutes minus processing time
    uint32_t millisDelay = minutesBetweenMeasurements * 60000 - sinceStart;

    Utils::Delay(millisDelay);
  }
  SendData(data);
#endif
}

void setup() {
  Serial.begin(115200);
  Utils::Delay(100);

  Serial.print(F("\nHoopy name: "));
  Serial.print(hostName);
  Serial.print(F(" running version: "));
  Serial.println(CODE_VERSION);
  
#ifndef NO_SLEEP
  
  elapsedMillis sinceStart;
  EEPROM.begin(4096);

  // First, read the existing readings from the EEPROM
  Readings storedData;
  EEPROM.get(0, storedData);
  if (storedData.marker != CODE_VERSION) {
    Serial.println(F("No Readings marker, initializing"));
    InitializeStoredData(storedData);
  }

  
  int newIndex = storedData.readingsFinished;
  ReadValues(newIndex, storedData);

  if (newIndex == numReadingsUntilSend-1) {

    Serial.println(F("Last Reading: uploading!"));
    // this is the last reading before send, so send now
    SendData(storedData);

    InitializeStoredData(storedData);
  }
  else {
    ++storedData.readingsFinished;
  }

  // save the structure back
  // TODO: do this piecemeal, rather than the whole structure at once
  EEPROM.put(0, storedData);
  EEPROM.end();
  
  Serial.print(F("Sleeping for "));
  Serial.print(minutesBetweenMeasurements);
  Serial.println(F(" minutes. Zzzz..."));
  led.OnOff(false);

  Serial.print(F("Took this many millis: "));
  Serial.println(sinceStart);

  WakeMode wakeMode = WAKE_RF_DISABLED;
  if (storedData.readingsFinished == numReadingsUntilSend-1) {
    // wake up w/ Wifi next time
    wakeMode = WAKE_RF_DEFAULT;
  }

  // sleep for proper minutes minus processing time
  uint32_t timeToSleep = minutesBetweenMeasurements * MICROSECONDS_PER_MINUTE - sinceStart * 1000;
  
  ESP.deepSleep(timeToSleep, wakeMode);
#endif
}

///////// IMPLEMENTATION below


void InitializeStoredData(Readings& data) {
    data.marker = CODE_VERSION;
    data.readingsFinished = 0;

    for(int i=0; i < numReadingsUntilSend; ++i) {
      data.temperature[i] = -1;
      data.humidity[i] = -1;
      data.batteryLevel[i] = -1;
    }
}

void ReadValues(int index, Readings& storedData) {
    // read new values
    float
        temperature[numTimesPerReading],
        humidity[numTimesPerReading],
        batteryLevels[numTimesPerReading];

    Serial.print(F("Reading Values for index: "));
    Serial.println(index);

    // read and normalize values
    dht22.ReadTempAndHumidity(numTimesPerReading, temperature, humidity);
    int temp = normalizeReadings(temperature, numTimesPerReading);
    int hum = normalizeReadings(humidity, numTimesPerReading);

    battery.ReadLevels(numTimesPerReading, batteryLevels);
    int batt = normalizeReadings(batteryLevels, numTimesPerReading);

    int soilTemperature = -1;
    #ifdef HAS_SOIL_TEMP
    float soilTempF;
    soilTemp.ReadTemp(1, &soilTempF);
    soilTemperature = (int)soilTempF;
    #endif

    Serial.printf("Normalized Temp/Hum/Battery/Soil Temp: %d/%d/%d/%d\n", temp, hum, batt, soilTemperature);

    storedData.temperature[index] = temp;
    storedData.humidity[index] = hum;
    storedData.batteryLevel[index] = batt;
    storedData.soilTemperature[index] = soilTemperature;
}

// global QuickStats for simple stats
QuickStats stats;
int normalizeReadings(float* readings, int length) {
    stats.bubbleSort(readings, length);

    // trims off the largest and smallest values and averages the rest
    return (int)stats.average(&readings[1], length-2);  
}

String arrayToJson(int* arr, int length) 
{
  String result = F("[");
  for(int i=0; i < length; i++) 
  {
    int val = arr[i];
    result += arr[i];
    if (i < length-1) {
      result += F(",");
    }
  }
  result += F("]");

  return result;
}

void SendData(Readings& data) {
    Serial.print(F("\nConnecting to "));
    Serial.println(ssid);
  
    WiFi.begin(ssid, "");

  Serial.println(F("After Begin"));
  
  while (WiFi.status() != WL_CONNECTED) {
    Utils::Delay(500);
    Serial.print(F("."));
    yield();
  }
 
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  Serial.print(F("connecting to "));
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println(F("connection failed"));
    return;
  }
  
  // We now create a URI for the request
  String url = F("/stats");
  Serial.print(F("POSTING to URL: "));
  Serial.println(url);

  String messageBody = String(F("{\"version\": ")) + CODE_VERSION 
    + F(", \"temps\": ") + arrayToJson(data.temperature, numReadingsUntilSend) 
    + F(", \"humidity\": ") + arrayToJson(data.humidity, numReadingsUntilSend) 
    + F(", \"battery\": ") + arrayToJson(data.batteryLevel, numReadingsUntilSend) 
    + F(", \"soilTemp\": ") + arrayToJson(data.soilTemperature, numReadingsUntilSend) 
    + F("}\r\n");

  Serial.println(messageBody);
        
  client.print(String(F("POST ")) + url + F(" HTTP/1.1\r\n") +
        F("Host: ") + hostName + F("\r\n") +
        F("Content-Type: application/json\r\n") +
        F("Content-Length: ") +
        String(messageBody.length()) + F("\r\n\r\n"));
  client.print(messageBody);

  // wait for the server to do its thing
  Utils::Delay(500);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }
  
  Serial.println(F("closing connection"));
}


