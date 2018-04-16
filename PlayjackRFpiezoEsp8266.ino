#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <SPI.h>   // Comes with Arduino IDE
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include "RF24.h"  // Download and Install (See above)
#define FASTLED_ALLOW_INTERRUPTS 0 /// testing to stop flickering
#include <FastLED.h>

#define TRIGGER A0  // for piezo sensor
#define THRESHOLD 80 // hardware adjustment would be best?
byte val = 0; // trigger stuff

#define NUM_LEDS 15 //15 per mini testm 113 /31 144/m 300/5m 
#define DATA_PIN 5
//CRGB leds[NUM_LEDS];
CRGBArray<NUM_LEDS> leds;

//Add more library for using the ADS1115 - the expanding for 4 ADC pin by connecting SCL -> D1 ; SDA ->D2; ADDR-->GND;
#include <Wire.h>
  
//ESP8266WebServer server(80);
//define your default values here, if there are different values in config.json, they are overwritten.

RF24 myRadio (4, 15); // was (7, 8)"myRadio" is the identifier you will use in following methods
/*-----( Declare Variables )-----*/
byte addresses[][6] = {"1Node"}; // Create address for 1 pipe.
int dataReceived;  // Data that will be received from the transmitter
int dataTransmitted;  
int dot1 = NUM_LEDS+1; // led pixel number / tracker
int dot2 = NUM_LEDS+1; // led pixel number / tracker
int brightness = 0;    // how bright the LED is ( WAS 0 / then 21 )
int fadeAmount = 10 ;// = -10;    // how many points to fade the LED by (in main loop)
int ledSpeed = 25; // time between LED write cycles (was 15)
int bigHit = 1; // variable to indicate that 255 has been hit (use for if statement filters)
int ledFlickerDelay = 200; // flicker timing for bighit
int ledFlicker = 0; // flicker on/off
unsigned long previousMillis = 0;     // will store last time LED was updated
unsigned long previousFlickerMillis = 0;     // will store last time LED was updated
unsigned long interval = 0;   // keep track when data arrived
unsigned long lastReceivedMillis = 0;   // keep track when data arrived
unsigned long leadingDot;
unsigned long p1color; // color trails after leading dot
unsigned long p2color;
unsigned long p3color; // p3color is not in use... was thinking of using it to wipe

#define TRIGGER_PIN   16           // a button for reseting Wifi SSID - PASS WEMOS: D7 = GPIO13

const char* host_read;
const char* port_read;
const char* token_read;
const char* path_read;
const char* sleep_read;
//const char* player2tail; /// testing

char token[80];
char host[120];
char port[8]; 
char path[60] ;
char sleep[10] ;

String x = "";
float value;
char flag =0;
unsigned int count = 0;
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Time to sleep (in seconds):
const int sleepTimeS = 10;
//int16_t adc0, adc1, adc2, adc3;




void setup() {   /****** SETUP: RUNS ONCE ******/
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  delay(500);
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Arduino File: playjack_esp8266feedback"));
  Serial.print(F("Number of assigned LEDS:"));
  Serial.println(NUM_LEDS);
  Serial.print(F("ledSpeed = "));
  Serial.println(ledSpeed);
  Serial.println(F("** Assigned LEDS may not match the amount of LEDS connected to device"));
  Serial.print(F("Trigger Threshold = "));
  Serial.println(THRESHOLD);

  // NRF24l01+ radio section
  myRadio.begin();
  myRadio.setAutoAck(1);                    // Ensure autoACK is enabled
  myRadio.enableAckPayload();               // Allow optional ack payloads
  myRadio.setRetries(6,15);                 // x*250us and amount of tries first is microsecond interval, second is amount of tries. 
                                            // Smallest time between retries, max no. of retries
  myRadio.setChannel(108);  // Above most Wifi Channels
  myRadio.setDataRate(RF24_250KBPS); // maximum range
  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  // myRadio.setPALevel(RF24_PA_LOW); // was min power at first
  myRadio.setPALevel(RF24_PA_MAX);  // Uncomment for more power
  myRadio.openWritingPipe( addresses[0]); // was [1]
  myRadio.openReadingPipe(1, addresses[0]); // Use the first entry in array 'addresses' (Only 1 right now)
  myRadio.startListening();
  //myRadio.printDetails(); // uncomment for radio debugging (lot of info)
  FastLED.clear();
  FastLED.show(); // wipe LEDs when turned on /restarted


  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT);                 //Moisture sensor
