// Controller for Sonos soundsystems by frank
// provided as is
// work in progress

// a mix of all sorts of libraries found on github

// feature list 
// wifi setup manager to enter your wifi ssid and password
// OTA enabled for updating
// drives a color pixel ledstrip to illuminate buttons of jukebox
// drives a TFT display 240x240 
// reads the 20 jukebox keys using 1 adc pin and a switched resistor voltage divider to save wiring
// a rotary encoder to control the volume
// SD card to store music files and artwork
// scans network to find sonos, max 4 devices
// play internet radio or jukebox mode with your own playlist
// webserver for serving music files and more 
// printing of 1" x 3" paper jukebox strips to insert in jukebox or wallbox
// printing of 'menu' card with the offered selection of songs
// connection with MQTT broker to send/receive events and commands

// todo
// artwork records
// pimp up the web pages
// pimp up the webmanager portal
// setup of radiostations using sonos favorite list
// moving more sonos commands to asynchronous method
// clean up the code mess

#include "ConfigOnSwitch.h" // deals with Wifi connection, using WifiManager

#include <AsyncTCP.h>
static AsyncClient * aClient = NULL;

#include <TJpg_Decoder.h>
#include <arduino.h>
#include "hardcoded.h" // list of predefined radio stations
 
#define WALLBOX 1 // hardware is wallbox, uses volume up/down buttons
#define JUKEBOX 0 // 1 hardware is a large jukebox
int DeviceType = JUKEBOX;

#define SELECTSONG 1 // function of jukebox keys
#define SELECTRADIO 0 // function of jukebox keys
int DeviceMode = SELECTRADIO;

#define SONOSDEFAULTVOLUME 15 // reset volume to this level if lower

// sonos async processed commands, not all implemented yet
#define SONOSGETVOLUME 1
#define SONOSSETVOLUME 2
#define SONOSGETMODE 3 // radio, playing file, whatever
#define SONOSGETSTATE 4 // stopped, playing, paused
#define SONOSPLAY 5 // starts playing if not started yet
#define SONOSADDSONGTOQUEUE 6 // add song, from playlist, song stored on SD, or from Samba server
#define SONOSEMPTY QUEUE 7
#define SONOSSELECTRADIOPRESET 8
#define SONOSCHECKJUKEBOXQUEUE 9


// Orientation of hardware connections ESP32-DEV module, USB pointing downwards
// Left row
//    EN 
//    VP GPIO36 ADC RIGHTHAND/BOTTOM BUTTON ROW 1-2-3-4-5-6-7-8-9-10
//    VN GPIO39 ADC LEFTHAND/TOP BUTTON ROW A_B-C-D-E-F-G-H-J-K
//   D34 GPIO34 ADC CONTROL WALLBOX VOLUME UP/DN, SKIP (WALLBOX ONLY)
//   D35 (GPIO35) CANCEL BUTTON met 10K pullup naar 3V3
//   D32 BLUE BUTTON ROCKOLA
//   D33 ENCODER B
//   D25 ENCODER A
//   D26 SD_MISO
//   D27 
//   D14 SD_SCLK
//   D12
//   D13 SD_MOSI
//   GND
//   VIN
// Right row
//   D23 (GPION23) TFT MOSI (VSPI)
//   D22 (GPIO22)
//   TXO (GPIO1)
//   RXO (GPIO3)
//   D21 (GPIO21) SOLENOID
//   D19 (GPIO19) TFT MISO (VSPI) 
//   D18 (GPIO18) TFT CLK (VSPI)
//   D5  (GPIO5)  TFT CS
//   TX2 (GPIO17) WS2812 LED STRIP DATA
//   RX2 (GPIO16) ENCODER BUTTON
//   D4  (GPIO4)  TFT RESET
//   D2  (GPIO2)  TFT DC
//   D15 (GPIO15) SD CS
//   GND
//   3V3


#include <SD.h> 
//#include "FS.h"
#include <SPI.h>
SPIClass sdSPI(HSPI);
bool   SD_present = false;
#define SD_MISO 26  // Change from default pin to allow for reset without issues
#define SD_MOSI 13
#define SD_SCLK 14
#define SD_CS 15

const char KnobDecals[]="ABCDEFGHJK1234567890";

char LastRadioStream[128]="";
char LastRadioStation[128]="";

// setup of OTA
#include "OTA.h"


// invoke TFT_eSPI (by Bodmer - version 2.4.72) display library
// make sure to choose the correct dispay driver in User_Setup in the libray folder TFT_eSPI
// tested with ILI9341_DRIVER 320x240 display
// also tested with ST7789_DRIVER + #define TFT_RGB_ORDER TFT_BGR 240x240 display
// with the 320x240 ILI9341 display I only use 240x240 portion of it, as it is mounted behind a square bezel that exposes only 240x240 area of the display

#include "Arialnarrow.h"
#include <TFT_eSPI.h> // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(240,320); 

struct TFTline
{
  bool refresh;
  char content[256];
  bool scroll;
  int length;
  int pixelwidth;
  int scrollpos;
  int scrolldelay;
  int nchar;
  int toeat;
  int noffset;
  int textcolor;
  int backgroundcolor;
};


// note to self - connections for ST7789 display
// purple wire DC to pin 2 (TFT_DC in User_Setup.h)
// grey wire RES to pin D4 (TFT_RST in User_Setup.h)
// white wire SDA to pin 23 (TFT_MOSI in User_Setup.h)
// black wire SLCK to pin D18 (TFT_SLCK in User_Setup.h)

// note to self - connections for ILI9341 display
// blue wire DC to pin 2 (TFT_DC in User_Setup.h)
// orange wire MOSI to pin 23 (TFT_MOSI in User_Setup.h)
// orange-wit wire SLCK to pin D18 (TFT_SLCK in User_Setup.h)
// green wire CS to pin D15 (TFT_CS in User_Setup.h)
// green-white wire RST to pin D4 (TFT_RST in User_Setup.h)
// yellow wire MISO to pin D19


// also make sure you have the right pins selected in User_Setup.h
// defined in User_Setup.h:
// #define TFT_MISO 19
// #define TFT_MOSI 23
// #define TFT_SCLK 18
// #define TFT_CS   5  // Chip select control pin
// #define TFT_DC    2  // Data Command control pin
// #define TFT_RST   4  // Reset pin (could connect to RST pin)


TFT_eSprite needle = TFT_eSprite(&tft); // Sprite object for volume needle
#define DIAL_CENTRE_X 120
#define DIAL_CENTRE_Y 121
uint16_t* tft_buffer;
void createNeedle(void);

// display updates are done in seperate task
// mutex used to avoid simultanious acces to text arrays for display
SemaphoreHandle_t  xDisplayMutex;

#define LCD_RADIO_PLAYING 1
#define LCD_RADIO_SELECTING 2
#define LCD_JUKEBOX_PLAYING 3
#define LCD_JUKEBOX_SELECTING 4
#define LCD_JUKEBOX_DEBUG 5
#define LCD_SONOS_RADIO 5 // http mode eigenlijk


char PlayingTitle[556]=""; // die komt na ;&lt;dc:title&gt; en is de radio naam bij radio en de songtitel bij jukebox
char PlayingStreamContent[556]=""; // die komt na &lt;r:streamContent&gt; en is de artiest en tracktitel bij radio en leeg bij jukebox
char OldPlayingTitle[556]=""; // die komt na ;&lt;dc:title&gt; en is de radio naam bij radio en de songtitel bij jukebox
char OldPlayingStreamContent[556]=""; // die komt na &lt;r:streamContent&gt; en is de artiest en tracktitel bij radio en leeg bij jukebox

// eprom to store chosen sonos device
#include <EEPROM.h>
#define EEPROM_SIZE 256

// led animation
#include <FastLED.h>
#define DATA_PIN 17
#define NUM_LEDS 88 // 72 for the selection keys and 16 for the volume knob
CRGB leds[NUM_LEDS];

#include <AiEsp32RotaryEncoder.h>
#define ROTARY_ENCODER_A_PIN 25
#define ROTARY_ENCODER_B_PIN 33
#define ROTARY_ENCODER_BUTTON_PIN 16
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 2

#define BLUE_BUTTON 32 
#define CANCEL_BUTTON 35
#define ENCODER_BUTTON 16


#define MAXSONOS 4
// Max number of Sonos IP devices our network
struct SonosDevice { // /status/zp details Structure
  char Zonename[32]; // ZoneName
  char UID[32];      // LocalUID RINCON_xxx
  char Serialnumber[32];  // serial number full
  char Seriesid[16]  ;     //SeriesID
  } ACTIVE_sonosDV,G_SonosDeviceList[MAXSONOS];     // device array of max 4 Devices in your network


uint8_t SonosLastUsed = -1;          
uint8_t ActiveSonos = -1;  // 0-3 from the list of max 4 Sonos Devices       
int ActualVolumeFromSonos = -1; // initialize as invalid value
int NewVolumeForSonos = -1; // initialize as invalid value
int AsyncVolumeForSonos = -1;
int MinimumVolumeForSonos = SONOSDEFAULTVOLUME;
int SonosSkipRequest = 0;
int ShowVolume10mS = 0;
int Show45RPM10mS = 0;
int ShowRadio10mS = 0;
int ShowArt10mS = 0;
int Actual45RPMShown = 999;
int ActualRadioShown = 999;
int ActualArtShown = 999;
int UpdateTimeOut10mS = 0; // timeout timer after Sonos SetVolume

#define ANALOG_LEFT 39 // rows of switches are wired as voltage divider to save wires
#define ANALOG_RIGHT 36 // rows of switches are wired as voltage divider to save wires
#define ANALOG_CONTROL 34 // wallbox only
#define SOLENOID 21 // latch magnet on rockola jukebox

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(
    ROTARY_ENCODER_A_PIN, 
    ROTARY_ENCODER_B_PIN, 
    ROTARY_ENCODER_BUTTON_PIN, 
    ROTARY_ENCODER_VCC_PIN,
    ROTARY_ENCODER_STEPS
    );

