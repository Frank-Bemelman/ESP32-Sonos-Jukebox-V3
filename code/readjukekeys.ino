// functions to read and debounce keys
// wallbox uses up/down buttons for volume control
// also latches the electric magnet to latch keys, jukebox uses a magnet latch

// the magnet latch (solenoid) on the Rockola is drive by 12 volts. I use a 12V power spupply for everything with a stepdown converter to 5V for the ESP32
// the magnet works on 12V and is switched with a optocoupler fet driver. these can be found in the usual places.

int AdcConvert(int value)
{ // converts an analog input value and return the key pressed
  // in series connected resistors that are all normally closed by the unpressed keys - a classic voltage divider
  // by measuring the voltage on the analog input pin, we can figure out which key is pressed
  // top resistor is 560 ohm
  // S10 no resistor
  // S9 4K7
  // S8 2K2
  // S7 1K5
  // S6 820 ohm
  // S5 560 ohm
  // S4 390 ohm
  // S3 220 ohm
  // S2 150 ohm
  // S1 68 ohm

  //
  //                      no
  //                    resistor
  // +3V3 ----[560]---+  here   +--[4K7]--+--[2K2]--+--[1K5]--+--[820]--+--[560]--+--[390]--+--[220]--+--[150]--+---[68]--+
  //                  |         |         |         |         |         |         |         |         |         |         |        
  //     NC CONTACTS  +----=----+----=----+----=----+----=----+----=----+----=----+----=----+----=----+----=----+----=----+
  //     KEY    ROW   |   10         9         8         7         6         5         4         3         2         1    | 
  //                  |                                                                                                   |
  //                  +--------> to analog input pin ESP32 D36                                                            +--------> to GND
  //

  //
  //                      no
  //                    resistor
  // +3V3 ----[560]---+  here   +--[4K7]--+--[2K2]--+--[1K5]--+--[820]--+--[560]--+--[390]--+--[220]--+--[150]--+---[68]--+
  //                  |         |         |         |         |         |         |         |         |         |         |        
  //     NC CONTACTS  +----=----+----=----+----=----+----=----+----=----+----=----+----=----+----=----+----=----+----=----+
  //     KEY    ROW   |    K         J         H         G         F         E         D         C         B         A    | 
  //                  |                                                                                                   |
  //                  +--------> to analog input pin ESP32 D39                                                            +--------> to GND
  //

  // wallbox only - control of volume and skip/next
  // skip/nect button has multiple functions - long press to switch between radio and jukebox, >20 seconds press to start webmanager portal.
  //                      
  //                    
  // +3V3 ----[560]---+         +--[4K7]--+--[2K2]--+                                                           +---[68]--+
  //                  |         |         |         |                                                           |         |        
  //     NC CONTACTS  +---------+----=----+----=----+-----------------------------------------------------------+----=----+
  //     KEY    ROW   |              V+       V-                                                                     S    | 
  //                  |                                                                                              K    |
  //                  +--------> to analog input pin ESP32                                                           I    +--------> to GND
  //                                                                                                                 P
  //
  //        +------------------+
  // +12V --+                  +------> +5V ESP32
  //        | Stepdown 12V-5V  |
  // GND ---+                  +------> GND ESP32
  //        +------------------+
  //
  //        +--------------+         +----------------+
  // +12V --+   solenoid   +---------+                +----------> ESP32 D21 SOLENOID
  //        +--------------+         +   FET DRIVER   +
  //                             +---+                +---+
  //                             |   +----------------+   |
  //                             |                        |
  //                             +------------------------+------> GND 
  //
  //


    
  if(value>3864)return 10;
  if(value>3364)return 9;
  if(value>2846)return 8;
  if(value>2421)return 7;
  if(value>2051)return 6;
  if(value>1673)return 5;
  if(value>1227)return 4;
  if(value>815)return 3;
  if(value>459)return 2;
  if(value>127)return 1;
  return 0; // no key pressed, all closed
}

#define DEBOUNCE 5

