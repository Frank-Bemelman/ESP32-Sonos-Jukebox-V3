void notFound(AsyncWebServerRequest *request);
bool checkUserWebAuth(AsyncWebServerRequest * request);
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void ShowDirectory(AsyncWebServerRequest *request);
void ShowPlayList(AsyncWebServerRequest *request);
void ShowJukeStrips(AsyncWebServerRequest *request);
void CreateArrayFromSD(AsyncWebServerRequest *request);
void makedesignator(char *Designator, int ShowSong);

char CardLayout[16] = "Rockola";


// parses and processes webpages
// if the webpage has %SOMETHING% or %SOMETHINGELSE% it will replace those strings with the ones defined
String processor(const String& var) {
  if (var == "FIRMWARE") {
    return FIRMWARE_VERSION;
  }

  if (var == "FREESPIFFS") {
    return humanReadableSize((SD.totalBytes() - SD.usedBytes())/1024); // was SPIFFS
  }

  if (var == "USEDSPIFFS") {
    return humanReadableSize(SD.usedBytes()/1024); // was SPIFFS
  }

  if (var == "TOTALSPIFFS") {
    return humanReadableSize(SD.totalBytes()/1024); // was SPIFFS
  }

  return FIRMWARE_VERSION; // dan returned hij tenminste wat
}

void configureWebServer() {
  // configure web server

  // if url isn't found
  server->onNotFound(notFound);

  // run handleUpload function when any file is uploaded
  server->onFileUpload(handleUpload);

  // visiting this page will cause you to be logged out
  server->on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->requestAuthentication();
    request->send(401);
  });

  // presents a "you are now logged out webpage
  server->on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);
    request->send_P(401, "text/html", logout_html, processor);
  });

  

  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

    if (checkUserWebAuth(request)) {
      request->send(200, "text/html", reboot_html);
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      shouldReboot = true;
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      ShowDirectory(request);
//      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server->on("/playlist", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      ShowPlayList(request);
//      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server->on("/jukestrips", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    strcpy(CardLayout, "Rockola");
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    handler->setCacheControl("max-age=6000");
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      ShowJukeStrips(request);
//      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server->on("/wallbox", HTTP_GET, [](AsyncWebServerRequest * request)
  { strcpy(CardLayout, "Wallbox");
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    handler->setCacheControl("max-age=6000");
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      ShowJukeStrips(request);
//      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });



  server->on("/createarray", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      CreateArrayFromSD(request);
//      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });


server->on("/listfiles-original", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      request->send(200, "text/plain", listFiles(true));
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  
  server->on("/file", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);

      if (request->hasParam("name") && request->hasParam("action")) {
        const char *fileName = request->getParam("name")->value().c_str();
        const char *fileAction = request->getParam("action")->value().c_str();
        char fileName2[64] = "/";
        strcat(fileName2, fileName);


        logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

        if (!SD.exists(fileName2)) { // was SPIFFS
          Serial.println(fileName2);
          Serial.println(logmessage + " ERROR1: file does not exist");
          request->send(400, "text/plain", "ERROR2: file does not exist");
        } else {
          Serial.println(logmessage + " file exists");
          if (strcmp(fileAction, "download") == 0) {
            logmessage += " downloaded";
            request->send(SD, fileName2, "application/octet-stream"); // was spiffs
          } else if (strcmp(fileAction, "delete") == 0) {
            logmessage += " deleted";
            SD.remove(fileName2); // was spiffs
            request->send(200, "text/plain", "Deleted File: " + String(fileName));
          } else {
            logmessage += " ERROR3: invalid action param supplied";
            request->send(400, "text/plain", "ERROR: invalid action param supplied");
          }
          Serial.println(logmessage);
        }
      } else {
        request->send(400, "text/plain", "ERROR: name and action params required");
      }
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server->on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      request->send_P(200, "text/html", index_html, processor);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }

  });
}

void notFound(AsyncWebServerRequest *request) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);
  request->send(404, "text/plain", "Not found");
}