void IRAM_ATTR readEncoderISR()
{ rotaryEncoder.readEncoder_ISR();
}    

volatile int interruptCounter;
volatile int MagnetPower10mS = 0;
int MagnetDeadTime10mS = 0;
bool ScrollNow = false;
bool bUpdateDisplay = false;
 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#define EEPROM_JBM 0
#define JBM_PLAYQUEUE 1
#define JBM_RADIO 2
#define JBM_MAX 2

#define EEPROM_RADIOSTATION 1
int OldRadioStation = 0;
int NewRadioStation; // fetched from eeprom at boot
#define EEPROM_DeviceType 2
#define EEPROM_SONOSIP1 3 // saved IP addresses from found sonos devices
#define EEPROM_SONOSIP2 7
#define EEPROM_SONOSIP3 11
#define EEPROM_SONOSIP4 15
#define EEPROM_SONOS_LASTUSED 19


char Keys[]="1234"; // last 4 keys pressed, used as secret way to set device type (jukebox or wallbox) 4711J 4711B 

volatile byte  deb_left_key = 0;
volatile byte  deb_right_key = 0;
volatile byte  deb_control_key = 0;
int deb_cancel_button = 0;
int deb_blue_button = 0;
int deb_encoder_button = 0;

int blue_buttonf = 1234;
int cancel_buttonf = 1234;
int encoder_buttonf = 1234;
int left_keyf = 0;
int right_keyf = 0;
int control_keyf = 0;

volatile byte cancel_button_long_pressed = 0;
int cancel_button_very_long_pressed = 0;
int encoder_button_very_long_pressed = 0;
int magnet = 0;
int magnetdeadtime = 0;
int selectedsong = 0;
char SonosTrack[256];

byte LastSongAddedToQueue = 16;
int SongsInQueueCnt = 0;
int CancelButtonEnabled = 0;
int BootFase = 0;


void ethConnectError(void) // callback from sonos library
{ static int fail = 0;
  char text[32];
  // Sonos does not respond to command
  Serial.println("Sonos: No connecton...");
  Serial.print("Free Heap: ");Serial.println(ESP.getFreeHeap());

  fail++;
  sprintf(text, "SONOS ERROR"); 
  TFT_line_print(2, text);    
  sprintf(text, "COM FAIL %d", fail); 
  TFT_line_print(3, text);    
//  if(fail>5)
//  { //ACTIVE_sonosIP(0);
    //BootFase = 1; // start all over
//    fail = 0;
// }  
}



#include "SonosUPnP.h"
#include "sonosscan.h"
int ActLcdMode = 999; 
int NewLcdMode = -1; 


#include <MicroXPath_P.h>

#include <ESPAsyncWebServer.h>

#include "webpages.h"

#define FIRMWARE_VERSION "JUKEBOX V0.9"
const String default_ssid = "somessid";
const String default_wifipassword = "mypassword";
const String default_httpuser = "admin";
const String default_httppassword = "admin";
const int default_webserverporthttp = 80;

// configuration structure
struct Config {
  String ssid;               // wifi ssid
  String wifipassword;       // wifi password
  String httpuser;           // username to access web admin
  String httppassword;       // password to access web admin
  int webserverporthttp;     // http port number for web admin
};

// variables
Config config;                        // configuration
bool shouldReboot = false;            // schedule a reboot
AsyncWebServer *server;               // initialise webserver
void configureWebServer(void);

// function defaults
String listFiles(bool ishtml = false);

#include <DNSServer.h>

// why not
void setupMQTT(void);
void loopMQTT(void);

const int JUKEKEYMODE[SONOS_MAXSOURCE]={
SELECTRADIO, //UNKNOWN_SCHEME,
SELECTRADIO, //SPOTIFY_SCHEME,
SELECTSONG, //FILE_SCHEME,
SELECTSONG, //LIBRARY_SCHEME
SELECTSONG, //HTTP_SCHEME,
SELECTRADIO, //RADIO_SCHEME,
SELECTRADIO, //RADIO_AAC_SCHEME,
SELECTRADIO, //LINEIN_SCHEME,
SELECTRADIO, //MASTER_SCHEME, 
SELECTRADIO, //QUEUE_SCHEME , 
SELECTRADIO, //SPOTIFYSTATION_SCHEME,
SELECTSONG, //LOCALHTTP_SCHEME,
SELECTSONG, //LOCALHTTPS_SCHEME 
SELECTRADIO, //SPOTIFY_RADIO_SCHEME
};


const uint32_t JUKESCANCOLOR[SONOS_MAXSOURCE]={
0x00FF00, //UNKNOWN_SCHEME,
0xFF0000, //SPOTIFY_SCHEME,
0xFF00FF, //FILE_SCHEME,
0xFF00FF, //LIBRARY_SCHEME
0xFF00FF, //HTTP_SCHEME,
0xFF0000, //RADIO_SCHEME,
0xFF0000, //RADIO_AAC_SCHEME,
0xFF0000, //LINEIN_SCHEME,
0xFF0000, //MASTER_SCHEME, 
0xFF0000, //QUEUE_SCHEME , 
0xFF0000, //SPOTIFYSTATION_SCHEME,
0xFF00FF, //LOCALHTTP_SCHEME,
0xFF00FF, //LOCALHTTPS_SCHEME 
0xFF0000, //SPOTIFY_RADIO_SCHEME
};

// UpdateLcd2() treats some modes in similar way
const uint32_t JUKELCDMODEOVERIDES[SONOS_MAXSOURCE]={
SONOS_SOURCE_UNKNOWN, //#define SONOS_SOURCE_UNKNOWN 0
SONOS_SOURCE_SPOTIFY, //#define SONOS_SOURCE_SPOTIFY 1
SONOS_SOURCE_FILE, //#define SONOS_SOURCE_FILE 2
SONOS_SOURCE_HTTP, //#define SONOS_SOURCE_HTTP 3
SONOS_SOURCE_RADIO, //#define SONOS_SOURCE_RADIO 4
SONOS_SOURCE_RADIO, //#define SONOS_SOURCE_RADIO_AAC 5
SONOS_SOURCE_RADIO, //#define SONOS_SOURCE_LINEIN 6
SONOS_SOURCE_MASTER, //#define SONOS_SOURCE_MASTER 7
SONOS_SOURCE_QUEUE, //#define SONOS_SOURCE_QUEUE 8
SONOS_SOURCE_SPOTIFYSTATION, //#define SONOS_SOURCE_SPOTIFYSTATION 9
SONOS_SOURCE_FILE, //#define SONOS_SOURCE_LOCALHTTP 10
SONOS_SOURCE_FILE, //#define SONOS_SOURCE_LOCALHTTPS 11
};

//#define SONOS_SOURCE_UNKNOWN 0
//#define SONOS_SOURCE_SPOTIFY 1
//#define SONOS_SOURCE_FILE 2
//#define SONOS_SOURCE_HTTP 3
//#define SONOS_SOURCE_RADIO 4
//#define SONOS_SOURCE_RADIO_AAC 5
//#define SONOS_SOURCE_LINEIN 6
//#define SONOS_SOURCE_MASTER 7
//#define SONOS_SOURCE_QUEUE 8
//#define SONOS_SOURCE_SPOTIFYSTATION 9
//#define SONOS_SOURCE_LOCALHTTP 10
//#define SONOS_SOURCE_LOCALHTTPS 11


int SonosState;
int SonosSecondsStopped100mS =0;
int NewSonosSourceMode = -1;
int SonosSourceMode = -1;

bool PollSonosVolume = false;
bool PollSonosSourceMode = false;
bool PollSonosState = false;
bool Ticker100mS = false;

bool AllSet = false;
bool g_Playlistfound = false;
bool g_SDfallback = false;

static int next;
void IRAM_ATTR onTimer() {
  
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  if(MagnetPower10mS)
  { digitalWrite(SOLENOID, HIGH);
    MagnetPower10mS--;
  }
  else digitalWrite(SOLENOID, LOW);
  if(MagnetDeadTime10mS)MagnetDeadTime10mS--;
  if(ShowVolume10mS)ShowVolume10mS--;
  if(Show45RPM10mS)Show45RPM10mS--;
  if(ShowRadio10mS)ShowRadio10mS--;
  if(ShowArt10mS)ShowArt10mS--;
  if(UpdateTimeOut10mS)UpdateTimeOut10mS--;  

  if((interruptCounter % 5)==0)ScrollNow = true; // ticker for the vertical scrolling

  if((interruptCounter % 10)==0)
  { bUpdateDisplay = true; // display refresh max 10 times a second
    Ticker100mS = true; // ticker for Sonos communication
  }

    if((interruptCounter % 100)==0) // each second
    { next++;
     switch(next)
     { case 1:
         PollSonosVolume = true;  // synchroniseer volume control with external volume changes
         break;
       case 2:
         PollSonosSourceMode = true;
         break;
       case 3:  
         PollSonosState = true;
         break;
       default:
         next = 0;
         break;  
     }
   }
 portEXIT_CRITICAL_ISR(&timerMux);
}





char uri[100] = "";
char response[256] = "";

char metaBuffer[2048] = "";
char metaBuffer2[2048] = "";
String lastCmd;

void handleHomePage(AsyncWebServerRequest *request);
void handleCmd();
void handleNotFound();
void handleResponse();
void handleGet();
void handleGt();

void HomePage(AsyncWebServerRequest *request);
void File_Download(AsyncWebServerRequest *request);
void SD_file_download(AsyncWebServerRequest *request, String filename);
void File_Upload(AsyncWebServerRequest *request);
void handleFileUpload(AsyncWebServerRequest *request);
void SD_dir(AsyncWebServerRequest *request);
void printDirectory(AsyncWebServerRequest *request, const char * dirname, uint8_t levels);
void File_Stream(AsyncWebServerRequest *request);
void SD_file_stream(AsyncWebServerRequest *request, String filename); 
void File_Delete(AsyncWebServerRequest *request);
void SD_file_delete(String filename); 
void SendHTML_Header(AsyncWebServerRequest *request);
void SendHTML_Content(AsyncWebServerRequest *request);
void SendHTML_Stop(AsyncWebServerRequest *request);
void SelectInput(AsyncWebServerRequest *request, String heading1, String command, String arg_calling_name);
void ReportSDNotPresent(AsyncWebServerRequest *request);
void ReportFileNotPresent(AsyncWebServerRequest *request, String target);
void ReportCouldNotCreateFile(AsyncWebServerRequest *request, String target);
void ServePage(AsyncWebServerRequest *request, int page);