// task to read the keys at a regular interval
void ReadJukeKeys(void * pvParameters)
{ int analog_leftvalue;
  int analog_rightvalue;
  int analog_controlvalue; // only wallbox which uses up/down button for volume control
  int act_left_key;
  int act_right_key;
  int act_control_key;
  int act_cancel_button;
  int act_blue_button;
  int act_encoder_button;

  int blue_button_changed = 0;
  int cancel_button_changed = 0;
  int encoder_button_changed = 0;
  int left_changed = 0;
  int right_changed = 0;
  int control_changed = 0;
  


  
  while(1)
  { analog_leftvalue = analogRead(ANALOG_LEFT);
    act_left_key = AdcConvert(analog_leftvalue);
    analog_rightvalue = analogRead(ANALOG_RIGHT);
    act_right_key = AdcConvert(analog_rightvalue);
    analog_controlvalue = analogRead(ANALOG_CONTROL);
    act_control_key = AdcConvert(analog_controlvalue);
    act_cancel_button = digitalRead(CANCEL_BUTTON);
    act_blue_button = digitalRead(BLUE_BUTTON);
    act_encoder_button = digitalRead(ENCODER_BUTTON);


if (P.displayAnimate())
  {
    if (newMessageAvailable)
    {
      strcpy(curMessage, newMessage);
      newMessageAvailable = false;
    }
    P.displayReset();
  }

   
    if(DeviceType == WALLBOX)
    { if(act_control_key == 1) // skip button also used to switch between radio/jukebox when pressed long
      { act_cancel_button = 1;
      }
      else act_cancel_button = 0;
    }
 
    // debounce encoder button
    if(act_encoder_button != deb_encoder_button)
    { if(encoder_button_changed<DEBOUNCE)encoder_button_changed++;
      else
      { deb_encoder_button = act_encoder_button;
        encoder_buttonf = deb_encoder_button;
        encoder_button_changed = 0;
      } 
    }
    else encoder_button_changed = 0;

    if(deb_encoder_button==0) // pressed?
    { encoder_button_very_long_pressed++; // use that to activate portal wifi manager
    }
    else
    { encoder_button_very_long_pressed = 0;
    }


    
    // debounce blue
    if(act_blue_button != deb_blue_button)
    { if(blue_button_changed<DEBOUNCE)blue_button_changed++;
      else
      { deb_blue_button = act_blue_button;
        blue_buttonf = deb_blue_button;
        blue_button_changed = 0;
      } 
    }
    else blue_button_changed = 0;

    // debounce cancel - resets wrong key pressed
    if(act_cancel_button != deb_cancel_button)
    { if(cancel_button_changed<DEBOUNCE)cancel_button_changed++;
      else
      { deb_cancel_button = act_cancel_button;
        cancel_buttonf = deb_cancel_button;
        cancel_button_changed = 0;
        if(DeviceMode == SELECTSONG)
        { MagnetPower10mS = 0; 
          Show45RPM10mS = 0; // removes artwork display
        }
        else ShowRadio10mS = 0;
       } 
    }
    else cancel_button_changed = 0;



    // debounce left
    if(act_left_key != deb_left_key)
    { if(left_changed<DEBOUNCE)left_changed++;
      else
      { deb_left_key = act_left_key;
        left_keyf = deb_left_key;
        left_changed = 0;
      } 
    }
    else left_changed = 0;

    // debounce right
    if(act_right_key != deb_right_key)
    { if(right_changed<DEBOUNCE)right_changed++;
      else
      { deb_right_key = act_right_key;
        right_keyf = deb_right_key;
        right_changed = 0;
      } 
    }
    else right_changed = 0;

    // debounce control
    if(act_control_key != deb_control_key)
    { if(control_changed<DEBOUNCE)control_changed++;
      else
      { deb_control_key = act_control_key;
        control_keyf = deb_control_key;
        control_changed = 0;
      } 
    }
    else control_changed = 0;

    if(deb_cancel_button)
    { if(CancelButtonEnabled)cancel_button_long_pressed++;
      cancel_button_very_long_pressed++; 
    }
    else
    { cancel_button_long_pressed = 0;   
      CancelButtonEnabled = 1;
      cancel_button_very_long_pressed = 0;
    }

    if(!MagnetDeadTime10mS)
    { if(left_keyf == deb_left_key)
      { left_keyf = 1234;
        // Serial.print("L ");
        // Serial.println(deb_left_key);
        // Serial.println(analog_leftvalue);
        // Serial.println(analog_rightvalue);
    
        if(deb_right_key == 0) // first key pressed
        { if(deb_left_key>0) // key L1-L10
          { if(DeviceMode == SELECTRADIO)
            { if(DeviceType == JUKEBOX)
              { NewRadioStation = deb_left_key; // radio choice will be picked up in loop()
                OldRadioStation = 0; // forces the made choice no matter what is playing
                MagnetPower10mS = 150; // first key pressed, release key after 1.5 seconds
                ShowRadio10mS = 300;  // display a retro radio to confirm selection
              }  
            }
            else
            { MagnetPower10mS = 700; // unlatch the key after 7 seconds if no 2nd key is pressed
              Show45RPM10mS = 700;  // display 45rpm vinyl record
            }  
          }
          else MagnetPower10mS = 0; 
        }
        else
        { // second key pressed
          if(deb_left_key>0) // key L1-L10
          { MagnetPower10mS = 200;  // release keys after 2 seconds
            MagnetDeadTime10mS = 250;
            selectedsong = (deb_left_key - 1) * 10 + deb_right_key;
            if(DeviceType == WALLBOX)
            { DeviceMode = SELECTSONG;
              SonosSourceMode = SONOS_SOURCE_FILE; // forces quicker update of lcd
            }
            if(DeviceMode == SELECTSONG)
            { Actual45RPMShown = selectedsong;
              Show45RPM10mS = 200; // keep the display for two seconds
            }  
            // Serial.print("S");
            // Serial.println(selectedsong);
          }
          else MagnetPower10mS = 700; // usually between 0-700, but when releasing a half pressed button back to7 seconds
        }
      }

      if( right_keyf == deb_right_key)
      { right_keyf = 1234;
        // Serial.print("R");
        // Serial.println(deb_right_key);
        // Serial.println(analog_leftvalue);
        // Serial.println(analog_rightvalue);
        if(deb_left_key == 0) // first key pressed
        { if(deb_right_key>0)
          { if(DeviceMode == SELECTRADIO)
            { if(DeviceType == JUKEBOX)
              { NewRadioStation = deb_right_key + 10; // radio choice will be picked up in loop()
                OldRadioStation = 0; // forces the made choice no matter what is playing
                MagnetPower10mS = 150; // first key pressed, release key after 1.5 seconds
                ShowRadio10mS = 300;  // display a retro radio to confirm selection
              }  
            }
            else
            { MagnetPower10mS = 700; // unlatch the key after 7 seconds if no 2nd key is pressed
              Show45RPM10mS = 700;  // display 45rpm vinyl record
            }
          }
          else MagnetPower10mS = 0; 
        }
        else
        { // second key pressed
          if(deb_right_key>0) // toets R1-R10
          { MagnetPower10mS = 200;  // release keys after 2 seconds
            MagnetDeadTime10mS = 250;
            selectedsong = (deb_left_key - 1) * 10 + deb_right_key;
            if(DeviceType == WALLBOX)
            { DeviceMode = SELECTSONG;
              SonosSourceMode = SONOS_SOURCE_FILE; // forces quicker update of lcd
            }
            if(DeviceMode == SELECTSONG)
            { Actual45RPMShown = selectedsong;
              Show45RPM10mS = 200; // keep the display for two seconds
            }  
            // Serial.print("S");
            // Serial.println(selectedsong);
          }
          else MagnetPower10mS = 700; // usually between 0-700, but when releasing a half pressed button back to7 seconds
        }
      }

      if(DeviceType == WALLBOX)
      { if( control_keyf == deb_control_key)
        { control_keyf = 1234;
          // Serial.print("C");
          // Serial.println(deb_control_key);
          // Serial.println(analog_controlvalue);
           if(deb_control_key>0) // key C1-C10
          { if(deb_control_key==1)
            { SonosSkipRequest = 1;
            }
            else if(deb_control_key==8) // volume down
            { if((ActualVolumeFromSonos>0) && (ActualVolumeFromSonos<90))
              { //Serial.print("Volume-down ");
                //Serial.println(ActualVolumeFromSonos);
                NewVolumeForSonos = ActualVolumeFromSonos -5;
                NewVolumeForSonos = rotary_loop(NewVolumeForSonos); 
                AsyncVolumeForSonos = NewVolumeForSonos;
              }  
            }    
            else if(deb_control_key==9) // volume up
            { if((ActualVolumeFromSonos>0) && (ActualVolumeFromSonos<90))
              { //Serial.print("Volume-up ");
                //Serial.println(ActualVolumeFromSonos);
                NewVolumeForSonos = ActualVolumeFromSonos +5;
                NewVolumeForSonos = rotary_loop(NewVolumeForSonos); 
                AsyncVolumeForSonos = NewVolumeForSonos;
              }
            }
          }
        }
      }   
    }
    vTaskDelay(20 / portTICK_PERIOD_MS); // portTICK_PERIOD_MS = 1 ;-)
  } 
}
// end of file
