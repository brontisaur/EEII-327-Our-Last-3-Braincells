#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFiAP.h>

const char * ssid = "ESPAP";
const char * password = "hellodaar";


String ledState;
String mode; //placeholder for "led" in absence of breadboard and such

AsyncWebServer server(80);


String processor(const String& var)  //html string variable processor - will be understood later.
{
  Serial.println(var);
  if (var == "STATE")
  {
    if (mode = "ON")
    {
      ledState = "ON";
    }
    else 
    {
      ledState = "OFF";
    }
    Serial.println(ledState);
    return ledState;

  }
  return String();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mode = "ON";

  //SPIFFS Initialisation
  if(!SPIFFS.begin(true))
  {
  Serial.println("An Error has occurred while mounting SPIFFS");
  return;
  }
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  
  Serial.println(IP);
  //path to html file for GET requests (html):

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //path to html file for GET requests (css):
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  //path to toggle LED on:
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    mode = "ON";    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //path to toggle LED off:
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    mode = "OFF";    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });


  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  //maybe put checking of json files here and sending of UART commands
}