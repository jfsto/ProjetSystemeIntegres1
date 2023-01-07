#include <Arduino.h>
#include <Stepper.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#define DEBUG

// Serveur
const char *ssid = "RobotGouttières"; //Nom du wifi
const char *password = "robotgouttieres"; //mot de passe
String mode = "Automatique"; 
String action = "Stop";
int vitesse;
int vinit = 0;

AsyncWebServer server(80);

// Moteur PWM
int pwmChannel = 5;   // Choisit le canal 0
int frequence = 1000; // Fréquence PWM de 1 KHz
int resolution = 8;   // Résolution de 8 bits, 256 valeurs possibles
// In out motor
const int ENA = 23; // activer désactiver le moteur + gestion vitesse PWM
const int IN1 = 18; // IN1 = 1 -> avancer
const int IN2 = 19; // IN2 = 1 -> reculer /\si les 2 à 1 alors rien

// Out LED
#define LED_PIN 22 //GPIO22 pour la led

// Extérieur
#define CAPTSAC 33     // entrée capteur à hauteur du sac de récupération => PIN33
#define FINDECOURSE 32 // entrée capteur à l'avant du robot, 1= robot en butée contre un objet/gouttière => PIN 32
#define BATTERIE 34   //Entré ADC pour mesurer la charge de batterie


// //Interupt
hw_timer_t *timer_AV5s = NULL;
int Flag_5s = 0; //Flag pour l'interrupt
int count = 1; // compter 1x 5s puis 2x5s
int temps = 5000000; // temps = 5 secondes
hw_timer_t *timer_FDC = NULL;
boolean Flag_FDC; //flag si Fin de course détecte front montant

// servo
Servo servo1; // rotation de 180° vidage de la pelle
Servo servo2; // rotation de 90° de la pelle audessus du sac

#define SERVO1_PIN 25 // Define directive donc pas de ; et pour économiser de la place dans la mémoire
#define SERVO2_PIN 26 // dans les bons compilateur il n'y a plus de différence entre const et define sauf que avec const on peut définir le type et donc vérifier pour le debuggage

#define minUS 500  // valeur min pour PWM SG90
#define maxUS 2400 // valeur max pour PWM SG90
int pos;
// Vérin electrique
//  Number of steps per output rotation
const int stepsPerRevolution = 2800; // 3000= full course
// Create Instance of Stepper library
Stepper myStepper(stepsPerRevolution, 15, 2, 0, 4); // GPIO15=IN1_AC=(out1 fil vert) GPIO2=IN2_AC=(out2 Noir), GPIO0=IN3_AC=(out3)rouge, GPIO4=IN4_AC= (out4 bleu)
//

/////////////Fonctions

void Avancer();
void Reculer();
void Acceleration();
void Stop();
void AllerPelle();
void RetourPelle();
void IRAM_ATTR Callback_5s();
void IRAM_ATTR MUR();
String GetBatterie();

void setup()
{
  // ###Debug###//----------------------------------------------------------------
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
  if (!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }
#endif
  File root = SPIFFS.open("/");
  File file = root.openNextFile();

#ifdef DEBUG
  while (file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }
#endif
  //----------------------------------------------------SERVER-------------------------------------------------
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/style.css", "text/css"); });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/script.js", "text/javascript"); });

  server.on("/jquery-3.6.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/jquery-3.6.1.min.js", "text/javascript"); });

  server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    mode = "Automatique";
#ifdef DEBUG
    Serial.println(mode);
#endif
    request->send(200); });

  server.on("/manu", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    mode = "Manuel";
    action="stop";
#ifdef DEBUG
    Serial.println(mode);
#endif
    request->send(200); });

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
    request->send(200); });

  server.on("/avancer", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    action="Avancer";
#ifdef DEBUG
    Serial.println(action);
#endif
    request->send(200); });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    action="Stop";
#ifdef DEBUG
    Serial.println(action);
