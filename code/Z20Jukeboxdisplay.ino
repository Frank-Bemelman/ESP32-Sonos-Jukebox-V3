// LCD display 240x320 - only 240x240 used
extern char CREATOR_BUFFER[];
extern char ARTIST_BUFFER[];
extern char TITLE_BUFFER[];
extern char ALBUM_BUFFER[];
extern char DURATION_BUFFER[];
extern char POSITION_BUFFER[];
extern char ZONE_BUFFER[];
extern char UID_BUFFER[];
extern char SERIAL_BUFFER[];
extern char SERIESID_BUFFER[];
extern char MEDIUM_BUFFER[];
extern char STATUS_BUFFER[];
extern char PLAYMODE_BUFFER[];
extern char SOURCE_BUFFER[];

#include "dial3.h"
#include "45rpm240.h"
#include "radio11400.h"


#define NEEDLE_LENGTH 18  // Visible length
#define NEEDLE_WIDTH   7  // Width of needle - make it an odd number
#define NEEDLE_RADIUS 53  // Radius at tip
#define NEEDLE_COLOR1 TFT_BLACK  // Needle periphery colour



uint16_t  bg_color = 0;
int canvascolor;



#define TFTNUMOFLINES 6
TFTline MyDisplay[TFTNUMOFLINES] = {{ true, "", false }, { true, "", false }, { true, "", false }, { true, "", false }, { true, "", false }, { true, "", false }};


// need gxAdvance and gdX for all characters for vertical scrolling, but don't want to change the TFT_eSPI library itself - so have to make some references

int TablegdX[256];
int TablegxAdvance[256];

void BuildGdxTable(void)
{ int i = 0;
  int extascii = 0;
//  Serial.println(tft.gFont.gCount);
  for(extascii = 0; extascii<256; extascii++)
  { TablegdX[extascii] = 0;
//    Serial.print("Doe:");Serial.println(extascii);
    for (i = 0; i < tft.gFont.gCount; i++)
    { if (tft.gUnicode[i] == extascii)
      { // Serial.print("i=");Serial.print(i);Serial.print(" extascii=");Serial.println(extascii, HEX);Serial.print(" gdX=");Serial.println(tft.gdX[i]);
        TablegdX[extascii] = tft.gdX[i];
        TablegxAdvance[extascii] = tft.gxAdvance[i];
        break;
      }
    }
  }
  TablegxAdvance[0x20] = tft.gFont.spaceWidth;
  TablegdX[0x20] = 0;
}



void convertToExtAscii(char *target)
{ char *p;
  char *p2;
  p = target;
  p2 = target;
  while (*p)
  { if (*p == 0xc2)
    { p++; *p2++ = *p;
    }
    else if (*p == 0xc3)
    { p++; *p2++ = *p + (0xC0 - 0x80);
    }
    else *p2++ = *p;
    p++;
  }
  *p2 = 0;
}

void TFT_line_color(int line, int newcolor)
{ if(MyDisplay[line].textcolor == newcolor)return;
  MyDisplay[line].textcolor = newcolor;
  MyDisplay[line].refresh = true;
}

// buffers for 6 lines of text to display
// deals with lenghts, checks if it fits on display or flags it as horizontal scroll text
// converts UTF-8 up to U+00FF (latin-1 supplement) back to old school extended ascii
void TFT_line_print(int line, const char *content)
{ char *p;
  char extAscii[256];
  int n;

  if(line>=TFTNUMOFLINES)return;

 xSemaphoreTake(xDisplayMutex, portMAX_DELAY);

  // max 255 characters
  strncpy(extAscii, content, 255);
  extAscii[255]=0;
  convertToExtAscii(extAscii);

  if (MyDisplay[line].scroll == false) // non extended content, no " --- " added
  { if (strcmp(MyDisplay[line].content, extAscii) == 0)
    { // nothing new, why bother
      xSemaphoreGive(xDisplayMutex);
      return;
    }
  }

  if (MyDisplay[line].scroll == true)
  { if (strncmp(MyDisplay[line].content, extAscii, MyDisplay[line].length - 5) == 0)
    { // nothing new, why bother
      xSemaphoreGive(xDisplayMutex);
      return;
    }
  }

  strcpy(MyDisplay[line].content, extAscii);

  MyDisplay[line].refresh = true;
  MyDisplay[line].backgroundcolor = TFT_DARKGREY;
  MyDisplay[line].textcolor = TFT_WHITE;  
  MyDisplay[line].length = strlen(MyDisplay[line - 1].content);
  MyDisplay[line].pixelwidth = tft.textWidth(MyDisplay[line].content); // myTextWidth(MyDisplay[line].content); //

  

  if (MyDisplay[line].pixelwidth > 190) // does not fit, resort to horizontal scroll of this text
  { MyDisplay[line].scroll = true;
    strcat(extAscii, " --- "); // add --- to make it nicer when looping around
    strcpy(MyDisplay[line].content, extAscii);
    MyDisplay[line].length = strlen(MyDisplay[line].content);
    MyDisplay[line].pixelwidth = tft.textWidth(MyDisplay[line].content); // myTextWidth(MyDisplay[line].content); //tft.textWidth(MyDisplay[line].content);
  }
  else
  { MyDisplay[line].scroll = false;
  }

  MyDisplay[line].scrollpos = 0;
  MyDisplay[line].scrolldelay = 200; // delayed start of scolling (not implemented)
  xSemaphoreGive(xDisplayMutex);

}