void BuildGdxTable(void);

bool WifiConnected; // global status 

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // Return 1 to decode next block
  return 1;
}

void UpdateLCDTask(void * pvParameters)
{ while(1)
  { if(ScrollNow)
    { UpdateLCD2();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // portTICK_PERIOD_MS = 1 ;-)
  }
}

AsyncStaticWebHandler* handler;

void setup()
{ int ret;
  // read settings 
  EEPROM.begin(EEPROM_SIZE);
  NewRadioStation = EEPROM.read(EEPROM_RADIOSTATION);
  if((NewRadioStation<1) || (NewRadioStation>20))
  { NewRadioStation = 1;
    EEPROM.write(EEPROM_RADIOSTATION, NewRadioStation);
    EEPROM.commit();
  }
  OldRadioStation = NewRadioStation;
  DeviceType = EEPROM.read(EEPROM_DeviceType); // 0 = jukebox, 1 = Wallbox
  if((DeviceType<0) || (DeviceType>1))
  { DeviceType = 0;
    EEPROM.write(EEPROM_DeviceType, DeviceType);
    EEPROM.commit();
  }
  EEPROM.end();

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  for(int i=0; i<NUM_LEDS; i++){
        leds[i].setRGB(0, 0, 255);
  }        
  FastLED.show();
 
  // timer for led strip timing en magneet timing
  timer = timerBegin(0, 80, true); // prescaler 1MHz
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000, true); // elke 10mS
  timerAlarmEnable(timer);
  
  xTaskCreatePinnedToCore(
                    AnimateLedstrip,   /* Task function. */
                    "AnimateLedstrip",     /* name of task. */
                    2000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */      
  
 
  xDisplayMutex = xSemaphoreCreateMutex(); 
  
  // 320x240 ILI9341 display  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setViewport(0, 0, 240, 240, true);
  tft.loadFont(Arialnarrow26);
  

  tft.setAttribute(UTF8_SWITCH, false); 

 
  
  tft.setTextSize(2);
  tft.setTextWrap(false, false);
  tft.setPivot(DIAL_CENTRE_X, DIAL_CENTRE_Y);
  createNeedle(); // volume dial needle
  // voor de jpeg decoder
  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);
  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);
  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);


  BuildGdxTable(); 

 
 
  delay(1000);

  xTaskCreatePinnedToCore(
                    ReadJukeKeys,   /* Task function. */
                    "ReadJukeKeys",     /* name of task. */
                    2000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */      


xTaskCreatePinnedToCore(
                    UpdateLCDTask,   /* Task function. */
                    "UpdateLCDTask",     /* name of task. */
                    5000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */      

  TFT_line_print(0, "SETUP");
  TFT_line_print(1, "SD CARD");
  pinMode(19,INPUT_PULLUP);
  Serial.print(F("Initializing SD card...")); 
  pinMode(SD_MISO,INPUT_PULLUP);
  pinMode(SD_MOSI,INPUT_PULLUP);

  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, sdSPI))
  {  
    Serial.println(F("Card failed or not present, no SD Card data logging possible..."));
    SD_present = false; 
  } 
  else
  {
    Serial.println(F("Card initialised... file access enabled..."));
    SD_present = true; 
  }
  delay(2000);
  if(SD_present == true)TFT_line_print(1, "SD CARD OK");
  else TFT_line_print(1, "NO SD CARD");
  delay(2000);
  
  
  TFT_line_print(0, "WIFI SETUP");
  TFT_line_print(1, "");
  
  setup2();
  
  delay(2000);
  if (WiFi.status() != WL_CONNECTED)
  { TFT_line_print(1, "FAIL");
    TFT_line_print(2, "NO SSID");
    TFT_line_print(3, "NO PASSWORD");
    TFT_line_print(4, "");
    TFT_line_print(5, "Use Your Smartphone And Connect To JUKEBOX-PORTAL For Your Wifi Setup");
    bOpenPortal = true;
    loop2();  
    TFT_line_print(5, "Setup Done --- Will Reboot Now");
    delay(2000);
    ESP.restart();
  }

// configure web server
  config.ssid = default_ssid;
  config.wifipassword = default_wifipassword;
  config.httpuser = default_httpuser;
  config.httppassword = default_httppassword;
  config.webserverporthttp = default_webserverporthttp;

  Serial.println("Configuring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer();
  handler = &server->serveStatic("/", SD, "/").setCacheControl("max-age=6000");

  // startup web server
  Serial.println("Starting Webserver ...");
  server->begin();

  TFT_line_print(0, "SERVICES");
  TFT_line_print(1, "WebServer");
  char text[32];
  sprintf(text, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);  
  TFT_line_print(2, text);
  TFT_line_print(3, "");
  TFT_line_print(4, "");
  TFT_line_print(5, "");

  delay(1000);

  if(DeviceType==WALLBOX)
  { sprintf(text, "WALLBOX-%d", WiFi.localIP()[3]);  
  }
  else if(DeviceType==JUKEBOX)
  { sprintf(text, "JUKEBOX-%d", WiFi.localIP()[3]);  
  }
  else
  { sprintf(text, "ESP32-%d", WiFi.localIP()[3]);  
  }
  setupOTA(text); 
  TFT_line_print(3, "HostName");
  TFT_line_print(4, WiFi.getHostname());
  delay(2500);


  Serial.println(WiFi.localIP());

  pinMode(SOLENOID, OUTPUT);


  pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ROTARY_ENCODER_A_PIN, INPUT);  
  pinMode(ROTARY_ENCODER_B_PIN, INPUT);
  pinMode(CANCEL_BUTTON, INPUT);  // dit is de grote witte knop links, normally closed - zit op D35 en heeft een externe 10K pullup
  pinMode(BLUE_BUTTON, INPUT_PULLUP);  // dit is de blauwe knop rechts, normally open
  pinMode(ENCODER_BUTTON, INPUT_PULLUP);  // dit is de volume drukknop links, normally open
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR); 
  rotaryEncoder.setBoundaries(0,100,false); // with end stop
  rotaryEncoder.setAcceleration(0); // at 25 already questionable behaviour


  // mqtt for sending and receiving events to home assistant domotica
  // broker ip address defined in mqtt.h
  setupMQTT();
  BootFase = 1;

//  Serial.println("\n\nNetwork Configuration:");
//  Serial.println("----------------------");
//  Serial.print("         SSID: "); Serial.println(WiFi.SSID());
//  Serial.print("  Wifi Status: "); Serial.println(WiFi.status());
//  Serial.print("Wifi Strength: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
//  Serial.print("          MAC: "); Serial.println(WiFi.macAddress());
//  Serial.print("           IP: "); Serial.println(WiFi.localIP());
//  Serial.print("       Subnet: "); Serial.println(WiFi.subnetMask());
//  Serial.print("      Gateway: "); Serial.println(WiFi.gatewayIP());
//  Serial.print("     Hostname: "); Serial.println(WiFi.getHostname());
//  Serial.print("        DNS 1: "); Serial.println(WiFi.dnsIP(0));
//  Serial.print("        DNS 2: "); Serial.println(WiFi.dnsIP(1));
//  Serial.print("        DNS 3: "); Serial.println(WiFi.dnsIP(2));
//  Serial.println();
  
  Serial.println("Setup done.");
}


extern const char *p_MediaSource[];
extern JukeBoxSong JukeBoxSongs[];
static int songcount=0;
int getSDPlayList(void);

extern TFTline MyDisplay[];