//  pinMode(RED_LED, OUTPUT);
//  pinMode(GREEN_LED, OUTPUT);
  //pinMode(SCL, OUTPUT);
  //pinMode(SDA, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  
  digitalWrite(LED_BUILTIN,HIGH);
//  digitalWrite(RED_LED,LOW);
//  digitalWrite(GREEN_LED,LOW);

  //pinMode(POWER_OF_SENSOR, OUTPUT);
  //digitalWrite(POWER_OF_SENSOR, HIGH);
  
  
  Serial.print("AIN0: "); Serial.println(A0);
//  Serial.print("AIN1: "); Serial.println(adc1);
//  Serial.print("AIN2: "); Serial.println(adc2);
//  Serial.print("AIN3: "); Serial.println(adc3);
  
  
   //------------------Reset when pushing the button at first-------------------------
       while(digitalRead(TRIGGER_PIN) ==0)
        {
          Serial.println("CLICKED>>>");       //when reseting, the button is pushed, it will reset all SSID & password, restaurant_ID and table_ID
          for(int i=0;i<10;i++)               //GREEN LED blinks 10 times fast.
          {
            digitalWrite(LED_BUILTIN,1);          
//            digitalWrite(GREEN_LED,1);
            delay(100);
            digitalWrite(LED_BUILTIN,0);
//            digitalWrite(GREEN_LED,0);
            delay(100);
          }
         
            WiFiManager wifiManager;
          //reset saved settings
             wifiManager.resetSettings();   // Clear only SSID & pass
            // SPIFFS.format();             // Clear only restaurant_ID & table ID
            // delay(500);  
        }

 //-----------------read configuration from FS json------------------------------
      Serial.println("mounting FS...");
     
      if (SPIFFS.begin()) {
        Serial.println("mounted file system");
        //SPIFFS.format();
        
        if (SPIFFS.exists("/config.json")) {
          //file exists, reading and loading
          Serial.println("reading config file");
          File configFile = SPIFFS.open("/config.json", "r");
          if (configFile) {
            Serial.println("opened config file");
            size_t size = configFile.size();
            // Allocate a buffer to store contents of the file.
            std::unique_ptr<char[]> buf(new char[size]);
    
            configFile.readBytes(buf.get(), size);
            DynamicJsonBuffer jsonBuffer;
            JsonObject& json = jsonBuffer.parseObject(buf.get());
            json.printTo(Serial);
            if (json.success()) {
              Serial.println("\nparsed json");
              strcpy(token, json["token"]);
              strcpy(host, json["host"]);
              strcpy(port, json["port"]);
              strcpy(sleep, json["sleep"]);
    
              
            } else {
              Serial.println("failed to load json config");
            }
          }
        }
      } else {
        Serial.println("failed to mount FS");
      }
      //end read
      Serial.println("Unclicked .............");
      delay (300);
      
//      digitalWrite(GREEN_LED,1);      // green LED emits to wait for setting the Wifi SSID/Password
 
  // -------The extra parameters to be configured (can be either global or just in the setup)-----------
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length

  
 /*     WiFiManagerParameter  custom_token("token","token",token, 80);
      WiFiManagerParameter  custom_host("host","host",host, 120);
      WiFiManagerParameter  custom_port("port","port",port, 8);
      WiFiManagerParameter  custom_sleep("sleep","sleep",sleep, 10);
*/
      WiFiManagerParameter  custom_token("0x000000","0x000000",token, 80);
      WiFiManagerParameter  custom_host("0x0d7287","0x0d7287",host, 120);
      WiFiManagerParameter  custom_port("20","20",port, 8);
      WiFiManagerParameter  custom_sleep("15","15",sleep, 10);

  //--------------------------WiFiManager---------------------------
    WiFiManager wifiManager;
          

