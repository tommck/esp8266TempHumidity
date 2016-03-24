#include <elapsedMillis.h>

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <QuickStats.h>

#include "Battery.h"
#include "Led.h"
#include "Dht22.h"
#include "Utils.h"

const char* ssid     = "McKearneys";
const char* password = "";
const char* hostName = "hoopy-inside"; // TODO: CHANGE FOR EACH DEVICE!
 
const char* host = "192.168.0.106";
const int httpPort = 8080;

#define DHTPIN  12
#define MICROSECONDS_PER_MINUTE 60*1000000  // 60 seconds * 1 million 

const int numberOfMinutesToSleep = 1;

const int numTimesPerReading = 5; // times to read sensor per fread
const int blueLedPin = 2;
const int numTimesPerReadingUntilSend = 30;

const uint8_t CODE_VERSION = 0x04; // Change/Increment this if we want to treat the EEPROM as "fresh"
struct Readings {
  uint8_t marker; // indicator that we're reading proper data
  uint8_t readingsFinished;

  int temperature[numTimesPerReadingUntilSend];
  int humidity[numTimesPerReadingUntilSend];
  int batteryLevel[numTimesPerReadingUntilSend];
};

// forward declarations
void InitializeStoredData(Readings& data);
int normalizeReadings(float* readings, int length);
String arrayToJson(int* arr, int length);
void SendData(Readings& data);

void setup() {
  elapsedMillis sinceStart;

  Serial.print("\nHoop house version: ");
  Serial.println(CODE_VERSION);

  // temp/humidity + battery
  Dht22 dht22(DHTPIN);
  Battery battery(A0);
  
  // LED for debugging
  Led led(2); // pin 2

  EEPROM.begin(4096);
  Serial.begin(115200);
  Utils::Delay(100);
  Serial.println(); // newline after wakeup junk

  // First, read the existing readings from the EEPROM
  Readings storedData;
  EEPROM.get(0, storedData);
  if (storedData.marker != CODE_VERSION) {
    Serial.println("No Readings marker, initializing");
    InitializeStoredData(storedData);
  }

  Serial.print("Num Readings So Far: ");
  Serial.println(storedData.readingsFinished);

  // read new values
  float 
    temperature[numTimesPerReading], 
    humidity[numTimesPerReading], 
    batteryLevels[numTimesPerReading];
      
  dht22.ReadTempAndHumidity(numTimesPerReading, temperature, humidity);
  battery.ReadLevels(numTimesPerReading, batteryLevels);

  // normalize values

  int temp = normalizeReadings(temperature, numTimesPerReading);
  int hum = normalizeReadings(humidity, numTimesPerReading);
  int batt = normalizeReadings(batteryLevels, numTimesPerReading);

  Serial.print(F("Normalized Temp/Hum/Battery: "));
  Serial.print(temp);
  Serial.print(F(" "));
  Serial.print(hum);
  Serial.print(F(" "));
  Serial.print(batt);
  Serial.println();

  int newIndex = storedData.readingsFinished;
  storedData.temperature[newIndex] = temp;
  storedData.humidity[newIndex] = hum;
  storedData.batteryLevel[newIndex] = batt;

  if (newIndex == numTimesPerReadingUntilSend-1) {

    Serial.println("Last Reading: uploading!");
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
  Serial.print(numberOfMinutesToSleep);
  Serial.println(F(" minutes. Zzzz..."));
  led.OnOff(false);

  Serial.print("Took this many millis: ");
  Serial.println(sinceStart);

  WakeMode wakeMode = WAKE_RF_DISABLED;
  if (storedData.readingsFinished == numTimesPerReadingUntilSend-1) {
    // wake up w/ Wifi next time
    wakeMode = WAKE_RF_DEFAULT;
  }

  // sleep for proper minutes minus processing time
  uint32_t timeToSleep = numberOfMinutesToSleep * MICROSECONDS_PER_MINUTE - sinceStart * 1000;
  
  ESP.deepSleep(timeToSleep, wakeMode);
}

void InitializeStoredData(Readings& data) {
    data.marker = CODE_VERSION;
    data.readingsFinished = 0;

    for(int i=0; i < numTimesPerReadingUntilSend; ++i) {
      data.temperature[i] = -1;
      data.humidity[i] = -1;
      data.batteryLevel[i] = -1;
    }
}


void loop() {
  // never gets here, so do nothing
}


///////// IMPLEMENTATION below

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
    + F(", \"temps\": ") + arrayToJson(data.temperature, numTimesPerReadingUntilSend) 
    + F(", \"humidity\": ") + arrayToJson(data.humidity, numTimesPerReadingUntilSend) 
    + F(", \"battery\": ") + arrayToJson(data.batteryLevel, numTimesPerReadingUntilSend) 
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