void loop()
{ unsigned long currentMillis; // doorlopende milliseconde timer
  static int FoundSonosDevices;
  static int Select;
int t;
char c,text[128];
static int CountDown;
int n;

//FullTrackInfo info;


//  currentMillis = millis();  
//  Serial.println(currentMillis); 

  if(BootFase) // after setup, BootFase == 1
  { if(UpdateTimeOut10mS == 0)
    {  Serial.print("Bootfase=");Serial.println(BootFase);
       if(BootFase<100 && BootFase!=7) // bootfase 7 is the countdown timer in the top bar - let it go undisturbed - yes clumsy it is
       { sprintf(text, "BOOTFASE: %d", BootFase);
         TFT_line_print(0, text);
       }  
       switch(BootFase)
       { case 1:
          AllSet = false;
          g_SDfallback = false;
          g_Playlistfound = false;
          NewSonosSourceMode = SONOS_SOURCE_UNKNOWN;
          SonosSourceMode = SONOS_SOURCE_UNKNOWN;        
          ShowVolume10mS = 0;
          BootFase++;
          TFT_line_print(1, "");
          TFT_line_print(2, "");
          TFT_line_print(3, "");
          TFT_line_print(4, "");
          if(DeviceMode==0)TFT_line_print(5, "JUKEBOX");
          else if(DeviceMode==1)TFT_line_print(5, "WALLBOX");
          else TFT_line_print(5, "ESP32");
          
          UpdateTimeOut10mS = 100;
          break;
        case 2:
          TFT_line_print(5, "SEARCHING FOR SONOS DEVICES");
          BootFase++;
          UpdateTimeOut10mS = 100; // one second
          break;
        case 3:  
          // connect to a Sonos box or fail
          ACTIVE_sonosIP = GetSonosSetup(&FoundSonosDevices);
          if(ACTIVE_sonosIP)
          { sprintf(ACTIVE_sonosHeaderHost, HEADER_HOST, ACTIVE_sonosIP[0], ACTIVE_sonosIP[1], ACTIVE_sonosIP[2], ACTIVE_sonosIP[3], UPNP_PORT); // 29 bytes max  
            BootFase=6;
          }
          else
          { NewSonosSourceMode = SONOS_SOURCE_UNKNOWN;
            SonosSourceMode = SONOS_SOURCE_UNKNOWN;
            CountDown = 15;
            UpdateTimeOut10mS = 100;
            ShowVolume10mS = 0;
            TFT_line_print(5, "No Sonos Found --- New Scan Will Start In A Moment");
            BootFase++;
          }
          UpdateTimeOut10mS = 200;
          break;
          
       case 4:   
          UpdateTimeOut10mS = 100;
          if(CountDown)
          { sprintf(text, "%d", CountDown);
            TFT_line_print(4, text);
            CountDown--;
          }
          else
          { TFT_line_print(4, "0");
            TFT_line_print(5, "New Scan Starting");
            BootFase++;
            UpdateTimeOut10mS = 500;
          }
          break;

        case 5:
          TFT_line_print(4, "");
          TFT_line_print(5, "");
          BootFase = 2;
          break;

        case 6: // we have wifi and one or more sonos devices found, one selected already
          if(FoundSonosDevices>1) // offer oppertuntiy to change selected sonos device
          { CountDown = 150; // 15 seconds time to give user oppertunity to select other sonos device from the list of found ones
            TFT_line_print(5, "Select Sonos With Volume Knob Or Just Wait");
            sprintf(text, "WAIT %d", CountDown/10);
            TFT_line_print(0, text);
            UpdateTimeOut10mS = 100;
            Select = rotary_loop(SonosLastUsed*5)/5; // sets knob to last sonos used (default choice) and use that for starters
            BootFase++;
          }
          else BootFase = 9; // skip the selection party
          break;

       case 7: // let user select an other sonos or just timeout
          UpdateTimeOut10mS = 10;
          if(CountDown)
          { sprintf(text, "WAIT %d", CountDown/10);
            TFT_line_print(0, text);
            CountDown--;
            Select = rotary_loop(-1) / 3;
            if(Select>FoundSonosDevices-1)
            { Select = FoundSonosDevices-1;
              Select = rotary_loop(Select*3)/3; // limit knob to max choice
            }
            for(n=0;n<FoundSonosDevices;n++)
            { if(n==Select)
              { //sprintf(text, "<%d> - %s", n+1, G_SonosDeviceList[n].Zonename);
                sprintf(text, "%s", G_SonosDeviceList[n].Zonename);
                TFT_line_print(n+1, text);
                TFT_line_color(n+1, TFT_WHITE);
              }
              else 
              { // sprintf(text, "%d - %s", n+1, G_SonosDeviceList[n].Zonename);
                sprintf(text, "%s", G_SonosDeviceList[n].Zonename);
                TFT_line_print(n+1, text);
                TFT_line_color(n+1, TFT_SILVER);
            }
            }
          }  
          else
          { TFT_line_print(0, "0");
            Serial.print("Select=");Serial.print(Select);Serial.print("SonosLastUsed=");Serial.println(SonosLastUsed);
            if(Select != SonosLastUsed)
            { Serial.print("Saved Select=");Serial.print(Select);Serial.print("SonosLastUsed=");Serial.println(SonosLastUsed);
              EEPROM.begin(EEPROM_SIZE);
              EEPROM.write(EEPROM_SONOS_LASTUSED, Select);
              EEPROM.commit();
              EEPROM.end();
              SonosLastUsed = Select;
            }
            BootFase++;
          }
          break;
        
        case 9: 
          ACTIVE_sonosIP = G_SonosFound_IPList[SonosLastUsed];
          sprintf(ACTIVE_sonosHeaderHost, HEADER_HOST, ACTIVE_sonosIP[0], ACTIVE_sonosIP[1], ACTIVE_sonosIP[2], ACTIVE_sonosIP[3], UPNP_PORT); // 29 bytes max  
          TFT_line_print(1, G_SonosDeviceList[SonosLastUsed].Zonename);
          TFT_line_print(2, "MODE");
          TFT_line_print(3, "");
          TFT_line_print(4, "");
          TFT_line_print(5, "RETRIEVING SONOS MODUS OPERANDI");
          runAsyncClient(SONOSGETMODE);
          BootFase++;
          UpdateTimeOut10mS = 500;
          break;

        case 10:
          TFT_line_print(3, "STATE");
          runAsyncClient(SONOSGETSTATE);
          BootFase++;
          UpdateTimeOut10mS = 200;
          break;

        case 11:
          TFT_line_print(4, "VOLUME");
          runAsyncClient(SONOSGETVOLUME);          
          BootFase++;
          UpdateTimeOut10mS = 200;
          break;

        case 12:
          BootFase++;
          UpdateTimeOut10mS = 200;
          break;

        case 13:
          if(ActualVolumeFromSonos>=0) // valid reading received from sonos
          { AsyncVolumeForSonos = ActualVolumeFromSonos;
            NewVolumeForSonos = rotary_loop(ActualVolumeFromSonos);
            MinimumVolumeForSonos = SONOSDEFAULTVOLUME;
            // all volume stuff initialised
            BootFase++;
          }
          else BootFase = 9; // loop back 
          break;
        case 15:
          TFT_line_print(2, "");
          TFT_line_print(3, "");
          TFT_line_print(4, "");
          BootFase++;
          break;
        case 20: // 
          TFT_line_print(5, "LOADING PLAYLIST TRACKS FROM SONOS");
          // try to load a list of songs from playlist 'Jukebox'on Sonos
          // 101 for a full playlist including the notification tune for to use for empty slots
          songcount = G_Sonos.getSonosPlayLists2(ACTIVE_sonosIP); // 101 if full playlist plus AALEEG.MP3

          n = songcount; 
          if(n<0) // no playlist found - fallback to music from SD card
          { if(SD_present)
            { 
              TFT_line_print(5, "NO SONOS PLAYLIST \'Jukebox\' FOUND - LOADING SONGS FROM SD CARD");
              songcount = getSDPlayList(); // alternative source for our jukebox
              n = songcount; 
              if(n>1)
              { g_SDfallback = true;
              }
              else
              { TFT_line_print(1, "");
                TFT_line_print(5, "NO SD PLAYLIST FOUND --- NO MUSIC FOR JUKEBOX - ONLY RADIO CAN BE USED");
                songcount = 0; 
              }
            }   
          }
          else
          { g_Playlistfound = true;
            while(n<101)
            { if(songcount>0) // fill remains of the list of 100 with copies of last song
              { strcpy(JukeBoxSongs[n].path, JukeBoxSongs[songcount-1].path); 
                JukeBoxSongs[n].scheme = JukeBoxSongs[songcount-1].scheme;
              }
              else // initialize entire list with blanks - should have initialized with SD cart contents but hey
              { strcpy(JukeBoxSongs[n].path, ""); 
                JukeBoxSongs[n].scheme = 0;
              }
              n++;
            }
            if(songcount>100)songcount=100;
          }

          TFT_line_print(1, "LOADING DONE");
          BootFase++;
          UpdateTimeOut10mS = 200;
         break;

       case 25:
          // cleanup from playlist loading
          TFT_line_print(1, "");
          TFT_line_print(2, "");
          BootFase++;
          break;  

        case 100:
          // connected with Sonos, source mode, state and volumelevel all known
          if(NewSonosSourceMode == SONOS_SOURCE_UNKNOWN)
          { NewSonosSourceMode = SONOS_SOURCE_RADIO; // better than the sound of silence
          }  
          SonosSourceMode = NewSonosSourceMode;
          DeviceMode = JUKEKEYMODE[NewSonosSourceMode]; // SELECTSONG of SELECTRADIO
          if(NewSonosSourceMode == SONOS_SOURCE_RADIO)
          { if(SonosState==3)OldRadioStation = 0; // prepare selection of radiostation later
          }
          BootFase++;
          UpdateTimeOut10mS = 100;
          break;
         
        case 101: // eventually we reach here
          AllSet = true;
          BootFase = 0; // end of boot stuff
          break;

        default: 
          BootFase++; 
          break;
      }
    }
    else
    { // 
    }
  }

  loopMQTT();
  
  if (shouldReboot)
  { //rebootESP("Web Admin Initiated Reboot");
    Serial.println("shouldReboot????????????");
    shouldReboot = false;
  }

  if(encoder_button_very_long_pressed >= 2000) // 10 seconden
  { encoder_button_very_long_pressed = 0;
    AllSet = false; // forces lcd to green color
    server->end();
    TFT_line_print(0, "WIFI SETUP");
    TFT_line_print(1, "");
    TFT_line_print(2, "");
    TFT_line_print(3, "");
    TFT_line_print(4, "");
    TFT_line_print(5, "Use Your Smartphone And Connect To JUKEBOX-PORTAL For Your Wifi Setup");
    bOpenPortal = true;
    loop2();  
    TFT_line_print(5, "Setup Done --- Will Reboot Now");
    delay(2000);
    ESP.restart();
  }


  if(AllSet && SonosSkipRequest)
  { G_Sonos.skip(ACTIVE_sonosIP, SONOS_DIRECTION_FORWARD);
    SonosSkipRequest = 0;
  }   
  

  if(SonosSecondsStopped100mS > 150)
  { // check to see if jukebox finished playing its queue of songs
    //sprintf(text,"STOPPED %d", SonosSecondsStopped100mS);
    //TFT_line_print(4, text);
    //sprintf(text,"QUEUE %d", SongsInQueueCnt);
    //TFT_line_print(4, text);
    //delay(3000);
    if(DeviceMode == SELECTSONG)
    { if(SongsInQueueCnt > 0)
      {  // Serial.print("Sonos Queue Finished - Stopped  -> Radio");
         DeviceMode = SELECTRADIO;
         NewSonosSourceMode = SONOS_SOURCE_RADIO; // faster update to red radio display
         OldRadioStation = 0; // prepares a forced activation of radio
         SonosSecondsStopped100mS = 0;
         UpdateTimeOut10mS = 200; // ignore mode updates from sonos for 2 seconds, until it settled
      }
      else
      { if(SonosSecondsStopped100mS > 200) // no songs selected, back to radio
        { DeviceMode = SELECTRADIO;
          NewSonosSourceMode = SONOS_SOURCE_RADIO; // faster update to red radio display 
          OldRadioStation = 0; // prepares a forced activation of radio
          SonosSecondsStopped100mS = 0;
          UpdateTimeOut10mS = 200; // ignore mode updates from sonos for 2 seconds, until it settled
        }  
      }
    }  
  }
 
  if(AllSet && (DeviceMode == SELECTSONG) && (selectedsong>0))
  { // secret detecting of PIN code 4711 + J/B (4x7x1x1xJ or 4x7x1x1xB) to set the device as [J]ukebox or wall[B]ox
    strcpy(Keys, Keys+1);
    Keys[3] = (selectedsong % 10)+ 0x30;
//    Serial.println("KEYS"); 
//    Serial.println(Keys); 
    if(strcmp(Keys, "4711")==0)
    { Serial.println("ALPHA"); 
      if((selectedsong/10)==1)
      { Serial.println("wallBox");
        DeviceType = WALLBOX;
        EEPROM.write(EEPROM_DeviceType, DeviceType);
        EEPROM.commit();
      }
      else if((selectedsong/10)==8)
      { Serial.println("Jukebox");
        DeviceType = JUKEBOX;
        EEPROM.write(EEPROM_DeviceType, DeviceType);
        EEPROM.commit();
      }
    }

    MinimumVolumeForSonos = SONOSDEFAULTVOLUME;   

    if(!g_SDfallback)
    { Serial.println(p_MediaSource[JukeBoxSongs[selectedsong-1].scheme]); 
      Serial.println(JukeBoxSongs[selectedsong-1].path); 

//        G_Sonos.addTrackToQueue(ACTIVE_sonosIP, "x-sonos-http:librarytrack", "%3ai.GEGNo2YT9W3YYG.mp4?sid=204&flags=8224&sn=5"); // voegt niets toe aan queue, een &amp; is nodig
//        G_Sonos.addTrackToQueue(ACTIVE_sonosIP, "x-sonos-http:librarytrack", "%3ai.GEGNo2YT9W3YYG.mp4?sid=204&amp;flags=8224&amp;sn=5"); // voegt toe aan queue, maar start nog niet
//        G_Sonos.addTrackToQueue(ACTIVE_sonosIP, "x-sonos-http:librarytrack", "%3ai.GEGNo2YT9W3YYG.mp4?sid=204&amp;amp;flags=8224&amp;amp;sn=5"); // voegt toe aan queue, maar start nog niet

      G_Sonos.addTrackToQueue(ACTIVE_sonosIP, p_MediaSource[JukeBoxSongs[selectedsong-1].scheme], JukeBoxSongs[selectedsong-1].path); //// voegt toe aan queue, maar start nog niet

      SongsInQueueCnt++;
    }
    else // files from ESP32 http server
    { char CardFileName[128];
      int n=0;
      const char *p;
      p = JukeBoxSongs[selectedsong-1].path;
      // urencode spaces
      while(*p && (n<120))
      { if( *p != ' ')CardFileName[n++] = *p;
        else
        { CardFileName[n++] = '%';
          CardFileName[n++] = '2';
          CardFileName[n++] = '0';
        }
        CardFileName[n] = 0;
        p++;
      }

      sprintf(SonosTrack, "http://%s/%s", WiFi.localIP().toString().c_str(), CardFileName);

      // sprintf(SonosTrack, "http://192.168.1.38/%s", CardFileName);
      // sprintf(SonosTrack, "http://192.168.1.38/%s", JukeBoxSongs[selectedsong-1].path);
      // Serial.println(SonosTrack); 
      // G_Sonos.addTrackToQueue(ACTIVE_sonosIP, SONOS_SOURCE_LOCALHTTP_SCHEME, SonosTrack); // does not work
      // G_Sonos.addTrackToQueue(ACTIVE_sonosIP, SONOS_SOURCE_HTTP_SCHEME, SonosTrack); // does not work
      G_Sonos.addTrackToQueue(ACTIVE_sonosIP, "", SonosTrack); // works, plays as local http 

      // G_Sonos.playHttp(ACTIVE_sonosIP, "https://www.somewebsite.com/jukebox/J09-Cheerleader.mp3"); // works - plays directly, not queued
      // G_Sonos.addTrackToQueue(ACTIVE_sonosIP, "", "https://www.somewebsite.com/jukebox/J09-Cheerleader.mp3"); // works - plays as local https queued
      SongsInQueueCnt++;
    }
    
    LastSongAddedToQueue = selectedsong;
    selectedsong = 0;

    if(SongsInQueueCnt==1)G_Sonos.playQueue(ACTIVE_sonosIP, ACTIVE_sonosSerialnumber );


    // put sonos in playmode from queue
    if(SonosSourceMode == SONOS_SOURCE_FILE || SonosSourceMode == SONOS_SOURCE_LOCALHTTP || SONOS_SOURCE_LIBRARY) // already in some sort of playlist
    { if(SonosState!=1) // not playing anything yet, could be stopped (3) or paused (2) 
      { Serial.println("Start of hervat spelen uit queue"); 
        G_Sonos.play(ACTIVE_sonosIP);
      }
    }
  }  


  if(AllSet && Ticker100mS == true) 
  { static int SonosStatedPrinted = -1;
    Ticker100mS = false;
    //Serial.print("PollSonosSourceMode: ");Serial.println(PollSonosSourceMode);
    //Serial.print("PollSonosState: ");Serial.println(PollSonosState);
    //Serial.print("PollSonosVolume: ");Serial.println(PollSonosVolume);
    if(PollSonosVolume == true)
    { if(runAsyncClient(SONOSGETVOLUME))
      { PollSonosVolume = false;
      }
    }
    else if(PollSonosSourceMode && (ShowVolume10mS==0))
    { if(runAsyncClient(SONOSGETMODE))
      { PollSonosSourceMode = false; // 
      }
    }
    else if(PollSonosState && (ShowVolume10mS==0))
    { if(runAsyncClient(SONOSGETSTATE))
      { PollSonosState = false; // try again next round
      }
    }

    // Serial.print("Free Heap: ");Serial.println(ESP.getFreeHeap());
    
    if(SonosState==3)SonosSecondsStopped100mS +=1;
    else SonosSecondsStopped100mS =0;

    if(SonosStatedPrinted != SonosState)
    { SonosStatedPrinted = SonosState;
      if(SonosState==3)TFT_line_print(4, "STOPPED");
      else if(SonosState==2)TFT_line_print(4, "PAUSED");
      else if(SonosState==1)TFT_line_print(4, "PLAYING");
      else TFT_line_print(4, "STATE???");
    }  


    if(SonosSourceMode!=SONOS_SOURCE_UNKNOWN)
    { if(NewSonosSourceMode!=SonosSourceMode)
      { NewSonosSourceMode = SonosSourceMode;
        Serial.print("SonosSourceMode changed = ");
        Serial.println(SonosSourceMode); 
        DeviceMode = JUKEKEYMODE[SonosSourceMode]; // SELECTSONG of SELECTRADIO
      }
    }            
  }


  // volume commands from various events such as setup, mode-changes, volume encoder pot or external wallbox, sonos app
  if(AllSet) 
  { // Serial.print("1-ActualVolumeFromSonos:");Serial.println(ActualVolumeFromSonos);
    // Serial.print("2-AsyncVolumeForSonos:");Serial.println(AsyncVolumeForSonos);

    if(AsyncVolumeForSonos>=0)
    { if(AsyncVolumeForSonos != ActualVolumeFromSonos)
      { // Serial.print("3-AsyncVolumeForSonos:");Serial.println(AsyncVolumeForSonos);
        if(UpdateTimeOut10mS==0)
        { UpdateTimeOut10mS = 25; // 200mS timeout 
          // Serial.print("SET AsyncVolumeForSonos:");Serial.println(AsyncVolumeForSonos);
          runAsyncClient(SONOSSETVOLUME);
          PollSonosVolume = true; // get the result faster
        }
      }
      else AsyncVolumeForSonos = -1; // done, synched
    }
    else // we are synched
    {  if(ActualVolumeFromSonos != NewVolumeForSonos)
       { // volume discrepancy must come from external app or whatever
         NewVolumeForSonos = rotary_loop(ActualVolumeFromSonos);
      }
    }
        
    if(MinimumVolumeForSonos>0)
    { // Serial.print("MinimumVolumeForSonos:");Serial.println(MinimumVolumeForSonos);
      if(MinimumVolumeForSonos > rotary_loop(-1))
      { // Serial.print("ActualVolumeFromSonos:");Serial.println(ActualVolumeFromSonos);
        NewVolumeForSonos = rotary_loop(MinimumVolumeForSonos); 
        MinimumVolumeForSonos=0;
      }
    }

    { NewVolumeForSonos = rotary_loop(-1); // only read value   
      if(NewVolumeForSonos!=ActualVolumeFromSonos)
      { //if(AsyncVolumeForSonos==0) 
         AsyncVolumeForSonos = NewVolumeForSonos;
      }
    }
  }    

  

  // moved to dedicated task - Z10Jukeboxledstrip sketch
  // AnimateLedstrip(); 

  if(blue_buttonf == deb_blue_button) // state change blue button next to display
  { blue_buttonf = 1234;
    if(deb_blue_button==0)  // pressed?
    { Serial.print("Blue Button -> Cancel");
      cancel_button_long_pressed = 200; // simulate long press of cancel button
    }
  }

  if(encoder_buttonf == deb_encoder_button) // state change
  { encoder_buttonf = 1234;
    if(deb_encoder_button==0)  // pressed?
    { Serial.print("Volume Button -> skip");
      { if(DeviceMode == SELECTSONG)SonosSkipRequest = 1;
        else // radio mode
        { // place for a new idea 
        }
      }
    }
  }


  
  if(AllSet && (cancel_button_long_pressed >= 200))
  { // cancel pressed for > 2 seconds
    Serial.print("Cancel Long Pressed");
    cancel_button_long_pressed = 0;
    CancelButtonEnabled = 0;
    MinimumVolumeForSonos = SONOSDEFAULTVOLUME; // set minimum value for volume
    
    if(DeviceMode == SELECTRADIO)
    { DeviceMode = SELECTSONG;
      ShowArt10mS=0; // away with radio logo
      Serial.print("Cancel -> Jukebox");
      G_Sonos.removeAllTracksFromQueue(ACTIVE_sonosIP); // fraai is het niet
      SongsInQueueCnt = 0;
      G_Sonos.playQueue(ACTIVE_sonosIP, ACTIVE_sonosSerialnumber); // also stops radio playing
      NewSonosSourceMode = SONOS_SOURCE_FILE; // dit om te verkomen dat dat zou gebeuren
      SonosSourceMode=SONOS_SOURCE_UNKNOWN; // we never know unless received new status
      UpdateTimeOut10mS = 200; // ignore mode updates from sonos for 2 seconds, until it settled
    }
    else 
    { Serial.print("Cancel -> Radio");
      DeviceMode = SELECTRADIO;
      NewSonosSourceMode = SONOS_SOURCE_RADIO;
      SonosSourceMode=SONOS_SOURCE_UNKNOWN; // we never know unless received new status
      OldRadioStation = 0; // forces radio activation
      UpdateTimeOut10mS = 200; // ignore mode updates from sonos for 2 seconds, until it settled
    } 
  }
  
  
  if((AllSet) && (DeviceMode == SELECTRADIO))  
  { if(NewRadioStation != OldRadioStation)
    { if((NewRadioStation<1) || (NewRadioStation>20))NewRadioStation = 1; // force valid number
      OldRadioStation = NewRadioStation; 
      TFT_line_print(2, RadioStations[NewRadioStation-1][1]);
      TFT_line_print(3, "");
      TFT_line_print(4, "");
      ShowArt10mS = 500;
      PollSonosSourceMode = true; // speed up inquiry
      MinimumVolumeForSonos = SONOSDEFAULTVOLUME; // set minimum value for volume
      Serial.print("Select NewRadioStation = ");
      Serial.println(NewRadioStation); 
      G_Sonos.playRadio(ACTIVE_sonosIP, RadioStations[NewRadioStation-1][0], RadioStations[NewRadioStation-1][1]);   
      EEPROM.write(EEPROM_RADIOSTATION, NewRadioStation);
      EEPROM.commit();
    }
  }

  
}