//----------------------set custom ip for portal--------------------------
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,0,1), IPAddress(10,0,0,1), IPAddress(255,255,255,0));
    //set config save notify callback
   wifiManager.setSaveConfigCallback(saveConfigCallback);

   
    wifiManager.addParameter(&custom_sleep);
    wifiManager.addParameter(&custom_host);
    wifiManager.addParameter(&custom_port);
    wifiManager.addParameter(&custom_token);
    
    
 
  //set minimu quality of signal so it ignores AP's under that quality
  wifiManager.setMinimumSignalQuality(30);      // Show Wifi signal at least 30%
  
 

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("PlayJack_V2.0proto")) {  //Walkgreen_v2.0
    Serial.println("failed to connect and hit timeout");
    delay(5000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(2000);

}
    
  //if you get here you have connected to the WiFi
  Serial.println("connected...yay :)");
  digitalWrite(LED_BUILTIN,0);
   for(int i=0;i<5;i++)               // top LED blinks 10 times fast.
          {
//            digitalWrite(GREEN_LED,1);
            delay(200);
//            digitalWrite(GREEN_LED,0);
            delay(200);
          }
          
   strcpy(host, custom_host.getValue());
   strcpy(port, custom_port.getValue());
   strcpy(token, custom_token.getValue());
   strcpy(sleep, custom_sleep.getValue());
    
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    json["host"] = host;
    json["port"] = port;
    json["token"] = token;
    json["sleep"] = sleep;
    
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  FS_data_read();
  Serial.println("begin to loop");
 // NUM_LEDS = atoi(sleep_read);
  Serial.print(F("sleep_read:"));
  Serial.println(sleep_read);
  Serial.print(F("host_read:"));
  Serial.println(host_read);
  Serial.print(F("port_read:"));
  Serial.println(port_read);
  Serial.print(F("token_read:"));
  Serial.println(token_read);
  ledSpeed = strtol(sleep_read, NULL, 16);
  p1color = strtol(host_read, NULL, 16);
  fadeAmount = strtol(port_read, NULL, 16);
  p2color = strtol(token_read, NULL, 16);
  Serial.print(F("LedSpeed:"));
  Serial.println(sleep_read);
  Serial.print(F("p1color hex color:"));
  Serial.println(host_read);
  Serial.print(F("fadeAmount:"));
  Serial.println(port_read);
  Serial.print(F("p3color hex color:"));
  Serial.println(token_read);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show(); // wipe LEDs when turned on /restarted
}




