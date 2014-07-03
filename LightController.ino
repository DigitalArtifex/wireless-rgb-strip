#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <LPD8806.h>
#include <Time.h>
#include <SoftwareSerial.h>
#include <Streaming.h>
#include <WiFlySerial.h>
#include "Credentials.h"
#include "bytearray.h"
#include "bytearray.c"
#include "EEPROMTemplate.h"

#define SUNRISE 1
#define SUNSET 2
#define SHOW_CUR 3
#define SHOW_TGT 4

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

// various buffer sizes
#define REQUEST_BUFFER_SIZE 180
#define POST_BUFFER_SIZE 180
#define TEMP_BUFFER_SIZE 180

#define MSG_CONNECTION_OPEN "*OPEN*"
#define MSG_CONNECTION_CLOSE "*CLOSE*"

/////////////////////////////////////////////
//// PROGMEM Data
/////////////////////////////////////////////

//WiFlySerial data
prog_char s_WT_SETUP_00[] PROGMEM = "nist1-la.ustiming.org";  /* change to your favorite NTP server */
prog_char s_WT_SETUP_01[] PROGMEM = "set u m 0x1";
prog_char s_WT_SETUP_02[] PROGMEM = "set comm remote 0";
prog_char s_WT_SETUP_03[] PROGMEM = "set comm idle 30";
prog_char s_WT_SETUP_04[] PROGMEM = "set comm time 2000";
prog_char s_WT_SETUP_05[] PROGMEM = "set comm size 180";
prog_char s_WT_SETUP_06[] PROGMEM = "set comm match 0x9";
prog_char s_WT_SETUP_07[] PROGMEM = "time";

PROGMEM const char *WT_string_table[] = 	   
{   
  s_WT_SETUP_00,
  s_WT_SETUP_01,
  s_WT_SETUP_02,
  s_WT_SETUP_03,
  s_WT_SETUP_04,
  s_WT_SETUP_05,
  s_WT_SETUP_06,
  s_WT_SETUP_07
};

/////////////////////////////////////////////
//// Enums
/////////////////////////////////////////////
enum NetworkStatus
{
  Disconnected = 0,
  Connected = 1,
  ClientConnected = 2,
  CommandProcess = 4
};

enum Status
{
    //Command statuses
    AOK = 0x00,
    ERR_CMD = 0x01,
    ERR_PARAM = 0x02,
    ERR_TARGET = 0x04,
    ERR_LINE = 0x08,
    
    EEPROM_CONFIG = 0xFA
};

/////////////////////////////////////////////
//// Configuration Struct
/////////////////////////////////////////////
struct Configuration
{
  unsigned long unique_id;
  unsigned long client_id;
  char client_name[32];
  unsigned char current_red;
  unsigned char current_green;
  unsigned char current_blue;
  unsigned char target_red;
  unsigned char target_green;
  unsigned char target_blue;
} configuration;

char chMisc;

//Indexing
int iRequest = 0;
int iTrack = 0;
int iLoopCounter = 0;
int index = -1;

//Color settings
uint32_t color_tmp = 0;

//Animation settings
byte animation = 0;
int animation_length = 180000;
int animation_last_update = 0;

int nLEDs = 150; // Number of RGB LEDs in strand
int CurrentStatus = 0x00;

char bufTemp[TEMP_BUFFER_SIZE];
char netstat = 0;

struct ByteArray *data_buffer;
struct ByteArray *color_buffer;
struct ByteArray *line_buffer;

LPD8806 strip = LPD8806(nLEDs);
WiFlySerial wifi(ARDUINO_RX_PIN ,ARDUINO_TX_PIN);

/////////////////////////////////////////////
//// WiFlySerial Example Functions
/////////////////////////////////////////////

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
  wifi.join(NETWORK_SSID);

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

/////////////////////////////////////////////
//// Light Controller Functions
/////////////////////////////////////////////
bool wifiConnected() { return ((netstat & Connected) == Connected); }
bool clientConnected() { return ((netstat & ClientConnected) == ClientConnected); }

char GetHighTargetColor() 
{ 
  char high = 0;
  
  if(configuration.target_red > high) high = configuration.target_red;
  if(configuration.target_green > high) high = configuration.target_green;
  if(configuration.target_blue > high) high = configuration.target_blue;
  
  return high; 
}

void Print(char *data)
{
    Serial.println(data);
    Serial.flush();
}

