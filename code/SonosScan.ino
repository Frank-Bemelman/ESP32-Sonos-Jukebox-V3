//#define DEBUG_X 1

// this is a mess

void SonosCheckDeviceList(void);
void SortIPList(IPAddress *List,int Listsize);


IPAddress GetSonosSetup(int *FoundSonosDevices)
{ 
  int n, n2;
  byte buf[4];
  IPAddress ip;
  ActiveSonos = 0;

  G_Sonos = SonosUPnP(G_Sonosclient, ethConnectError);
  
  EEPROM.begin(EEPROM_SIZE);
  SonosLastUsed = EEPROM.read(EEPROM_SONOS_LASTUSED);
  EEPROM.end();
  
  if(SonosLastUsed>(MAXSONOS-1))SonosLastUsed = 0; // SonosLastUsed ranges from 0 to MAXSONOS-1
  Serial.print("******************* SonosLastUsed=");Serial.println(SonosLastUsed);


  // do a search for sonos devices and return a list with unique IP addresses and perhaps blank 0.0.0.0 addresses
  // shows addresses on display as they are found
  *FoundSonosDevices = G_Sonos.CheckUPnP(&G_SonosFound_IPList[0], MAXSONOS);  

  if(*FoundSonosDevices<1)G_SonosFound_IPList[0]; // it makes no sense to continue, return with 0.0.0.0
  
  // sort list from low to high and display again
  SortIPList(&G_SonosFound_IPList[0], MAXSONOS);

  // read list of stored IP addresses
  for(n=0;n<MAXSONOS;n++)
  { EEPROM.begin(EEPROM_SIZE);
    for(n2=0;n2<4;n2++)
    { ip[n2] = (byte)EEPROM.read(EEPROM_SONOSIP1+(n*4)+n2);
    }
    G_SonosIPList[n] = ip;
    if(ip!=0)Serial.print("* Read IP # from eeprom");Serial.print(", IP:");Serial.println(G_SonosIPList[n]);
    EEPROM.end();
  } 

  if(G_SonosIPList[SonosLastUsed]!= G_SonosFound_IPList[SonosLastUsed]) // situation changed, either new sonos devices or devices gone, new ip addresses assigned, whatever
  { for(n=0;n<MAXSONOS;n++) // check if last sonos used  is still member of the current family of found devices
    { if(G_SonosIPList[SonosLastUsed] == G_SonosFound_IPList[n])
      { SonosLastUsed = n; // yes, still there
        break;
      }
    }
    // last used sonos could be gone, then use the first on on the list
    if(G_SonosIPList[SonosLastUsed] != G_SonosFound_IPList[SonosLastUsed])
    { // SonosLastUsed=0; // better not - prefer a rescan until the thing turns back on again
    }
    // update eeprom list as the situation changed
    for(n=0;n<MAXSONOS;n++)
    { ip = G_SonosFound_IPList[n];
      EEPROM.begin(EEPROM_SIZE);
      Serial.print("* Write IP # to eeprom");Serial.print(", IP:");Serial.println(ip);
      for(n2=0;n2<4;n2++)
      { if(ip)
        { EEPROM.write(EEPROM_SONOSIP1+(n*4)+n2, (int)ip[n2]);
          EEPROM.commit();
        }  
      }
      Serial.println("!");
      EEPROM.end();
    }
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(EEPROM_SONOS_LASTUSED, SonosLastUsed);
    EEPROM.commit();
    EEPROM.end();
  }
  
  // check the found ones and put their names on the display
  SonosCheckDeviceList(); // check if these devices are alive and more important, get their actual names and serialnumbers
  
  Serial.print("Lastused:");Serial.print(SonosLastUsed);Serial.print(" ");Serial.println(G_SonosDeviceList[SonosLastUsed].Zonename);

  if(G_SonosDeviceList[SonosLastUsed].Zonename[0]!=0) // last used Sonos device is active - should always be the case
  { 
    // 00-0E-58-28-22-40:5

    for(n2=0,n=0;n<17;n++)
    { if(G_SonosDeviceList[SonosLastUsed].Serialnumber[n]!='-')ACTIVE_sonosSerialnumber[n2++]=G_SonosDeviceList[SonosLastUsed].Serialnumber[n];
      ACTIVE_sonosSerialnumber[n2]=0;
    } 
    Serial.print("UID:");Serial.println(ACTIVE_sonosSerialnumber);
    
    return G_SonosFound_IPList[SonosLastUsed]; // let's use it
  }
  return G_SonosFound_IPList[0];
}




