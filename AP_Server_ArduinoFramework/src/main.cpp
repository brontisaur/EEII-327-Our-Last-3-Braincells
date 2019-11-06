#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFiAP.h>
#include <Arduino.h>


const char * ssid = "ESPAP";
const char * password = "hellodaar";


String armState,firing, tiltdeg, pandeg, armDEGREE, usrMESSAGE;
String mode, fire;
String response;
String armDEGCOM;
int tilt, pan, armdeg, power, check; //define values for tilt angle, pan angle, power and check
int panalive, firealive;
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

//declaration of GPIO LED Pins
#define motorSwitch 27
#define microSwitch 26
#define powerLED 25
#define allGOOD 32
#define intervene 33

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

void checkStatus(void)
{
  panalive = 0;
  firealive = 0;
  PANT.println("CHEC");
  FARM.println("CHEC");
  if (PANT.available())
  {
    response = PANT.readStringUntil('&');
    if (response.startsWith("ON",0))
    {
      panalive = 1;
    }
  }
  if (PANT.available())
  {
    response = FARM.readStringUntil('&');
    if (response.startsWith("ON",0))
    {
      firealive = 1;
    }
  }
  //send out a command and wait for a reply - will be programmed in during integration
  //delay(10) //delay for 10 seconds for the purposes of demonstration
  //set a result message
}

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
  if (var == "PERCENT")
  {
    return armDEGREE;
  }
  return String();
}


void setup() {
  // put your setup code here, to run once:

  //Set Up GPIO pins
  pinMode(motorSwitch, OUTPUT); //set up motor switch pin as output
  pinMode(microSwitch, OUTPUT); //set up micro switch pin as output
  pinMode(powerLED, OUTPUT); //set up power led pin as output
  pinMode(allGOOD, OUTPUT); //set up allGOOD led pin as output
  pinMode(intervene, OUTPUT); //set up intervene led pin as output


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
  armdeg = 50;

  armDEGREE = String(armdeg);

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

  server.on("/ret", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(intervene, LOW);
    if ((mode == "ON")&&(fire == "NO"))
    {
      mode = "OFF";
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //path to html file for power on
   server.on("/turnon", HTTP_GET, [](AsyncWebServerRequest *request){

    power = 1;
    digitalWrite(powerLED, HIGH); //indicate power has been turned on
    digitalWrite(microSwitch, HIGH); //turn on microcontrollers
    check = 1;
    delay(1000);
    digitalWrite(intervene, HIGH);
   // checkStatus(); //check the status of the system
    //else if (timeout?) show warning screen: user please restart!
   /* if ((firealive == 1)&&(panalive == 1))
    {
      digitalWrite(motorSwitch, HIGH); // turn on motors
      digitalWrite(allGOOD, HIGH); //indicate that all is well with the system and functional
      digitalWrite(intervene, LOW);  
    }*/
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //path to html file for power off
  server.on("/turnoff", HTTP_GET, [](AsyncWebServerRequest *request){
    if (mode == "OFF")
    {
      power = 0;
      firealive = 0;
      panalive = 0;
      digitalWrite(allGOOD, LOW); //turn off all-good signal
      digitalWrite(motorSwitch, LOW); //turn off motors
      digitalWrite(microSwitch, LOW); //turn off micros
      digitalWrite(powerLED, LOW); //turn off power LED
      request->send(SPIFFS, "/startup.html", String(), false, processor);  
    }
    else if (mode == "ON")
    {
      usrMESSAGE = "The ATRED is still in its armed state, please disarm the ATRED and try again.";
      digitalWrite(intervene, HIGH);
      request->send(SPIFFS, "/message.html", String(), false, processor);
    }
  });

  //path to toggle LED on:
  server.on("/arm", HTTP_GET, [](AsyncWebServerRequest *request){
    if (mode == "ON")
      {
        if (fire == "NO")
        {
          digitalWrite(intervene, HIGH);
          usrMESSAGE = "Please remove the ammunition from the ATRED and take the elastic off the trigger before you continuing. OL3BC takes no responsibility for stupidity.";
          request->send(SPIFFS, "/message.html", String(), false, processor);
        }
        else if (fire == "YES")
        {
          mode = "OFF";
          command(2, "DARM", mode);
          fire = "NO";
          digitalWrite(intervene, HIGH);
          request->send(SPIFFS, "/index.html", String(), false, processor);
        }
      }
      else if (mode == "OFF")
      {
        //mode = "INTERMEDIATE";
        command(2,"ARM1",mode);
      /* if (FARM.available())
      {
        response = FARM.readStringUntil('&');
      }*/
        digitalWrite(intervene, HIGH);
        usrMESSAGE = "Please put the elastic in place over the trigger before continuing";
        request->send(SPIFFS, "/degree.html", String(), false, processor);
        
      }
       //turn this into a toggle?   
   // Serial.println(response);
   // Serial.println(mode);
  });

  server.on("/more", HTTP_GET, [](AsyncWebServerRequest *request){
    if (armdeg < 100)
    {
      armdeg+=1;
      armDEGREE = String(armdeg);
    }
    request->send(SPIFFS, "/degree.html", String(), false, processor);
    
  });

  server.on("/less", HTTP_GET, [](AsyncWebServerRequest *request){
    if (armdeg > 0)
    {
      armdeg-=1;
      armDEGREE = String(armdeg);
    }
    request->send(SPIFFS, "/degree.html", String(), false, processor);
    
  });

  server.on("/degret", HTTP_GET, [](AsyncWebServerRequest *request){
    mode = "ON";
    command(FARM, "ARM2", mode);
    command(FARM, armDEGREE, mode);
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
      /*if (FARM.available())
      {
        response = FARM.readStringUntil('&');
      }*/
       //Serial.println(response);
       //Serial.println(fire);
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
     /*if (PANT.available())
     {
        response = PANT.readStringUntil('\n');
     }*/
     // Serial.println(response);
      //Serial.println(tiltdeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
    
  });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode != "ON")&&(tilt > 0))
    {
      tilt-=1;
      tiltdeg = String(tilt);
      command(1,"TILT",tiltdeg);
      /*if (PANT.available())
      {
        response = PANT.readStringUntil('\n');
      }*/
      //Serial.println(tiltdeg);
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((mode!="ON")&&(pan > -60))
    {
      pan-=1;
      pandeg = String(pan);
      command(1,"PAN",pandeg);
      /*if (PANT.available())
      {
        response = PANT.readStringUntil('\n');
      }*/
      //Serial.println(response);
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
      /*if (PANT.available())
      {
        response = PANT.readStringUntil('\n');
      }*/
      //Serial.println(response);
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
    /*if (FARM.available())
     {
       response = FARM.readStringUntil('\n');
     }*/
    //Serial.println(response);
    command(1,"PAN",pandeg);
    /*if (PANT.available())
      {
        response = PANT.readStringUntil('\n');
      }*/
    //Serial.println(response);
    Serial.println(tiltdeg);
    Serial.println(pandeg); 
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.begin();
}

void loop() {

  //digitalWrite(motorSwitch, HIGH);
  //digitalWrite(microSwitch, HIGH);

  // put your main code here, to run repeatedly:
}