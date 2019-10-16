#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFiAP.h>
#include <HardwareSerial.h>
#include <pins_arduino.h>


const char * ssid = "ESPAP";
const char * password = "hellodaar";


String ledState,firing, tiltdeg, pandeg;
String mode, fire; //placeholder for "led" in absence of breadboard and such
int tilt = 0;
int pan = 0;

//destinations: PANT, FARM - the destination names for the pan&tilt and fire&arm mechanisms

AsyncWebServer server(80);


//Serial port definitions
//HardwareSerial Serial1(1); //declare first serial communication set - PANT
//HardwareSerial Serial2(2); //declare second serial communication set - FARM

//definition of TX/RX pins for serial communication
//#define RX1 4
//#define TX1 2
//#define RX2 16
//#define TX2 17

//Command manager

/*void command(String destination, String command, String value)
{
  switch (destination){
    case "PANT":
    {
      Serial1.print(command);   //print command to PANT
      Serial1.println(value);   //print value to PANT
      break;
    }
    case "FARM":
    {
      Serial2.print(command);   //print command to FARM
      Serial2.println(value);   //print value to FARM
      break;
    }
    default:
    {
      Serial.println("Destination not provided");
      break;
    }
  };
}*/

String processor(const String& var)  //html string variable processor - will be understood later.
{
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
  //Set up Serial Communications:
  Serial.begin(115200);
  //Serial1.begin(9600, SERIAL_8N1, RX1, TX1); //begin communication with PANT at 9600 baud
  //Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //begin communication with FARM at 9600 baud
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
  //IPAddress IP = WiFi.softAPIP();
  
  //Serial.println(IP);
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
    //command("FARM","ARM",mode);
    Serial.println(mode);
  });

  //path to toggle LED off:
  server.on("/fire", HTTP_GET, [](AsyncWebServerRequest *request){
    if (mode == "OFF")
    {
      fire = "NO";
    }
    else if (fire == "YES")
    {
      Serial.println("Cannot fire again. Disarm.");
    }
    else
    {
      fire = "YES";
      //command("FARM","FIRE",fire);
       Serial.println(fire);
    }
      //make this the fire button
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

//http get request setup for pan and tilt buttons
  server.on("/up", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode != "ON") && (tilt < 90))
    {
      tilt+=1;
      tiltdeg = String(tilt);
      //command("PANT","TILT",tiltdeg);
      Serial.println(tiltdeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
    
  });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode != "ON")&&(tilt > 0))
    {
      tilt-=1;
      tiltdeg = String(tilt);
      //command("PANT","TILT",tiltdeg);
      Serial.println(tiltdeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode!="ON")&&(pan > -60))
    {
      pan-=1;
      pandeg = String(pan);
      //command("PANT","PAN",pandeg);
      Serial.println(pandeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode!="ON")&&(pan < 60))
    {
      pan+=1;
      pandeg = String(pan);
      //command("PANT","PAN",pandeg);
      Serial.println(pandeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.on("/zero", HTTP_GET, [](AsyncWebServerRequest *request){
    if (mode != "ON")
    {
     tilt = 0;
    pan = 0;
    pandeg = String(pan);
    tiltdeg = String(tilt);
    //command("PANT","TILT",tiltdeg);
    //command("PANT","PAN",pandeg);
    Serial.println(tiltdeg);
    Serial.println(pandeg); 
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