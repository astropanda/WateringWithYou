#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>


const int pinVCC = D0;     // pin that powers the sensor @ 3.3V
const int sensorPin = A0;  // analog pin to measure sensor value


int SensorValue[3];
float meanHumidity = 0.0;
int calcHumidity = 0;
float voltage = 0;

size_t fileSizeTemp;

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

void writeHumidity(int actualHumidity, String ntpTime) {
  Serial.print(F("Appending humidity to file: "));
  Serial.print(actualHumidity);
  Serial.print(" @ ");
  Serial.println(ntpTime);

  File humLog = LittleFS.open("/humidity.csv", "a"); // Write the humidity to the csv file
  humLog.print(ntpTime);
  humLog.print(',');
  humLog.println(actualHumidity);
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

//---------------------WEBSERVER FUNCTIONS (Not implemted)-----------------------
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



