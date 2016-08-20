// Idea Fab Labs Pyrosphere Mini 
// Code with Single Flamethrower
// By Jordan Layman, Peter DiFalco 2016
// www.ideafablabs.com
// Idea Fab Labs, Chico // Santa Cruz
#include <Adafruit_Pixie.h>
#include <SoftwareSerial.h>
#include "driver.h"

#define HOOPSWITCH 13
#define SHAKESWITCH 12
#define LEDPIN 14
#define pixelsTotal 1

/*
Browser changeable things
Duration of time to be on Fire
IP Address of Pyrosphere
*/

// Setup Pixies
SoftwareSerial pixieSerial(-1, LEDPIN);
Adafruit_Pixie* ledmeter = new Adafruit_Pixie(pixelsTotal, &pixieSerial);

// Adafruit_NeoPixel* ledmeter = new Adafruit_NeoPixel(16, LEDPIN, NEO_GRB + NEO_KHZ800);
ColorWipe* ledEffect = new ColorWipe(pixelsTotal);

// -- Fire
int speed = 100;
int duration = 200;
boolean shot = false;
boolean score = false;
long shotHappened = 0;
uint8_t scoreCount = 0;
uint32_t pixelColor = 0;

#define shotTimeout 2000


const char* ssid= "LEDpaint";
const char* pass= "betafish";
const char* host= "pyrohoop";


void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t lenght) {

  switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                Serial.printf("[WSc] Connected to url: %s\n",  payload);
        
          // send message to server when Connected
                pyrosphere.sendTXT("Connected");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[WSc] get text: %s\n", payload);

            // send message to server
            if(payload[0] == '0') {trigger();}
            if(payload[0] == '1') {Serial.printf("FIRED!\n");}                
            
            break;
        case WStype_BIN:
            Serial.printf("[WSc] get binary lenght: %u\n", lenght);
            hexdump(payload, lenght);

            // send data to server
            // webSocket.sendBIN(payload, lenght);
            break;
    }

}

// Websocket Event Handler WSEH
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
    } 
        break;
    case WStype_TEXT:
        // if (DEBUG) Serial.printf("[%u] get Text: %s\n", num, payload);

        if(payload[0] == '0') {trigger();}
        if(payload[0] == '1') {Serial.printf("FIRED!\n");}                
        // if(payload[0] == '!') {
          
        //   if(payload[1] == '>'){
        //     forwardSelector();
        //     Serial.println("forward selector");
        //   } else if(payload[1] == '<'){
        //     backSelector();
        //     Serial.println("backward selector");
        //   } else {
        //     /// not optimized... char to int is what we need
        //     uint8_t s = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        //     setSelector(s);    
        //   }
          
        // }
        if(payload[0] == '@') {
            uint8_t f = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
            setFrequency(f);
            webSocket.sendTXT(num, "Duration set to "+String(duration));
        }
        // if(payload[0] == '#') {
        //     uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        //     setBrightness(b);            
        //     webSocket.sendTXT(num, "Brightness set to "+String(brightness));
        // }
        // if(payload[0] == 'P') {
        //     uint32_t p = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
        //     Serial.println(p);

        //     updatePrimary(p);
        //     webSocket.sendTXT(num, "Primary updated");
        // }
        
        // if(payload[0] == 'G') {     
        //   /// Probably replace this with a switch statement. 
        //   if(payload[1]=='!'){
        //     // send message to client
        //     Serial.printf("Get selector request...Replying: %d\n",selector);
        //     webSocket.sendTXT(num, String(selector));    
        //   }
        // }
        break;
  }

}