int rotary_loop(int resetvalue) {
  static int oldEncoderValue = -1;
  static int encoderValue;

  if(resetvalue>=0)
  { rotaryEncoder.reset(resetvalue);
    oldEncoderValue = resetvalue;
    return resetvalue;
  }
  
  encoderValue = rotaryEncoder.readEncoder();
  if(encoderValue == oldEncoderValue)return encoderValue;
  oldEncoderValue = encoderValue;
  if(BootFase)return encoderValue;

  if(NewSonosSourceMode!=SONOS_SOURCE_UNKNOWN) // don't do anything with no Sonos present
  if(Show45RPM10mS)return encoderValue; // geen volumeregelaar tonen als er een plaatje in beeld is
  if(ShowRadio10mS)return encoderValue; // geen volumeregelaar tonen als er een plaatje in beeld is

  if(ShowArt10mS)ShowArt10mS=0; // volume display is more important than art
  if(ShowVolume10mS==0)plotNeedle(1234, 0); // position needle before volume control is displayed
  if(ShowVolume10mS>300)ShowVolume10mS = 6000; // verlenging van de lange modus bij een klik in radio mode
  else ShowVolume10mS = 300; // alleen de volume regelaar laten zien in een van de hoofdmenus

  Serial.print("Rotaryloop - Sonos Volume set encoderValue= ");
  Serial.println(encoderValue);
  return encoderValue;
}



