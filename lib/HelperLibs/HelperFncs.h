#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);     // Create a webserver instance

const uint8_t pinVCC = D2;     // pin that powers the sensor @ 3.3V
const uint8_t sensorPin = A0;  // analog pin to measure sensor value

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

void writeHumidity(int actualHumidity, String ntpTime, unsigned long unixTime) {
  Serial.print(F("Appending humidity to file: "));
  Serial.print(actualHumidity);
  Serial.print(" @ ");
  Serial.println(ntpTime);

  File humLog = LittleFS.open("/humidity.csv", "a"); // Write the humidity to the csv file
  humLog.print(unixTime);
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

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
    Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(LittleFS.exists(pathWithGz) || LittleFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(LittleFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = LittleFS.open(path, "r");                    // Open the file
    server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;   
}

void startWebServer() {  

  server.onNotFound([]() {                             
    if (!handleFileRead(server.uri())) {
      String errpath = "/404.html";          
      File errfile = LittleFS.open(errpath, "r");                   
      server.streamFile(errfile, "text/html");   
      errfile.close();                                          
      Serial.println(String("\tSent 404 page"));
    }                                                 
  });

  server.begin();
  Serial.println("HTTP server started");
}



