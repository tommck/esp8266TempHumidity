#include <DHT.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
 
const char* ssid     = "McKearneys";
const char* password = "";
const char* hostName = "Hoopy"; // TODO: CHANGE FOR EACH DEVICE!
 
const char* host = "192.168.0.106";

#define DHTTYPE DHT22
#define DHTPIN  12

ADC_MODE(ADC_VCC);

DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
float humidity, temp_f;  // Values read from sensor

float batteryLevel = -1.0;


void getTemperature() {
 
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  humidity = dht.readHumidity();          // Read humidity (percent)
  temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temp_f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}

void readBatteryLevel() {
  batteryLevel = ESP.getVcc();
}

void setup() {
  Serial.begin(115200);
  delay(100);
 
  // We start by connecting to a WiFi network
 
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
}

void readAllAndReport() {
  getTemperature();

  readBatteryLevel();

     Serial.print("Temp: ");
 Serial.println(temp_f);
 Serial.print("Humidity: ");
 Serial.println(humidity);
 Serial.print("Voltage: ");
 Serial.println(batteryLevel);

 
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 8080;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/stats";
  Serial.print("POSTING to URL: ");
  Serial.println(url);

  String messageBody = "{\"temp\": ";
  messageBody += temp_f;
  messageBody += ", \"humidity\": ";
  messageBody += humidity;
  messageBody += ", \"voltage\": ";
  messageBody += batteryLevel;
  messageBody += "}\r\n";       
        
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
        "Host: " + hostName + "\r\n" +
        "Content-Type: application/json\r\n" +
        "Content-Length: " +
        String(messageBody.length()) + "\r\n\r\n");
    client.print(messageBody);
  delay(500);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}
 
int value = 0;
 
void loop() {
  delay(5000);
  ++value;

  readAllAndReport();

}