#endif
    request->send(200); });

  server.on("/reculer", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    action="Reculer";
#ifdef DEBUG
    Serial.println(action);
#endif
    request->send(200); });

  server.on("/vider", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    action="Vider";
#ifdef DEBUG
    Serial.println(action);
#endif
    request->send(200); });

  server.on("/synchromode", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", mode); });

  server.on("/getbatterie", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain",GetBatterie()); });

  server.on("/vitesse", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if(request->hasParam("valeurvitesse", true))
    {
      String message;
      message = request->getParam("valeurvitesse", true)->value();
      vitesse = message.toInt();
    }
#ifdef DEBUG
    Serial.println(vitesse);
#endif
    request->send(204); });

  server.begin();
  //----------------------------------------------------END SERVER-------------------------------------------------
  //----------------------------------------------------Cycle-------------------------------------------------
  //----motor-----------
  //  Configuration du canal 0 avec la fréquence et la résolution choisie
  ledcSetup(pwmChannel, frequence, resolution);
  // Assigne le canal PWM au pin 23
  ledcAttachPin(ENA, pwmChannel);
  ledcWrite(pwmChannel, 0);

  pinMode(IN1, OUTPUT); // pin 19 = sortie pour IN1
  pinMode(IN2, OUTPUT); // pin 20 = sortie pour IN2

  digitalWrite(IN1, 0); //initialise l'état à 0
  digitalWrite(IN2, 0);

  //--------extérieur------------
  pinMode(FINDECOURSE, INPUT_PULLDOWN); 
  attachInterrupt(FINDECOURSE,MUR,RISING); //interruption sur pin si le fin de course détecte le mur
  pinMode(CAPTSAC, INPUT_PULLDOWN);

  //------LED--------------
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0);

  //------Servo----------
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  servo1.setPeriodHertz(50); // Standard 50hz servo
  servo2.setPeriodHertz(50); // Standard 50hz servo

  //-------Verin----------------
  // set the speed at 20 rpm:
  myStepper.setSpeed(20);

  //--------Interrupt-----------
  timer_AV5s = timerBegin(2, 80, true);
  timerAttachInterrupt(timer_AV5s, &Callback_5s, true);
  timerAlarmWrite(timer_AV5s, temps, true);

  // timer_FDC = timerBegin(3, 80, true);
  // timerAttachInterrupt(timer_FDC, &MUR, true);
  // timerAlarmWrite(timer_FDC, 200, true);
  // timerAlarmEnable(timer_FDC);
}

void loop()
{
  digitalWrite(LED_PIN, 1);
  if (mode == "Automatique" && action == "Start") //mode automatique et on lance un cycle
  {
    timerAlarmEnable(timer_AV5s); //démarre le timer 5s
    Avancer(); 
    Serial.print(Flag_5s);
    if (Flag_5s > count) //si le flag est plus grand que count -> 5s*x écoulées donc on doit vider
    {
      Stop();
      while (digitalRead(CAPTSAC) == 0)//recule jusqu'au sac de récupération
      {
        Reculer();
      }
      Stop();
      AllerPelle();
      delay(1000);
      RetourPelle();
      Flag_5s = 0; 
      count++;
      timerAlarmDisable(timer_AV5s);
      // temps = temps + 5000000
    }
    else if (Flag_FDC == 1) //si on touche la fin de la gouttière
    {
      Stop();
      while (digitalRead(CAPTSAC) == 0) //on va vider le sac
      {
        Reculer();
      }
      Stop();
      AllerPelle();
      delay(1000);
      RetourPelle();
      action = "Stop"; //vu qu'on a parcouru toute la gouttière on s'arrête
      Serial.println(action);
      count = 0; //remise à 0 du compteur car on change de gouttière
      Flag_FDC = 0;
    }
  }
  else if (mode == "Manuel")
  {
    timerAlarmDisable(timer_AV5s);
    count = 0;
    if (action == "Avancer")
    {
      Avancer();
    }
    else if (action == "Reculer")
    {
      Reculer();
    }
    else if (action == "Vider")
    {
      AllerPelle();
      delay(2000);
      RetourPelle();
    }
    else if (action == "Stop")
    {
      Stop();
    }
  }
  else if (mode == "Automatique" && action == "Stop")
  {
    Stop();
  }
}

