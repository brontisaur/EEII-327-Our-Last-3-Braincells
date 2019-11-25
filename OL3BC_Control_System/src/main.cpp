#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFiAP.h>
#include <Arduino.h>


const char * ssid = "ESPAP";
const char * password = "hellodaar";


String armState,firing, tiltdeg, pandeg, armDEGREE, usrMESSAGE;
String pandegpre, tiltdegpre;
String mode, fire;
String response;
String armDEGCOM;

String padding ="";

int tilt, pan, armdeg, power, check; //define values for tilt angle, pan angle, power and check
int panalive, firealive;
int complete = 1;
int powercheck = 0;

int len = 0; //response length variable

int timeout = 1500; // max wait period for system check
unsigned long now = 0; //time to be taken at the start of the system check 

int timedout = 0; //timeout flag

//parameters for input forms:

const char* PARAM_INPUT_1 = "armdeg"; //arm degree input
const char* PARAM_INPUT_2 = "panVAL"; //pan degree value input
const char* PARAM_INPUT_3 = "tiltVAL"; //tilt degree value input

char resp[] = "bit";


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

#define FARMALLOW 18 //pins to enable and disable listening on the FIRING Mechanism
#define PANTALLOW 19  //pins to enable and disable listening on the PANT Mechanism

//Command manager

