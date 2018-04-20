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
//#define THRESHOLD 80 // hardware adjustment would be best?
byte val = 0; // trigger stuff

#define NUM_LEDS 15 //15 per mini testm 113 /31 144/m 300/5m 
#define DATA_PIN 5
//CRGB leds[NUM_LEDS];
CRGBArray<NUM_LEDS> leds;
  
//ESP8266WebServer server(80);
//define your default values here, if there are different values in config.json, they are overwritten.

RF24 myRadio (4, 15); // was (7, 8)"myRadio" is the identifier you will use in following methods
/*-----( Declare Variables )-----*/
byte addresses[][6] = {"1Node"}; // Create address for 1 pipe.
//int dataReceived;  // Data that will be received from the transmitter
int dataTransmitted;  
//String dataTransmitted = "";


int dot1 = NUM_LEDS+1; // led pixel number / tracker
int dot2 = NUM_LEDS+1; // led pixel number / tracker
int triggerSample = 0; // for calibrating threshold;
int triggerHighest = 0; // for calibrating threshold;
int threshold = 70; // hardware adjustment would be best?
int fadeAmount = 10 ;// how many points to fade the LED by (in main loop)
int ledSpeed = 25; // time between LED write cycles (was 15)
int ledPattern = 0; // choose which LED pattern to use
unsigned long previousMillis = 0;     // will store last time LED was updated
unsigned long previousFlickerMillis = 0;     // will store last time LED was updated
unsigned long interval = 0;   // keep track when data arrived
unsigned long lastReceivedMillis = 0;   // keep track when data arrived
unsigned long deviceColor; // color for this device
unsigned long receivedColor; // color received from controller

#define PORTAL_PIN   0           // a button for reseting Wifi SSID - PASS WEMOS: D3 = GPIO 0

const char* host_read;
const char* port_read;
const char* token_read;
const char* path_read;
const char* sleep_read;
const char* color_code; // for sending hex values

char token[80];
char host[120];
char port[8]; 
char path[60] ;
char sleep[10] ;

//char myArray[] = "This is not a test";
//char myArray[] = "777,1,ff00ff,170,80"; // test sending code between devices
//char customArray[] = "777100ff0010080"; // array to store special data for device

String x = "";
float value;
char flag = 0;
unsigned int count = 0;
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config (line 72 before setup)");
  shouldSaveConfig = true;
}

// Time to sleep (in seconds):
const int sleepTimeS = 10;
//int16_t adc0, adc1, adc2, adc3;

struct devInst{  /// data typedevice instructions (all variables)
  int hsNo; // handshake number
  int ledPattern; // choose which LED pattern to use
  unsigned long colorCode; // will this hold a hex code?
  //const char* color_code; // for sending hex values
  int fadeTime;// how many points to fade the LED by (in main loop)
  int runSpeed; // time between LED write cycles (was 15)
  int analogSignal; // option to send ADC value to other controllers
  bool echo = false; // false prints 0, true prints 0
  bool ignoreMe = false; // if true, then controller will only act as trigger for the settings in the receiver
        // used to set ignoreReceived to true
}storedInst, receivedInst; // Stored instructions, Received Instructions (write received instructions into here)

bool ignoreThem = false; // if this is set to true, this device will react usings its own stored data but not