void loop() {     /****** LOOP: RUNS CONSTANTLY ******/
  //Data_sending(); // I dont need to send anything to the server
    //--------Befor getting to the sleep mode----------
      //----Turn of the energy for Transistors: supply voltage to the Moisture sensor, connection with the chargers
      
      // digitalWrite(..., LOW);
  
     //------------------------------------------------
  //deep_sleep();

     // check to see if it's time to change the state of the LED
  unsigned long currentMillis = millis(); // use this instead of delay
   val = analogRead(TRIGGER);
   if(val > THRESHOLD) {
     Serial.print(F("Trigger ADC = "));
     Serial.println(val);
    //// experiment
     dataTransmitted = 777; // dot1
     myRadio.stopListening();
     Serial.println(F("radio stop listening"));
     myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
     Serial.print(F("Signal Sent = "));
     Serial.println(dataTransmitted); // was dataTransmitted
     myRadio.startListening();
     leds = p2color; // trigger color?
     //// end experiment
   }

  if ( myRadio.available()) // Check for incoming data from transmitter
  {
    /*while (myRadio.available())  // While there is data ready
    {   // AVOID LOOPS IN THE STATE MACHINE
      myRadio.read( &dataReceived, sizeof(dataReceived) ); // Get the data payload (You must have defined that already!)
    } */
    // DO something with the data, like print it
     myRadio.read( &dataReceived, sizeof(dataReceived) ); 
    Serial.print("Data received = ");
    Serial.println(dataReceived);
    lastReceivedMillis = currentMillis;
    // interval = currentMillis - lastReceivedMillis; // perfect 4/4 would not work with interval check
    //Serial.print("   Time received = ");
    //Serial.println(lastReceivedMillis);



    // NEED if / else statement here to prevent datareceived from resetting brightness on every cycle
    if (dataReceived == 777){
      dot1 = 0; //off
      Serial.println(" Dot1 Hit");
    }
    else if (dataReceived == 666){
      dot2 = 0;
      Serial.println(" Dot2 Hit");
   }

   else {
    Serial.println(" INVALID MESSAGE INVALID MESSAGE INVALID MESSAGE INVALID MESSAGE INVALID MESSAGE ");
    Serial.print("Invalid Data received = ");
    Serial.println(dataReceived);
    dataReceived = 0;
   }
    
    brightness = dataReceived; // analog signal from trigger sets LED brightness
  
  }
  // ^^^ ///////This is where radio transmission ends!////////////////////
  // ^^^ ///////This is where radio transmission ends!////////////////////
  // ^^^ ///////This is where radio transmission ends!////////////////////

   
   
   
   /*-----------IF THERE'S NO RADIO SIGNAL LOOP THIS ---------------------*/
   /*-----------IF THERE'S NO RADIO SIGNAL LOOP THIS ---------------------*/
   /*-----------IF THERE'S NO RADIO SIGNAL LOOP THIS ---------------------*/





//<<<<<<<<<<<<<<<<<<<<<<<<< WS2182 CONTROL SECTION >>>>>>>>>>>>>>>>>>>>>>>>>>>>
//<<<<<<<<<<<<<<<<<<<<<<<<< WS2182 CONTROL SECTION >>>>>>>>>>>>>>>>>>>>>>>>>>>>
//<<<<<<<<<<<<<<<<<<<<<<<<< WS2182 CONTROL SECTION >>>>>>>>>>>>>>>>>>>>>>>>>>>>

 if((currentMillis - previousMillis >= ledSpeed))
      {
      previousMillis = currentMillis; // creating a checkpoint since the last change


 /*if (dot1 < NUM_LEDS || dot2 < NUM_LEDS) //need one delay time for everything
     {
      delay(15);
     }
*/
 if (dot1 <= NUM_LEDS)  //START
      { 
       //for(int dot = 0; dot < NUM_LEDS; dot++) { 
            //leds[dot] = CRGB::Blue;
            leds[dot1+1].setRGB( 255, 255, 255); //RGB method
            //leds[dot1] = leadingDot;
         //FastLED.show();
            // clear this led for the next time around the loop
            // leds[dot1] = CRGB::Black; // black means off
            //delay(15);
            //leds[dot1].setRGB( atoi(token_read), atoi(host_read), atoi(port_read)); //RGB method
            leds[dot1] = p1color;
            dot1 = dot1+1;
            Serial.print(F("dot1 = "));
            Serial.println(dot1);
        }
     
if (dot1 == NUM_LEDS) // 
    {
     //FastLED.clear();
     leds[dot1] = CRGB::Black; 
     //FastLED.show();
     Serial.print(F("dot1 terminated at "));
     dot1 = (NUM_LEDS+1);
     Serial.println(dot1);
     ///*
     //// experiment
     dataTransmitted = 666; // dot1
     myRadio.stopListening();
     Serial.println(F("radio stop listening"));
     myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
     Serial.print(F("Signal Sent = "));
     Serial.println(dataTransmitted); // was dataTransmitted
     myRadio.startListening();
     //// end experiment
     //*/
   }
        //// END dot1



if (dot2 <= NUM_LEDS)  //START
      { 
       //for(int dot = 0; dot < NUM_LEDS; dot++) { 
            //leds[dot] = CRGB::Blue;
            leds[(NUM_LEDS-(dot2+1))].setRGB(255, 255, 255); //RGB method
            //leds[(NUM_LEDS-(dot2+1))] = leadingDot;
            //FastLED.show();
            // clear this led for the next time around the loop
            //leds[dot2] = CRGB::Black; // black means off
          //leds[(NUM_LEDS-(dot2+1))].setRGB( atoi(host_read), atoi(port_read), atoi(token_read)); //RGB method
            //leds[(NUM_LEDS-(dot2+1))] = strtol(sleep_read, NULL, 16);
            leds[(NUM_LEDS-(dot2))] = p2color;
            //delay(15);
            dot2 = dot2+1;
            Serial.print(F("dot2 = "));
            Serial.println(dot2);
        }



     
if (dot2 == NUM_LEDS) // # of cycles for first note　// I think this is the problem...
    {
     //FastLED.clear();
     leds[0] = p2color; 
     //FastLED.show();
     Serial.print(F("dot2 terminated at "));
     dot2 = (NUM_LEDS+1);
     Serial.println(dot2);
     //// experiment
     dataTransmitted = 777; // dot1
     myRadio.stopListening();
     Serial.println(F("radio stop listening"));
     myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
     Serial.print(F("Signal Sent = "));
     Serial.println(dataTransmitted); // was dataTransmitted
     myRadio.startListening();
     //// end experiment
     
   }


      leds.fadeToBlackBy(fadeAmount);// make this a single variable for all patterns
       //delay(15);
       FastLED.show();
        //// END dot2
    } // end ledSpeed governed section
   


  //<<<<<<<<<<<<<<<<<<<<<<<<< PONG PING SECTION >>>>>>>>>>>>>>>>>>>>>>>>>>>>
  //<<<<<<<<<<<<<<<<<<<<<<<<< PING PONG SECTION >>>>>>>>>>>>>>>>>>>>>>>>>>>>
  //<<<<<<<<<<<<<<<<<<<<<<<<< PING PONG SECTION >>>>>>>>>>>>>>>>>>>>>>>>>>>>


     

     
}//--(end main loop )---




