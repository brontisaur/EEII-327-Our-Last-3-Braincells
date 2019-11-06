#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFiAP.h>


//Set up variables for AP SSID and Password:

const char * ssid = "ESPAP";
const char * password = "hellodaar";

//Set up an Async Server Listening on Port 80:

AsyncWebServer server(80);

//state variable definitions:

//display string and command variable definitions:

//String Processor for variables in the html var array: (these are variable strings displayed onscreen)

String processor(const String& var)
{
  if (var == "STATE")
  {
    if (mode == "ON")
    {
      armState = "ON";
    }
    else 
    {
      armState = "OFF";
    }
    return armState;
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
  if (var == "MESSAGE")
  {
    return usrMESSAGE;   //Message to User
  }
  if (var == "PERCENT")
  {
    return armDEGREE;
  }
  return String();
}


//Command Structure
void command(int destination, String command, String value)
{
  switch (destination){
    case 1:
    {
      PANT.print(command);   //print command to PANT
      PANT.println(value);   //print value to PANT
      //Serial.println(value);
      break;
    }
    case 2:
    {
      FARM.println(command);   //print command to FARM
      //FARM.println(value);   //print value to FARM
      //Serial.println(value);
      Serial.println(command);
      break;
    }
    default:
    {
      Serial.println("Destination not provided");
      break;
    }
  };
}

void setup() {

//Serial Setup

Serial.begin(115200);

//Spiffs mount check

if(!SPIFFS.begin(true))
  {
  Serial.println("An Error has occurred while mounting SPIFFS");
  return;
  }

//AP setup

WiFi.softAP(ssid, password);
IPAddress IP = WiFi.softAPIP();



}

void loop() {
  // put your main code here, to run repeatedly:
}