void setup() {   /****** SETUP: RUNS ONCE ******/
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  delay(500);
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Arduino File: PlayjackESP8266newRadio"));
  Serial.print(F("Number of assigned LEDS:"));
  Serial.println(NUM_LEDS);
  Serial.print(F("ledSpeed = "));
  Serial.println(ledSpeed);
  Serial.println(F("** Assigned LEDS may not match the amount of LEDS connected to device"));
//  threshold = (analogRead(TRIGGER)+10); // auto calibrate trigger.... should i take a larger sample?
//  Serial.print(F("Trigger Threshold = "));
//  Serial.println(threshold);

  // NRF24l01+ radio section
  myRadio.begin();
  myRadio.setAutoAck(1);                    // Ensure autoACK is enabled
  myRadio.enableAckPayload();               // Allow optional ack payloads
  myRadio.setRetries(8,15);                 // x*250us and amount of tries first is microsecond interval, second is amount of tries. 
                                            // Smallest time between retries, max no. of retries
                                            // (6,15) was used for shibuya testing
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
  pinMode(A0, INPUT);                 //ADC trigger
  pinMode(PORTAL_PIN, INPUT_PULLUP); //
 

  
  digitalWrite(LED_BUILTIN,HIGH);
  
  
  Serial.print("AIN0: "); Serial.println(A0);

  
  
 /*  //------------------Reset when pushing the button at first-------------------------
       while(digitalRead(PORTAL_PIN) ==0)
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
             Serial.println("SSID and Pass cleared");
            // SPIFFS.format();             // Clear only restaurant_ID & table ID
            // delay(500);  
        }
*/
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
  //if (!wifiManager.autoConnect()) {  //"PlayJack_V2.0proto" 
 
/*  
  if (!wifiManager.startConfigPortal()) {  //"PlayJack_V2.0proto" 
    // leaving blank will bring up ESP + serial number
    Serial.println("failed to connect and hit timeout");
    delay(5000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(2000);

}
*/
    
  //if you get here you have connected to the WiFi
  Serial.println("connected...yay :) line 248");
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
 // NUM_LEDS = atoi(sleep_read);
  Serial.print(F("sleep_read:"));
  Serial.println(sleep_read);
  Serial.print(F("host_read:"));
  Serial.println(host_read);
  Serial.print(F("port_read:"));
  Serial.println(port_read);
  Serial.print(F("token_read:"));
  Serial.println(token_read);
  deviceColor = strtol(sleep_read, NULL, 16);
  ledPattern = atoi(host_read);
  //fadeAmount = strtol(port_read, NULL, 16);
  fadeAmount = atoi(port_read);
  ledSpeed = atoi(token_read);


  storedInst.hsNo = 777;
  storedInst.ledPattern = ledPattern; // 0 is all fade, 1 is run, 2 glitter, 3 rainbow?
  color_code = sleep_read; // does this work?
  storedInst.colorCode = deviceColor; // unsigned long
  storedInst.fadeTime = fadeAmount;
  storedInst.runSpeed = ledSpeed;

  Serial.print(F("Handshake number:"));
  Serial.println(storedInst.hsNo);
  Serial.print(F("ledPattern:"));
  Serial.println(storedInst.ledPattern);
  Serial.print(F("deviceColor hex:"));
  Serial.println(color_code); //
  Serial.print(F("deviceColor hex sent as:"));
  Serial.println(storedInst.colorCode);
  Serial.print(F("fadeAmount:"));
  Serial.println(storedInst.fadeTime);
  Serial.print(F("ledSpeed:")); // token read
  Serial.println(storedInst.runSpeed);
  Serial.print(F("analogSignal:")); // checking an unassigned value
  Serial.println(storedInst.analogSignal);
  Serial.print(F("echo:")); // token read
  Serial.println(storedInst.echo); //prints true or false?
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show(); // wipe LEDs when turned on /restarted
  Serial.println("Taking ADC value to calibrate threshold");
     for(int i=0;i<5;i++)  {             // top LED blinks 10 times fast.
          triggerSample = analogRead(TRIGGER);
          Serial.println(triggerSample);
          if (triggerSample > triggerHighest) {
            triggerHighest = triggerSample;
          }
         delay(100); 
       }
  Serial.print(F("Highest Trigger ADC sample = "));
  Serial.println(triggerHighest);
  triggerHighest = triggerHighest + 20;
  if (triggerHighest > threshold){
    Serial.println("Threshold adjusted");
    threshold = triggerHighest;     
  }
  Serial.print(F("Trigger Threshold = "));
  Serial.println(threshold);
  Serial.println("begin to loop");

}