/////////////////////////////////////////////
//// Set Functions
/////////////////////////////////////////////
void SetCurrentValue()
{
    Print("Setting Current Color");
    if((index = ByteArray_indexOf("=", 1, color_buffer)) > 0)
    {
        char *color = (char *)malloc((sizeof(char) * (index + 1)));
        
        ByteArray_grabChars(0, index, color_buffer, color);
        ByteArray_remove(0, 1, color_buffer);
        
        if(!strcmp(color, "red"))
        {
            ByteArray_grabChars(0, color_buffer->data_len > 3 ? 3 : color_buffer->data_len, color_buffer, color);
            configuration.current_red = atoi(color);
        }
        else if(!strcmp(color, "green"))
        {
            ByteArray_grabChars(0, color_buffer->data_len > 3 ? 3 : color_buffer->data_len, color_buffer, color);
            configuration.current_green = atoi(color);
        }
        else if(!strcmp(color, "blue"))
        {
            ByteArray_grabChars(0, color_buffer->data_len > 3 ? 3 : color_buffer->data_len, color_buffer, color);
            configuration.current_blue = atoi(color);
        }
        else
            CurrentStatus |= ERR_PARAM;
            
        free(color);
    }

    ByteArray_clear(color_buffer);
}

void SetTargetValue()
{
    Print("Setting Target Color");
    if((index = ByteArray_indexOf("=", 1, color_buffer)) > 0)
    {
        char *color = (char *)malloc((sizeof(char) * (index + 1)));
        
        ByteArray_grabChars(0, index, color_buffer, color);
        ByteArray_remove(0, 1, color_buffer);
        
        if(!strcmp(color, "red"))
        {
            ByteArray_grabChars(0, color_buffer->data_len > 3 ? 3 : color_buffer->data_len, color_buffer, color);
            configuration.target_red = atoi(color);
        }
        else if(!strcmp(color, "green"))
        {
            ByteArray_grabChars(0, color_buffer->data_len > 3 ? 3 : color_buffer->data_len, color_buffer, color);
            configuration.target_green = atoi(color);
        }
        else if(!strcmp(color, "blue"))
        {
            ByteArray_grabChars(0, color_buffer->data_len > 3 ? 3 : color_buffer->data_len, color_buffer, color);
            configuration.target_blue = atoi(color);
        }
        else
            CurrentStatus |= ERR_PARAM;
            
        free(color);
    }

    ByteArray_clear(color_buffer);
}

void SetAnimation()
{
    Print("Setting Animation");
    if((index = ByteArray_indexOf("=", 1, color_buffer)) > 0)
    {
        char *ani = (char *)malloc((sizeof(char) * (21)));
        memset(ani, '\0', 21);
        
        ByteArray_grabChars(0, index, color_buffer, ani);
        ByteArray_remove(0, 1, color_buffer);
        
        if(!strcmp(ani, "id"))
        {
            ByteArray_grabChars(0, color_buffer->data_len, color_buffer, ani);
            animation = atoi(ani);
        }
        else if(!strcmp(ani, "length"))
        {
            ByteArray_grabChars(0, color_buffer->data_len, color_buffer, ani);
            animation_length = atoi(ani);
        }
        else
            CurrentStatus |= ERR_PARAM;
            
        free(ani);
    }

    ByteArray_clear(color_buffer);
}

/////////////////////////////////////////////
//// Client Status Commands
/////////////////////////////////////////////
void ClientStatusCheck()
{
    if((ByteArray_indexOf(MSG_CONNECTION_OPEN, 0, data_buffer) >= 0) && !clientConnected())
    {
        netstat |= (char)ClientConnected;
        ByteArray_replace(MSG_CONNECTION_OPEN, "", 0, 0, data_buffer);
    }
    else if((ByteArray_indexOf(MSG_CONNECTION_CLOSE, 0, data_buffer) >= 0) && clientConnected())
    {
        netstat &= (char)(~ClientConnected);
        ByteArray_replace(MSG_CONNECTION_CLOSE, "", 0, 0, data_buffer);
    }
}

