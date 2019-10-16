#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFiAP.h>
#include <string.h>

const char * ssid = "ESPAP";
const char * password = "hellodaar";


String ledState,firing, tiltdeg, pandeg;
String mode, fire; //placeholder for "led" in absence of breadboard and such
int tilt = 0;
int pan = 0;

AsyncWebServer server(80);


String processor(const String& var)  //html string variable processor - will be understood later.
{
  Serial.println(var);  //how do html placeholder variables work??
  if (var == "STATE")
  {
    if (mode == "ON")
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
  if (var =="FIRE")
  {
    if (fire == "YES")
    {
      firing = "YES";
    }
    else
    {
      firing = "NO";
    }
    return firing;
  }
  if (var == "DEGREE")
  {
    tiltdeg = String(tilt);
    return tiltdeg;
  }
  if (var == "HOZ")
  {
    pandeg = String(pan);
    return pandeg;
  }
  return String();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mode = "OFF";
  fire = "NO";
  tiltdeg = "0";
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
  server.on("/arm", HTTP_GET, [](AsyncWebServerRequest *request){
    if (mode == "ON")
      {
        mode = "OFF";
        fire = "NO";
      }
      else
      {
        mode = "ON";
      }
       //turn this into a toggle?   
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //path to toggle LED off:
  server.on("/fire", HTTP_GET, [](AsyncWebServerRequest *request){
    if (mode == "OFF")
    {
      fire = "NO";
    }
    else
    {
      fire = "YES";
    }
      //make this the fire button
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

//http get request setup for pan and tilt buttons
  server.on("/up", HTTP_GET, [](AsyncWebServerRequest *request){
    if (tilt < 90)
    {
      tilt+=1;
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request){
    if (tilt > 0)
    {
      tilt-=1;
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    if (pan > -60)
    {
      pan-=1;
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    if (pan < 60)
    {
      pan+=1;
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  //delay 20 ms
  //send out commands via uart
  //maybe put checking of json files here and sending of UART commands
}