void Avancer()
{
  // Sens avancer
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  // Créer la tension en sortie choisi
  ledcWrite(pwmChannel, vitesse); // vitesse dépend ce la valeur de vitesse auto = 50% (normal) lent 33% max 90%
}
void Reculer()
{
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 2);
  ledcWrite(pwmChannel, vitesse);
}
// void Acceleration()
// {
//   if(vitesse = 110 && vinit<vitesse)
//   {
//     ledcWrite(pwmChannel, 80);
//     delay(100);
//     ledcWrite(pwmChannel, vitesse);
//   }
//   else if(vitesse = 150 && vinit<vitesse)
//   {
//     ledcWrite(pwmChannel, 80);
//     delay(100);
//     ledcWrite(pwmChannel, 110);
//     delay(100);
//     ledcWrite(pwmChannel, 130);
//     delay(100);
//     ledcWrite(pwmChannel, vitesse);
//   }
//   else if(vitesse = 220 && vinit<vitesse)
//   {
//     ledcWrite(pwmChannel, 80);
//     delay(100);
//     ledcWrite(pwmChannel, 110);
//     delay(100);
//     ledcWrite(pwmChannel, 140);
//     delay(100);
//     ledcWrite(pwmChannel, 170);
//     delay(100);
//     ledcWrite(pwmChannel, 200);
//     delay(100);
//     ledcWrite(pwmChannel, vitesse);
//   }
// for (int vinit = 80; vinit < vitesse; vinit = vinit+20) // il boucle donc accelere puis ralenti puis réaccelere
// {
//   ledcWrite(pwmChannel, vinit);
//   delay(500);
// }
// ledcWrite(pwmChannel, vitesse);

// }
void Stop()
{
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  ledcWrite(pwmChannel, 0);
}
void AllerPelle() // mouvement à vérifier en fonction des anglais et des mouvement
{
  // monter vérin (sortie vérin)
  myStepper.step(-stepsPerRevolution);
  delay(500);

  servo1.attach(SERVO1_PIN, minUS, maxUS);
  servo2.attach(SERVO2_PIN, minUS, maxUS);

  for (pos = 0; pos <= 90; pos++)
  { // sweep from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo2.write(pos);
    delay(1); // waits 20ms for the servo to reach the position
  }
  delay(200);
  for (pos = 0; pos <= 180; pos++)
  { // sweep from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo1.write(pos);
    delay(1); // waits 20ms for the servo to reach the position
  }
  delay(200);
  servo1.detach();
  servo2.detach();
}
void RetourPelle()
{
  servo1.attach(SERVO1_PIN, minUS, maxUS);
  servo2.attach(SERVO2_PIN, minUS, maxUS);
  for (pos = 180; pos >= 0; pos--)
  { // sweep from 180 degrees to 0 degrees
    servo1.write(pos);
    delay(1);
  }
  delay(200);
  for (pos = 90; pos >= 0; pos--)
  { // sweep from 180 degrees to 0 degrees
    servo2.write(pos);
    delay(1);
  }
  delay(200);
  servo1.detach();
  servo2.detach();

  // descente vérin (rentrer verin)
  // step one revolution in one direction:
  myStepper.step(stepsPerRevolution);
  delay(500);
}

String GetBatterie()
{
  int Niveau;
  Niveau=analogRead(BATTERIE);
  
  return String(Niveau*100/4095);
}

void IRAM_ATTR Callback_5s()
{
  Flag_5s++;
}

void IRAM_ATTR MUR()
{
  if (digitalRead(FINDECOURSE) == 1)
  {
    Flag_FDC = true;
  }
}
