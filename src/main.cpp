/*
 TBD
*/

#include <HelperFncs.h>
#include <NtpFncs.h>
#include <StartupFncs.h>


ESP8266WebServer server(80);     // Create a webserver instance

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
