#include <DallasTemperature.h>
#include <DHT.h>
#pragma once

struct Config {
    static const uint8_t codeVersion = 0x10; // Change/Increment this each version

    // Networking
    static constexpr char* ssid     = "McKearneys";
    static constexpr char* password = "";
    static constexpr char* hostName = "toms-office"; // TODO: CHANGE FOR EACH DEVICE!
    static constexpr char* serverHost = "192.168.0.106";
    static constexpr int serverPort = 8080;

    // sensor config
    static const int dhtPin = 12;
    static const int dhtType = DHT22;
    static const bool hasSoilTemp = false;
    static const int ds18b20Pin = 4;
    static const int batteryPin = A0;

    // app config
    static const bool shouldSleep = false;
    static const int minutesBetweenMeasurements = 1;
    static const int numTimesPerReading = 5; // times to read sensor per reading
    static const int numReadingsUntilSend = 30;
};