int BackGroundColor;

char Artist[128] = "AAAAAA";
char SongTitle[128] = "SSSSSS";

// please uncomment section
// if (gdY[gNum] > gFont.maxAscent)
// in smooth_font.ccp ~line 210

#define TYOFF 7 // y offset for all text lines to position it nicely in background
#define LOGOSIZE 114

// show radio station logo
bool UpdateRadioLogo(int NewLogo)
{ 
  char text[64];

  // prepare a folder \art114\ with numbered jpg files for the station logo
  // filename format radio-114-NN.jpg
  // when no file with such name is found, a generic logo will be displayed (defined in radio11400.h)
  tft.setViewport(120-(LOGOSIZE/2), ((40+40+40+ 20) - (LOGOSIZE/2) -2), LOGOSIZE, LOGOSIZE, true);
  sprintf(text, "/art114/radio-114-%02d.jpg", NewLogo);
  Serial.println(text);
  File logo = SD.open(text); 
  if(logo)
  { logo.close();
    TJpgDec.drawSdJpg(0, 0, text);
  }
  else
  { logo.close();
    TJpgDec.drawJpg(0, 0, radio11400, sizeof(radio11400));
  }
  return true;
}


bool UpdateRadio(int NewShowRadio10mS)
{ // 45RPM-240.jpg
  static int OldShowRadio10mS = 0;
  static int CurrentRadioShown = -1;

  char text[32];
  int tw, line, ycor;
  if (OldShowRadio10mS != NewShowRadio10mS)
  { if (OldShowRadio10mS == 0)
    { // init display
      //tft.fillScreen(MAGENTA);
      uint16_t w = 0, h = 0;
      TJpgDec.drawSdJpg(0, 0, "/art240/radio-240.jpg");
      CurrentRadioShown = -1;
    }
    OldShowRadio10mS = NewShowRadio10mS;
    if (NewShowRadio10mS)
    { // laat zien dan
      if (CurrentRadioShown != ActualRadioShown)
      { CurrentRadioShown = ActualRadioShown;
        TFT_line_print(5, RadioStations[NewRadioStation - 1][1]);
        tw = MyDisplay[5].pixelwidth;
        tft.fillRoundRect(  ((240 - tw) / 2) - 15   , (5*40)+2, tw + 30, 32, 16, BackGroundColor); // 12=radius 5 is de helft van 10
        tft.setTextDatum(TC_DATUM);
        tft.drawString(MyDisplay[5].content, 120, (5*40)+TYOFF); // centered around x coordinate 120
      }
    }
    else
    { // LCD back to normal
      // so refresh entire display
      MyDisplay[0].refresh = true;
      MyDisplay[1].refresh = true;
      MyDisplay[2].refresh = true;
      MyDisplay[3].refresh = true;
      MyDisplay[4].refresh = true;
      MyDisplay[5].refresh = true;

      return true;
    }
  }
  return false;
}

