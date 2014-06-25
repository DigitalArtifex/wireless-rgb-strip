
#include <Arduino.h>
#include <SPI.h>
#include <LPD8806.h>
#include <Time.h>
#include <SoftwareSerial.h>
#include <Streaming.h>
#include <WiFlySerial.h>
#include "Credentials.h"
#include "MemoryFree.h"
#include "bytearray.h"

#define SUNRISE 1
#define SUNSET 2

// initialize WiFly
// loop
// send a parameterized GET to a web server
//    - show connection and response
// send a parameterized POST to a web server
//    - show connection and response
// 

// Connect the WiFly TX pin to the Arduino RX pin  (Transmit from WiFly-> Receive into Arduino)
// Connect the WiFly RX pin to the Arduino TX pin  (Transmit from Arduino-> Receive into WiFly)
// 
// Connect the WiFly GND pin to an Arduino GND pin
// Finally, connect the WiFly BATT pin to the 3.3V pin (NOT the 5v pin)

#define ARDUINO_RX_PIN  16
#define ARDUINO_TX_PIN  15

// server hosting GET example php script
#define NETWORK_SSID "Yates$Repeater"
#define NETWORK_PASS "47622123456789123456789123"

#define IDX_WT_SETUP_00 0
#define IDX_WT_SETUP_01 IDX_WT_SETUP_00 
#define IDX_WT_SETUP_02 IDX_WT_SETUP_01 +1
#define IDX_WT_SETUP_03 IDX_WT_SETUP_02 +1
#define IDX_WT_SETUP_04 IDX_WT_SETUP_03 +1
#define IDX_WT_SETUP_05 IDX_WT_SETUP_04 +1
#define IDX_WT_SETUP_06 IDX_WT_SETUP_05 +1
#define IDX_WT_SETUP_07 IDX_WT_SETUP_06 +1

#define IDX_WT_STATUS_SENSORS    IDX_WT_SETUP_07 +1
#define IDX_WT_STATUS_TEMP       IDX_WT_STATUS_SENSORS +1
#define IDX_WT_STATUS_RSSI       IDX_WT_STATUS_TEMP +1
#define IDX_WT_STATUS_BATT       IDX_WT_STATUS_RSSI +1

#define IDX_WT_MSG_JOIN          IDX_WT_STATUS_BATT +1
#define IDX_WT_MSG_START_WEBCLIENT IDX_WT_MSG_JOIN +1
#define IDX_WT_MSG_RAM           IDX_WT_MSG_START_WEBCLIENT +1
#define IDX_WT_MSG_START_WIFLY   IDX_WT_MSG_RAM +1
#define IDX_WT_MSG_WIFI          IDX_WT_MSG_START_WIFLY +1
#define IDX_WT_MSG_APP_SETTINGS  IDX_WT_MSG_WIFI +1
#define IDX_WT_MSG_WIRE_RX       IDX_WT_MSG_APP_SETTINGS +1
#define IDX_WT_MSG_WIRE_TX       IDX_WT_MSG_WIRE_RX +1
#define IDX_WT_MSG_FAIL_OPEN     IDX_WT_MSG_WIRE_TX +1

#define IDX_WT_HTML_HEAD_01      IDX_WT_MSG_FAIL_OPEN + 1
#define IDX_WT_HTML_HEAD_02      IDX_WT_HTML_HEAD_01 + 1
#define IDX_WT_HTML_HEAD_03      IDX_WT_HTML_HEAD_02 + 1
#define IDX_WT_HTML_HEAD_04      IDX_WT_HTML_HEAD_03 + 1

#define IDX_WT_POST_HEAD_01      IDX_WT_HTML_HEAD_04 + 1
#define IDX_WT_POST_HEAD_02      IDX_WT_POST_HEAD_01 + 1
#define IDX_WT_POST_HEAD_03      IDX_WT_POST_HEAD_02 + 1
#define IDX_WT_POST_HEAD_04      IDX_WT_POST_HEAD_03 + 1