/**** Sonosroutine  *****/
//void  SonosCheck(IPAddress SAdress, SonosDevice* devicetocheck)
void  SonosCheckDeviceList(void)
{ int n, t;
  SonosInfo info;
  char text[64];
//struct SonosInfo // JV new, pass text info as Char string
//{
//  uint16_t number;
//  char *uid;        // Rincon-xxxx 32 bytes
//  char *serial;      // 16 bytes serialnumber short - no '-'
//  char *seriesid;   // Series ID or Sonos Type - 16bytes
//  char *zone;        // Zone name - 32 bytes
//  char *medium;      // medium - network, linein etc
//  char *status;      // Status - play/stop/pause etc
//  char *playmode;    // playmode, see SONOS_PLAY_MODE definitions
//  char *source;      // source, defined in URI , see  SONOS_SOURCE definitions
//  };

//struct SonosDevice { // /status/zp details Structure
//  char Zonename[32]; // ZoneName
//  char UID[32];      // LocalUID RINCON_xxx
//  char Serialnumber[32];  // serial number full
//  char Seriesid[16]  ;     //SeriesID
//  }

  for(n=0; n<MAXSONOS;n++)
//  for(n=0; n<1;n++)
  { // assume and make empty
    G_SonosDeviceList[n].Zonename[0]=0;
    G_SonosDeviceList[n].UID[0]=0;
    G_SonosDeviceList[n].Serialnumber[0]=0;
    G_SonosDeviceList[n].Seriesid[0]=0;      
    if(G_SonosFound_IPList[n]!=0) // no test on empty ip addresses
    { //Serial.print("* Checking n: ");Serial.println(n);
      //Serial.print("* Checking Sonos Device at IP: ");Serial.println(G_SonosFound_IPList[n]);
      //tft.print("Chk IP:");tft.println(G_SonosIPList[n]);
      info = G_Sonos.getSonosInfo(G_SonosFound_IPList[n]);
      if(info.zone[0]!=0)
      { for(t=0;info.zone[t]!=0;++t) {G_SonosDeviceList[n].Zonename[t]=info.zone[t];} G_SonosDeviceList[n].Zonename[t]=0;
        for(t=0;info.uid[t]!=0;++t) {G_SonosDeviceList[n].UID[t]=info.uid[t];} G_SonosDeviceList[n].UID[t]=0;
        for(t=0;info.serial[t]!=0;++t) {G_SonosDeviceList[n].Serialnumber[t]=info.serial[t];} G_SonosDeviceList[n].Serialnumber[t]=0;
        for(t=0;info.seriesid[t]!=0;++t) {G_SonosDeviceList[n].Seriesid[t]=info.seriesid[t];} G_SonosDeviceList[n].Seriesid[t]=0;      
//        Serial.print("Zonename: ");Serial.println(G_SonosDeviceList[n].Zonename);
//        Serial.print("UID: ");Serial.println(G_SonosDeviceList[n].UID);
//        Serial.print("Serialnumber: ");Serial.println(G_SonosDeviceList[n].Serialnumber);
//        Serial.print("Seriesid: ");Serial.println(G_SonosDeviceList[n].Seriesid);
//       if(n==SonosLastUsed)sprintf(text, "<%d> - %s", n+1, G_SonosDeviceList[n].Zonename);
        if(n==SonosLastUsed)sprintf(text, "%s", G_SonosDeviceList[n].Zonename);
        else sprintf(text, "%s", G_SonosDeviceList[n].Zonename);
        TFT_line_print(n+1, text);
        if(n!=SonosLastUsed)TFT_line_color(n+1, TFT_SILVER);
        MyDisplay[n+1].refresh = true;

      }
    }
  }
}


void SortIPList(IPAddress *List, int Listsize)
{ int n, n2;
  char text[32];
  IPAddress swap;
  swap[0]=0;
  for(n=0; n<Listsize-1;n++)  
  { for(n2=0; n2<Listsize-1;n2++)  
    { sprintf(text, "swaptest1 %d.%d.%d.%d", List[n2][0], List[n2][1], List[n2][2], List[n2][3]);  
      Serial.println(text);    
      sprintf(text, "swaptest2 %d.%d.%d.%d", List[n2+1][0], List[n2+1][1], List[n2+1][2], List[n2+1][3]);  
      Serial.println(text);    
      if(List[n2][3]>List[n2+1][3] && List[n2+1][3]!=0) // only swap if both are valid ip
      { swap = List[n2];
        List[n2] = List[n2+1];
        sprintf(text, "%d.%d.%d.%d", List[n2][0], List[n2][1], List[n2][2], List[n2][3]);  
        TFT_line_print(n2+1, text);
        List[n2+1]=swap;
        sprintf(text, "%d.%d.%d.%d", List[n2+1][0], List[n2+1][1], List[n2+1][2], List[n2+1][3]);  
        TFT_line_print(n2+2, text);
      }
    }
  }
  for(n=0; n<Listsize;n++)  
  { sprintf(text, "%d.%d.%d.%d", List[n][0], List[n][1], List[n][2], List[n][3]);  
    Serial.println(text);
  }
}