bool UpdateVinylRecord(int NewShow45RPM10mS)
{ // 45RPM-240.jpg
  static int OldShow45RPM10mS = 0;
  static int Current45RPMShown = -1;

  char text[32];
  int tw, line, ycor;
  if (OldShow45RPM10mS != NewShow45RPM10mS)
  { if (OldShow45RPM10mS == 0)
    { // init display
      //tft.fillScreen(TFT_MAGENTA);
      uint16_t w = 0, h = 0;
//      TJpgDec.drawSdJpg(0, 0, "/art240/45RPM-240.jpg"); // leave out to test if this crashes while playing music from SD - and yes that is the case - maybe try later with FatSD
      TJpgDec.drawJpg(0, 0, vinyl45rpm240, sizeof(vinyl45rpm240));

      Current45RPMShown = -1;
    }
    OldShow45RPM10mS = NewShow45RPM10mS;
    if (NewShow45RPM10mS)
    { // laat zien dan
      if (Current45RPMShown != Actual45RPMShown)
      { Current45RPMShown = Actual45RPMShown;
        strcpy(text, "Selection [ - ]");
        if (deb_left_key)text[11] = KnobDecals[deb_left_key - 1];
        if (deb_right_key)text[13] = KnobDecals[deb_right_key + 9];
        TFT_line_print(5, text);
        tw = MyDisplay[5].pixelwidth;
        tft.fillRoundRect(  ((240 - tw) / 2) - 15   , (5*40)+2, tw + 30, 32, 16, BackGroundColor); 
        tft.setTextDatum(TC_DATUM);
        tft.drawString(MyDisplay[5].content, 120, (5*40)+TYOFF); // centered around x coordinate 120
      }
    }
    else
    { // LCD back to normal
      // so refresh entire display
      MyDisplay[0].refresh = true;
      MyDisplay[1].refresh = true;
      MyDisplay[2].refresh = true;
      MyDisplay[3].refresh = true;
      MyDisplay[4].refresh = true;
      MyDisplay[5].refresh = true;

      return true;
    }
  }
  return false;
}

bool UpdateLCDpotentiometer(int NewShowVolume10mS)
{ static int OldShowVolume10mS = 0;
  static int VolumeShown = -1;
  static int NewVolumeToShow;
  char text[32];
  int tw;

  if (OldShowVolume10mS != NewShowVolume10mS)
  { if (OldShowVolume10mS == 0)
    { // init display
      //tft.fillScreen(TFT_YELLOW);
      uint16_t w = 0, h = 0;
      // dial3 is defined in dial3.h for fast loading
      TJpgDec.drawJpg(0, 0, dial3, sizeof(dial3));
      VolumeShown = -1;
    }
    OldShowVolume10mS = NewShowVolume10mS;
    if (NewShowVolume10mS)
    { // laat zien dan
      NewVolumeToShow = rotary_loop(-1);
      if (VolumeShown != NewVolumeToShow)
      { VolumeShown = NewVolumeToShow;
        plotNeedle( ((NewVolumeToShow * 270) / 100), 0);
        snprintf(text, 30, "VOLUME %ddB", NewVolumeToShow);
//Serial.println(text);
        TFT_line_print(5, text);
        tw = MyDisplay[5].pixelwidth;
        tft.fillRoundRect(  ((240 - tw) / 2) - 15   , (5*40)+2, tw + 30, 32, 16, BackGroundColor); // 12=radius 5 is de helft van 10
        tft.setTextDatum(TC_DATUM);
        tft.drawString(MyDisplay[5].content, 120, (5*40)+TYOFF); // centered around x coordinate 120
      }
    }
    else
    { // LCD weer naar normale weergave
      // en dus refresh display
      MyDisplay[0].refresh = true;
      MyDisplay[1].refresh = true;
      MyDisplay[2].refresh = true;
      MyDisplay[3].refresh = true;
      MyDisplay[4].refresh = true;
      MyDisplay[5].refresh = true;

      return true;
    }
  }
  return false;
}

void UpdateLCD(void)
{
  
}

