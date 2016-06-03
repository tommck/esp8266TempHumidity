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

#include "Config.h"

#define MICROSECONDS_PER_MINUTE 60*1000000  // 60 seconds * 1 million 

struct Readings {
  uint8_t marker; // indicator that we're reading proper data
  uint8_t readingsFinished;

  int temperature[Config::numReadingsUntilSend];
  int humidity[Config::numReadingsUntilSend];
  int batteryLevel[Config::numReadingsUntilSend];
  int soilTemperature[Config::numReadingsUntilSend];
};

// forward declarations
void InitializeStoredData(Readings& data);
int normalizeReadings(float* readings, int length);
String arrayToJson(int* arr, int length);
void SendData(Readings& data);
void ReadValues(int index, Readings& storedData);

// temp/humidity + battery
Dht22 dht22(Config::dhtPin);
Battery battery(A0); // TODO: config?

Ds18b20 soilTemp(Config::ds18b20Pin);

// Using Blue LED for debugging
Led led(2, true); // pin 2 needs inverting

void loop() {
  if (Config::shouldSleep == false) {
    Serial.println("Next Batch");
    Readings data;
    InitializeStoredData(data);
    for(int i=0; i < Config::numReadingsUntilSend; ++i) {
      elapsedMillis sinceStart;
      led.OnOff(true);
      ReadValues(i, data);
      led.OnOff(false);
      
      Serial.print(F("Took this many millis: "));
      Serial.println(sinceStart);
  
      // sleep for proper minutes minus processing time
      uint32_t millisDelay = Config::minutesBetweenMeasurements * 60000 - sinceStart;
  
      Utils::Delay(millisDelay);
    }
    SendData(data);
  }
}

void setup() {
  Serial.begin(115200);
  Utils::Delay(100);

  Serial.print(F("\nHoopy name: "));
  Serial.print(Config::hostName);
  Serial.print(F(" running version: "));
  Serial.println(Config::codeVersion);

  if (Config::shouldSleep == true) {
      elapsedMillis sinceStart;
      EEPROM.begin(4096);
    
      // First, read the existing readings from the EEPROM
      Readings storedData;
      EEPROM.get(0, storedData);
      if (storedData.marker != Config::codeVersion) {
        Serial.println(F("No Readings marker, initializing"));
        InitializeStoredData(storedData);
      }
      
      int newIndex = storedData.readingsFinished;
      ReadValues(newIndex, storedData);
    
      if (newIndex == Config::numReadingsUntilSend-1) {
    
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
      Serial.print(Config::minutesBetweenMeasurements);
      Serial.println(F(" minutes. Zzzz..."));
      led.OnOff(false);
    
      Serial.print(F("Took this many millis: "));
      Serial.println(sinceStart);
    
      WakeMode wakeMode = WAKE_RF_DISABLED;
      if (storedData.readingsFinished == Config::numReadingsUntilSend-1) {
        // wake up w/ Wifi next time
        wakeMode = WAKE_RF_DEFAULT;
      }
    
      // sleep for proper minutes minus processing time
      uint32_t timeToSleep = Config::minutesBetweenMeasurements * MICROSECONDS_PER_MINUTE - sinceStart * 1000;
      
      ESP.deepSleep(timeToSleep, wakeMode);
  }
}

///////// IMPLEMENTATION below


void InitializeStoredData(Readings& data) {
    data.marker = Config::codeVersion;
    data.readingsFinished = 0;

    for(int i=0; i < Config::numReadingsUntilSend; ++i) {
      data.temperature[i] = -1;
      data.humidity[i] = -1;
      data.batteryLevel[i] = -1;
    }
}

void ReadValues(int index, Readings& storedData) {
    // read new values
    float
        temperature[Config::numTimesPerReading],
        humidity[Config::numTimesPerReading],
        batteryLevels[Config::numTimesPerReading];

    Serial.print(F("Reading Values for index: "));
    Serial.println(index);

    // read and normalize values
    dht22.ReadTempAndHumidity(Config::numTimesPerReading, temperature, humidity);
    int temp = normalizeReadings(temperature, Config::numTimesPerReading);
    int hum = normalizeReadings(humidity, Config::numTimesPerReading);

    battery.ReadLevels(Config::numTimesPerReading, batteryLevels);
    int batt = normalizeReadings(batteryLevels, Config::numTimesPerReading);

    int soilTemperature = -1;
    if (Config::hasSoilTemp == true) {
      float soilTempF;
      soilTemp.ReadTemp(1, &soilTempF);
      soilTemperature = (int)soilTempF;
    }

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
    Serial.println(Config::ssid);
  
    WiFi.begin(Config::ssid, Config::password);

  Serial.println(F("After Begin"));
  
  while (WiFi.status() != WL_CONNECTED) {
    Utils::Delay(500);
    Serial.print(F("."));
    yield();
  }
 
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  Serial.print(F("connecting to "));
  Serial.println(Config::serverHost);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(Config::serverHost, Config::serverPort)) {
    Serial.println(F("connection failed"));
    return;
  }
  
  // We now create a URI for the request
  String url = F("/stats");
  Serial.print(F("POSTING to URL: "));
  Serial.println(url);

  String messageBody = String(F("{\"version\": ")) + Config::codeVersion 
    + F(", \"temps\": ") + arrayToJson(data.temperature, Config::numReadingsUntilSend) 
    + F(", \"humidity\": ") + arrayToJson(data.humidity, Config::numReadingsUntilSend) 
    + F(", \"battery\": ") + arrayToJson(data.batteryLevel, Config::numReadingsUntilSend) 
    + F(", \"soilTemp\": ") + arrayToJson(data.soilTemperature, Config::numReadingsUntilSend) 
    + F("}\r\n");

  Serial.println(messageBody);
        
  client.print(String(F("POST ")) + url + F(" HTTP/1.1\r\n") +
        F("Host: ") + Config::hostName + F("\r\n") +
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