#define MSG_CONNECTION_OPEN "*OPEN*"
#define MSG_CONNECTION_CLOSE "*CLOSE*"

prog_char s_WT_SETUP_00[] PROGMEM = "nist1-la.ustiming.org";  /* change to your favorite NTP server */
prog_char s_WT_SETUP_01[] PROGMEM = "set u m 0x1";
prog_char s_WT_SETUP_02[] PROGMEM = "set comm remote 0";
prog_char s_WT_SETUP_03[] PROGMEM = "set comm idle 30";
prog_char s_WT_SETUP_04[] PROGMEM = "set comm time 2000";
prog_char s_WT_SETUP_05[] PROGMEM = "set comm size 180";
prog_char s_WT_SETUP_06[] PROGMEM = "set comm match 0x9";
prog_char s_WT_SETUP_07[] PROGMEM = "time";
prog_char s_WT_STATUS_SENSORS[] PROGMEM = "show q 0x177 ";
prog_char s_WT_STATUS_TEMP[] PROGMEM = "show q t ";
prog_char s_WT_STATUS_RSSI[] PROGMEM = "show rssi ";
prog_char s_WT_STATUS_BATT[] PROGMEM = "show battery ";
prog_char s_WT_MSG_JOIN[] PROGMEM = "Credentials Set, Joining ";
prog_char s_WT_MSG_START_WEBCLIENT[] PROGMEM = "Starting WebClientGetPut - Please wait. ";
prog_char s_WT_MSG_RAM[] PROGMEM = "RAM :";
prog_char s_WT_MSG_START_WIFLY[] PROGMEM = "Started WiFly, RAM :";
prog_char s_WT_MSG_WIFI[] PROGMEM = "Initial WiFi Settings :";
prog_char s_WT_MSG_APP_SETTINGS[] PROGMEM = "Configure WebClientGetPost Settings...";
prog_char s_WT_MSG_WIRE_RX[] PROGMEM = "Arduino Rx Pin (connect to WiFly Tx):";
prog_char s_WT_MSG_WIRE_TX[] PROGMEM = "Arduino Tx Pin (connect to WiFly Rx):";
prog_char s_WT_MSG_FAIL_OPEN[] PROGMEM = "Failed on opening connection to:";
prog_char s_WT_HTML_HEAD_01[] PROGMEM = "HTTP/1.1 200 OK \r ";
prog_char s_WT_HTML_HEAD_02[] PROGMEM = "Content-Type: text/html;charset=UTF-8\r ";
prog_char s_WT_HTML_HEAD_03[] PROGMEM = " Content-Length: ";
prog_char s_WT_HTML_HEAD_04[] PROGMEM = "Connection: close \r\n\r\n";
prog_char s_WT_POST_HEAD_01[] PROGMEM = "HTTP/1.1\n";
prog_char s_WT_POST_HEAD_02[] PROGMEM = "Content-Type: application/x-www-form-urlencoded\n";
prog_char s_WT_POST_HEAD_03[] PROGMEM = "Content-Length: ";
prog_char s_WT_POST_HEAD_04[] PROGMEM = "Connection: close\n\n";

using namespace DigitalArtifex;

enum NetworkStatus
{
  Disconnected = 0,
  Connected = 1,
  ClientConnected = 2,
  CommandProcess = 4
};

