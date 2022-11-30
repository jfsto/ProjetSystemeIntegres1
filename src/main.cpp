#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
// #define DEBUG

const char* ssid     = "RobotGouttières";
const char* password = "robotgouttieres";
String mode = "Automatique";
String action ="Stop";
int vitesse;

//Moteur PWM
int pwmChannel = 0; //Choisit le canal 0
int frequence = 1000; //Fréquence PWM de 1 KHz
int resolution = 8; // Résolution de 8 bits, 256 valeurs possibles
//In out motor
const int ENA = 23; //activer désactiver le moteur + gestion vitesse PWM
const int IN1 =  18; //IN1 = 1 -> avancer 
const int IN2 = 19; //IN2 = 1 -> reculer /\si les 2 à 1 alors rien

TaskHandle_t Task1;

AsyncWebServer server(80); 

/////////////Fonctions

void Avancer();
void Reculer();
void Stop();

void setup() {
  // put your setup code here, to run once:
#ifdef DEBUG
  Serial.begin(9600);
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());
#endif

  //....Setup Access Point Wifi
#ifdef DEBUG
  Serial.println("\n[*] Creating AP");
#endif

  WiFi.softAP(ssid, password);

#ifdef DEBUG
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
#endif
  //.....Setup SPIFFS
#ifdef DEBUG
    if(!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }
#endif
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
#ifdef DEBUG
  while(file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }
#endif
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

  server.on("/jquery-3.6.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/jquery-3.6.1.min.js", "text/javascript");
  });


  server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    mode = "Automatique";
    #ifdef  DEBUG
    Serial.println(mode);
    #endif
    request->send(200);
  });

  server.on("/manu", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    mode = "Manuel";
    #ifdef  DEBUG
    Serial.println(mode);
    #endif
    request->send(200);
  });

  server.on("/startstop", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if (action=="Stop")
    {
      action="Start";
    }
    else{
      action="Stop";
    }
    #ifdef DEBUG
    Serial.println(action);
    #endif
    request->send(200);
  });

  server.on("/avancer", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    action="Avancer";
    #ifdef DEBUG
    Serial.println(action);
    #endif
    request->send(200);
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    action="Stop";
    #ifdef DEBUG
    Serial.println(action);
    #endif
    request->send(200);
  });

  server.on("/reculer", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    action="Reculer";
    #ifdef DEBUG
    Serial.println(action);
    #endif
    request->send(200);
  });

    server.on("/vider", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    action="Vider";
    #ifdef DEBUG
    Serial.println(action);
    #endif
    request->send(200);
  });

  server.on("/synchromode", HTTP_GET, [](AsyncWebServerRequest *request)
  {
   
    request->send(200, "text/plain", mode);
  });

  server.on("/vitesse", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(request->hasParam("valeurvitesse", true))
    {
      String message;
      message = request->getParam("valeurvitesse", true)->value();
      vitesse = message.toInt();
    }
    #ifdef DEBUG
    Serial.println(vitesse);
    #endif
    request->send(204);
  });



server.begin();

////////////////////////////////////Cycle
//motor
    // Configuration du canal 0 avec la fréquence et la résolution choisie
    ledcSetup(pwmChannel, frequence, resolution);
    // Assigne le canal PWM au pin 23
    ledcAttachPin(ENA, pwmChannel);
    ledcWrite(pwmChannel,0);

    pinMode(IN1,OUTPUT); //pin 19 = sortie pour IN1
    pinMode(IN2,OUTPUT); //pin 20 = sortie pour IN2
    
    digitalWrite(IN1,0);
    digitalWrite(IN2,0);


}

void loop() {
  // put your main code here, to run repeatedly:
  //  Serial.print("loop() running on core ");
  // Serial.println(xPortGetCoreID());
 if(mode == "Manuel"){
        if(action == "Avancer"){
            Avancer();
        }
        else if(action == "Reculer"){
            Reculer();
        }
        else if(action == "Vider"){
            // AllerPelle();
            // RetourPelle();
        }
        else if(action == "Stop"){
            Stop();
        }
        else if(action == "Lent"){
            vitesse = 90; // +/- 33% de sa vitesse max
        }
        else if(action == "Normal"){
            vitesse = 127;
        }
        else if(action == "Rapide"){
            vitesse = 200;
        }
    }
  
}

void Avancer()
{
    //Sens avancer 
    digitalWrite(IN1,1);
    digitalWrite(IN2,0);
    // Créer la tension en sortie choisi
    ledcWrite(pwmChannel, vitesse); //vitesse dépend ce la valeur de vitesse auto = 50% (normal) lent 33% max 90%
}
void Reculer()
{
    digitalWrite(IN1,0);
    digitalWrite(IN2,2);
    ledcWrite(pwmChannel, vitesse);  
}
void Stop()
{
    digitalWrite(IN1,0);
    digitalWrite(IN2,0);
    ledcWrite(pwmChannel,0);
}