// used by server.on functions to discern whether a user has the correct httpapitoken OR is authenticated by username and password
bool checkUserWebAuth(AsyncWebServerRequest * request) {
  bool isAuthenticated = true;  // was false but allow everyone

  if (request->authenticate(config.httpuser.c_str(), config.httppassword.c_str())) {
//    Serial.println("is authenticated via username and password");
    isAuthenticated = true;
  }
  return isAuthenticated;
}

// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  // make sure authenticated before allowing upload
  if (checkUserWebAuth(request)) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
//    Serial.println(logmessage);

    if (!index) {
      logmessage = "Upload Start: " + String(filename);
      // open the file on first call and store the file handle in the request object
      request->_tempFile = SD.open("/" + filename, "w"); // was spiffs
//      Serial.println(logmessage);
    }

    if (len) {
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
      logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
//      Serial.println(logmessage);
    }

    if (final) {
      logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      // close the file handle as the upload is now done
      request->_tempFile.close();
//      Serial.println(logmessage);
      request->redirect("/");
    }
  } else {
//    Serial.println("Auth: Failed");
    return request->requestAuthentication();
  }
}

void ShowDirectory(AsyncWebServerRequest *request)
{ static File root1;
  static File file1;
  static bool firsttime = true;
    AsyncWebServerResponse *response = request->beginChunkedResponse("text/plain", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t 
    { if(!root1 && firsttime == true)
      { Serial.println("Directory opgevraagd");
        firsttime = false;
        if(SD_present)
        {  root1 = SD.open("/");
//        Serial.println(root1.name());
          file1 = root1.openNextFile();
          if(file1.isDirectory())file1 = root1.openNextFile();
//        Serial.println(file1.name());
          String to_send = "";
          to_send += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
          to_send.getBytes(buffer, maxLen); 
          return to_send.length();
        }
        else
        { String to_send = "";
          to_send += "No SD Card present<br>";
          to_send.getBytes(buffer, maxLen); 
          return to_send.length();
        }
      }  

      while(file1)
      { Serial.print("whilefile1 - ");
        Serial.println(file1.name());



        if(!file1.isDirectory())
        {  Serial.print("FILE - ");
           Serial.println(file1.name());

           String to_send = "";
           to_send += "<tr align='left'><td>" + String(file1.name()) + "</td><td>" + humanReadableSize(file1.size()/1024) + "</td>";
           to_send += "<td><button onclick=\"downloadDeleteButton(\'" + String(file1.name()) + "\', \'download\')\">Download</button>";
           to_send += "<td><button onclick=\"downloadDeleteButton(\'" + String(file1.name()) + "\', \'delete\')\">Delete</button></tr>";
           file1 = root1.openNextFile();
           if(!file1)
           { to_send += "</table>";  
             Serial.println("DIRECTORY finished");
           }
           to_send.getBytes(buffer, maxLen); 
           return to_send.length();
        }
        else
        {  Serial.print("DIRECTORY - ");
           Serial.println(file1.name());
           String to_send = "";
           to_send += "<tr align='left'><td>" + String(file1.name()) + "</td><td>" + humanReadableSize(file1.size()/1024) + "</td>";
//           to_send += "<td><button onclick=\"downloadDeleteButton(\'" + String(file1.name()) + "\', \'download\')\">Download</button>";
//           to_send += "<td><button onclick=\"downloadDeleteButton(\'" + String(file1.name()) + "\', \'delete\')\">Delete</button></tr>";
             file1 = root1.openNextFile();
             Serial.print("DIRECTORY is nu - ");
             Serial.println(file1.name());
             if(!file1)to_send += "</table>";  
             to_send.getBytes(buffer, maxLen); 
             return to_send.length();
        }
      }
      file1.close();
      root1.close();
      firsttime = true;
      return 0;
          
    });
    request->send(response);
}


