/*
 TBD
*/

#include <HelperFncs.h>
#include <StartupFncs.h>

const unsigned int measureTime = 10 * 1000; // measure every X seconds (approx)
unsigned long previousTime = 0;

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
      timeClient.begin();
      separator();

      startWebServer();
      separator();

      pinMode(pinVCC, OUTPUT);
      previousTime = millis();
}

void loop() {
  
  MDNS.update();         // Check if there is a mDNS request
  ArduinoOTA.handle();   // Check if there is a OTA update request
  server.handleClient(); // Listen for HTTP requests from clients


  if((fileSizeTemp < 2E6) && (millis() - previousTime > measureTime)) { // if the time is right, measure and append
    File tempfile = LittleFS.open("/humidity.csv", "a");
    fileSizeTemp = tempfile.size();
    tempfile.close();

    Serial.print("Il file occupa: ");
    Serial.println(formatBytes(fileSizeTemp));
  
    timeClient.update();

    writeHumidity(measureHumidity(), timeClient.getFormattedTime(), timeClient.getEpochTime());

    previousTime = millis();
  }

}
