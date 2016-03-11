#include <DHT.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
 
const char* ssid     = "McKearneys";
const char* password = "";
const char* hostName = "Hoopy"; // TODO: CHANGE FOR EACH DEVICE!
 
const char* host = "192.168.0.106";
const int httpPort = 8080;

//#define BATTERY_TEST

#define DHTTYPE DHT22
#define DHTPIN  12

#define POLL_INTERVAL_MICROSECONDS 15*60*1000000  // Minutes * 60 seconds * 1 million 

DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

const int numReadings = 5;
float temps[numReadings];
float hums[numReadings];
float batteryLevels[numReadings];

void getTempHum() {
  float temp = -1;
  float humidity = -1;

  for (int i=0; i < numReadings; i++) {
#ifndef BATTERY_TEST
    delay(2000);
    
    temp = dht.readTemperature(true);     // Read temperature as Fahrenheit
    if (isnan(temp)) {
      temp = -1;
    }
    humidity = dht.readHumidity();          // Read humidity (percent)
    if (isnan(humidity)) {
      humidity = -1;
    }
    Serial.print("Temp/Hum: ");
    Serial.print(temp);
    Serial.print("/");
    Serial.println(humidity);
#endif

    temps[i] = temp;
    hums[i] = humidity;
  }
}

void readBatteryLevel() {

  for(int i=0; i < numReadings; ++i) {
    int level = analogRead(A0);
   
    batteryLevels[i] = level;
  }
}

String arrayToJson(float arr[]) 
{
  String result = "[";
  for(int i=0; i < numReadings; i++) 
  {
    float val = arr[i];
    result += arr[i];
    if (i < numReadings-1) {
      result += ",";
    }
  }
  result += "]";

  return result;
}

void readAllAndReport() {
  Serial.println("");

  getTempHum();

  readBatteryLevel();

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/stats";
  Serial.print("POSTING to URL: ");
  Serial.println(url);

  String messageBody = String("{\"temps\": ") 
    + arrayToJson(temps) + ", \"humidity\": " 
    + arrayToJson(hums) + ", \"battery\": "
    + arrayToJson(batteryLevels) + "}\r\n";       

  Serial.println(messageBody);
        
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
        "Host: " + hostName + "\r\n" +
        "Content-Type: application/json\r\n" +
        "Content-Length: " +
        String(messageBody.length()) + "\r\n\r\n");
  client.print(messageBody);

  // wait for the server to do its thing
  delay(500);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}



void setup() {
  Serial.begin(115200);
  delay(100);

#ifndef BATTERY_TEST
  // We start by connecting to a WiFi network
  readAllAndReport();

  Serial.println("Zzzz...");
  ESP.deepSleep(POLL_INTERVAL_MICROSECONDS, WAKE_RF_DEFAULT);
#endif
}


void loop() {
  // never gets here, so do nothing
#ifdef BATTERY_TEST
  readAllAndReport();
  delay(60000);
#endif
}