PROGMEM const char *WT_string_table[] = 	   
{   
  s_WT_SETUP_00,
  s_WT_SETUP_01,
  s_WT_SETUP_02,
  s_WT_SETUP_03,
  s_WT_SETUP_04,
  s_WT_SETUP_05,
  s_WT_SETUP_06,
  s_WT_SETUP_07,
  s_WT_STATUS_SENSORS,
  s_WT_STATUS_TEMP,
  s_WT_STATUS_RSSI,
  s_WT_STATUS_BATT,
  s_WT_MSG_JOIN,
  s_WT_MSG_START_WEBCLIENT,
  s_WT_MSG_RAM,
  s_WT_MSG_START_WIFLY,
  s_WT_MSG_WIFI,
  s_WT_MSG_APP_SETTINGS,
  s_WT_MSG_WIRE_RX,
  s_WT_MSG_WIRE_TX,
  s_WT_MSG_FAIL_OPEN,
  s_WT_HTML_HEAD_01,
  s_WT_HTML_HEAD_02,
  s_WT_HTML_HEAD_03,
  s_WT_HTML_HEAD_04,
  s_WT_POST_HEAD_01,
  s_WT_POST_HEAD_02,
  s_WT_POST_HEAD_03,
  s_WT_POST_HEAD_04
};

// various buffer sizes
#define REQUEST_BUFFER_SIZE 180
#define POST_BUFFER_SIZE 180
#define TEMP_BUFFER_SIZE 180

char chMisc;

int iRequest = 0;
int iTrack = 0;
int iLoopCounter = 0;
int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;
int targetRed = 0;
int targetGreen = 0;
int targetBlue = 0;

WiFlySerial wifi(ARDUINO_RX_PIN ,ARDUINO_TX_PIN);
char bufTemp[TEMP_BUFFER_SIZE];
char NetStat = 0;

ByteArray dataBuffer(180);
ByteArray colorBuffer(21);
ByteArray lineBuffer(64);

// Number of RGB LEDs in strand:
int nLEDs = 150;
int LightUpdateTime = 0;
byte animation = 0;

LPD8806 strip = LPD8806(nLEDs);

// Function for setSyncProvider
time_t GetSyncTime() {
  time_t tCurrent = (time_t) wifi.getTime();
  wifi.exitCommandMode();
  return tCurrent;
}

// GetBuffer_P
// Returns pointer to a supplied Buffer, from PROGMEM based on StringIndex provided.
// based on example from http://arduino.cc/en/Reference/PROGMEM

char* GetBuffer_P(const int StringIndex, char* pBuffer, int bufSize) { 
  strncpy_P(pBuffer, (char*)pgm_read_word(&(WT_string_table[StringIndex])), bufSize);  
  return pBuffer; 
}

//// Reconnects to a wifi network.
//// DHCP is enabled explicitly.
//// You may need to add the MAC address to your MAC filter list.
//// Static IP settings available if needed.
boolean Reconnect() {

  wifi.SendCommand(GetBuffer_P(IDX_WT_SETUP_01,bufTemp,TEMP_BUFFER_SIZE), ">",bufTemp, REQUEST_BUFFER_SIZE);
  wifi.setDHCPMode(WIFLY_DHCP_CACHE );
  wifi.SendCommand(GetBuffer_P(IDX_WT_SETUP_02,bufTemp,TEMP_BUFFER_SIZE),">",bufTemp, REQUEST_BUFFER_SIZE);
  wifi.leave();
  wifi.setPassphrase(NETWORK_PASS);    
  Serial << GetBuffer_P(IDX_WT_MSG_JOIN,bufTemp,TEMP_BUFFER_SIZE) << NETWORK_SSID << endl;
  wifi.join(NETWORK_SSID);

  Serial << GetBuffer_P(IDX_WT_MSG_APP_SETTINGS, bufTemp, TEMP_BUFFER_SIZE) << endl;
  for (int i = 0; i< 7 ; i++) {
    wifi.SendCommand(GetBuffer_P(IDX_WT_SETUP_01 + i,bufTemp,TEMP_BUFFER_SIZE),">",bufTemp, REQUEST_BUFFER_SIZE);
  }
  wifi.getDeviceStatus();

  // reboot if not working right yet.
  iTrack++;
  if ( iTrack > 5 ) {
    wifi.reboot();
    iTrack = 0;
  }

}

bool wifiConnected() { return ((NetStat & Connected) == Connected); }
bool clientConnected() { return ((NetStat & ClientConnected) == ClientConnected); }