void command(int destination, String command, String value)
{
  pinMode(PANTALLOW, OUTPUT);
  pinMode(FARMALLOW, OUTPUT);
  pinMode(ACK, INPUT);
  complete = 0;
  response = " ";
  switch (destination){
    case 1:
    {
      if (value.length() < 4)
      {
        if (value.length() == 3)
        {
          padding = "0";
          padding.concat(value);
          value = padding;
        }
        else if(value.length() == 2)
        {
          padding = "00";
          padding.concat(value);
          value = padding;
        }
        else if (value.length() == 1)
        {
          padding = "000";
          padding.concat(value);
          value = padding;
        }
      }
      digitalWrite(PANTALLOW, HIGH);
      delay(100);
      Serial1.println(command);   //print command to PANT
      //Serial1.println(value);
      complete = 0;

     while(complete == 0)
      {
        if (Serial1.available())
        {
          response = Serial1.readStringUntil('&');
          len = response.length();
          response = response.substring(len-3,len);

          if (response.startsWith("ACK",0))
          {
            complete = 1;
          }
          else if(response.startsWith("NAK",0))
          {
            complete = 2;
          }
          else if(response.startsWith("NEE",0))
          {
            complete = 2;
          }
        }
      }
      
      delay(100); 
      digitalWrite(PANTALLOW, LOW);
    
    if ((!command.startsWith("CONT",0))&&(!command.startsWith("ZERO",0)))
    { 
      digitalWrite(PANTALLOW, HIGH);
      delay(100);

      Serial1.println(value);
      complete = 0;

      while(complete == 0)
      {
        if (Serial1.available())
        {
          response = Serial1.readStringUntil('&');
          len = response.length();
          response = response.substring(len-3,len);
          if (response.startsWith("ACK",0))
          {
            complete = 1;
          }
          else if(response.startsWith("NAK",0))
          {
            complete = 2;
          }
          else if(response.startsWith("NEE",0))
          {
            complete = 2;
          }
        }
      }

      delay(100);
      digitalWrite(PANTALLOW, LOW);
    }
      
      break;
    }
    case 2:
    {
      if (command.length() < 4)
      {
        if (command.length() == 3)
        {
          padding = "0";
          padding.concat(command);
          command = padding;
        }
        else if(command.length() == 2)
        {
          padding = "00";
          padding.concat(command);
          command = padding;
        }
        else if(command.length() == 1)
        {
          padding = "000";
          padding.concat(command);
          command = padding;
        }
      }
      digitalWrite(FARMALLOW, HIGH);
      delay(100); //give time for debouncing on other end
      Serial2.flush(); //flush before sending
      Serial2.println(command);   //print command to FARM
      //Serial1.println(command); //test
      response = " ";
      complete = 0;
      
      while(complete == 0)
      {
        if (Serial2.available())  //if there are bytes to receive from the other end
        {
          response = Serial2.readStringUntil('&');
          len = response.length();
          response = response.substring(len-3,len);
          if (response.startsWith("ACK",0))
          {
            complete = 1;
          }
          if(response.startsWith("NAK",0))
          {
            complete = 2;
          }
          if(response.startsWith("NEE",0))
          {
            complete = 2;
          }
        }
      }
      delay(100); 
      digitalWrite(FARMALLOW, LOW);
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
  pinMode(PANTALLOW, OUTPUT);
  pinMode(FARMALLOW, OUTPUT);
  pinMode(ACK, INPUT);
  panalive = 0;
  firealive = 0;
  response = "bitch";
  digitalWrite(PANTALLOW,HIGH);
  digitalWrite(FARMALLOW, HIGH);
  delay(100);
  Serial1.flush();
  Serial2.flush();
  Serial1.println("CHEC");
  Serial2.println("CHEC");

  now = millis(); //get current time
  timedout = 0;

  while ((millis() < now + timeout)&&((panalive == 0)&&(firealive == 0))) //while we are not yet timed out and we have no living parts yet
  {

  if (Serial1.available())
  {
    response = Serial1.readStringUntil('&');
    len = response.length();
    response.substring(len-3,len);
    Serial.println(response);
    if ((response.startsWith("ACK",0))||(response.startsWith("NAK",0)))
    {
      panalive = 1;
    }
  }
  if (Serial2.available())
  {
    response = Serial2.readStringUntil('&');
    len = response.length();
    response = response.substring(len-3,len);
    Serial.println(response);
    Serial1.println(len);
    Serial1.println(response);
   if ((response.startsWith("ACK",0)) || (response.startsWith("ACK",0)))
    {
      firealive = 1;
    }
  }

  }

  if (millis() >= (now + timeout))
  {
    timedout = 1;
  }

  delay(100);
  digitalWrite(PANTALLOW, LOW);
  digitalWrite(FARMALLOW, LOW);
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
    return tiltdeg;
  }
  if (var == "HOZ")
  {
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

  //Set Up GPIO pins
  pinMode(motorSwitch, OUTPUT); //set up motor switch pin as output
  pinMode(microSwitch, OUTPUT); //set up micro switch pin as output
  pinMode(powerLED, OUTPUT); //set up power led pin as output
  pinMode(allGOOD, OUTPUT); //set up allGOOD led pin as output
  pinMode(intervene, OUTPUT); //set up intervene led pin as output



  //Set up Serial Communications:
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RX1, TX1); //begin communication with PAN and TILT at 9600 baud
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); //begin communication with FIRE and ARM at 9600 baud
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
    if ((mode == "ON")&&(fire == "NO")&&(powercheck == 0))
    {
      mode = "OFF";
      command(2, "DARM", mode); //command a disarm after user has confirmed elastic is removed
      if (complete == 2)
      {
        mode = "ON";
      }
      digitalWrite(allGOOD, HIGH);
      request->send(SPIFFS, "/index.html", String(), false, processor);
    }
    else if (powercheck == 1)
    {
      powercheck = 0;
      digitalWrite(allGOOD, HIGH);
      request->send(SPIFFS, "/index.html", String(), false, processor);
    }
    if (power == 0)
    {
      digitalWrite(powerLED, LOW);
      digitalWrite(PANTALLOW, LOW);
      digitalWrite(FARMALLOW, LOW);
      request->send(SPIFFS, "/startup.html", String(), false, processor);
    }
    //digitalWrite(allGOOD, HIGH);
    //request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //path to html file for power on
   server.on("/turnon", HTTP_GET, [](AsyncWebServerRequest *request){

    power = 1;
    digitalWrite(powerLED, HIGH); //indicate power has been turned on
    digitalWrite(microSwitch, HIGH); //turn on microcontrollers
    check = 1;
    delay(1000);
    digitalWrite(intervene, HIGH);
    checkStatus(); //check the status of the system
   if ((firealive == 1)&&(panalive == 1))
    {
      digitalWrite(motorSwitch, HIGH); // turn on motors
      digitalWrite(allGOOD, HIGH); //indicate that all is well with the system and functional
      digitalWrite(motorSwitch, HIGH);
      digitalWrite(intervene, LOW);  
      request->send(SPIFFS, "/index.html", String(), false, processor);
    }
    else if(timedout == 1)
    {
      power = 0;
      digitalWrite(microSwitch, LOW);
      digitalWrite(motorSwitch, LOW);
      check = 0;
      usrMESSAGE = "Responses have not been received from the rest of the system. Please check for faults and restart.";
      request->send(SPIFFS, "/message.html", String(), false, processor);
    }
    //request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  //path to html file for power off
  server.on("/turnoff", HTTP_GET, [](AsyncWebServerRequest *request){
    if (power == 0)
    {
      request->send(SPIFFS, "/startup.html", String(), false, processor);
    }
    else if (power != 0)
    {

    command(1, "CONT", "0000");
    if (complete != 2)
    {
      command(2, "CONT", "0000");  
    }
    if (complete == 2)
    {
      request->send(SPIFFS, "/index.html", String(), false, processor);
    }
    if ((mode == "OFF")&&(complete != 2))
    {
      power = 0;
      firealive = 0;
      panalive = 0;
      command(1, "ZERO", "0"); //set tilt to 0 and PAN to 135 degrees
     
      digitalWrite(PANTALLOW, LOW);
      digitalWrite(FARMALLOW, LOW);
      digitalWrite(allGOOD, LOW); //turn off all-good signal
      digitalWrite(motorSwitch, LOW); //turn off motors
      digitalWrite(microSwitch, LOW); //turn off micros
      digitalWrite(powerLED, LOW); //turn off power LED
      digitalWrite(intervene, LOW); //turn off intervene LED
      request->send(SPIFFS, "/startup.html", String(), false, processor);    
    }
    else if (mode == "ON")
    {
      usrMESSAGE = "The ATRED is still in its armed state, please disarm the ATRED and try again.";
      digitalWrite(intervene, HIGH);
      powercheck = 1;

      request->send(SPIFFS, "/message.html", String(), false, processor);
    }

    }
  
  });

  //path to ARM atred:
  server.on("/arm", HTTP_GET, [](AsyncWebServerRequest *request){
    if (power == 0)
    {
      request->send(SPIFFS, "/startup.html", String(), false, processor);
    }
    else if(power !=  0)
    {

      if (mode == "ON")
      {
        if (fire == "NO")
        {
          digitalWrite(intervene, HIGH);
          usrMESSAGE = "Please ensure that the mechanism is still. Remove the ammunition from the ATRED before continuing. Stand back from elastic during the disarm process";
          request->send(SPIFFS, "/message.html", String(), false, processor);
        }
        else if (fire == "YES")
        {
          mode = "OFF";
          command(2, "DARM", mode); //command a disarm after user has confirmed elastic is removed
          fire = "NO";
          if (complete == 2)
          {
            mode = "ON";
            fire = "YES";
          }
          request->send(SPIFFS, "/index.html", String(), false, processor);
        }
      }
      else if (mode == "OFF")
      {
        digitalWrite(intervene, HIGH);
        usrMESSAGE = "Please put the elastic in place over the trigger before continuing";
        request->send(SPIFFS, "/degree.html", String(), false, processor); 
        /*command(2,"ARM1",mode);
        if (complete == 2)
        {
          request->send(SPIFFS, "/index.html", String(), false, processor);
        }
        else
        {
          digitalWrite(intervene, HIGH);
          usrMESSAGE = "Please put the elastic in place over the trigger before continuing";
          request->send(SPIFFS, "/degree.html", String(), false, processor);  
        } */       
        
      }

    }

  });


  server.on("/degget", HTTP_GET, [](AsyncWebServerRequest *request){
    if (power == 0)
    {
      request->send(SPIFFS, "/startup.html", String(), false, processor);
    }
    else if (power != 0)
    {
      if (request->hasParam(PARAM_INPUT_1)) 
      {
      armDEGREE = request->getParam(PARAM_INPUT_1)->value();
      }
      request->send(SPIFFS, "/degree.html", String(), false, processor);  
    }
    
  });


  server.on("/degret", HTTP_GET, [](AsyncWebServerRequest *request){
    if (power == 0)
    {
      request->send(SPIFFS, "/startup.html", String(), false, processor);
    }
    else if(power != 0)
    {
      command(2,"ARM1",mode);
      if (complete == 2)
      {
        request->send(SPIFFS, "/index.html", String(), false, processor);
      }
      else if(complete != 2)
      {
        mode = "ON";
        digitalWrite(intervene, LOW);
        digitalWrite(allGOOD, HIGH);
        command(2, armDEGREE, mode);
        request->send(SPIFFS, "/index.html", String(), false, processor);
      }
    }
  });

  //path to enable firing:
  server.on("/fire", HTTP_GET, [](AsyncWebServerRequest *request){
    if (power == 0)
    {
      request->send(SPIFFS, "/startup.html", String(), false, processor);
    }
    else if (power != 0)
    {
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
        if (complete == 2)
        {
         fire = "NO";
        }
      }
      request->send(SPIFFS, "/index.html", String(), false, processor);
    }
  });

 server.on("/pan", HTTP_GET, [](AsyncWebServerRequest *request){

   if (power == 0)
   {
     request->send(SPIFFS,"/startup.html", String(), false, processor);
   }
   else if (power != 0)
   {
    if(request->hasParam(PARAM_INPUT_2))
    {
      pandegpre = request->getParam(PARAM_INPUT_2)->value();
    }
    pan = pandegpre.toInt();
    if (pan > 270)
    {
      pan = 270;
    }
    else if(pan < 0)
    {
      pan = 0;
    }
    if (mode!="ON")
    {
      pandegpre = pandeg;
      pandeg = String(pan);
      command(1,"PANR",pandeg);
      if (complete == 2)
      {
        pandeg = pandegpre;
      }
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
   }

  });

  //path to tilt mechanism
  server.on("/tilt", HTTP_GET, [](AsyncWebServerRequest *request){
    if (power == 0)
    {
      request->send(SPIFFS, "/startup.html", String(), false, processor);
    }
  else if (power !=0)
  {
    if(request->hasParam(PARAM_INPUT_3)){
      tiltdegpre = request->getParam(PARAM_INPUT_3)->value();
   }
    tilt = tiltdegpre.toInt();

    if (tilt > 75)
    {
      tilt = 75;
    }
    else if(tilt < 0)
    {
      tilt = 0;
    }

    if ((mode!="ON")&&((tilt < 71)&&(tilt > -1)))
    {
      tiltdegpre = tiltdeg;
      tiltdeg = String(tilt);
      command(1,"TILT",tiltdeg);
      if (complete == 2)
      {
        tiltdeg = tiltdegpre;
      }
    }
    request->send(SPIFFS, "/index.html", String(), false, processor); 
  }
  });

  //path to zero the pan/tilt mechanisms
  server.on("/zero", HTTP_GET, [](AsyncWebServerRequest *request){
    if (power == 0)
    {
      request->send(SPIFFS, "/startup.html", String(), false, processor);
    }
    else if(power != 0)
    {
      if (mode != "ON")
    {
    command(1,"ZERO","0");

    if (complete != 2)
    {
      tilt = 0;
      pan = 135;

      pandeg = String(pan);
      tiltdeg = String(tilt);
    }
    
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);  
    }
  });


  server.begin();
}

void loop() {

}