/////////////////////////////////////////////
//// Parsing Functions
/////////////////////////////////////////////
void ParseCommandLine()
{    
    if(ByteArray_startsWith("?set:", 5, line_buffer))
    {
        ByteArray_remove(0, 5, line_buffer);

        if(ByteArray_startsWith("current:", 8, line_buffer))
        {
            ByteArray_remove(0, 8, line_buffer);

            while((index = ByteArray_indexOf("&", 1, line_buffer)) > 0)
            {
                ByteArray_grab(0, index, line_buffer, color_buffer);
                ByteArray_remove(0, 1, line_buffer);
                SetCurrentValue();
            }
            
            //Grab the last/whole chunk, and place it in the color_buffer
            if(line_buffer->data_len > 0)
            {
                ByteArray_grab(0, line_buffer->data_len, line_buffer, color_buffer);
                SetCurrentValue();
            }
        }
        else if(ByteArray_startsWith("target:", 7, line_buffer))
        {
            ByteArray_remove(0, 7, line_buffer);

            while((index = ByteArray_indexOf("&", 1, line_buffer)) > 0)
            {
                ByteArray_grab(0, index, line_buffer, color_buffer);
                ByteArray_remove(0, 1, line_buffer);
                SetTargetValue();
            }
            
            //Grab the last/whole chunk, and place it in the color_buffer
            if(line_buffer->data_len > 0)
            {
                ByteArray_grab(0, line_buffer->data_len, line_buffer, color_buffer);
                SetTargetValue();
            }
        }
        else if(ByteArray_startsWith("animation:", 10, line_buffer))
        {
            ByteArray_remove(0, 10, line_buffer);
            while((index = ByteArray_indexOf("&", 1, line_buffer)) > 0)
            {
                ByteArray_grab(0, index, line_buffer, color_buffer);
                ByteArray_remove(0, 1, line_buffer);
                SetAnimation();
            }
            
            //Grab the last/whole chunk, and place it in the color_buffer
            if(line_buffer->data_len > 0)
            {
                ByteArray_grab(0, line_buffer->data_len, line_buffer, color_buffer);
                SetAnimation();
            }
        }
        else
            CurrentStatus |= ERR_TARGET;
            
        if((CurrentStatus & ERR_TARGET) != ERR_TARGET)
        {
            Print("Saving Configuration");
            EEPROM_write(0, (char)0x00);
            EEPROM_write(1, configuration);
            EEPROM_write(0, (char)EEPROM_CONFIG);
            Print("Done");
        }
    }
    else if(ByteArray_startsWith("?show:", 6, line_buffer))
    {
        ByteArray_remove(0, 6, line_buffer);

        if(ByteArray_startsWith("current:", 8, line_buffer))
            animation = SHOW_CUR;
        else if(ByteArray_startsWith("target:", 7, line_buffer))
            animation = SHOW_TGT;
        else
            CurrentStatus |= ERR_TARGET;
    }
    else
        CurrentStatus |= ERR_CMD;
}

void ParseBuffer()
{
  ClientStatusCheck();

  while(ByteArray_indexOf("\r\n", 2, data_buffer) >= 0)
      ByteArray_replace("\r\n", "\r", 2, 1, data_buffer);

  if((index = ByteArray_indexOf("\r", 1, data_buffer)) >= 0)
  {
    Print("Found line");
      //Grab the line, and place it in a new buffer
      ByteArray_grab(0, index, data_buffer, line_buffer);

      //Remove the trailing '\r'
      ByteArray_remove(0, 1, data_buffer);
      
      index = ByteArray_indexOf("?", 1, line_buffer);
      
      if(index >= 0)
      {
        Print("Command Found");
        
        if(index > 0)
          ByteArray_remove(0, index, line_buffer);//Remove any possible data prior to the actual command
          
        ParseCommandLine();
      }
      else
      {
          Print("No Command Found");
          CurrentStatus |= ERR_LINE;
      }
      
      //Clear the line buffer
      ByteArray_clear(line_buffer);
      
      wifi << CurrentStatus;
      CurrentStatus = 0x00;
  }

  ClientStatusCheck();
}

/////////////////////////////////////////////
//// Receive functions
/////////////////////////////////////////////
void GetMessages()
{
  int Timeout = millis() + 500;
  bool DataAquired = false;
  
  while(wifi.available() > 0 && Timeout > millis())
  {
    //Print("Data aquired");
    char *data = (char *)malloc(sizeof(char));
    data[0] = (char)wifi.read();
    ByteArray_append(data, 1, data_buffer);
    
    DataAquired = true;
    free(data);
  }
  
  if(DataAquired) ParseBuffer();
}