void UpdateLCD2(void)
{ int line;
  int ycor;
  char text[32];
  int tw;
  static bool logoprinted = false;

  static int scrollpos = 0;
  static bool refreshdisplay = false;
  int TextDatum;

  static unsigned long startMillis;
  static unsigned long currentMillis;

  //Serial.println(ShowVolume10mS);
//  currentMillis = micros();
//  Serial.print(currentMillis);Serial.println("uS entry");

//   Serial.print(ShowVolume10mS);Serial.println("-ShowVolume10mS");

  // refresh display is true after timer has run out for a special display 
  refreshdisplay = UpdateLCDpotentiometer(ShowVolume10mS);
  // while volume control is displayed, no further update of display
  if (ShowVolume10mS)
  { return;
  }

  if (!refreshdisplay)
  { refreshdisplay = UpdateVinylRecord(Show45RPM10mS);
    // while 45rpm vinyl record is displayed, no further update of display
    if (Show45RPM10mS)
    { return;
    }
  }
  else
  { Show45RPM10mS = 0; // // refresh caused by end of display time
    UpdateVinylRecord(Show45RPM10mS); // so wrap it up
  }

  if (!refreshdisplay)
  { refreshdisplay = UpdateRadio(ShowRadio10mS);
    // while retro radio is displayed, no further update of display
    if (ShowRadio10mS)
    { return;
    }
  }
  else
  { ShowRadio10mS = 0; // refresh caused by end of display time
    UpdateRadio(ShowRadio10mS); // // so wrap it up
  }


  if (!AllSet)NewLcdMode = SONOS_SOURCE_UNKNOWN;
  else NewLcdMode = JUKELCDMODEOVERIDES[NewSonosSourceMode];

  // eerste de static content printen op LCD
  if (NewLcdMode != ActLcdMode)
  { ActLcdMode = NewLcdMode;
    logoprinted = false;
    // good riddance
    for (line = 0; line < TFTNUMOFLINES; line++)
    {  MyDisplay[line].refresh = true;
    }
    
    refreshdisplay = true;
    //#define SONOS_SOURCE_SPOTIFY 1
    //#define SONOS_SOURCE_FILE 2
    //#define SONOS_SOURCE_HTTP 3
    //#define SONOS_SOURCE_RADIO 4
    //#define SONOS_SOURCE_RADIO_AAC 5
    //#define SONOS_SOURCE_LINEIN 6
    //#define SONOS_SOURCE_MASTER 7
    //#define SONOS_SOURCE_QUEUE 8
    //#define SONOS_SOURCE_SPOTIFYSTATION 9
  }

  if (refreshdisplay == true)
  { refreshdisplay = false;

    switch (ActLcdMode)
    { case SONOS_SOURCE_RADIO:
      case SONOS_SOURCE_RADIO_AAC:
        canvascolor = TFT_RED;
        BackGroundColor = TFT_DARKGREY;
        tft.setTextColor(TFT_WHITE, BackGroundColor, true);
        tft.fillScreen(canvascolor);
        TFT_line_print(0, "INTERNET RADIO");
        TFT_line_print(1, ""); // remove leftovers from jukebox mode
        TFT_line_print(2, ""); // remove leftovers from jukebox mode

        Serial.println("INTERNET RADIO");
//        TFT_line_print(5, "Choóse Rädio Statioñ & Adjust Vôlume"); // test for extended ascii characters
        TFT_line_print(5, "Choose Radio Station & Adjust Volume");
        logoprinted = UpdateRadioLogo(NewRadioStation);
      
        break;

      case SONOS_SOURCE_HTTP:
      case SONOS_SOURCE_FILE:
      case SONOS_SOURCE_LOCALHTTP:
      case SONOS_SOURCE_LOCALHTTPS:
        if (ActLcdMode == SONOS_SOURCE_HTTP)
        { canvascolor = TFT_YELLOW;
          BackGroundColor = TFT_DARKGREY;
          tft.setTextColor(TFT_WHITE, BackGroundColor, true);
        }
        else
        { canvascolor = TFT_MAGENTA;
          BackGroundColor = TFT_DARKGREY;
          tft.setTextColor(TFT_WHITE, BackGroundColor, true);
        }
        tft.fillScreen(canvascolor);
        if (g_Playlistfound)
        { sprintf(text, "JUKEBOX %d", songcount); // play from sonos playlist 'jukebox'
        }
        else if (g_SDfallback)
        { sprintf(text, "SD-JUKE %d", songcount); // play files from SD card
        }
        else
        { sprintf(text, "SONOS"); // happens only when slave can't load it's own playlist and follows the sonos box playing from a playlist
        }
        TFT_line_print(0, text);
        TFT_line_print(1, ""); // remove leftovers from radio
        TFT_line_print(2, ""); // remove leftovers from radio
        TFT_line_print(5, "Select Your Favorite Song & Adjust Volume");

        break;
      case SONOS_SOURCE_UNKNOWN:
        canvascolor = TFT_GREEN;
        BackGroundColor = TFT_DARKGREY;
        tft.setTextColor(TFT_WHITE, BackGroundColor, true);
        tft.fillScreen(canvascolor);
        break;
      default: // Mode not yet covered
        canvascolor = TFT_WHITE;
        BackGroundColor = TFT_DARKGREY;
        tft.setTextColor(TFT_WHITE, BackGroundColor, true);
        tft.fillScreen(canvascolor);
        sprintf(text, "MODE: %d %d", ActLcdMode, NewSonosSourceMode);
        TFT_line_print(0, text);
        break;
    }
  }

 for (line = 2; line < 5; line++)
 { if(logoprinted)
   { MyDisplay[line].refresh=false; // avoid printing over logo
     MyDisplay[line].scroll=false; // avoid printing over logo
   }
 }
  
 xSemaphoreTake(xDisplayMutex, portMAX_DELAY);  
 for (line = 0; line < 6; line++)
 { if (MyDisplay[line].refresh)
   { tft.setTextDatum(TC_DATUM); // horizontally centered for text that is not scrolling
     tw = MyDisplay[line].pixelwidth;
     if (tw > 200)tw = 200;
     tft.setViewport(0, (line * 40), 240, 40, true);
     // nu eerst oude tegeltje wegpoetsen
     if (line > 0)
     { if (MyDisplay[line].scroll == false)
       { tft.fillRoundRect(0, 2, 240, 32, 0, canvascolor); // radius 0 makes it a square
       }
     }
     if (line == 0)tft.fillRoundRect(0, 0, 240, 34, 0, BackGroundColor); // radius 0 makes it a square
     else tft.fillRoundRect(  ((240 - tw) / 2) - 15   ,  2, tw + 30, 32, 16, BackGroundColor); // 12=radius 5 is de helft van 10
     if (!MyDisplay[line].scroll)
     { tft.setTextColor(MyDisplay[line].textcolor, MyDisplay[line].backgroundcolor, true);
       tft.drawString(MyDisplay[line].content, 120, TYOFF); // centered around x coordinate 120
     }
     MyDisplay[line].refresh = false;
   }
 } 
  
  if(ScrollNow == true) // set true again every 50mS by interrupt
  { ScrollNow = false; 
    for (line = 1; line < 6; line++)
    { if (MyDisplay[line].scroll)
      { startMillis = micros();

        TextDatum = tft.getTextDatum();
        tft.setTextDatum(TL_DATUM);

        if (MyDisplay[line].scrollpos >= MyDisplay[line].pixelwidth)
        { MyDisplay[line].scrollpos = 0;
        }
        
        if (MyDisplay[line].scrollpos==0)
        { MyDisplay[line].nchar = 0;
          MyDisplay[line].toeat = 0;
        }

        int c;
        if(MyDisplay[line].toeat<1)
        { c = MyDisplay[line].content[MyDisplay[line].nchar];
          if(MyDisplay[line].nchar==0) MyDisplay[line].toeat = TablegxAdvance[c] -  TablegdX[c];
          else MyDisplay[line].toeat =   TablegxAdvance[c];   
          MyDisplay[line].noffset = MyDisplay[line].toeat;
          MyDisplay[line].nchar++;
        }
  

//if(line==5)
//{         sprintf(text, "Pw-%03d Sp-%03d nc-%03d of-%03d eat-%03d", MyDisplay[line].pixelwidth, MyDisplay[line].scrollpos, MyDisplay[line].nchar, MyDisplay[line].noffset, MyDisplay[line].toeat);
//          Serial.println(text);
//          Serial.println(&MyDisplay[line].content[MyDisplay[line].nchar-1]);
//} 

          
        if(bUpdateDisplay) // set true every 100mS for a 10 frames per seconde update
        { tft.setViewport(25, (line * 40), 190, 40);
          tft.setTextColor(MyDisplay[line].textcolor, MyDisplay[line].backgroundcolor, true);
          tft.drawString(&MyDisplay[line].content[MyDisplay[line].nchar-1], MyDisplay[line].toeat - MyDisplay[line].noffset, TYOFF);
          if((MyDisplay[line].pixelwidth - MyDisplay[line].scrollpos) < 215)
          { tft.drawString(MyDisplay[line].content, MyDisplay[line].pixelwidth - MyDisplay[line].scrollpos, TYOFF);
          }
        }
  

        MyDisplay[line].toeat -=1;
        MyDisplay[line].scrollpos++;
          

          
//currentMillis = micros();
//Serial.print(currentMillis - startMillis);Serial.println("uS");
        // restore TextDatum
        tft.setTextDatum(TextDatum);
      }
    }
    bUpdateDisplay = false;
    tft.setViewport(0, 0, 240, 240, true);
    
  }
  xSemaphoreGive(xDisplayMutex);  
}