void ShowPlayList(AsyncWebServerRequest *request)
{ static int ShowSong = 0;
  static int Chunks = 0;
    AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t 
    { String to_send = "";
      if(Chunks==0)
      { to_send += "<!doctype html><html><head>";
        to_send += "<style type=\"text/css\">";
        to_send += ".card{width: 3in;height: 1in;background-image: url(/artweb/jukestrip.jpg);background-size: 3in 1in;}";
        to_send += ".line{width: 3in;height: 7mm;text-align: center;font-family: Georgia, serif;}";
        to_send += "</style></head><body>";
        Chunks++;
        to_send.getBytes(buffer, maxLen); //??
        return to_send.length();
      }
      else if(Chunks==1)
      { if(ShowSong<99) // 98 & 99 is the last card
        { to_send += "<table cellspacing=\"0\" cellpadding=\"0\" ><tr><td class=\"card\"><table cellspacing=\"0\" cellpadding=\"0\" >";
          to_send += "<tr><td class =\"line\">";
          to_send += String(JukeBoxSongs[ShowSong].path);
          to_send += "</td></tr>";
          to_send += "<tr><td class =\"line\">";
          to_send += String(JukeBoxSongs[ShowSong].artist);
          to_send += "</td></tr>";
          to_send += "<tr><td class =\"line\">";
          to_send += String(JukeBoxSongs[ShowSong+1].path);
          to_send += "</td></tr>";
          to_send += "</table></td></tr></table>";
          ShowSong+=2;
          to_send.getBytes(buffer, maxLen); //??
          return to_send.length();
        }
        else
        { ShowSong=0; // for the next round
          Chunks++;
          to_send += "</table></body></html>";
          to_send.getBytes(buffer, maxLen); //??
          return to_send.length();
        }
      }  
      else if(Chunks==2)
      {  Chunks=0;
         return to_send.length();
      }  
      
    });
    request->send(response);
}


void makedesignator(char *Designator, int ShowSong)
{ char *p = Designator;
  *p++=KnobDecals[ShowSong/10];
  *p++='<';
  *p++='b';
  *p++='r';
  *p++='>';
  *p++=KnobDecals[(ShowSong%10)+10];
  if(*(p-1)=='0') // change a 0 into 10
  { *(p-1)='1';
  *p++='<';
  *p++='b';
  *p++='r';
  *p++='>';
  *p++='0'; // digit 0
  }
  *p++='<';
  *p++='b';
  *p++='r';
  *p++='>';
  *p++=KnobDecals[(ShowSong+1)/10];
  *p++='<';
  *p++='b';
  *p++='r';
  *p++='>';
  *p++=KnobDecals[((ShowSong+1)%10)+10];
  if(*(p-1)=='0') // change a 0 into 10
  { *(p-1)='1';
  *p++='<';
  *p++='b';
  *p++='r';
  *p++='>';
  *p++='0';
  }
  *p=0;
}

void fitoncard(char *songtitle)
{ char *p5;
  // song titles can be too long to fit on the card
  while(strlen(songtitle)>29)
  { if((p5=strrchr(songtitle, '('))!=NULL)*p5=0; // get rid of additional (...) text in song title
    else
    { if((p5=strrchr(songtitle, ' '))!=NULL)*p5=0; // or trim after a space
      else if(strlen(songtitle)>29)
      { songtitle[29]=0; // or just cut it with an axe
      }
    }
  }
}  


void makedesignator2(char *Designator, int ShowSong)
{ char *p = Designator;

  *p++=KnobDecals[ShowSong/10];
  if(strcmp(CardLayout, "Rockola")==0)
  { *p++=KnobDecals[(ShowSong%10)+10];
    *p=0; // terminate
    return; // and done
  }  
  
  *p++='<';
  *p++='b';
  *p++='r';
  *p++='>';
  *p++=KnobDecals[(ShowSong%10)+10];

  if(*(p-1)=='0') // change a 0 into 10
  { *(p-1)='1';
    *p++='<';
    *p++='b';
    *p++='r';
    *p++='>';
    *p++='0';
  }
  
  *p++='<';
  *p++='b';
  *p++='r';
  *p++='>';
  *p=0; // terminate
}