//-----------------Function----------------------------------------
void FS_data_read()
{
         
 File configFile = SPIFFS.open("/config.json", "r");   
          
        WiFiManagerParameter  custom_token("token","token",token, 80);
        WiFiManagerParameter  custom_host("host","host",host, 120);
        WiFiManagerParameter  custom_port("port","port",port, 8);
        WiFiManagerParameter  custom_sleep("sleep","sleep",sleep, 10);
        
         Serial.println("INSIDE THE FS DATA READ FUNCTION");
           if (!configFile) {
            Serial.println("failed to open config file for reading");
            
          }
          else{
            
            Serial.println("reading config file");
            String s=configFile.readStringUntil('\n');
            Serial.print("CONTENT");
            Serial.print(":");
            Serial.println(s);

            token_read = custom_token.getValue();
            Serial.print("Token: ");
            Serial.println(token_read);

            
            Serial.print("Chip_ID: ");
            Serial.println(ESP.getChipId());
            Serial.println("");
            
            host_read = custom_host.getValue();
            Serial.print("host: ");
            Serial.println(host_read);
            
            port_read = custom_port.getValue();
            Serial.print("port: ");
            Serial.println(port_read);

            sleep_read = custom_sleep.getValue();
            Serial.print("sleep: ");
            Serial.println(sleep_read);
            
                 
          }      
}
//------------FUNCTIONS FOR SENSOR VALUES----------------
String wet_dry(){
   if(digitalRead(14)) { return "DRY"; }
    else{ return "WET";}
}
int wet_dry_2(){
   if(digitalRead(14)) { return 1; }
    else{ return 0; }
}
float percent()
{
  value = analogRead(A0);
  value = 100-((value - 350)/630)*100;
  if(value>100){value = 100;}
  if(value<0){value = 0;}
  return value;
}

float humid()
{
  value = analogRead(A0);
  Serial.print("ADC value= ");
  Serial.println(value);
  value = 1-((value - 350)/630);
  if(value>1){value = 1;}
  if(value<0){value = 0;}
  Serial.print("humid= ");
  Serial.println(value);
  return value;
}