// this is crap but it was a lot of work
/*
void ReadMetaBuffer(void)
{ char *p, *q, *start;
  FullTrackInfo FTI;
  return;
  if ((SonosSourceMode == SONOS_SOURCE_RADIO_AAC) || (SonosSourceMode == SONOS_SOURCE_RADIO))
  { // FullTrackInfo werkt niet op radio
    // dus....
    G_Sonos.getTrackInfo(ACTIVE_sonosIP, uri, sizeof(uri), metaBuffer, sizeof(metaBuffer));
    //    G_Sonos.getTrackInfo(ACTIVE_sonosIP, metaBuffer, sizeof(metaBuffer));
    metaBuffer[sizeof(metaBuffer) - 1] = 0; 

    p = metaBuffer;
    //    Serial.println(metaBuffer);
    strcpy(PlayingStreamContent, "");


    while (*p)
    { if (strncmp(p, "http", 4) == 0)
      { p += 1;
        break;
      }
      p++;
    }

    //Serial.println("2");
    //Serial.println(metaBuffer);

    while (*p)
    { 
      if (strncmp(p, "http", 4) == 0)
      { p += 4;
        if (*p == 's')p++; // https?
        p += 1; // skip :
        strcpy(metaBuffer2, metaBuffer);
        start = metaBuffer2;
        q = start;
        while (*q)
        { if (*q == '&')
          { *q = 0;
            break;
          }
          if (*q == '?')
          { *q = 0;
            break;
          }
          if (*q == ';')
          { *q = 0;
            break;
          }
          q++;
        }
        strcpy(PlayingStreamContent, start);
        RemoveHtmlEntities(PlayingStreamContent);
        Serial.println("URL stream:");
        Serial.println(PlayingStreamContent);
        strcpy(LastRadioStream, PlayingStreamContent);
      }

      if (strncmp(p, "&lt;r:streamContent&gt;", 23) == 0)
      { p += 23;
        strcpy(metaBuffer2, p);
        start = metaBuffer2;
        q = start;
        while (*q)
        { if (strncmp(q, "&lt;", 4) == 0)
          { *q = 0;
            break;
          }
          q++;
        }
        strcpy(PlayingStreamContent, start);
        RemoveHtmlEntities(PlayingStreamContent); // is dit wel perfect of halfbakken
        if (strncmp(PlayingStreamContent, "ZPSTR", 5) == 0)
        { strcpy(PlayingStreamContent, "Verbinden...");
        }
        else
        { //Serial.println("Je hoort nu:");
          //Serial.println(PlayingStreamContent);
          strcpy(LastRadioStation, PlayingStreamContent);

        }
        TFT_line_print(3, PlayingStreamContent);
        break;
      }
      p++;
    }

    G_Sonos.getMediaInfo(ACTIVE_sonosIP, uri, sizeof(uri), metaBuffer, sizeof(metaBuffer));
    //    G_Sonos.getMediaInfo(ACTIVE_sonosIP, metaBuffer, sizeof(metaBuffer));
    metaBuffer[sizeof(metaBuffer) - 1] = 0; // dan is het tenminste geen oneindige string, of ben ik paranoia..
    p = metaBuffer;
    while (*p)
    { if (strncmp(p, "&lt;dc:title&gt;", 16) == 0)
      { p += 16;
        start = p;
        q = p;
        while (*q)
        { if (strncmp(q, "&lt;", 4) == 0)
          { *q = 0;
            break;
          }
          q++;
        }
        strcpy(PlayingTitle, start);
        RemoveHtmlEntities(PlayingTitle); // opschonen, rare smuk eruit
        //Serial.println("Radio Station:");
        //Serial.println(PlayingTitle);
        strcpy(OldPlayingTitle, PlayingTitle);
        TFT_line_print(2, PlayingTitle);
        break;
      }
      p++;
    }
  }
  else if (SonosSourceMode == SONOS_SOURCE_HTTP)
  { G_Sonos.getTrackInfo(ACTIVE_sonosIP, uri, sizeof(uri), metaBuffer, sizeof(metaBuffer));

    //    G_Sonos.getTrackInfo(ACTIVE_sonosIP, metaBuffer, sizeof(metaBuffer));
    metaBuffer[sizeof(metaBuffer) - 1] = 0; // dan is het tenminste geen oneindige string, of ben ik paranoia..

    p = metaBuffer;
    while (*p)
    { if (strncmp(p, "&lt;dc:title&gt;", 16) == 0)
      { p += 16;
        strcpy(PlayingTitle, p);
        //       Serial.println(PlayingTitle);
        p = PlayingTitle;
        q = p;
        while (*q)
        { if (strncmp(q, "&lt;", 4) == 0)
          { *q = 0;
            break;
          }
          q++;
        }
        RemoveHtmlEntities(PlayingTitle); // rare reut eruit
        if (DeviceType == WALLBOX)Serial.println("Sonos Radio Speelt: ");
        if (DeviceType == JUKEBOX)Serial.println("Sonos Radio Speelt: ");
        Serial.println(PlayingTitle);
        TFT_line_print(2, PlayingTitle);
        break;
      }
      p++;
    }
  }
  else if (SonosSourceMode == SONOS_SOURCE_FILE || SonosSourceMode == SONOS_SOURCE_LOCALHTTP || SONOS_SOURCE_LIBRARY) // ok
  { return;
    FTI = G_Sonos.getFullTrackInfo(ACTIVE_sonosIP);

    Serial.print(">>>track = ");
    Serial.print(FTI.number);
    Serial.print(", pos = ");
    Serial.print(FTI.position);
    Serial.print(" of ");
    Serial.println(FTI.duration);

    Serial.print("CREATOR_BUFFER - ");
    Serial.println(CREATOR_BUFFER);
    Serial.print("ARTIST_BUFFER - ");
    Serial.println(ARTIST_BUFFER);
    Serial.print("TITLE_BUFFER - ");
    Serial.println(TITLE_BUFFER);
    Serial.print("ALBUM_BUFFER - ");
    Serial.println(ALBUM_BUFFER);

    if (strlen(TITLE_BUFFER) > 5)
    { if (TITLE_BUFFER[strlen(TITLE_BUFFER) - 4] == '.')TITLE_BUFFER[strlen(TITLE_BUFFER) - 4] = 0; // 3 letterige extensies na een punt wegzappn
    }

    if (isdigit(TITLE_BUFFER[1]) && isdigit(TITLE_BUFFER[2])) // remove my own style jukebox prefixes of files like KO5 etcetera
    { TFT_line_print(1, TITLE_BUFFER + 4);
    }
    else
    { TFT_line_print(1, TITLE_BUFFER);
    }

    //      TFT_line_print(2, ARTIST_BUFFER); // empty?
    TFT_line_print(2, CREATOR_BUFFER);
    TFT_line_print(2, ALBUM_BUFFER);

  }





  else
  { Serial.print("Onbekende Modus = ");
    Serial.println(SonosSourceMode);
    //Serial.println(metaBuffer);
  }
}

*/