void combineartists(char *ArtistName, int ShowSong)
{ char *p5;
  char name1[40];
  char name2[40];
  bool toggle = false;
  char *target;

  strcpy(name1, JukeBoxSongs[ShowSong].artist);
  strcpy(name2, JukeBoxSongs[ShowSong+1].artist);
  
  // combine artist names in one name if need be
  if(strcmp(name1, name2)==0 || strlen(name2)==0)
  { // same artist
    strcpy(ArtistName, name1);
    return;
  }

  // fit both names in 33 characters, name1 + "/" + name2 
  strcpy(ArtistName, JukeBoxSongs[ShowSong].artist);
  while(strlen(name1) + strlen(name2)>32)
  { // shorten the longest first
    if(strlen(name1)>strlen(name2))
    { target = name1;
    }
    else
    { target = name2;
    }
    // shorten
    if((p5=strrchr(target, '/'))!=NULL)*p5=0; // get rid of additional /.... text in artist name
    else
    { if((p5=strrchr(target, ','))!=NULL)*p5=0; // get rid of additional /.... text in artist name
      else
      { if((p5=strrchr(target, ' '))!=NULL)*p5=0; // or trim after a space
        else // use axe and cut one character off
        { target[strlen(target)]=0;
        }
      }  
    }
  }
  // white diamond seperator between names
  sprintf(ArtistName, "%s&#9671;%s", name1, name2);
}