/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*--------------------                             ----------------------*/
/*--------------------        void loop()          ----------------------*/
/*--------------------        void loop()          ----------------------*/
/*--------------------        void loop()          ----------------------*/
/*--------------------                             ----------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/


void loop() {     /****** LOOP: RUNS CONSTANTLY ******/
  unsigned long currentMillis = millis(); // use this instead of delay
  //Data_sending(); // I dont need to send anything to the server
    //--------Befor getting to the sleep mode----------
      //----Turn of the energy for Transistors: supply voltage to the Moisture sensor, connection with the chargers
      
      // digitalWrite(..., LOW);
  
     //------------------------------------------------
  //deep_sleep();

     // check to see if it's time to change the state of the LED
   if(digitalRead(PORTAL_PIN) ==0) // was while
       {
         open_wifi(); // its looping back to this?
         Serial.println("Back to void loop");
        } 
   //Serial.println("outside of Portal loop");     
   
   
   
   val = analogRead(TRIGGER);
   if(val > threshold) {
     Serial.print(F("Trigger ADC = "));
     Serial.println(val);
    //// experiment
     dataTransmitted = 777; // dot1
     myRadio.stopListening();
     Serial.println(F("radio stop listening"));
     //myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
     if (!myRadio.write(&storedInst,sizeof(storedInst))){
       Serial.println(F("RADIO TRANSMISSION FAILED!!! YOU ARE NOTHING!!!"));
        for(int i=0;i<5;i++)               // top LED blinks 10 times fast.
          {
              leds = 0xff0000; //
              //leds[0].setRGB(255, 0, 0);
              FastLED.show();
            delay(5);
              leds = 0x000000;
              FastLED.show();
            delay(30);
          }
       
     };
     Serial.print(F("Signal Sent"));
     //Serial.println(myArray); // was dataTransmitted
     //Serial.println(dataTransmitted); // was dataTransmitted
     myRadio.startListening();
     leds = storedInst.colorCode; // 
     //// end experiment
   }

  if ( myRadio.available()) // Check for incoming data from transmitter
  {
    //char tmpArray[19];                                               // This generally should be the same size as the sending array
    /*while (myRadio.available())  // While there is data ready
    {   // AVOID LOOPS IN THE STATE MACHINE
      myRadio.read( &dataReceived, sizeof(dataReceived) ); // Get the data payload (You must have defined that already!)
    } */
    // DO something with the data, like print it
    // myRadio.read( &dataReceived, sizeof(dataReceived) ); 
    myRadio.read(&receivedInst,sizeof(receivedInst));  // Reading 19 bytes of payload (18 characters + NULL character)
    Serial.println("INSTRUCTIONS RECEIVED ACT IMMEDIATELY // INSTRUCTIONS RECEIVED ACT IMMEDIATELY");
    //Serial.println(receivedInst);                                   // Prints only the received characters because the array is NULL terminated
    Serial.print(F("Received Handshake number:"));
    Serial.println(receivedInst.hsNo);
    Serial.print(F("Received ledPattern:"));
    Serial.println(receivedInst.ledPattern);
    Serial.print(F("Received deviceColor (converted from hex):"));
    Serial.println(receivedInst.colorCode);
    Serial.print(F("Received fadeAmount:"));
    Serial.println(receivedInst.fadeTime);
    Serial.print(F("Received ledSpeed:")); // token read
    Serial.println(receivedInst.runSpeed);
    Serial.print(F("Received analogSignal:")); // checking an unassigned value
    Serial.println(receivedInst.analogSignal);
    Serial.print(F("Received bool echo:")); // token read
    Serial.println(receivedInst.echo);
    Serial.print(F("Received bool ignoreMe:")); // token read
    Serial.println(receivedInst.ignoreMe);
    lastReceivedMillis = currentMillis;
    // interval = currentMillis - lastReceivedMillis; // perfect 4/4 would not work with interval check
    //Serial.print("   Time received = ");
    //Serial.println(lastReceivedMillis);


//////////// HANDSHAKE STARTS BELOW /////////////////
    
    if (receivedInst.hsNo == storedInst.hsNo){ // if handshake numbers match then react

       if (ignoreThem == true || receivedInst.ignoreMe == true){
        // change all receivedInst. to storedInst. to use device specific settings
        Serial.println("Controller Settings being ignored");
       }
       else {
        // make sure received instructions are followed
       }

      if (receivedInst.ledPattern == 0){ // pattern 0 = FADE
        leds = receivedInst.colorCode;
        fadeAmount = receivedInst.fadeTime;
      }
      if (receivedInst.ledPattern == 1){ // pattern 1 = Run
        deviceColor = receivedInst.colorCode; // is this okay?
        fadeAmount = receivedInst.fadeTime;
        ledSpeed =  receivedInst.runSpeed;
        dot1 = 0; //off
      Serial.println(" Run Started");  
      Serial.println(" Dot1 Hit");
      }
      
    } /// END HANDSHAKE
    /*else if (dataReceived == 666){
      dot2 = 0;
      Serial.println(" Dot2 Hit");
   }*/

   else {
    Serial.println(" HAND SHAKE WAS TOO AWKWARD. BAILING OUT. ");
    Serial.print(F("They tried shaking with  "));
    Serial.println(receivedInst.hsNo);
    Serial.print(F("And I had given them "));
    Serial.println(receivedInst.hsNo);
    //dataReceived = 0;
   }
    
  
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
         //FastLED.show();
            // clear this led for the next time around the loop
            // leds[dot1] = CRGB::Black; // black means off
            //delay(15);
            //leds[dot1].setRGB( atoi(token_read), atoi(host_read), atoi(port_read)); //RGB method
            leds[dot1] = receivedInst.colorCode; // converted from char hex using strtol
            //leds[dot1] = strtol(receivedInst.color_code, NULL, 16);
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
     if (receivedInst.echo){
     myRadio.stopListening();
     Serial.println(F("radio stop listening"));
     if (!myRadio.write(&storedInst,sizeof(storedInst))){
       Serial.println(F("ECHO FAILED!!! NO ONE EVER LISTENS TO YOU!!!"));
     }
     Serial.print(F("Stored instructions echoed"));
     myRadio.startListening();
     }
     //// end experiment
     //*/
   }
        //// END dot1