void RemoveHtmlEntities(char* target)
{ /* This mapping table can be extended if necessary. */
  static const struct {
    const char* encodedEntity;
    const char decodedChar;
  } entityToChars[] = {
    {"&lt;", '<'},
    {"&gt;", '>'},
    {"&amp;", '&'},
    {"&quot;", '"'},
    {"&apos;", '\''},
    {"&#039;", '\''},
  };

  int n = 0;
  int n1 = 0;
  int cnt;

  cnt = strlen(target);
  for (n = 0; n < cnt; n++)
  { for (n1 = 0; n1 < 5; n1++)
    { if (strncmp(&target[n], entityToChars[n1].encodedEntity, strlen(entityToChars[n1].encodedEntity)) == 0)
      { strcpy(&target[n + 1], &target[n + strlen(entityToChars[n1].encodedEntity)]);
        target[n] = entityToChars[n1].decodedChar;
        n--;
        break;
      }
    }
  }

}




// =======================================================================================
// Create the needle Sprite
// =======================================================================================
void createNeedle(void)
{
  needle.setColorDepth(16);
  needle.createSprite(NEEDLE_WIDTH, NEEDLE_LENGTH);  // create the needle Sprite

  needle.fillSprite(TFT_BLACK); // Fill with black

  // Define needle pivot point relative to top left corner of Sprite
  uint16_t piv_x = NEEDLE_WIDTH / 2; // pivot x in Sprite (middle)
  uint16_t piv_y = NEEDLE_RADIUS;    // pivot y in Sprite
  needle.setPivot(piv_x, piv_y);     // Set pivot point in this Sprite

  // Draw the red needle in the Sprite
  needle.fillRect(0, 0, NEEDLE_WIDTH, NEEDLE_LENGTH, TFT_DARKGREY);
  needle.fillRect(1, 1, NEEDLE_WIDTH - 2, NEEDLE_LENGTH - 2, TFT_BLACK);

  // Bounding box parameters to be populated
  int16_t min_x;
  int16_t min_y;
  int16_t max_x;
  int16_t max_y;

  // Work out the worst case area that must be grabbed from the TFT,
  // this is at a 45 degree rotation
  needle.getRotatedBounds(45, &min_x, &min_y, &max_x, &max_y);

  // Calculate the size and allocate the buffer for the grabbed TFT area
  tft_buffer =  (uint16_t*) malloc( ((max_x - min_x) + 2) * ((max_y - min_y) + 2) * 2 );
}

