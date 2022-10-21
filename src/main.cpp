#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>


const char* ssid     = "RobotGouttières";
const char* password = "robotgouttieres";

AsyncWebServer server(80);

void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);

  //....Setup Access Point Wifi
  Serial.println("\n[*] Creating AP");
  // WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
   // Serial.print("[+] AP Created with IP Gateway ");
   // Serial.println(WiFi.softAPIP());

  //.....Setup SPIFFS
    if(!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while(file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }

    //----------------------------------------------------SERVER
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/style.css", "text/css");
  });

   server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 4; i++)
  {
    Serial.println("Hello World");
    delay(1000);
  }
  
  Serial.end();
  

  
}