// #setup
void setup()
{

  pinMode(HOOPSWITCH, INPUT_PULLUP);  
  pixieSerial.begin(115200);
  ledmeter->setBrightness(15);
  // ledmeter->begin();
  ledmeter->show(); // Initialize all pixels to 'off'

  ledEffect->updatePrimary(SGEffect::color(0,255,0));
  ledEffect->updateFrequency(200);

  Serial.begin(115200);
  Serial.print("\n");
  Serial.setDebugOutput(true);

  SPIFFS.begin();
  if (DEBUG) {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
  
  // Load config.
  // loadConfig();
  // if (!loadConfig()) {
  //   Serial.printf("Config failed to load.\n");
  //   // setup defaults
  // }

  //WIFI INIT
  Serial.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.begin(ssid, pass);
  }
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); /// 
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.begin(host);
  Serial.print("Open http://");
  Serial.print(host);
  Serial.println(".local/edit to see the file browser");
  
  httpUpdater.setup(&server);

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, [](){
    if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, [](){
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ", \"analog\":"+String(analogRead(A0));
    json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.begin();
  Serial.println("HTTP server started");
  
  pyrosphere.begin("pyrosphere.local",81);
  pyrosphere.onEvent(webSocketClientEvent);    

  // intervalCount = 1;
  interval = 0;
  iter = 0;
  
  first_run = 1; // This is the first run flag.

  if (DEBUG) statusUpdate();
}


void loop() {

  now = millis();   // This moment is beautiful.
  
  if (now >= then+frameRate) {
    ledEffect->render();
    uint32_t newColor;
    for(int i=0; i<pixelsTotal; i++){    
      newColor = ledEffect->p(i);            
      ledmeter->setPixelColor(i,newColor);
    }       
    Serial.println(newColor);
    
    ledmeter->setPixelColor(pixelColor,0xFFFFFF);
    // ledmeter->show();

    // Did we shake?
    if (digitalRead(SHAKESWITCH)) {        
        if(shot == false) {
          shotHappened = now;
          shot = true;          
        }
        
        if (now >= shotHappened + shotTimeout) {
            shot = false;
        }
    }

    // Check the switch
    if (digitalRead(HOOPSWITCH) == LOW) {        
        
        score = true;
        scoreCount++;

        if (scoreCount < 3) {
          trigger();  
        } else {
          triggerOnFire();
          scoreCount = 0;
        }
        
      
    }

    then = now;    

  }  

  

  //  Listeners

  byte y = 10;
  while(Serial.available() > 0 && (y-- > 0)) { // Listen on the serial connection.
    char x = Serial.read();
    interceptSerial(x);
  }
  
  server.handleClient();
  // webSocket.loop();
  pyrosphere.loop();
} 

// Burn.
void trigger() {
    Serial.printf("%d HOOPTY!\n",millis());
    pyrosphere.sendTXT("0");
    // pyrosphereModel.sendTXT("0");
}

void triggerOnFire() {
    Serial.printf("%d HES ON FIRE!\n",millis());
    pyrosphere.sendTXT("3");
    // pyrosphereModel.sendTXT("3");
}

// Burn.
void rapid() {
  
    Serial.println("Rapid!");

    digitalWrite(HOOPSWITCH, HIGH);
    delay(duration);
    digitalWrite(HOOPSWITCH, LOW);
    digitalWrite(HOOPSWITCH, HIGH);
    delay(duration);
    digitalWrite(HOOPSWITCH, LOW);
    digitalWrite(HOOPSWITCH, HIGH);
    delay(duration);
    digitalWrite(HOOPSWITCH, LOW);
    digitalWrite(HOOPSWITCH, HIGH);
    delay(duration);
    digitalWrite(HOOPSWITCH, LOW);
    digitalWrite(HOOPSWITCH, HIGH);
    delay(duration);
    digitalWrite(HOOPSWITCH, LOW);

}


// Serial Functions
void interceptSerial(char x) {

    //Flags, set read mode., begin
  
  if        ( x == '!' )    {   readMode  = 1;    }         //Set Selector
  else if   ( x == '@' )    {   readMode  = 2;    }         //Set Frequency
  else if   ( x == '#' )    {   readMode  = 3;    }         //Set Brightness
  else if   ( x == '+' )    {   readMode  = 4;    }         //Set Mode
  else if   ( x == '-' )    {   readMode  = 5;    }         //
  else if   ( x == '~' )    {   readMode  = 6;    }         //Reload Config
  else if   ( x == '*' )    {   readMode  = 7;    }         //Toggle Debug
  else if   ( x == '?' )    {   readMode  = 8;    }         //Status Update
  // else if    ( x == '/' )    {   getFiles();       }       //    
  // else if    ( x == '?' )    {   statusUpdate();   }       //  
  //Add custom flags here...
  
  //Finish up
  else if   (x == '.')    {   //...

    //This will update the global variables accordingly.
    switch(readMode){
      case 1:      trigger();        break;
      case 2:      sSetFrequency();       break;
      case 3:      sSetConnection();      break;
      // case 4:      sSetMode();            break;
      // case 5:      setValveOff();      break;
      // case 6:      loadConfig();          break;      
      case 7:      toggleDebug();         break;
      case 8:      statusUpdate();        break;
      default:                            break;  
    }
    
      // lastSerialCMD = now;         //Used for switching to autoPilot
      readMode = 0;                   //We're done reading. (until another.)
      bufferIndex = 0;
      resetMessageBuffer();         
  }
  else { messageBuffer[bufferIndex++] = x; }        //Magic.

}

void sSetFrequency() {
  uint16_t s = atoi(messageBuffer);
  setFrequency(s);
  if (DEBUG) Serial.print(F("User updated duration: "));Serial.println(duration);
}
void setFrequency(uint16_t s) {
  /// Add some bounds here.

  /// ADD MAXDURATION instead of 1000
  if (s<1000) duration = s;
  
}

void sSetConnection() {
    String str(messageBuffer);
    setConnection(messageBuffer);
    if (DEBUG) Serial.print(F("User updated connection IP: "));Serial.println(messageBuffer);
}
void setConnection(String ip) {
    // pyrosphere.begin(ip,81);
    // pyrosphere.onEvent(webSocketEvent);    
}

void resetMessageBuffer(){
	memset( messageBuffer, '\0', sizeof(messageBuffer) );   
}


void statusUpdate() {

	Serial.println(F("<=== Status Update ===>")); 
  	Serial.print(F("Speed: "));
  	Serial.println(speed); 
  	Serial.print(F("Duration: "));
  	Serial.println(duration); 
  
}
void toggleDebug() {if (DEBUG){DEBUG = 0;} else {DEBUG=1;}Serial.print(F("Toggling debug:"));Serial.println(DEBUG);}


// ==== Webserver Functions

//format bytes

String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if(fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) fsUploadFile.close();
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileDelete(){
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}