// =======================================================================================
// Move the needle to a new position
// =======================================================================================
void plotNeedle(int16_t angle, uint16_t ms_delay)
{ static bool buffer_loaded = false;
  static int16_t old_angle = -135; // Starts at -120 degrees

  // Bounding box parameters
  static int16_t min_x;
  static int16_t min_y;
  static int16_t max_x;
  static int16_t max_y;

  if (angle == 1234) // magic value
  { old_angle = ((ActualVolumeFromSonos * 270) / 100) - 135; // initial display should show needle at right position right away
    return;
  }

  if (angle < 0) angle = 0; // Limit angle to emulate needle end stops
  if (angle > 270) angle = 270;

  angle -= 135; // Starts at -120 degrees

  // Move the needle until new angle reached
  while (angle != old_angle || !buffer_loaded) {

    #ifndef ST7789_DRIVER
    if (old_angle < angle) old_angle++;
    else old_angle--;
    #else
    old_angle = angle;
    #endif
    // Only plot needle at even values and final position to improve plotting performance
    if (((old_angle & 1) == 0) || (old_angle==angle))
    { // fb - note to self - corruption seen in this buffer - very rare, not yet figured out
      if (buffer_loaded) {
        // Paste back the original needle free image area
        #ifndef ST7789_DRIVER
        tft.pushRect(min_x, min_y, 1 + max_x - min_x, 1 + max_y - min_y, tft_buffer);
        #else
//        tft.setViewport(50, min_y, 100, 1 + max_y - min_y);
//        TJpgDec.drawJpg(-50, -min_y, dial3, sizeof(dial3));
        tft.setViewport(min_x, min_y, 1 + max_x - min_x, 1 + max_y - min_y);
        TJpgDec.drawJpg(-min_x, -min_y, dial3, sizeof(dial3));

        tft.setViewport(0, 0, 240, 240);
        #endif
        buffer_loaded = false; 
      }

      if ( needle.getRotatedBounds(old_angle, &min_x, &min_y, &max_x, &max_y) )
      {
        // Grab a copy of the area before needle is drawn
        #ifndef ST7789_DRIVER
        tft.readRect(min_x, min_y, 1 + max_x - min_x, 1 + max_y - min_y, tft_buffer);
        #endif
        buffer_loaded = true;
      }

      // Draw the needle in the new position, black in needle image is transparent
      needle.pushRotated(old_angle, TFT_VIOLET);

      // Wait before next update
      delay(ms_delay);
    }

    // Slow needle down slightly as it approaches the new position
    if (abs(old_angle - angle) < 10) ms_delay += ms_delay / 5;
  }
}