/*
if (dot2 <= NUM_LEDS)  //START
      { 
       //for(int dot = 0; dot < NUM_LEDS; dot++) { 
            //leds[dot] = CRGB::Blue;
            leds[(NUM_LEDS-(dot2+1))].setRGB(255, 255, 255); //RGB method
            //FastLED.show();
            // clear this led for the next time around the loop
            //leds[dot2] = CRGB::Black; // black means off
          //leds[(NUM_LEDS-(dot2+1))].setRGB( atoi(host_read), atoi(port_read), atoi(token_read)); //RGB method
            //leds[(NUM_LEDS-(dot2+1))] = strtol(sleep_read, NULL, 16);
            leds[(NUM_LEDS-(dot2))] = deviceColor;
            //delay(15);
            dot2 = dot2+1;
            Serial.print(F("dot2 = "));
            Serial.println(dot2);
        }



     
if (dot2 == NUM_LEDS) // # of cycles for first note　// I think this is the problem...
    {
     //FastLED.clear();
     leds[0] = deviceColor; 
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
*/

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

/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*--------------------                             ----------------------*/
/*--------------------      void open_wifi()       ----------------------*/
/*--------------------      void open_wifi()       ----------------------*/
/*--------------------      void open_wifi()       ----------------------*/
/*--------------------                             ----------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/




void open_wifi(){
  Serial.println("in void open_wifi");
  // nrf24 has data received and reacts after update
  FastLED.clear();
  FastLED.show(); // wipe LEDs when turned on /restarted
   // -------The extra parameters to be configured (can be either global or just in the setup)-----------
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length

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
  //WiFiManager wifiManager;
  if (!wifiManager.startConfigPortal()) {  //"PlayJack_V2.0proto" 
    // leaving blank will bring up ESP + serial number
    //Serial.println("failed to connect and hit timeout");
    Serial.println("Exit startConfigPortal");
    //delay(5000);
    //reset and try again, or maybe put it to deep sleep
    //ESP.reset();
    //delay(2000);
   }
             
          
   strcpy(host, custom_host.getValue());
   strcpy(port, custom_port.getValue());
   strcpy(token, custom_token.getValue());
   strcpy(sleep, custom_sleep.getValue());

//save the custom parameters to FS // THIS IS WHERE YOU SAVE YER STUFFF
 // if (shouldSaveConfig) {
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
 // }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  FS_data_read();
 // NUM_LEDS = atoi(sleep_read);
  Serial.print(F("sleep_read:"));
  Serial.println(sleep_read);
  Serial.print(F("host_read:"));
  Serial.println(host_read);
  Serial.print(F("port_read:"));
  Serial.println(port_read);
  Serial.print(F("token_read:"));
  Serial.println(token_read);
  deviceColor = strtol(sleep_read, NULL, 16);
  ledPattern = atoi(host_read);
  //fadeAmount = strtol(port_read, NULL, 16);
  fadeAmount = atoi(port_read);
  ledSpeed = atoi(token_read);


  storedInst.hsNo = 777;
  storedInst.ledPattern = ledPattern; // 0 is all fade, 1 is run, 2 glitter, 3 rainbow?
  color_code = sleep_read; // does this work?
  storedInst.colorCode = deviceColor; // unsigned long
  storedInst.fadeTime = fadeAmount;
  storedInst.runSpeed = ledSpeed;

  Serial.print(F("Handshake number:"));
  Serial.println(storedInst.hsNo);
  Serial.print(F("ledPattern:"));
  Serial.println(storedInst.ledPattern);
  Serial.print(F("deviceColor hex:"));
  Serial.println(color_code); //
  Serial.print(F("deviceColor hex sent as:"));
  Serial.println(storedInst.colorCode);
  Serial.print(F("fadeAmount:"));
  Serial.println(storedInst.fadeTime);
  Serial.print(F("ledSpeed:")); // token read
  Serial.println(storedInst.runSpeed);
  Serial.print(F("analogSignal:")); // checking an unassigned value
  Serial.println(storedInst.analogSignal);
  Serial.println("open_wifi: new settings saved!");
   for(int i=0;i<5;i++)               // top LED blinks 10 times fast.
          {
              leds[0].setRGB(0, 255, 0);
              FastLED.show();
            delay(200);
              leds[0].setRGB(0, 0, 0);
              FastLED.show();
            delay(200);
          }
  
  
                
}



