/*
 TBD
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

ESP8266WebServer server(80);     // Create a webserver instance
WiFiUDP UDP;                     // Create an instance of the WiFiUDP class to send and receive

// change these values to match your network
const char* ssid = "Vodafone-34321872";       //  your network SSID (name)
const char* pass = "2smid42fpfix3pe";         // your network password
const char* mDNSname = "edera";               // mDNS adress

const int pinVCC = D0;     // pin that powers the sensor @ 3.3V
const int sensorPin = A0;  // analog pin to measure sensor value

const unsigned int measureTime = 5 * 1000; // measure every X seconds (approx)
unsigned long previousTime = 0;

int SensorValue[3];
float meanHumidity = 0.0;
int calcHumidity = 0;
float voltage = 0;

size_t fileSizeTemp;

IPAddress timeServerIP;          // ntp1.inrim.it NTP server address
const char* NTPServerName = "ntp1.inrim.it";
const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message
byte NTPBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

//---------------------USEFUL FUNCTIONS-----------------------

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  return String("Too many bytes");
}

int measureHumidity() {
  digitalWrite(pinVCC, HIGH);
  for (int i = 0; i < 3; i++) {                     //get three readings
    delay(100);                                     //time biding for stability
    SensorValue[i] = analogRead(sensorPin);
  }
  digitalWrite(pinVCC, LOW);
  meanHumidity = (SensorValue[0]+SensorValue[1]+SensorValue[2])/3.0;
  voltage = (meanHumidity/1023.0)*3.3;
  calcHumidity = round((((1.0/voltage)*2.48)-0.72)*100.0);
  calcHumidity = constrain(calcHumidity, 0, 100);
  return calcHumidity;
}

void writeHumidity(int actualHumidity) {
  Serial.print(F("Appending humidity to file: "));
  Serial.println(actualHumidity);
  File humLog = LittleFS.open("/humidity.csv", "a"); // Write the humidity to the csv file
  humLog.print(actualHumidity);
  humLog.print(',');
  humLog.close();
}

void separator() {
  Serial.println();
  Serial.println("<<>><<>><<>><<>><<>><<>><<>><<>><<>><<>><<>>");
  Serial.println();
}

void welcome() {
  Serial.printf("              ______\n   _        ,',----.`. WATERING WITH YOU\n  '.`-.  .-' '----. ||\n     `.`-'--------| ;; developed by\n       `.|--------|//  Luca Della Mora\n         |         /   2021\n         '--------' ");
  Serial.println();
}

//---------------------WEBSERVER FUNCTIONS-----------------------
/*
void handleRoot() { }

void handleNotFound(){ }

void startWebServer() {
  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}
*/

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

void startUDP() {
  Serial.println("Starting UDP");
  UDP.begin(123);                          // Start listening for UDP messages on port 123
  Serial.print("Local port:\t");
  Serial.println(UDP.localPort());
}

//---------------------NTP FUNCTIONS-----------------------

void sendNTPpacket(IPAddress& address) {
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);  // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  // send a packet requesting a timestamp:
  UDP.beginPacket(address, 123); // NTP requests are to port 123
  UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

uint32_t getTime() {
  if (UDP.parsePacket() == 0) { // If there's no response (yet)
    return 0;
  }
  UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  // Combine the 4 timestamp bytes into one 32-bit number
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
  // Convert NTP time to a UNIX timestamp:
  // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
  const uint32_t seventyYears = 2208988800UL;
  // subtract seventy years:
  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}

startNTP() {
  if(!WiFi.hostByName(NTPServerName, timeServerIP)) { // Get the IP address of the NTP server
    Serial.println("DNS lookup failed. Rebooting.");
    Serial.flush();
    ESP.reset();
  }
  Serial.print("Time server IP:\t");
  Serial.println(timeServerIP);
  
  Serial.println("\r\nSending first NTP request ...");
  sendNTPpacket(timeServerIP);
}

//---------------------MAIN PROGRAM-----------------------

void setup() {

      startSerial();
      separator();

      welcome();
      separator();

      startWiFi();
      separator();

      startmDNS();
      separator();

      startLittleFS();
      separator();

      startOTA();
      separator();

      startUDP();
      separator();

      previousTime = millis();
}

void loop() {

  MDNS.update();         // Check if there is a mDNS request
  ArduinoOTA.handle();   // Check if there is a OTA update request

  if((fileSizeTemp < 2E6) && (millis() - previousTime > measureTime)) { // if the time is right, measure and append
    File tempfile = LittleFS.open("/humidity.csv", "a");
    fileSizeTemp = tempfile.size();
    tempfile.close();

    Serial.print("Il file occupa: ");
    Serial.println(formatBytes(fileSizeTemp));

    writeHumidity(measureHumidity());

    previousTime = millis();
  }

}