void Data_sending(){
  flag=1;
  Serial.print("connecting to ");
  Serial.println(host_read);
 
  const char* host_new = host_read;
   
  if(strstr(host_new,"https"))
  {
    Serial.println("there is : 'https'");
    for(int i =8 ; i<strlen(host_new);i++) 
    {
      x = x+ host_new[i];
     }
  
   Serial.print("host after cutting: ");
   Serial.println(x);
   sending_frame_2(x.c_str(),atoi(port_read),path_read, token_read,ESP.getChipId());
   x = "";
  }
  else
  {
    
      for(int i =7 ; i<strlen(host_new);i++) 
      {
        x = x+ host_new[i];
       }
       Serial.print("host after cutting: ");
      Serial.println(x);
      sending_frame_1(x.c_str(),atoi(port_read),path_read, token_read,ESP.getChipId());
      x = "";
  }

  

}
//-------------SENDING SENSOR DATA TO SERVER--------------
void sending_frame_1(const char* host,int port, const char* path_read, const char* token, int ID)
{
  WiFiClient client;
  if (!client.connect(host, port)) {   
    Serial.println("connection failed");
    flag = 0;
    return;
  }
  
  
     //   String url = "/notify?device_id=test&humidity=0.7";
  String url = "/notify?device_id=";
  url += String(ESP.getChipId());
  url += "&humidity=";
  // just sendind analog voltage for server to covert to a percentage
  url += String(analogRead(A0)); // was String(humid());
  url += "&A0=";
//  url += String(adc0);
  url += "&A1=";
//  url += String(adc1);
  url += "&A2=";
//  url += String(adc2);
  url += "&A3=";
 // url += String(adc3);
  
  Serial.println("Frame send to server: GET " + url + " HTTP/1.1");
 //--------FRAME OF SENDING DATA TO THE SERVER------------------
 
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");

 //--------CHECKING THE RESPONSE FROM THE SERVER---------------
      
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
 //---------EXPORT SERVER'S RESPONSE TO EXAMINE------------------ 
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
 //   String line = client.readString();
    Serial.print(line);
    if(line=="HTTP/1.1 200 OK")
    {
      Serial.println("OKKKK");
      flag=1;
      break;
    }
    else
    {
      
      flag = 0;
      break;
    }
  
  }
  Serial.println();
  Serial.println("<------------Ending of receiving frame from server------------->");
  Serial.println("closing connection");
  delay(1000);
}
void sending_frame_2(const char* host,int port, const char* path_read, const char* token, int ID)
{
   WiFiClientSecure client;
  if (!client.connect(host, port)) {   
    Serial.println("connection failed");
      flag = 0;
    return;
  }
  

    //   String url = "/notify?device_id=test&humidity=0.7";
  String url = "/notify?device_id=";
  url += String(ESP.getChipId());
  url += "&humidity=";
  url += String(humid());
  url += "&A0=";
//  url += String(adc0);
  url += "&A1=";
 // url += String(adc1);
  url += "&A2=";
//  url += String(adc2);
  url += "&A3=";
//  url += String(adc3);
  
  Serial.println("Frame send to server: GET " + url + " HTTP/1.1");  
 //--------FRAME OF SENDING DATA TO THE SERVER------------------
 
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");

 //--------CHECKING THE RESPONSE FROM THE SERVER---------------
      
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

 //---------EXPORT SERVER'S RESPONSE TO EXAMINE------------------ 
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
 //   String line = client.readString();
   Serial.print(line);
  if(line=="HTTP/1.1 200 OK")
    {
      Serial.println("OKKKK");
      flag=1;
      break;
    }
    else
    {
      flag = 0;
      break;
    }
  }
  Serial.println();
  Serial.println("<------------Ending of receiving frame from server------------->");
  Serial.println("closing connection");
  delay(1000);
  
}

void deep_sleep(){
  Serial.println("ESP8266 in sleep mode");
  //digitalWrite(POWER_OF_SENSOR,LOW);
  ESP.deepSleep(atoi(sleep_read) * 1000000);
//ESP.deepSleep(1000000);
  delay(500);
  
}
