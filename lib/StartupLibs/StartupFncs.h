#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>


const char* ssid = "Vodafone-34321872";       //  your network SSID (name)
const char* pass = "2smid42fpfix3pe";         // your network password
const char* mDNSname = "edera";               // mDNS adress


//---------------------STARTUP FUNCTIONS-----------------------

void startSerial() {
  Serial.begin(9600);
  delay(1000);
  Serial.println();
  Serial.println(F("Serial started at 9600 baud"));
}

void startWiFi() { // Connect to WiFI Network
      Serial.print(F("Connecting to "));
      Serial.println(ssid);
      WiFi.begin(ssid, pass);

      while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(500); }

      Serial.println("");
      Serial.println(F("[CONNECTED]"));
      Serial.print(F("[IP "));
      Serial.print(WiFi.localIP());
      Serial.println(F("]"));
}

void startmDNS() { // Start the mDNS responder
      if (!MDNS.begin("edera")) { Serial.println(F("Error setting up MDNS responder!")); }
      else  { Serial.print(F("mDNS responder started at: http://")); Serial.print(mDNSname); Serial.println(".local");}
}

void startLittleFS() { // Start the LITTLEFS and list all contents
  LittleFS.begin();
  Serial.println(F("LittleFS started. Contents:"));
  Dir dir = LittleFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
}

void startOTA() {
  ArduinoOTA.setHostname("edera8266");
  ArduinoOTA.setPassword("luca");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}