void setTargetValue()
{
  int index = -1;
  
  if((index = colorBuffer.indexOf("=")) >= 0)
  {
    if(colorBuffer.startsWith("red="))
      targetRed = atoi(colorBuffer.substring(4, (colorBuffer.size() - 4)));
    else if(colorBuffer.startsWith("green="))
      targetGreen = atoi(colorBuffer.substring(6, (colorBuffer.size() - 6)));
    else if(colorBuffer.startsWith("blue="))
      targetBlue = atoi(colorBuffer.substring(5, (colorBuffer.size() - 5)));
      
    Serial << F("Set ") << colorBuffer.c_str() << endl;
  }
  
  if(colorBuffer.size() > 0)
    colorBuffer.remove(0, colorBuffer.size());
}

void setCurrentValue()
{
  int index = -1;
  
  if((index = colorBuffer.indexOf("=")) >= 0)
  {
    if(colorBuffer.startsWith("red="))
      currentRed = atoi(colorBuffer.substring(4, (colorBuffer.size() - 4)));
    else if(colorBuffer.startsWith("green="))
      currentGreen = atoi(colorBuffer.substring(6, (colorBuffer.size() - 6)));
    else if(colorBuffer.startsWith("blue="))
      currentBlue = atoi(colorBuffer.substring(5, (colorBuffer.size() - 5)));
      
    Serial << F("Set ") << colorBuffer.c_str() << endl;
    Serial << currentRed << " " << currentBlue << " " << currentGreen << endl;
  }
  
  if(colorBuffer.size() > 0)
    colorBuffer.remove(0, colorBuffer.size());
}

void _ParseBuffer()
{
  int index = -1;
  if(dataBuffer.contains(MSG_CONNECTION_OPEN))
  {
    NetStat |= (char)ClientConnected;
    Serial << F("Client Connected") << endl;
    
    dataBuffer.replace(MSG_CONNECTION_OPEN, "");
  }
  else if(dataBuffer.contains(MSG_CONNECTION_CLOSE))
  {
    NetStat &= (char)(~Connected);
    Serial << F("Client Disconnected") << endl;
    
    dataBuffer.replace(MSG_CONNECTION_CLOSE, "");
  }
  
  while(dataBuffer.contains("\r\n"))
    dataBuffer.replace("\r\n","\r");
    
  if((index = dataBuffer.indexOf('\r')) >= 0)
  {
    Serial << F("Line found. ") << dataBuffer.size() << endl;
    
    lineBuffer.append(dataBuffer.grab(0, index));
    dataBuffer.remove(0,1);

    //Save cycles and do a quick check to see if its a command at all
    if((index = lineBuffer.indexOf("?")) >= 0)
    {
      lineBuffer.remove(0, index);
      Serial << F("Command found.") << lineBuffer.size() << endl;
      
      //Is it a set command?
      if((index = lineBuffer.indexOf("?set")) >= 0)
      {
        Serial << F("Set command found.") << endl;
        
        lineBuffer.remove(0,index + 5);

        if(lineBuffer.startsWith("current:"))
        {
          lineBuffer.remove(0,8);

          int red = 0;
          int blue = 0;
          int green = 0;

          while((index = lineBuffer.indexOf('&')) >= 0)
          {
            colorBuffer.append(lineBuffer.grab(0, index));
            lineBuffer.remove(0,1);
            
            setCurrentValue();
          }
          
          colorBuffer.append(lineBuffer.c_str());
          setCurrentValue();
        }
        else if(lineBuffer.startsWith("target:"))
        {
          lineBuffer.remove(0,7);

          int red = 0;
          int blue = 0;
          int green = 0;

          while((index = lineBuffer.indexOf('&')) >= 0)
          {
            colorBuffer.append(lineBuffer.grab(0, index));
            lineBuffer.remove(0,1);
            
            setTargetValue();
          }
          
          colorBuffer.append(lineBuffer.c_str());
          setTargetValue();
        }
        
        else if(lineBuffer.startsWith("animation:"))
        {
          lineBuffer.remove(0,10);
          animation = atoi(lineBuffer.c_str());
        }
      }

      //Is it a show command?
      else if((index = lineBuffer.indexOf("?show")) >= 0)
      {
        Serial << F("Show Command found.") << endl;
        
        lineBuffer.remove(0,index + 6);

        if(lineBuffer.startsWith("current"))
        {
          for (int i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(currentRed,currentGreen,currentBlue));
          }
          
          strip.show();
        }
        else if(lineBuffer.startsWith("target"))
        {
          for (int i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(targetRed,targetGreen,targetBlue));
          }
          
          strip.show();
        }
      }
    }
    
    if(lineBuffer.size() > 0)
      lineBuffer.remove(0, linebuffer.size());
  }
}

