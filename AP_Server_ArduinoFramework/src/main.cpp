#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFiAP.h>


const char * ssid = "ESPAP";
const char * password = "hellodaar";


String armState,firing, tiltdeg, pandeg, usrMESSAGE;
String mode, fire;
String response;
int tilt, pan, power, check; //define values for tilt angle, pan angle, power and check
int complete = 0;

//destinations: PANT, FARM - the destination names for the pan&tilt and fire&arm mechanisms

AsyncWebServer server(80);


//Serial port definitions
HardwareSerial PANT(1); //declare first serial communication set - PANT @uart1
HardwareSerial FARM(2); //declare second serial communication set - FARM @uart2

//definition of TX/RX pins for serial communication
#define RX1 4
#define TX1 2
#define RX2 16
#define TX2 17

//Command manager

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
      FARM.print(command);   //print command to FARM
      FARM.println(value);   //print value to FARM
      //Serial.println(value);
      break;
    }
    default:
    {
      Serial.println("Destination not provided");
      break;
    }
  };
}

/*void checkStatus(void)
{
  //send out a command and wait for a reply - will be programmed in during integration
  //delay(10) //delay for 10 seconds for the purposes of demonstration
  //set a result message
}*/

//html processor function - for the VARIABLES!!!

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
  return String();
}


void setup() {
  // put your setup code here, to run once:
  //Set up Serial Communications:
  Serial.begin(115200);
  PANT.begin(9600, SERIAL_8N1, RX1, TX1); //begin communication with PANT at 9600 baud
  FARM.begin(9600, SERIAL_8N1, RX2, TX2); //begin communication with FARM at 9600 baud
  //initialise modes
  mode = "OFF";
  fire = "NO";
  tiltdeg = "0";
  pandeg = "0";
  power = 0; 
  check = 0;
  tilt = 0;
  pan = 0;

  //SPIFFS Initialisation
  if(!SPIFFS.begin(true))
  {
  Serial.println("An Error has occurred while mounting SPIFFS");
  return;
  }

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  
  //Serial.println(IP);

  //path to html file for GET requests (html):

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (power) //check to see if powered on
    {
      request->send(SPIFFS, "/index.html", String(), false, processor);  
    }
    else
    {
      request->send(SPIFFS, "/startup.html", String(), false, processor);  
    }
    
  });

  //path to html file for GET requests (css):
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  //path to html file for power on
   server.on("/turnon", HTTP_GET, [](AsyncWebServerRequest *request){

    power = 1;
    //check = 1;

    //checkStatus(); //check the status of the system

    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //path to html file for power off
  server.on("/turnoff", HTTP_GET, [](AsyncWebServerRequest *request){
    power = 0;
    request->send(SPIFFS, "/startup.html", String(), false, processor);
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

    command(2,"ARM",mode);
    if (FARM.available())
      {
        response = FARM.readStringUntil('\n');
      }
      Serial.println(response);
    Serial.println(mode);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //path to enable firing:
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
      command(2,"FIRE",fire);
      if (FARM.available())
      {
        response = FARM.readStringUntil('\n');
      }
       Serial.println(response);
       Serial.println(fire);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

//http get request setup for pan and tilt buttons
  server.on("/up", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode != "ON") && (tilt < 90))
    {
      tilt+=1;
      tiltdeg = String(tilt);
      command(1,"TILT",tiltdeg);
      complete = 0;
      if (PANT.available())
      {
        response = PANT.readStringUntil('\n');
      }
      Serial.println(response);
      Serial.println(tiltdeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
    
  });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode != "ON")&&(tilt > 0))
    {
      tilt-=1;
      tiltdeg = String(tilt);
      command(1,"TILT",tiltdeg);
      if (PANT.available())
      {
        response = PANT.readStringUntil('\n');
      }
      Serial.println(tiltdeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode!="ON")&&(pan > -60))
    {
      pan-=1;
      pandeg = String(pan);
      command(1,"PAN",pandeg);
      if (PANT.available())
      {
        response = PANT.readStringUntil('\n');
      }
      Serial.println(response);
      Serial.println(pandeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode!="ON")&&(pan < 60))
    {
      pan+=1;
      pandeg = String(pan);
      command(1,"PAN",pandeg);
      if (PANT.available())
      {
        response = PANT.readStringUntil('\n');
      }
      Serial.println(response);
      Serial.println(pandeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  //path to zero the pan/tilt mechanisms
  server.on("/zero", HTTP_GET, [](AsyncWebServerRequest *request){
    if (mode != "ON")
    {
    tilt = 0;
    pan = 0;
    pandeg = String(pan);
    tiltdeg = String(tilt);
    command(1,"TILT",tiltdeg);
    if (FARM.available())
      {
        response = FARM.readStringUntil('\n');
      }
    Serial.println(response);
    command(1,"PAN",pandeg);
    if (PANT.available())
      {
        response = PANT.readStringUntil('\n');
      }
    Serial.println(response);
    Serial.println(tiltdeg);
    Serial.println(pandeg); 
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.begin();
}

void loop() {

  //if (PANT.available())
  //{
   // response = PANT.readStringUntil('&');
    //Serial.println(response);
  //}
  // put your main code here, to run repeatedly:
  //delay 20 ms
  //send out commands via uart
  //maybe put checking of json files here and sending of UART commands
}