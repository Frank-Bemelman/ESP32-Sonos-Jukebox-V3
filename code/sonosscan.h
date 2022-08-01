// keeping track of available Sonos devices present in our network



IPAddress ACTIVE_sonosIP;
char ACTIVE_sonosHeaderHost[32]; // will be filled with Host: 192.168.1.29:1400 upon finding a Sonos device

IPAddress  G_SonosIPList[MAXSONOS];  // addresses of Sonos devices
char ACTIVE_sonosSerialnumber[16];
IPAddress  G_SonosFound_IPList[MAXSONOS];  // addresses of Sonos devices


WiFiClient G_Sonosclient;                           // setup wifiClient for Sonos Control

SonosUPnP G_Sonos = SonosUPnP(G_Sonosclient, ethConnectError);


// prototypes
IPAddress GetSonosSetup(void);
byte Read_BaseIP(IPAddress * adress);
void SonosCheck(IPAddress SAdress);