void GetMessages()
{
  int Timeout = millis() + 500;
  bool DataAquired = false;
  
  while(wifi.available() > 0 && Timeout > millis())
  {
    dataBuffer.append((char)wifi.read());
    DataAquired = true;
  }
  
  if(DataAquired) _ParseBuffer();
}

/////////////////////////////////////////////
////Arduino setup/loop
/////////////////////////////////////////////
void setup() 
{
  // Start up the LED strip
  strip.begin();
  
  // Update the strip, to start they are all 'off'
  strip.show();
  	
  //Wifi setup
  Serial.begin(9600);
  Serial << GetBuffer_P(IDX_WT_MSG_START_WEBCLIENT,bufTemp,TEMP_BUFFER_SIZE) << endl << GetBuffer_P(IDX_WT_MSG_RAM,bufTemp,TEMP_BUFFER_SIZE) << freeMemory() << endl
  << GetBuffer_P(IDX_WT_MSG_WIRE_RX,bufTemp,TEMP_BUFFER_SIZE) << ARDUINO_RX_PIN << endl << GetBuffer_P(IDX_WT_MSG_WIRE_TX,bufTemp,TEMP_BUFFER_SIZE) << ARDUINO_TX_PIN << endl;

  wifi.begin();
  Serial << F("Starting WebClientGetPost...") <<  wifi.getLibraryVersion(bufTemp, REQUEST_BUFFER_SIZE) << endl;

  // get MAC
  Serial << F("MAC: ") << wifi.getMAC(bufTemp, REQUEST_BUFFER_SIZE) << endl;

  Reconnect();
  // Set timezone adjustment: PST is -8h.  Adjust to your local timezone.
  //adjustTime( (long) (-8 * 60 * 60) );

  Serial << GetBuffer_P(IDX_WT_MSG_WIFI,bufTemp,TEMP_BUFFER_SIZE) << endl  
    << F("IP: ") << wifi.getIP(bufTemp, REQUEST_BUFFER_SIZE) << endl;
  
  memset (bufTemp,'\0',REQUEST_BUFFER_SIZE);
  
  NetStat = Connected;
  
  // close any open connections
  wifi.closeConnection();
  
  delete [] bufTemp;
}

void loop()
{
  GetMessages();
  
  //Check animation status
  switch(animation)
  {
    case SUNRISE:  
    //Wait to update lights so that sunrise takes 3 minutes
    if((LightUpdateTime + 1417) <= millis())
    {
        if(++currentRed > targetRed) currentRed = targetRed;
        if(++currentGreen > targetGreen) currentGreen = targetGreen;
        if(++currentBlue > targetBlue) currentBlue = targetBlue;
        
        for (int i=0; i < strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(currentRed,currentGreen,currentBlue));
        }
        
        if((currentRed == targetRed) && (currentGreen == targetGreen) && (currentBlue == targetBlue)) animation = 0;
        
        strip.show();
        LightUpdateTime = millis();
    }
    break;
  }
}