void ShowJukeStrips(AsyncWebServerRequest *request)
{ static int ShowSong = 0;
  static int Chunks = 0;
  static int RowCount = 0;
  static char Designator[32];
  static char ArtistName[50];
  static int Columns = 2; // vertical strips of 10 cards for wall box
  static int col;
  static char songtitle[48];  
  
    AsyncWebServerResponse *response = request->beginChunkedResponse("text/html; charset=UTF-8", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t 
    { String to_send = "";
      if(Chunks==0)
      { // full credits to Alexander Nied for his inspiring and neat ccs approach
        to_send += "<!doctype html><html>\n<head>\n";
//        to_send += "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\">";
        to_send += "<meta http-equiv=\"Cache-control max-age=2592000\">\n";
        to_send += "<style type=\"text/css\">\n";
        to_send += ".cardwrapper {display: flex;}\n";
        to_send += ".card {width: 3in;height: 1in;background-image: url(/artweb/jukestrip.jpg);background-size: 3in 1in;position: relative;}\n";
        to_send += ".info {position: absolute;width: 100%;text-align: center;}\n";
        to_send += ".title1 {top: .15in; font-family: Arial, Helvetica, sans-serif;font-weight: bold; font-size: 15px}\n";
        to_send += ".artist {top: .42in;font-family: Arial, Helvetica, sans-serif;font-weight: bold;font-size: 12px}\n";
        to_send += ".title2 {top: .65in;font-family: Arial, Helvetica, sans-serif;font-weight: bold; font-size: 15px}\n";
        to_send += ".spacer {width: 0.25in;height: 1in;position: relative;}\n";
        if(strcmp(CardLayout, "Rockola")==0)
        { to_send += ".cardnumber1 {position: absolute;width: 100%; top: .15in; left: .06in; font-family: Arial, Helvetica, sans-serif;font-weight: bold; font-size: 17px}\n";
          to_send += ".cardnumber2 {position: absolute;width: 100%; top: .65in; left: .06in; font-family: Arial, Helvetica, sans-serif;font-weight: bold; font-size: 17px}\n";
        }
        else
        { to_send += ".cardnumber1 {position: absolute;top: .05in;left: .02in;font-size: 10px;}\n";
          to_send += ".cardnumber2 {position: absolute;bottom: .08in;left: .02in;font-size: 10px;}\n";
        }
        to_send += ".content {margin: auto;}\n";
        
        if(strcmp(CardLayout, "Rockola")==0)
        { to_send += "body {background-image: url(/artweb/beton.jpg);background-size: cover;background-color: #cccccc;}\n";
        }
        to_send += "</style></head>\n<body>\n<br><br>\n<table class=\"content\">\n<tr>\n<td>\n";

        Chunks++;
        to_send.getBytes(buffer, maxLen); //??
        return to_send.length();
      }
      else if(Chunks==1)
      { if(ShowSong<99) // 98 & 99 is the last card
        { 
          
          
//<div class="cardwrapper">
//  <div class="card">
//    <div class="info title1">Let It Be</div>
//    <div class="info artist">Beatles</div>
//    <div class="info title2">Yellow Submarine</div>
//    <div class="cardnumber">H1<br>H2</div>
//  </div>
//  <div class="card">
//    <div class="info title1">Bad</div>
//    <div class="info artist">Michael Jackson</div>
//    <div class="info title2">Liberian Girl</div>
//    <div class="cardnumber">J7<br>J8</div>
//  </div>
//</div>
          
          
          

          // 1 card holds 2 songs
          // jukebox strips have only a middle bar for the artist
          // Old the 45rpm records had A/B side belonging to the same artist
          // but now we can have 2 different artists so the middle bar has to be used for two artist names
          // unless you create a playlist with pairs of songs from the same artist
          // the field for the artist will combine names if different        

          to_send += "<div class=\"cardwrapper\">\n";

          if(strcmp(CardLayout, "Rockola")==0)Columns = 5;
          else Columns = 2;

          for(col=0;col<Columns;col++)
          {          
            to_send += "<div class=\"card\">\n";
            to_send += "<div class=\"info title1\">";
            strcpy(songtitle, JukeBoxSongs[ShowSong].title);
            fitoncard(songtitle);
            to_send += songtitle;
            to_send += "</div>\n";
            to_send += "<div class=\"info artist\">";
            combineartists(ArtistName, ShowSong);
            to_send += String(ArtistName);
            to_send += "</div>\n";
            to_send += "<div class=\"info title2\">";
            strcpy(songtitle, JukeBoxSongs[ShowSong+1].title);
            fitoncard(songtitle);
            to_send += songtitle;
            to_send += "</div>\n";
            to_send += "<div class=\"cardnumber1\">";
            makedesignator2(Designator, ShowSong);
            to_send += Designator;
            to_send += "</div>\n";
            to_send += "<div class=\"cardnumber2\">";
            makedesignator2(Designator, ShowSong+1);
            to_send += Designator;
            to_send += "</div>\n";
            to_send += "</div>\n";

            if(Columns>(col+1)) // spacer if need be
            { to_send += "<div class=\"spacer\">";
              to_send += "</div>\n";
            }  
            ShowSong+=2;
          }

          to_send += "</div>\n"; // end of cardwarapper div

          RowCount++;
          if((RowCount % 10)==0)
          { to_send += "<p style=\"page-break-after: always;\">&nbsp;</p>";
            to_send += "<p style=\"page-break-before: always;\">&nbsp;</p>";
          }
          to_send.getBytes(buffer, maxLen); //??
          return to_send.length();
        }
        else
        { ShowSong=0; // for the next round
          RowCount=0; // for the next round
          Chunks++;
          to_send += "</td>\n</tr>\n</table>\n</body>\n</html>";
          to_send.getBytes(buffer, maxLen); //??
          return to_send.length();
        }
      }  
      else if(Chunks==2)
      {  Chunks=0;
         return to_send.length();
      }  
      
    });
    request->send(response);
}

void CreateArrayFromSD(AsyncWebServerRequest *request)
{ static int ShowSong = 0;
  static int Chunks = 0;
    AsyncWebServerResponse *response = request->beginChunkedResponse("text/plain", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t 
    { String to_send = "";
      if(Chunks==0)
      { to_send += "// here is your array to paste in your code\n";
        to_send += "const char *songs[101]={\n";
        Chunks++;
        to_send.getBytes(buffer, maxLen); //??
        return to_send.length();
      }
      else if(Chunks==1)
      { if(ShowSong<101) // 98 & 99 is the last card
        { to_send += "\"";
          to_send += String(JukeBoxSongs[ShowSong].path);
          if(ShowSong<100)to_send += "\",\n";
          else to_send += "\"\n";          
          ShowSong++;
          to_send.getBytes(buffer, maxLen); //??
          return to_send.length();
        }
        else
        { ShowSong=0; // for the next round
          Chunks++;
          to_send += "};\n";
          to_send.getBytes(buffer, maxLen); //??
          return to_send.length();
        }
      }  
      else if(Chunks==2)
      {  Chunks=0;
         return to_send.length();
      }  
      
    });
    request->send(response);
}
