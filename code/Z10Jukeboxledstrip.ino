// defines wich leds are under what keys - from left to right
// leds numbered from 0 (far right) to 71 (far left)
// it is a compromise - the pitch of the leds on the strip is different than the distance between the keys
int KeyPosToLed[20][4] ={ {1,63,62,61 },{2,60,59,58 },{3,57,56,55 },{4,55,54,53 },{5,52,51,50 },{6,49,48,47 },{7,47,46,45 },{8,44,43,42 },{9,41,40,39 },{10,38,37,36 },{11,0,0,0 },{12,0,0,0 },{13,0,0,0 },{14,0,0,0 },{15,0,0,0 },{16,0,0,0 },{17,0,0,0 },{18,0,0,0 },{19,0,0,0 },{20,0,0,0 } };
int KeyPosToLedWB[20][4] ={ {1,1,2,3 },{2,4,5,6 },{3,8,9,10},{4,11,12,13},{5,14,15,16 },{6,18,19,20},{7,22,23,24},{8,26,27,28 },{9,30,31,32},{10,34,35,36 },{11,0,0,0 },{12,0,0,0 },{13,0,0,0 },{14,0,0,0 },{15,0,0,0 },{16,0,0,0 },{17,0,0,0 },{18,0,0,0 },{19,0,0,0 },{20,0,0,0 } };