void rebootESP(String message) {
  Serial.print("Rebooting ESP32: "); Serial.println(message);
  ESP.restart();
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml) {
  String returnText = "";

  if(!SD_present)
  { returnText = "No SD Card Installed!!!";
    return returnText;
  }
  Serial.println("Listing files stored on SD Card");
  File root = SD.open("/"); // was SPIFFS.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()/1024) + "</td>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
    } else {
      returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
//String humanReadableSize(const size_t bytes) {
//  if (bytes < 1024) return String(bytes) + " B";
//  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
//  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
//  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
//}

String humanReadableSize(const size_t kbytes) {
  if (kbytes < (1024)) return String(kbytes) + " KB";
  else if (kbytes < (1024 * 1024)) return String(kbytes / 1024.0) + " MB";
  else return String(kbytes / 1024.0 / 1024.0) + " GB";
}


int GetSlotForMusicFile(char *musicfilename)
{ int n, slot=0;
  char c = musicfilename[0];
  // files on SD card should start with a letter followed by a number
  // this defines their link with the jukebox buttons
  // A00whatever.mp3 is reserved for the last 'slot' and the idea is that contains a 'oops/sorry/alas' sound to be played when nothing else is available
  
  if(isalpha(c))
  { c = toupper(c);
    for(n=0;n<10;n++)
    { if(c==KnobDecals[n])  // const char KnobDecals[]="ABCDEFGHJK1234567890";
      { if(n>0)slot=n*10;
      }
    }
    c = musicfilename[1];
    if(isdigit(c))
    { c &= 0x0f;
      if(c)slot+=10;
      c = musicfilename[2];
      if(isdigit(c))
      { c&=0x0f;
        slot += c;
        if(slot==0)return 100; // A00 file
        if(slot>100)return 100;
        return slot-1;
      }
    }
  }
  return -1;
}

bool ismusicfile(char *musicfilename)
{ char *dot;

  if((dot=strrchr(musicfilename, '.'))!=NULL)
  { dot++;
    if(strncmp(dot, "mp3",3)==0)return true;
    if(strncmp(dot, "wav",3)==0)return true;
    if(strncmp(dot, "flac",4)==0)return true;
    if(strncmp(dot, "ogg",4)==0)return true;
  }
  return false;
}

int getSDPlayList(void)
{ int songcount = 0;
  char musicfilename[128];
  char musicsongname[128];
  char text[16];
  int slot;
  char *dot;
  char *dash;
  // songtitles from SD card and sort them so the match the ABCDEFGHJK1234567890 keys on the jukbeox
  File root = SD.open("/"); // was SPIFFS.open("/");
  File foundfile = root.openNextFile();
  while (foundfile)
  { if(!foundfile.isDirectory())
    { strcpy(musicfilename, foundfile.name());
      if(ismusicfile(musicfilename))
      { if((dot=strrchr(musicfilename, '.'))!=NULL)*dot=0; // remove file extension
        slot=GetSlotForMusicFile(musicfilename);
        if(slot>=0)     
        { 
          //Serial.println(musicfilename);
          memcpy(musicfilename, &musicfilename[4], sizeof(musicfilename)-4);
          if((dash=strrchr(musicfilename, '-'))!=NULL)
          { *(dash)=0; // trim at '-' seperator artist - songtitle
            //Serial.println("dashfound");
            strcpy(musicsongname, dash+1);
            musicsongname[31]=0; // max 32 chars
            musicfilename[31]=0; // max 32 chars
            strcpy(JukeBoxSongs[slot].title, musicsongname);
            strcpy(JukeBoxSongs[slot].artist, musicfilename);
            //Serial.println(JukeBoxSongs[slot].title);
            //Serial.println(JukeBoxSongs[slot].artist);

          }
          else // no seperator found
          { //Serial.println("no dashfound");
            musicfilename[31]=0; // max 32 chars
            strcpy(JukeBoxSongs[slot].title, musicfilename);
            strcpy(JukeBoxSongs[slot].artist, "Unknown Artist");
            //Serial.println(JukeBoxSongs[slot].title);
            //Serial.println(JukeBoxSongs[slot].artist);
          }
          strcpy(JukeBoxSongs[slot].path, foundfile.name());
          JukeBoxSongs[songcount].scheme = 0; // not that it matters
          songcount++;
          if(songcount<=100)
          { sprintf(text, "TRACK %d", songcount); 
            TFT_line_print(2, text);
          }  
        }
      } 
    } 
    foundfile = root.openNextFile();
  }
  root.close();
  foundfile.close();
  if(songcount==0)return -1; // did not find any song files on SD card
  return songcount;
}


void SendCommandToSonos(int command);
void DecodeMessageFromSonos(void * data, size_t len);

bool runAsyncClient(int command){
  static int cmd;


  if(aClient)//client already exists
    return false;

  aClient = new AsyncClient();
  if(!aClient)//could not allocate client
    return false;

  cmd = command;


  aClient->onError([](void * arg, AsyncClient * client, int error){
    Serial.println("Connect Error");
    aClient = NULL;
    delete client;
  }, NULL);

  aClient->onConnect([](void * arg, AsyncClient * client){
    Serial.println("aClient Connected");
    aClient->onError(NULL, NULL);

    client->onDisconnect([](void * arg, AsyncClient * c){
      Serial.println("aClient Disconnected");
      aClient = NULL;
      delete c;
    }, NULL);

    client->onData([](void * arg, AsyncClient * c, void * data, size_t len){
//      Serial.print("\r\nData: ");
//      Serial.println(len);
//      uint8_t * d = (uint8_t*)data;
//      for(size_t i=0; i<len;i++)
//        Serial.write(d[i]);
      DecodeMessageFromSonos(data, len);  
    }, NULL);

    //send the request
    // client->write("GET / HTTP/1.0\r\nHost: www.google.com\r\n\r\n");
    SendCommandToSonos(cmd);
  }, NULL);

  if(!aClient->connect(ACTIVE_sonosIP, 1400)){
    Serial.println("Connection Async Sonos Failed!");
    AsyncClient * client = aClient;
    aClient = NULL;
    delete client;
    return false;
  }
  else
  { Serial.println("Connection Async Sonos Established!");
//    SendCommandToSonos(1);
  }
  return true;
}

void SendCommandToSonos(int command)
{ // first, lets try to get the current volume level
 int len;
 char value[16];

//#define SONOSGETVOLUME 1
//#define SONOSSETVOLUME 2
//#define SONOSGETMODE 3 // radio, playing file, whatever
//#define SONOSGETSTATE 4 // stopped, playing, paused

 
 switch(command)
 { case SONOSGETVOLUME: //getvolume
     aClient->write("POST /MediaRenderer/RenderingControl/Control HTTP/1.1\r\n");
     aClient->write(ACTIVE_sonosHeaderHost); // something like -> Host: 192.168.1.29:1400\r\n
     aClient->write("Content-Type: text/xml; charset=\"utf-8\"\r\nContent-Length: 290\r\nSOAPAction: \"urn:schemas-upnp-org:service:RenderingControl:1#GetVolume\"\r\nConnection: close\r\n\r\n"); 
     aClient->write("<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetVolume xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\"><InstanceID>0</InstanceID><Channel>Master</Channel></u:GetVolume></s:Body></s:Envelope>");
     Serial.println("POST voor volume get is de deur uit");
     break;
   case SONOSSETVOLUME: //setvolume   
     len = 322;
     if(AsyncVolumeForSonos>9)
     { len = 323;
       if(AsyncVolumeForSonos>99)len = 324;
     }
     sprintf(value, "%d", len);
     aClient->write("POST /MediaRenderer/RenderingControl/Control HTTP/1.1\r\n");
     aClient->write(ACTIVE_sonosHeaderHost); // something like -> Host: 192.168.1.29:1400\r\n
     aClient->write("Content-Type: text/xml; charset=\"utf-8\"\r\nContent-Length: ");
     aClient->write(value);
     aClient->write("\r\nSOAPAction: \"urn:schemas-upnp-org:service:RenderingControl:1#SetVolume\"\r\nConnection: close\r\n\r\n");

     sprintf(value, "%d", AsyncVolumeForSonos);
     aClient->write("<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetVolume xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\"><InstanceID>0</InstanceID><DesiredVolume>");
     aClient->write(value);
     aClient->write("</DesiredVolume><Channel>Master</Channel></u:SetVolume></s:Body></s:Envelope>");
     Serial.print(value);
     Serial.println(" -> POST voor volume set is de deur uit");
     break;
   case SONOSGETMODE: // sonos source mode
     aClient->write("POST /MediaRenderer/AVTransport/Control HTTP/1.1\r\n");
     aClient->write(ACTIVE_sonosHeaderHost); // something like -> Host: 192.168.1.29:1400\r\n
     aClient->write("Content-Type: text/xml; charset=\"utf-8\"\r\nContent-Length: 272\r\nSOAPAction: \"urn:schemas-upnp-org:service:AVTransport:1#GetPositionInfo\"\r\nConnection: close\r\n\r\n");
     aClient->write("<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetPositionInfo xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID></u:GetPositionInfo></s:Body></s:Envelope>");
     Serial.println(" -> POST voor Sonos source mode opvraag is de deur uit");
     break;
   case SONOSGETSTATE:
     aClient->write("POST /MediaRenderer/AVTransport/Control HTTP/1.1\r\n");
     aClient->write(ACTIVE_sonosHeaderHost); // something like -> Host: 192.168.1.29:1400\r\n
     aClient->write("Content-Type: text/xml; charset=\"utf-8\"\r\nContent-Length: 274\r\nSOAPAction: \"urn:schemas-upnp-org:service:AVTransport:1#GetTransportInfo\"\r\nConnection: close\r\n\r\n");
     aClient->write("<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetTransportInfo xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID></u:GetTransportInfo></s:Body></s:Envelope>");
     Serial.println(" -> POST voor Sonos get status opvraag is de deur uit");
     break;
  
  default:   
     Serial.println("POST voor wat???????????????????????????????????");
     break;

 }    
}


char Chop1[5000];
char MyMessage[1024];

void DecodeMessageFromSonos(void * data, size_t len)
{ uint8_t * d = (uint8_t*)data;
  int ssm;

//  Serial.println("Decoding Asynchronous response started"); 
//  for(size_t i=0; i<len;i++)
//  Serial.write(d[i]);
  if(len>4999)
  { Serial.print("DecodeMessageFromSonos Len = ");Serial.println(len); 
    len = 4999;
  }
  memmove(Chop1, data, len);
  Chop1[len]=0;

// <CurrentVolume>20</CurrentVolume>

 char * p = Chop1;
 char *p2, *p3, *p4, *p5;
 
 int n;
 //Serial.println(Chop1);
 
      while((p=strchr(p,'<'))!=NULL)
      { if((p2=strchr(p,'>'))!=NULL)
        { // <> found
          p2++;
          memmove(Chop1, p, p2-p);
          Chop1[p2-p]=0;
//          Serial.println(Chop1); // prints the found tag <....> 

          // series of tests on the tag found
          if((p3=strstr(Chop1, "<CurrentVolume>"))!=NULL)
          { // p2 = start of this tag content
            // if((p3=strstr(p2, "</CurrentVolume>"))!=NULL)
            if((p3=strstr(p2, "<"))!=NULL) // faster and same result
            { memmove(MyMessage, p2, p3-p2);
              MyMessage[p3-p2]=0;
//              Serial.println(MyMessage); // prints the found value as text
              sscanf(MyMessage, "%d", &ActualVolumeFromSonos);
//                rotary_loop(ActualVolumeFromSonos); // sync volume knob to this level  
            }
          }

          else if((p3=strstr(Chop1, "<CurrentTransportState>"))!=NULL)
          { // p2 = start of this tag content
            // if((p3=strstr(p2, "</CurrentVolume>"))!=NULL)
            if((p3=strstr(p2, "<"))!=NULL) // faster and same result
            { memmove(MyMessage, p2, p3-p2);
              MyMessage[p3-p2]=0;
              if(strcmp("PLAYING", MyMessage)==0)SonosState=1;
              else if(strcmp("PAUSED_PLAYBACK", MyMessage)==0)SonosState=2;
              else if(strcmp("STOPPED", MyMessage)==0)SonosState=3;
              else Serial.println(MyMessage); // prints the found value as text
            }
          }

          else if((p3=strstr(Chop1, "<TrackURI>"))!=NULL)
          { // p2 = start of this tag content
            // if((p3=strstr(p2, "</TrackURI>"))!=NULL)
            if((p3=strstr(p2, "<"))!=NULL) // faster and same result
            { memmove(MyMessage, p2, p3-p2);
              MyMessage[p3-p2]=0;
              if(BootFase || (UpdateTimeOut10mS ==0)) // ignore this source mode during changing from radio to jukebox and vice verca
              { SonosSourceMode = G_Sonos.getSourceFromURI(MyMessage); // check with lookuptable
              }
              Serial.println(MyMessage); // prints the found value as text
//              Serial.println(SonosSourceMode);
            }
          }
          else if(AllSet &&(p3=strstr(Chop1, "<TrackMetaData>"))!=NULL)
          { // p2 = start of this tag content
            // if((p3=strstr(p2, "</TrackMetaData>"))!=NULL)
            if((p3=strstr(p2, "<"))!=NULL) // faster and same result
            { memmove(MyMessage, p2, p3-p2);
              MyMessage[p3-p2]=0;
              Serial.println(MyMessage); // prints the found value as text
              CleanEncoded(MyMessage);
              UnRavelDidl(MyMessage);
            }
          }


                
                else if((p3=strstr(Chop1, "<item id=\'S:"))!=NULL)
                { p4 = p3+12; // start of path
                  //Serial.println(p4);

                  if((p3=strchr(p4, '\''))!=NULL)
                  { *p3=0;
                    strcpy(JukeBoxSongs[songcount].path, p4);                
//                    Serial.println(JukeBoxSongs[songcount].path); 
                  }
                }  
                else if((p3=strstr(Chop1, "<dc:title>"))!=NULL)
                 { // p2 = start of title
                 
                  if((p3=strstr(p2, "</dc:title>"))!=NULL)
                  { memmove(MyMessage, p2, p3-p2);
                    MyMessage[p3-p2]=0;
                    // splitten in geval van J03-Willeke Alberti-De Bruid.mp3" 
                    // volledige files komen nog met een <dc:.creator> enz.
                    if(GetSlotForMusicFile(MyMessage)>=0) // file, mp3 or wav, from own libray on NAS, with a Jukebox designator   
                    { if((p5=strrchr(MyMessage, '.'))!=NULL)*p5=0; // remove file extension
                      if((p5=strrchr(MyMessage, '-'))!=NULL)
                      { strcpy(JukeBoxSongs[songcount].title, (p5+1));
                        *p5=0;
                        if((p5=strrchr(MyMessage, '-'))!=NULL)
                        strcpy(JukeBoxSongs[songcount].artist, (p5+1));
                      } 
                    }
                    else // song from a sonos playlist, can be from own NAS library or apple music
                    { // file met tags - die zijn vaak lang
                      while(strlen(MyMessage)>29)
                      { if((p5=strrchr(MyMessage, '('))!=NULL)*p5=0; // get rid of additional (...) text in song title
                        else
                        { if((p5=strrchr(MyMessage, ' '))!=NULL)*p5=0; // or trim after a space
                          else if(strlen(MyMessage)>29)
                          { MyMessage[29]=0; // or just cut it with an axe
                          }
                        }
                      }
                    
                      strcpy(JukeBoxSongs[songcount].title, MyMessage);
                    }                
//                    Serial.println(JukeBoxSongs[songcount].title); 
                  }
                }                                  
                else if((p3=strstr(Chop1, "<dc:creator>"))!=NULL)
                 { // p2 = start of this tag
                  if((p3=strstr(p2, "</dc:creator>"))!=NULL)
                  { memmove(MyMessage, p2, p3-p2);
                    MyMessage[p3-p2]=0;
                    // some artist titles are just too long - shortening also makes them easier to fit on the jukebox strips
                    while(strlen(MyMessage)>31)
                    { if((p5=strrchr(MyMessage, '/'))!=NULL)*p5=0; // get rid of additional /.... text in artist name
                      else
                      { if((p5=strrchr(MyMessage, ' '))!=NULL)*p5=0; // or trim after a space
                        else if(strlen(MyMessage)>31)
                        { MyMessage[31]=0; // or just cut it with an axe
                        }
                      }
                    }
                    strcpy(JukeBoxSongs[songcount].artist, MyMessage);                
                    Serial.println(JukeBoxSongs[songcount].artist); 
                  }
                }
                else if((p3=strstr(Chop1, "<res "))!=NULL)
                { // &lt;res protocolInfo=&quot;x-file-cifs:*:audio/wav:*&quot;&gt;x-file-cifs://FILEHUB/SDCard_Volume1/Share/Jukebox1/B07-Sexual%20Healing.wav&lt;/res&gt;
                  // <res protocolInfo='sonos.com-http:*:audio/mp4:*' duration='0:03:26'>x-sonos-http:librarytrack:.DVENxPQHeA3vvX.mp4?sid=204&flags=8224&sn=5</res>
                  // p2 = start of res
                  if((p3=strstr(p2, "</res>"))!=NULL)
                  { memmove(MyMessage, p2, p3-p2);
                    // arrived at x-file-cifs://FILEHUB/SDCard_Volume1/Share/Jukebox1/B07-Sexual%20Healing.wav
                    // arrived at x-sonos-http:librarytrack%3a.DVENxPQHeA3vvX.mp4?sid=204&flags=8224&sn=5
//                      Serial.println(MyMessage);
                      // scheme of the song, convert to integer from table search
                      JukeBoxSongs[songcount].scheme = SONOS_SOURCE_UNKNOWN; 
                       for(n=0;n<SONOS_MAXSOURCE;n++)  // see list of defined schemes in SonosUPnp.h
                      { if(memcmp(p_MediaSource[n], MyMessage, strlen(p_MediaSource[n]))==0)
                        { JukeBoxSongs[songcount].scheme = n; 
//                          Serial.println(n);
                          break; // break out of the for loop
                        }                
                      }
                      //*(p5+1)='/'; // paste back
                      // get path
                      //if((p5=strrchr(MyMessage, ':'))!=NULL)
                      if((p5=strstr(MyMessage, "%3a"))!=NULL)
                      
                      {  strcpy(JukeBoxSongs[songcount].path, p5);
//                      Serial.println("pppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp");
//                      Serial.println(MyMessage);
//                      Serial.println(p5);
                      }
                  }
                }
                else if ((p3=strstr(Chop1, "<NumberReturned>"))!=NULL)
                { // p2 = start of message
                  if((p3=strstr(p2, "</NumberReturned>"))!=NULL)
                  { memmove(MyMessage, p2, p3-p2);
                    MyMessage[p3-p2]=0;
                    if(MyMessage[0]=='0')
                    { // apperently there are less than 101 songs in this playlist
                      // no need to fetch more empty responses
//                      songcount--; // count as one less
//                      Index = 9999; 
                    }
//                    Serial.println(MyMessage);
                  }
                }                                   
           p=p2;      
           }
           else p++;
//        p=p2;
//        Serial.println(p2);
      }  





}    


void CleanEncoded(char * BigBuffer)
{ char *p;    
      int len, len2;
      while((p=strstr(BigBuffer, "&lt;"))!=NULL)
      { *p++ ='<';
        len = strlen(BigBuffer);
        len2 = len - (BigBuffer - p - 3);
        memmove(p, p+3, len2);
      }
      while((p=strstr(BigBuffer, "&gt;"))!=NULL)
      { *p++ ='>';
        len = strlen(BigBuffer);
        len2 = len - (BigBuffer - p - 3);
        memmove(p, p+3, len2);
      }
      // haalt alle &amps; weg, ruimt ook &amp;amp; op  
      while((p=strstr(BigBuffer, "&amp;"))!=NULL)
      { *p++ ='&';
        len = strlen(BigBuffer);
        len2 = len - (BigBuffer - p - 4);
        memmove(p, p+4, len2);
      }
      while((p=strstr(BigBuffer, "&quot;"))!=NULL)
      { *p++ ='\'';
        len = strlen(BigBuffer);
        len2 = len - (BigBuffer - p - 5);
        memmove(p, p+5, len2);
      }
      while((p=strstr(BigBuffer, "&apos;"))!=NULL)
      { *p++ ='\'';
        len = strlen(BigBuffer);
        len2 = len - (BigBuffer - p - 5);
        memmove(p, p+5, len2);
      }

      // haalt alle &amps; weg, maar 1x dus &amp;amp; wordt &amp; 
//      p = BigBuffer;  
//      while((p=strstr(p, "&amp;"))!=NULL)
//      { len = strlen(p);
//        len2 = len - 4;
//         p++;
//         memmove(p, p+4, len2);
//      }
}


char tagcontent1[128];
char tagcontent2[128];
char tagcontent3[128];
char tagcontent4[128];

void UnRavelDidl(char *didl)
{ // songs
  // DIDL=<DIDL-Lite xmlns:dc='http://purl.org/dc/elements/1.1/' xmlns:upnp='urn:schemas-upnp-org:metadata-1-0/upnp/' xmlns:r='urn:schemas-rinconnetworks-com:metadata-1-0/' xmlns='urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/'><item id='-1' parentID='-1' restricted='true'><res protocolInfo='http-get:*:audio/mpeg:*' duration='0:03:24'>http://192.168.1.47/A01-Cor%20Bakker-Feel%20My%20Love.mp3</res><r:streamContent></r:streamContent><dc:title>Feel my love</dc:title><upnp:class>object.item.audioItem.musicTrack</upnp:class><dc:creator>Cor Bakker</dc:creator><upnp:album>Cor Bakker</upnp:album><r:albumArtist>Cor Bakker</r:albumArtist></item></DIDL-Lite>
  // tags of interest <dc:creator>
  // tags of interest <dc:title>
  // tags of interest <upnp:album>
  // radio
  // <DIDL-Lite xmlns:dc='http://purl.org/dc/elements/1.1/' xmlns:upnp='urn:schemas-upnp-org:metadata-1-0/upnp/' xmlns:r='urn:schemas-rinconnetworks-com:metadata-1-0/' xmlns='urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/'><item id='-1' parentID='-1' restricted='true'><res protocolInfo='x-rincon-mp3radio:*:*:*'>x-rincon-mp3radio://http://22613.live.streamtheworld.com:80/SRGSTR03.mp3</res><r:streamContent>Jason Mraz - Have It All</r:streamContent><dc:title>SRGSTR03.mp3
  // tags of interest <r:streamContent>
  
  Serial.print("DIDL=");Serial.println(didl);

  char * p = didl;
  char *p2, *p3, *p4, *p5;
  char restore; 
  int n;

  *tagcontent1=0;
  *tagcontent2=0;
  *tagcontent3=0;
  *tagcontent4=0;
  
  while((p=strchr(p,'<'))!=NULL)
  { if((p2=strchr(p,'>'))!=NULL)
    { // <> found
      p2++;
      restore = *p2;
      *p2=0;
      Serial.print("TAG=");Serial.println(p); // prints the found tag <....> 
      *p2=restore;
      
      // series of tests on the tag found
      if (strncmp(p, "<dc:title>", 10)==0)
      { //Serial.print("CONTENT=");Serial.println(p2); // prints the found value as text
        if((p3=strchr(p2, '<'))!=NULL) 
        { strncpy(tagcontent1, p2, p3-p2);
          tagcontent1[p3-p2]=0;
          Serial.print("1=");Serial.println(tagcontent1); // prints the found value as text
        }
      }
      else if (strncmp(p, "<dc:creator>", 12)==0)
      { //Serial.print("CONTENT=");Serial.println(p2); // prints the found value as text
        if((p3=strchr(p2, '<'))!=NULL) 
        { strncpy(tagcontent2, p2, p3-p2);
          tagcontent2[p3-p2]=0;
          Serial.print("2=");Serial.println(tagcontent2); // prints the found value as text
        }
      }
      else if (strncmp(p, "<upnp:album>", 12)==0)
      { //Serial.print("CONTENT=");Serial.println(p2); // prints the found value as text
        if((p3=strchr(p2, '<'))!=NULL) 
        { strncpy(tagcontent3, p2, p3-p2);
          tagcontent3[p3-p2]=0;
          Serial.print("3=");Serial.println(tagcontent3); // prints the found value as text
        }
      }
      else if (strncmp(p, "<r:streamContent>", 17)==0)
      { //Serial.print("CONTENT=");Serial.println(p2); // prints the found value as text
        if((p3=strchr(p2, '<'))!=NULL) 
        { strncpy(tagcontent4, p2, p3-p2);
          tagcontent4[p3-p2]=0;
          Serial.print("4=");Serial.println(tagcontent4); // prints the found value as text
          if(*tagcontent4==0)strcpy(tagcontent4, "Pitch Talk");
          if(strncmp(tagcontent4,"ZPSTR", 5)==0)
          { strcpy(tagcontent4, "Verbinden...");
          }
          Serial.print("4=");Serial.println(tagcontent4); // prints the found value as text
        }
      }
      p=p2;
    }
    else p++;   
  }

  if(DeviceMode == SELECTRADIO)
  { if(*tagcontent4)TFT_line_print(1, tagcontent4); // radio stream, not the title here
  }
  else
  {  TFT_line_print(1, tagcontent1); // song title
     TFT_line_print(2, tagcontent2); // artist
     TFT_line_print(3, tagcontent3); // album
  }  
  

}



// end of the file