/////////////////////////////////////////////
////Arduino setup/loop
/////////////////////////////////////////////
void setup() 
{  
  Serial.begin(9600);
  
  //Load Configuration from EEPROM
  if(EEPROM.read(0) == EEPROM_CONFIG)
  {
    EEPROM_read(1, configuration);
  }
  else
  {
    Print("Creating configuration");
    randomSeed(analogRead(0));
    
    //Assign ID by nibbles to increase randomness
    unsigned long id = 0;
    
    for(int i = 0; i < (32 / 4); i++)
    {
      unsigned char nbyte = ((random(0,127) - random(0,127)) + random(0,127));
      nbyte = (0x0F & nbyte);
      id = ((id << (4 * i)) | nbyte);
    }
      
    Serial << F("New ID: ") << id << endl;
    
    configuration.unique_id = id;
    configuration.client_id = 0;
    configuration.current_red = 0;
    configuration.current_blue = 0;
    configuration.current_green = 0;
    configuration.target_red = 0;
    configuration.target_green = 0;
    configuration.target_blue = 0;
    memset(&configuration.client_name, '\0', 32);
    
    EEPROM_write(1, configuration);
    EEPROM_write(0, (char)EEPROM_CONFIG);
    Print("Configuration saved");
  }
  
  Serial << F("ID: ") << configuration.unique_id << endl;
  
  // Start up the LED strip
  strip.begin();
  
  // Update the strip, to start they are all 'off'
  strip.show();
  	
  //Wifi setup
  wifi.begin();
  
  // get MAC
  Serial << F("MAC: ") << wifi.getMAC(bufTemp, REQUEST_BUFFER_SIZE) << endl;

  Reconnect();
  Serial << F("IP: ") << wifi.getIP(bufTemp, REQUEST_BUFFER_SIZE) << endl;
  
  memset(bufTemp,'\0',REQUEST_BUFFER_SIZE);
  
  netstat = Connected;
  
  // close any open connections
  wifi.closeConnection();
  
  //delete [] bufTemp;
  
  data_buffer = ByteArray_create(180,180);
  line_buffer = ByteArray_create(64,64);
  color_buffer = ByteArray_create(32,32);
}

void loop()
{
  GetMessages();
  
  //Check animation status
  switch(animation)
  {
    case SUNRISE:  
    //Wait to update lights so that sunrise takes 3 minutes
    if((animation_last_update + (animation_length / 127)) <= millis())
    {
        if(++configuration.current_red > configuration.target_red) configuration.current_red = configuration.target_red;
        if(++configuration.current_green > configuration.target_green) configuration.current_green = configuration.target_green;
        if(++configuration.current_blue > configuration.target_blue) configuration.current_blue = configuration.target_blue;
        
        for (int i=0; i < strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(configuration.current_red,configuration.current_green,configuration.current_blue));
        }
        
        if((configuration.current_red == configuration.target_red) && (configuration.current_green == configuration.target_green) && (configuration.current_blue == configuration.target_blue))
        { 
          animation = 0;
          EEPROM_write(1, configuration);
        }
        
        strip.show();
        animation_last_update = millis();
    }
    break;
    
    case SUNSET:  
    //Wait to update lights so that sunrise takes 3 minutes
    if((animation_last_update + (animation_length / GetHighTargetColor())) <= millis())
    {
        if(--configuration.current_red < 0) configuration.current_red = 0;
        if(--configuration.current_green < 0) configuration.current_green = 0;
        if(--configuration.current_blue < 0) configuration.current_blue = 0;
        
        for (int i=0; i < strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(configuration.current_red,configuration.current_green,configuration.current_blue));
        }
        
        if((configuration.current_red == 0) && (configuration.current_green == 0) && (configuration.current_blue == 0))
        { 
          animation = 0;
          EEPROM_write(1, configuration);
        }
        
        strip.show();
        animation_last_update = millis();
    }
    break;
    
    case SHOW_CUR:
    color_tmp = strip.Color(configuration.current_red, configuration.current_green, configuration.current_blue);
    
    for (int i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, color_tmp);
    }
    
    strip.show();
    animation = 0;
    break;
    
    case SHOW_TGT:
    color_tmp = strip.Color(configuration.target_red, configuration.target_green, configuration.target_blue);
    
    for (int i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, color_tmp);
    }
    
    strip.show();
    animation = 0;
    break;
  }
}