void AnimateLedstrip(void * pvParameters)
{ int ColorScan;
  int UseColorScan;
  int ColorPicked;
  int KnobToLit;
  byte KitScannerDirection=0;
  byte KitScanNewPos=1;
  byte KitScanOldPos=1;
  byte ReturnAt = 9;

  volatile byte leftkeyled = 1; // from  1 to 10 and back
  volatile byte leftkeydirection; 

  // animation of circle of leds under the volume knob
  byte PotScannerDirection=0;
  byte PotScanNewPos=1;
  byte PotScanOldPos=1;


  while(1)
  { if(PotScannerDirection)PotScanNewPos++;
    else PotScanNewPos++;
    PotScanNewPos &= 0x0f;
     
    ReturnAt = (DeviceMode == SELECTRADIO) ? 19 : 9; // 1 lange scan over beide banken of 2 korte scans op beide banken
    if(DeviceMode != SELECTRADIO)KitScanNewPos %=10;
    if(KitScannerDirection==0) // we lopen op
    { if(KitScanNewPos<ReturnAt)
      { KitScanNewPos++; // 2-3-4-5-6-7-8-9
      }
      else 
      { KitScannerDirection=1; // we gaan aflopen
        KitScanNewPos--; // 8
      }
    }
    else
    { if(KitScanNewPos>0)
      { KitScanNewPos--; // 7-6-5-4-3-2-1-0
      }
      else
      { KitScannerDirection=0;
        KitScanNewPos++; // 1
      }
    }

    if(NewSonosSourceMode>0)ColorScan = JUKESCANCOLOR[NewSonosSourceMode];
    else
    { if(SonosSourceMode<0)ColorScan = 0xFFFFFF;
      else ColorScan = JUKESCANCOLOR[SonosSourceMode];
    }  
  
    if(DeviceMode == SELECTSONG)
    { ColorPicked = 0xFF4000;
    }
    else // RADIO
    { if(DeviceType == WALLBOX)ColorScan = 0xFF3004;  // wat lichter rood
      ColorPicked = 0xFF4000;
    }
   
    // let's drive the leds and give them data
    // 28 leds for 10 keys
    // which leds belong to a certain key is defined in array KeyPosToLed[][]
    // totale strip is 72 leds[] long
    // animation from left to right
    // when a key(s) is/are pressed, those keys will be lit with different color
    // in jukebox mode it can be 2 keys
    // in radio mode it is just one key

    // de wallbox is different, has 37 leds in the top, not behind keys

    // KitScanNewPos goes from 0 to 19 and back and forth
    if(KitScanNewPos != KitScanOldPos)
    { KitScanOldPos = KitScanNewPos;
 
      // first, all leds are turned off
      for(int i=0; i<NUM_LEDS; i++)
      { leds[i].setRGB(0, 0, 0);
      }

      // potscan
      for(int i=0; i<16; i++)
      { if(i==PotScanNewPos)leds[i+72] = ColorScan;
      }


      UseColorScan = ColorScan;
      if(DeviceType == JUKEBOX)
      { // first for left row, for keys A-B-C-D-E-F-G-H-J-K
        if(deb_left_key<1) // kit-scan
        { KnobToLit = KitScanNewPos % 10;
          if((DeviceMode == SELECTSONG) || ((DeviceMode == SELECTRADIO) && (deb_right_key==0))) 
          { if((DeviceMode == SELECTRADIO) && (KitScanNewPos==(NewRadioStation-1)))UseColorScan = 0xFFFFFF;
            if((DeviceMode != SELECTRADIO) || ((KitScanNewPos / 10)==0))
            { leds[KeyPosToLed[KnobToLit][1]] = ColorScan;
              leds[KeyPosToLed[KnobToLit][2]] = UseColorScan;
              leds[KeyPosToLed[KnobToLit][3]] = ColorScan;
            }  
          }
        }
        else // key is pressed
        { KnobToLit = deb_left_key-1;
          leds[KeyPosToLed[KnobToLit][1]] = ColorPicked;
          leds[KeyPosToLed[KnobToLit][2]] = ColorPicked;
          leds[KeyPosToLed[KnobToLit][3]] = ColorPicked;
        }
       
        // same for right row, keys 1-2-3-4-5-6-7-8-9-0
        if(deb_right_key<1)
        { KnobToLit = KitScanNewPos % 10;
          if((DeviceMode == SELECTSONG) || ((DeviceMode == SELECTRADIO) && (deb_left_key==0))) // bij radio alleen kit-scannen als er aan de andere kant geen knop is ingedrukt
          { if((DeviceMode == SELECTRADIO) && (KitScanNewPos==(NewRadioStation-11)))UseColorScan = 0xFFFFFF;
            if((DeviceMode != SELECTRADIO) || ((KitScanNewPos / 10)==1))
            { leds[KeyPosToLed[KnobToLit][1]-35] = ColorScan;
              leds[KeyPosToLed[KnobToLit][2]-35] = UseColorScan;
              leds[KeyPosToLed[KnobToLit][3]-35] = ColorScan;
            }  
          }
        }
        else
        { KnobToLit = deb_right_key-1;
          leds[KeyPosToLed[KnobToLit][1]-35] = ColorPicked;
          leds[KeyPosToLed[KnobToLit][2]-35] = ColorPicked;
          leds[KeyPosToLed[KnobToLit][3]-35] = ColorPicked;
        } 
        // big cancel button will flash to communicate it can be useful
        if((deb_left_key>0)||(deb_right_key>0))
        { if((KitScanNewPos%2) && (MagnetPower10mS>200))
          { leds[70].setRGB(0, 0, 255);
            leds[69].setRGB(0, 0, 255);
            leds[68].setRGB(0, 0, 255);
            leds[67].setRGB(0, 0, 255);
            leds[66].setRGB(0, 0, 255);
          }
        }
      }
      else if(DeviceType == WALLBOX)  
      { if(DeviceMode == SELECTRADIO)
        { // default alles op orange
          if(NewSonosSourceMode == SONOS_SOURCE_HTTP) // Sonos RADIO of Apple Music
          { ColorScan = 0xFF6000;  // doe maar geel op de wallbox net als de TFT op de jukebox
          }
          else if(NewSonosSourceMode == SONOS_SOURCE_UNKNOWN) // bijv bij reclame of Spotify
          { ColorScan = 0x00FF00;  // doe maar geel op de wallbox net als de TFT op de jukebox
          }
          for(int i=0; i<NUM_LEDS; i++)
          { leds[i] = ColorScan;
          }
        }
        else if(DeviceMode == SELECTSONG)
        { KnobToLit = KitScanNewPos % 10;
          leds[KeyPosToLedWB[KnobToLit][1]] = ColorScan;
          leds[KeyPosToLedWB[KnobToLit][2]] = ColorScan;
          leds[KeyPosToLedWB[KnobToLit][3]] = ColorScan;
        }
      }
      // show it
      FastLED.show(); 
      vTaskDelay(100 / portTICK_PERIOD_MS); // portTICK_PERIOD_MS = 1 ;-)
    }
  }
}
// end of file
