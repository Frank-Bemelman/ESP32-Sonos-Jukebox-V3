/************************************************************************/
/* Sonos UPnP, an UPnP based read/write remote control library, v1.1.   */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Thomas Mittet (code@lookout.no) January 2015.             */
/************************************************************************/
#include <TFT_eSPI.h>

extern TFT_eSPI tft;
//extern void ethConnectError(void);
extern void TFT_line_print(int line, const char *content);
extern void UpdateLCD(void);


#include "SonosUPnP.h"

//int strpos(const char *haystack, const char *needle)
//{
//   const char *p = strstr(haystack, needle);
//   if (p)
//      return p - haystack;
//   return -1;
//}


const char p_HttpVersion[] PROGMEM = HTTP_VERSION;
const char p_HeaderHost[] PROGMEM = HEADER_HOST;
const char p_HeaderContentType[] PROGMEM = HEADER_CONTENT_TYPE;
const char p_HeaderContentLength[] PROGMEM = HEADER_CONTENT_LENGTH;
const char p_HeaderSoapAction[] PROGMEM = HEADER_SOAP_ACTION;
const char p_HeaderConnection[] PROGMEM = HEADER_CONNECTION;

const char p_SoapEnvelopeStart[] PROGMEM = SOAP_ENVELOPE_START;
const char p_SoapEnvelopeEnd[] PROGMEM = SOAP_ENVELOPE_END;
const char p_SoapBodyStart[] PROGMEM = SOAP_BODY_START;
const char p_SoapBodyEnd[] PROGMEM = SOAP_BODY_END;
const char p_SoapEnvelope[] PROGMEM = SOAP_TAG_ENVELOPE;
const char p_SoapBody[] PROGMEM = SOAP_TAG_BODY;

const char p_UpnpUrnSchema[] PROGMEM = UPNP_URN_SCHEMA;
const char p_UpnpAvTransportService[] PROGMEM = UPNP_AV_TRANSPORT_SERVICE;
const char p_UpnpAvTransportEndpoint[] PROGMEM = UPNP_AV_TRANSPORT_ENDPOINT;
const char p_UpnpRenderingControlService[] PROGMEM = UPNP_RENDERING_CONTROL_SERVICE;
const char p_UpnpRenderingControlEndpoint[] PROGMEM = UPNP_RENDERING_CONTROL_ENDPOINT;
const char p_UpnpDevicePropertiesService[] PROGMEM = UPNP_DEVICE_PROPERTIES_SERVICE;
const char p_UpnpDevicePropertiesEndpoint[] PROGMEM = UPNP_DEVICE_PROPERTIES_ENDPOINT;
const char p_UpnpContentDirectoryService[] PROGMEM = UPNP_CONTENT_DIRECTORY_SERVICE;           
const char p_UpnpContentDirectoryEndpoint[] PROGMEM = UPNP_CONTENT_DIRECTORY_ENDPOINT;           

const char p_GetZoneAttributesA [] = PROGMEM SONOS_TAG_GET_ZONE_ATTR;
const char p_GetZoneAttributesR [] = PROGMEM SONOS_TAG_GET_ZONE_ATTR_RESPONSE;
const char p_ZoneName[] = PROGMEM SONOS_TAG_ZONENAME ;
const char p_GetZoneInfoA [] = PROGMEM SONOS_TAG_GET_ZONE_INFO;
const char p_GetZoneInfoR [] = PROGMEM SONOS_TAG_GET_ZONE_INFO_RESPONSE;
const char p_Serial[] = PROGMEM SONOS_TAG_SERIAL ;

const char p_ZPSupportInfo[] = PROGMEM SONOS_GET_ZPSUPPORTINFO; 
const char p_ZPInfo[] = PROGMEM SONOS_GET_ZPINFO; 
const char p_ZPZone[] = PROGMEM SONOS_GET_ZPZONE;
const char p_ZPLocalUID[] = PROGMEM SONOS_GET_ZPLOCALUID;
const char p_ZPSerial[] = PROGMEM SONOS_GET_ZPSERIAL; 
const char p_ZPSeriesID[] = PROGMEM SONOS_GET_ZPSERIESID; 



const char p_Play[] PROGMEM = SONOS_TAG_PLAY;
const char p_SourceRinconTemplate[] PROGMEM = SONOS_SOURCE_RINCON_TEMPLATE;
const char p_Stop[] PROGMEM = SONOS_TAG_STOP;
const char p_Pause[] PROGMEM = SONOS_TAG_PAUSE;
const char p_Previous[] PROGMEM = SONOS_TAG_PREVIOUS;
const char p_Next[] PROGMEM = SONOS_TAG_NEXT;
const char p_InstenceId0Tag[] PROGMEM = SONOS_INSTANCE_ID_0_TAG;
const char p_Seek[] PROGMEM = SONOS_TAG_SEEK;
const char p_SeekModeTagStart[] PROGMEM = SONOS_SEEK_MODE_TAG_START;
const char p_SeekModeTagEnd[] PROGMEM = SONOS_SEEK_MODE_TAG_END;
const char p_TimeFormatTemplate[] PROGMEM = SONOS_TIME_FORMAT_TEMPLATE;
const char p_SetAVTransportURI[] PROGMEM = SONOS_TAG_SET_AV_TRANSPORT_URI;
const char p_UriMetaLightStart[] PROGMEM = SONOS_URI_META_LIGHT_START;
const char p_UriMetaLightEnd[] PROGMEM = SONOS_URI_META_LIGHT_END;
const char p_RadioMetaFullStart[] PROGMEM = SONOS_RADIO_META_FULL_START;
const char p_RadioMetaFullEnd[] PROGMEM = SONOS_RADIO_META_FULL_END;
const char p_BecomeCoordinatorOfStandaloneGroup[] PROGMEM = SONOS_TAG_BECOME_COORDINATOR_OF_STANDALONE_GROUP;
const char p_SetLEDState[] PROGMEM = SONOS_TAG_SET_LED_STATE;

const char p_AddURIToQueue[] PROGMEM = SONOS_TAG_ADD_URI_TO_QUEUE;
const char p_SavedQueues[] PROGMEM = SONOS_SAVED_QUEUES;
const char p_RemoveAllTracksFromQueue[] PROGMEM = SONOS_TAG_REMOVE_ALL_TRACKS_FROM_QUEUE;
const char p_PlaylistMetaLightStart[] PROGMEM = SONOS_PLAYLIST_META_LIGHT_START;
const char p_PlaylistMetaLightEnd[] PROGMEM = SONOS_PLAYLIST_META_LIGHT_END;

const char p_GetPositionInfoA[] PROGMEM = SONOS_TAG_GET_POSITION_INFO;
const char p_GetPositionInfoR[] PROGMEM = SONOS_TAG_GET_POSITION_INFO_RESPONSE;
const char p_Track[] PROGMEM = SONOS_TAG_TRACK;
const char p_NrTracks[] PROGMEM = SONOS_TAG_NRTRACKS;
const char p_TrackDuration[] PROGMEM = SONOS_TAG_TRACK_DURATION;
const char p_MediaDuration[] PROGMEM = SONOS_TAG_MEDIA_DURATION;

const char p_TrackURI[] PROGMEM = SONOS_TAG_TRACK_URI;
const char p_CurrentURI[] PROGMEM = SONOS_TAG_CURRENT_URI;
const char p_RelTime[] PROGMEM = SONOS_TAG_REL_TIME;
const char p_TrackMetaData[] PROGMEM = SONOS_TAG_TRACK_METADATA;

// nieuw
const char p_TrackMeta[] PROGMEM = SONOS_TAG_TRACKMETA;
const char p_TrackTitle[] PROGMEM = SONOS_ATTRIB_TITLE;
const char p_TrackCreator[] PROGMEM = SONOS_ATTRIB_CREATOR;
const char p_TrackAlbum[] PROGMEM = SONOS_ATTRIB_ALBUM;
const char p_TrackArtist[] PROGMEM = SONOS_ATTRIB_ARTIST;


const char p_GetMediaInfoA[] PROGMEM = SONOS_TAG_GET_MEDIA_INFO;
const char p_GetMediaInfoR[] PROGMEM = SONOS_TAG_GET_MEDIA_INFO_RESPONSE;
const char p_CurrentURIMetaData[] PROGMEM = SONOS_TAG_CURRENTURIMETADATA;

// ******************************************************************************************************************************************
const char p_BrowseA[] PROGMEM = SONOS_TAG_BROWSE;
const char p_BrowseR[] PROGMEM = SONOS_TAG_BROWSE_RESPONSE;




const char p_GetMuteA[] PROGMEM = SONOS_TAG_GET_MUTE;
const char p_GetMuteR[] PROGMEM = SONOS_TAG_GET_MUTE_RESPONSE;
const char p_CurrentMute[] PROGMEM = SONOS_TAG_CURRENT_MUTE;
const char p_GetVolumeA[] PROGMEM = SONOS_TAG_GET_VOLUME;
const char p_GetVolumeR[] PROGMEM = SONOS_TAG_GET_VOLUME_RESPONSE;
const char p_CurrentVolume[] PROGMEM = SONOS_TAG_CURRENT_VOLUME;
const char p_GetOutputFixedA[] PROGMEM = SONOS_TAG_GET_OUTPUT_FIXED;
const char p_GetOutputFixedR[] PROGMEM = SONOS_TAG_GET_FIXED_RESPONSE;
const char p_CurrentFixed[] PROGMEM = SONOS_TAG_CURRENT_FIXED;
const char p_GetBassA[] PROGMEM = SONOS_TAG_GET_BASS;
const char p_GetBassR[] PROGMEM = SONOS_TAG_GET_BASS_RESPONSE;
const char p_CurrentBass[] PROGMEM = SONOS_TAG_CURRENT_BASS;
const char p_GetTrebleA[] PROGMEM = SONOS_TAG_GET_TREBLE;
const char p_GetTrebleR[] PROGMEM = SONOS_TAG_GET_TREBLE_RESPONSE;
const char p_CurrentTreble[] PROGMEM = SONOS_TAG_CURRENT_TREBLE;
const char p_GetLoudnessA[] PROGMEM = SONOS_TAG_GET_LOUDNESS;
const char p_GetLoudnessR[] PROGMEM = SONOS_TAG_GET_LOUDNESS_RESPONSE;
const char p_CurrentLoudness[] PROGMEM = SONOS_TAG_CURRENT_LOUDNESS;

const char p_SetMute[] PROGMEM = SONOS_TAG_SET_MUTE;
const char p_SetVolume[] PROGMEM = SONOS_TAG_SET_VOLUME;
const char p_SetBass[] PROGMEM = SONOS_TAG_SET_BASS;
const char p_SetTreble[] PROGMEM = SONOS_TAG_SET_TREBLE;
const char p_SetLoudness[] PROGMEM = SONOS_TAG_SET_LOUDNESS;
const char p_ChannelTagStart[] PROGMEM = SONOS_CHANNEL_TAG_START;
const char p_ChannelTagEnd[] PROGMEM = SONOS_CHANNEL_TAG_END;

const char p_GetTransportSettingsA[] PROGMEM = SONOS_TAG_GET_TRANSPORT_SETTINGS;
const char p_GetTransportSettingsR[] PROGMEM = SONOS_TAG_GET_TRANSPORT_SETTINGS_RESPONSE;
const char p_PlayMode[] PROGMEM = SONOS_TAG_PLAY_MODE;
const char p_SetPlayMode[] PROGMEM = SONOS_TAG_SET_PLAY_MODE;

const char p_GetTransportInfoA[] PROGMEM = SONOS_TAG_GET_TRANSPORT_INFO;
const char p_GetTransportInfoR[] PROGMEM = SONOS_TAG_GET_TRANSPORT_INFO_RESPONSE;
const char p_CurrentTransportState[] PROGMEM = SONOS_TAG_CURRENT_TRANSPORT_STATE;

const char p_GetMediaMetadata[] PROGMEM = SONOS_TAG_GET_MEDIAMETADATA;    // frank 24JUL20
const char p_GetMediaMetadataResponse[] PROGMEM = SONOS_TAG_GET_MEDIAMETADATARESPONSE;    // frank 24JUL20
const char p_GetMediaMetadataResult[] PROGMEM = SONOS_TAG_GET_MEDIAMETADATARESULT;    // frank 24JUL20
const char p_Id[] PROGMEM = SONOS_TAG_ID;    // frank 24JUL20
const char p_Title[] PROGMEM = SONOS_TAG_TITLE;    // frank 24JUL20


const char p_CurrentMedium[] PROGMEM = SONOS_TAG_MEDIUM_STATUS;
const char p_CurrentArtist[] PROGMEM = SONOS_TAG_ARTIST_STATUS;

const char p_UPnPBroadcast[] PROGMEM = UPNP_DEVICE_SCAN;


const char *p_MediaSource[SONOS_MAXSOURCE]={
SONOS_SOURCE_UNKNOWN_SCHEME,
SONOS_SOURCE_SPOTIFY_SCHEME,
SONOS_SOURCE_FILE_SCHEME,
SONOS_SOURCE_LIBRARY_SCHEME,
SONOS_SOURCE_HTTP_SCHEME,
SONOS_SOURCE_RADIO_SCHEME,
SONOS_SOURCE_RADIO_AAC_SCHEME,
SONOS_SOURCE_LINEIN_SCHEME,
SONOS_SOURCE_MASTER_SCHEME,
SONOS_SOURCE_QUEUE_SCHEME,
SONOS_SOURCE_SPOTIFYSTATION_SCHEME,
SONOS_SOURCE_LOCALHTTP_SCHEME,
SONOS_SOURCE_LOCALHTTPS_SCHEME,
SONOS_SOURCE_SPOTIFY_RADIO_SCHEME };

const char *p_MediaSourceName[SONOS_MAXSOURCE]={
UNKNOWN_SCHEME,
SPOTIFY_SCHEME,
FILE_SCHEME,
LIBRARY_SCHEME,
HTTP_SCHEME,
RADIO_SCHEME,
RADIO_AAC_SCHEME,
LINEIN_SCHEME,
MASTER_SCHEME,
QUEUE_SCHEME,
SPOTIFYSTATION_SCHEME,
LOCALHTTP_SCHEME,
LOCALHTTPS_SCHEME,
SPOTIFY_RADIO_SCHEME };

#define SELECTSONG 1  // Bepaalt Led Scanner en Jukebox gedrag
#define SELECTRADIO 0 // Bepaalt Led Scanner en Jukebox gedrag

// new local Data Variables - is increasing footprint, but easies passing of extra char-string-info
char CREATOR_BUFFER[76] = "\0";
char ARTIST_BUFFER[76] = "\0";
char TITLE_BUFFER[100] = "\0";
char ALBUM_BUFFER[92] = "\0";
char DURATION_BUFFER[16]= "\0";
char POSITION_BUFFER[16]= "\0";
char ZONE_BUFFER[32]= "\0";
char UID_BUFFER[32]= "\0";
char SERIAL_BUFFER[24]= "\0";
char SERIESID_BUFFER[24]= "\0";
char MEDIUM_BUFFER[16]= "\0";
char STATUS_BUFFER[16]= "\0";
char PLAYMODE_BUFFER[16]= "\0";
char SOURCE_BUFFER[16]= "\0";

SonosUPnP::SonosUPnP(WiFiClient client, void (*ethernetErrCallback)(void))
{
  #ifndef SONOS_WRITE_ONLY_MODE
  this->xPath = MicroXPath_P();
  #endif
  this->ethClient = client;
  this->ethernetErrCallback = ethernetErrCallback;
}


void SonosUPnP::setAVTransportURI(IPAddress speakerIP, const char *scheme, const char *address)
{
  setAVTransportURI(speakerIP, scheme, address, p_UriMetaLightStart, p_UriMetaLightEnd, "");
}

void SonosUPnP::seekTrack(IPAddress speakerIP, uint16_t index)
{
  char indexChar[6];
  itoa(index, indexChar, 10);
  seek(speakerIP, SONOS_SEEK_MODE_TRACK_NR, indexChar);
}

void SonosUPnP::seekTime(IPAddress speakerIP, uint8_t hour, uint8_t minute, uint8_t second)
{
  char time[11];
  sprintf_P(time, p_TimeFormatTemplate, hour, minute, second);
  seek(speakerIP, SONOS_SEEK_MODE_REL_TIME, time);
}

void SonosUPnP::setPlayMode(IPAddress speakerIP, uint8_t playMode)
{
  const char *playModeValue;
  switch (playMode)
  {
    case SONOS_PLAY_MODE_REPEAT:
      playModeValue = SONOS_PLAY_MODE_REPEAT_VALUE;
      break;
    case SONOS_PLAY_MODE_SHUFFLE_REPEAT:
      playModeValue = SONOS_PLAY_MODE_SHUFFLE_REPEAT_VALUE;
      break;
    case SONOS_PLAY_MODE_SHUFFLE:
      playModeValue = SONOS_PLAY_MODE_SHUFFLE_VALUE;
      break;
    default:
      playModeValue = SONOS_PLAY_MODE_NORMAL_VALUE;
      break;
  }
  upnpSet(speakerIP, UPNP_AV_TRANSPORT, p_SetPlayMode, SONOS_TAG_NEW_PLAY_MODE, playModeValue);
}

void SonosUPnP::play(IPAddress speakerIP)
{
  upnpSet(speakerIP, UPNP_AV_TRANSPORT, p_Play, SONOS_TAG_SPEED, "1");
}

void SonosUPnP::playFile(IPAddress speakerIP, const char *path)
{
  setAVTransportURI(speakerIP, SONOS_SOURCE_FILE_SCHEME, path);
  play(speakerIP);
}

void SonosUPnP::playHttp(IPAddress speakerIP, const char *address)
{
  // "x-sonos-http:" does not work for me etAVTransportURI(speakerIP, SONOS_SOURCE_HTTP_SCHEME, address);
  setAVTransportURI(speakerIP, "", address);
  play(speakerIP);
}

void SonosUPnP::playRadio(IPAddress speakerIP, const char *address, const char *title)
{
  setAVTransportURI(speakerIP, SONOS_SOURCE_RADIO_SCHEME, address, p_RadioMetaFullStart, p_RadioMetaFullEnd, title);
  play(speakerIP);
}

void SonosUPnP::playLineIn(IPAddress speakerIP, const char *speakerID)
{
  char address[30];
  sprintf_P(address, p_SourceRinconTemplate, speakerID, UPNP_PORT, "");
  setAVTransportURI(speakerIP, SONOS_SOURCE_LINEIN_SCHEME, address);
  play(speakerIP);
}

void SonosUPnP::playQueue(IPAddress speakerIP, const char *speakerID)
{
  char address[30];
  sprintf_P(address, p_SourceRinconTemplate, speakerID, UPNP_PORT, "#0");
  setAVTransportURI(speakerIP, SONOS_SOURCE_QUEUE_SCHEME, address);
  play(speakerIP);
}

void SonosUPnP::playConnectToMaster(IPAddress speakerIP, const char *masterSpeakerID)
{
  char address[30];
  sprintf_P(address, p_SourceRinconTemplate, masterSpeakerID, UPNP_PORT, "");
  setAVTransportURI(speakerIP, SONOS_SOURCE_MASTER_SCHEME, address);
}

void SonosUPnP::disconnectFromMaster(IPAddress speakerIP)
{
  upnpSet(speakerIP, UPNP_AV_TRANSPORT, p_BecomeCoordinatorOfStandaloneGroup);
}

// deze wordt alleen aangeroepen als er een fout is? En dan de sonos nog commanderen om te stoppen?
void SonosUPnP::stop(IPAddress speakerIP)
{
  upnpSet(speakerIP, UPNP_AV_TRANSPORT, p_Stop);
}

void SonosUPnP::pause(IPAddress speakerIP)
{
  upnpSet(speakerIP, UPNP_AV_TRANSPORT, p_Pause);
}

void SonosUPnP::skip(IPAddress speakerIP, uint8_t direction)
{
  upnpSet(
    speakerIP, UPNP_AV_TRANSPORT, direction == SONOS_DIRECTION_FORWARD ? p_Next : p_Previous);
}

void SonosUPnP::setMute(IPAddress speakerIP, bool state)
{
  upnpSet(
    speakerIP, UPNP_RENDERING_CONTROL, p_SetMute,
    SONOS_TAG_DESIRED_MUTE, state ? "1" : "0", "", p_ChannelTagStart, p_ChannelTagEnd, SONOS_CHANNEL_MASTER);
}

void SonosUPnP::setVolume(IPAddress speakerIP, uint8_t volume)
{
  setVolume(speakerIP, volume, SONOS_CHANNEL_MASTER);
}

void SonosUPnP::setVolume(IPAddress speakerIP, uint8_t volume, const char *channel)
{
  if (volume > 100) volume = 100;
  char volumeChar[4];
  itoa(volume, volumeChar, 10);
  upnpSet(
    speakerIP, UPNP_RENDERING_CONTROL, p_SetVolume,
    SONOS_TAG_DESIRED_VOLUME, volumeChar, "", p_ChannelTagStart, p_ChannelTagEnd, channel);
}

void SonosUPnP::setBass(IPAddress speakerIP, int8_t bass)
{
  bass = constrain(bass, -10, 10);
  char bassChar[4];
  itoa(bass, bassChar, 10);
  upnpSet(
    speakerIP, UPNP_RENDERING_CONTROL, p_SetBass,
    SONOS_TAG_DESIRED_BASS, bassChar);
}

void SonosUPnP::setTreble(IPAddress speakerIP, int8_t treble)
{
  treble = constrain(treble, -10, 10);
  char trebleChar[4];
  itoa(treble, trebleChar, 10);
  upnpSet(
    speakerIP, UPNP_RENDERING_CONTROL, p_SetTreble,
    SONOS_TAG_DESIRED_TREBLE, trebleChar);
}

void SonosUPnP::setLoudness(IPAddress speakerIP, bool state)
{
  upnpSet(
    speakerIP, UPNP_RENDERING_CONTROL, p_SetLoudness,
    SONOS_TAG_DESIRED_LOUDNESS, state ? "1" : "0", "", p_ChannelTagStart, p_ChannelTagEnd, SONOS_CHANNEL_MASTER);
}

void SonosUPnP::setStatusLight(IPAddress speakerIP, bool state)
{
  upnpSet(
    speakerIP, UPNP_DEVICE_PROPERTIES, p_SetLEDState,
    SONOS_TAG_DESIRED_LED_STATE, state ? "On" : "Off");
}

void SonosUPnP::addPlaylistToQueue(IPAddress speakerIP, uint16_t playlistIndex)
{
  char path[45];
  sprintf_P(path, p_SavedQueues, playlistIndex);
  addTrackToQueue(speakerIP, "", path);
}

void SonosUPnP::addTrackToQueue(IPAddress speakerIP, const char *scheme, const char *address)
{
  upnpSet(
    speakerIP, UPNP_AV_TRANSPORT, p_AddURIToQueue,
    SONOS_TAG_ENQUEUED_URI, scheme, address, p_PlaylistMetaLightStart, p_PlaylistMetaLightEnd, "");
}

void SonosUPnP::removeAllTracksFromQueue(IPAddress speakerIP)
{
  upnpSet(speakerIP, UPNP_AV_TRANSPORT, p_RemoveAllTracksFromQueue);
}


#ifndef SONOS_WRITE_ONLY_MODE

void SonosUPnP::setRepeat(IPAddress speakerIP, bool repeat)
{
  bool current = getRepeat(speakerIP);
  if (repeat != current)
  {
    setPlayMode(speakerIP, current ^ SONOS_PLAY_MODE_REPEAT);
  }
}

void SonosUPnP::setShuffle(IPAddress speakerIP, bool shuffle)
{
  bool current = getShuffle(speakerIP);
  if (shuffle != current)
  {
    setPlayMode(speakerIP, current ^ SONOS_PLAY_MODE_SHUFFLE);
  }
}

void SonosUPnP::toggleRepeat(IPAddress speakerIP)
{
  setPlayMode(speakerIP, getPlayMode(speakerIP) ^ SONOS_PLAY_MODE_REPEAT);
}

void SonosUPnP::toggleShuffle(IPAddress speakerIP)
{
  setPlayMode(speakerIP, getPlayMode(speakerIP) ^ SONOS_PLAY_MODE_SHUFFLE);
}

void SonosUPnP::togglePause(IPAddress speakerIP)
{
  uint8_t state = getState(speakerIP);
  if (state == SONOS_STATE_PLAYING)
  {
    pause(speakerIP);
  }
  else if (state == SONOS_STATE_PAUSED)
  {
    play(speakerIP);
  }
}

void SonosUPnP::toggleMute(IPAddress speakerIP)
{
  setMute(speakerIP, !getMute(speakerIP));
}

void SonosUPnP::toggleLoudness(IPAddress speakerIP)
{
  setLoudness(speakerIP, !getLoudness(speakerIP));
}

// JV
// New function : fill Sonmosionfo Structure with info
// 1. use parse HTTP:/[ip.nu.mb.er]:1400/status/zp command - 
// 2.
SonosInfo SonosUPnP::getSonosInfo(IPAddress speakerIP)
{ SonosInfo ZP;
  UID_BUFFER[0]=0;SERIAL_BUFFER[0]=0;SERIESID_BUFFER[0]=0;ZONE_BUFFER[0]=0;MEDIUM_BUFFER[0]=0;STATUS_BUFFER[0]=0;PLAYMODE_BUFFER[0]=0;SOURCE_BUFFER[0]=0; // set all buffers to terminated 0 string

  // initialize ZP just to be safe  
  ZP.uid = UID_BUFFER;
  ZP.serial = SERIAL_BUFFER;
  ZP.seriesid = SERIESID_BUFFER;
  ZP.zone = ZONE_BUFFER;
  ZP.medium = MEDIUM_BUFFER;
  ZP.status = STATUS_BUFFER;
  ZP.playmode = PLAYMODE_BUFFER;
  ZP.source = SOURCE_BUFFER;
  

  if (upnpGetzp(speakerIP) )  // check if speaker is there
  {  //tft.print("PARSE:");tft.println(speakerIP);
    xPath.reset();
    char infoBuffer[20] = "";
    // Zone Info
    ZP.zone = ZONE_BUFFER;
    PGM_P zpath[] = { p_ZPSupportInfo, p_ZPInfo, p_ZPZone};
    ethClient_xPath(zpath, 3, ZONE_BUFFER, sizeof(ZONE_BUFFER));
    // Local UIDInfo
    ZP.uid = UID_BUFFER;
    PGM_P ypath[] = { p_ZPSupportInfo, p_ZPInfo, p_ZPLocalUID};
    ethClient_xPath(ypath, 3, UID_BUFFER, sizeof(UID_BUFFER));
    // Serial Info
    ZP.serial = SERIAL_BUFFER;
    PGM_P xpath[] = { p_ZPSupportInfo, p_ZPInfo, p_ZPSerial};
    ethClient_xPath(xpath, 3, SERIAL_BUFFER, sizeof(SERIAL_BUFFER));
    // Series Info
    // Did not find any Series info with a ZP90 and a PLAY5 ???  
    ZP.seriesid = SERIESID_BUFFER;
    PGM_P wpath[] = { p_ZPSupportInfo, p_ZPInfo, p_ZPSeriesID};
    ethClient_xPath(wpath, 3, SERIESID_BUFFER, sizeof(SERIESID_BUFFER));

// tft.print("STAT:");tft.println(speakerIP);
  getState(speakerIP,STATUS_BUFFER);

// tft.print("MED:");tft.println(speakerIP);
  getMedium(speakerIP,MEDIUM_BUFFER);

// tft.print("SRC:");tft.println(speakerIP);
  getSource(speakerIP,SOURCE_BUFFER);

 //tft.print("PM:");tft.println(speakerIP);
  getPlayMode(speakerIP,PLAYMODE_BUFFER);
    
    
  }
  ethClient_stop();

// tft.print("STAT:");tft.println(speakerIP);
//  getState(speakerIP,STATUS_BUFFER);

// tft.print("MED:");tft.println(speakerIP);
//  getMedium(speakerIP,MEDIUM_BUFFER);

// tft.print("SRC:");tft.println(speakerIP);
//  getSource(speakerIP,SOURCE_BUFFER);

// tft.print("PM:");tft.println(speakerIP);
// getPlayMode(speakerIP,PLAYMODE_BUFFER);

  return ZP;
}

uint8_t SonosUPnP::getState(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetTransportInfoR, p_CurrentTransportState };
  //             { p_SoapEnvelope, p_SoapBody, p_GetTransportInfoR, p_CurrentSpeed };
  char result[sizeof(SONOS_STATE_PAUSED_VALUE)] = "";
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetTransportInfoA, "", "", path, 4, result, sizeof(result));
  return convertState(result);
}

uint8_t SonosUPnP::getState(IPAddress speakerIP,char *buf) // New JV : string passthrough
{
  int t;
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetTransportInfoR, p_CurrentTransportState };
  char result[16] = "\0";
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetTransportInfoA, "", "", path, 4, result, sizeof(result));
  if(result[0]!=0) {
    for (t=0;result[t]!=0;++t) buf[t]=result[t];
    buf[t]=0;
    return convertState(result);
  }
  buf[0]=0; return 0;
}


uint8_t SonosUPnP::getMedium(IPAddress speakerIP) // New JV Medium state
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetMediaInfoR, p_CurrentMedium };
  char result[16] = "\0";
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetMediaInfoA, "", "", path, 4, result, sizeof(result));
    return convertMedium(result);
}

uint8_t SonosUPnP::getMedium(IPAddress speakerIP,char *buf) // New JV Medium with string passthrough
{
  int t;
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetMediaInfoR, p_CurrentMedium };
  char result[16] = "\0";
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetMediaInfoA, "", "", path, 4, result, sizeof(result));
  //Serial.print(" /1 ");Serial.println(result);
  if(result[0]!=0) {
    for (t=0;result[t]!=0;++t) buf[t]=result[t];
    buf[t]=0;
    return convertMedium(result);
  }
  buf[0]=0; return 0;
}

bool SonosUPnP::getZone(IPAddress speakerIP,char *buf) // New JV : string passthrough
{
  int t;
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetZoneAttributesR, p_ZoneName };
  char result[32] = "\0";
  upnpGetString(speakerIP, UPNP_DEVICE_PROPERTIES, p_GetZoneAttributesA, "", "", path, 4, result, sizeof(result));
  if(result[0]!=0) {
  for (t=0;result[t]!=0;++t) buf[t]=result[t];
  buf[t]=0;
  return 1;
  }
   buf[0]=0; return 0;
}

bool SonosUPnP::getSerial(IPAddress speakerIP,char *buf) // New JV : string passthrough
{
  int t;
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetZoneInfoR, p_Serial };
  char result[20] = "\0";
  upnpGetString(speakerIP, UPNP_DEVICE_PROPERTIES, p_GetZoneInfoA, "", "", path, 4, result, sizeof(result));
  if(result[0]!=0) {
  for (t=0;result[t]!=0;++t) buf[t]=result[t];
  buf[t]=0;
  return 1;
  }
  buf[0]=0; return 0;
}

uint8_t SonosUPnP::getPlayMode(IPAddress speakerIP,char *buf) // New JV : string passthrough
{
  int t;
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetTransportSettingsR, p_PlayMode };
  char result[16] = "\0";
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetTransportSettingsA, "", "", path, 4, result, sizeof(result));
  if(result[0]!=0) {
  for (t=0;result[t]!=0;++t) buf[t]=result[t];
  buf[t]=0;
  return convertPlayMode(result);
  }
  buf[0]=0; return 0;
}


uint8_t SonosUPnP::getPlayMode(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetTransportSettingsR, p_PlayMode };
  char result[sizeof(SONOS_PLAY_MODE_SHUFFLE_VALUE)] = "";
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetTransportSettingsA, "", "", path, 4, result, sizeof(result));
  return convertPlayMode(result);
}

bool SonosUPnP::getRepeat(IPAddress speakerIP)
{
  return getPlayMode(speakerIP) & SONOS_PLAY_MODE_REPEAT;
}

bool SonosUPnP::getShuffle(IPAddress speakerIP)
{
  return getPlayMode(speakerIP) & SONOS_PLAY_MODE_SHUFFLE;
}

TrackInfo SonosUPnP::getTrackInfo(IPAddress speakerIP, char *uriBuffer, size_t uriBufferSize, char *metaBuffer, size_t metaBufferSize)
{
  TrackInfo trackInfo;
  if (upnpPost(speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, ""))
  {
    xPath.reset();
    char infoBuffer[20] = "";
    // Track number
    PGM_P npath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_Track };
    ethClient_xPath(npath, 4, infoBuffer, sizeof(infoBuffer));
    trackInfo.number = atoi(infoBuffer);
    // Track duration
    PGM_P dpath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackDuration };
    ethClient_xPath(dpath, 4, infoBuffer, sizeof(infoBuffer));
    trackInfo.duration = getTimeInSeconds(infoBuffer);
// en hoe zit het met <TrackMetaData>[Meta data in DIDL-Lite]</TrackMetaData> een broddelwerkje van Frank - 21-5-2020
    PGM_P mpath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMetaData };
    ethClient_xPath(mpath, 4, metaBuffer, metaBufferSize);
    trackInfo.trackmetadata = metaBuffer;
    // Track URI
    PGM_P upath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackURI };
    ethClient_xPath(upath, 4, uriBuffer, uriBufferSize);
    trackInfo.uri = uriBuffer;
    // Track position
    PGM_P ppath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_RelTime };
    ethClient_xPath(ppath, 4, infoBuffer, sizeof(infoBuffer));
    trackInfo.position = getTimeInSeconds(infoBuffer);
  }
  ethClient_stop();
  return trackInfo;
}

uint16_t SonosUPnP::getTrackNumber(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_Track };
  char result[6] = "0";
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", path, 4, result, sizeof(result));
  return atoi(result);
}

void SonosUPnP::getTrackURI(IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackURI };
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", path, 4, resultBuffer, resultBufferSize);
}

uint8_t SonosUPnP::getSourceFromURI(const char *uri) // adapted JV
{
  uint8_t t,v;
  // Serial.println(uri);
  
  for (t=0;t<SONOS_MAXSOURCE;++t)
      {
     v=0;
      while (p_MediaSource[t][v] !=0)
          {
            if (p_MediaSource[t][v]!=uri[v]) break;
            v++;
          }
  if(p_MediaSource[t][v]==0 ) return t; // we have a match
  }
  return(0);
}

//uint8_t SonosUPnP::getSource(IPAddress speakerIP)
//{
//  char uri[25] = "";
//  getTrackURI(speakerIP, uri, sizeof(uri));
//  return getSourceFromURI(uri);
//}

uint8_t SonosUPnP::getSource(IPAddress speakerIP) // adapted JV
{
  uint8_t t,v;
  char uri[32] = "";
  getTrackURI(speakerIP, uri, sizeof(uri));
  for (t=0;t<SONOS_MAXSOURCE;++t)
      {
     v=0;
      while (p_MediaSource[t][v] !=0)
          {
            if (p_MediaSource[t][v]!=uri[v]) break;
            v++;
          }
  if(p_MediaSource[t][v]==0 ) return t; // we have a match
  }
  return(0);
}

uint8_t SonosUPnP::getSource(IPAddress speakerIP,char *buf) // new JV
{
  uint8_t t,u,v;
  char uri[32] = "";
  getTrackURI(speakerIP, uri, sizeof(uri));
  for (t=0;t<SONOS_MAXSOURCE;++t)
      {
     v=0;
      while (p_MediaSource[t][v] !=0)
          {
            if (p_MediaSource[t][v]!=uri[v]) break;
            v++;
          }
  if(p_MediaSource[t][v]==0 ) // we have a match
    {
    for (u=0; p_MediaSourceName[t][u]!=0 ;u++ ) buf[u]=p_MediaSourceName[t][u]; // copy name to buffer
    buf[u]=0; // end buffer with zero
    return t;
    }
  }
  buf[0]=0;     // make empty buffer string
  return(0);
}

uint32_t SonosUPnP::getTrackDurationInSeconds(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackDuration };
  char result[20] = "";
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", path, 4, result, sizeof(result));
  return getTimeInSeconds(result);
}

uint32_t SonosUPnP::getTrackPositionInSeconds(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_RelTime };
  char result[20] = "";
  upnpGetString(speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", path, 4, result, sizeof(result));
  return getTimeInSeconds(result);
}

uint16_t SonosUPnP::getTrackPositionPerMille(IPAddress speakerIP)
{
  uint16_t perMille = 0;
  if (upnpPost(speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, ""))
  {
    char result[20];
    xPath.reset();
    PGM_P dpath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackDuration };
    ethClient_xPath(dpath, 4, result, sizeof(result));
    uint32_t duration = getTimeInSeconds(result);
    PGM_P ppath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_RelTime };
    ethClient_xPath(ppath, 4, result, sizeof(result));
    uint32_t position = getTimeInSeconds(result);
    if (duration && position)
    {
      perMille = (position * 1000) / duration;
    }
  }
  ethClient_stop();
  return perMille;
}

bool SonosUPnP::getMute(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetMuteR, p_CurrentMute };
  char result[3] = "0";
  upnpGetString(
    speakerIP, UPNP_RENDERING_CONTROL, p_GetMuteA,
    SONOS_TAG_CHANNEL, SONOS_CHANNEL_MASTER, path, 4, result, sizeof(result));
  return strcmp(result, "1") == 0;
}

uint8_t SonosUPnP::getVolume(IPAddress speakerIP)
{ 
  return getVolume(speakerIP, SONOS_CHANNEL_MASTER);
}

//int8_t SonosUPnP::getVolume(IPAddress speakerIP, const char *channel)
//{
//  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetVolumeR, p_CurrentVolume };
//  char result[5] = "0";
//  if( upnpGetString(
//    speakerIP, UPNP_RENDERING_CONTROL, p_GetVolumeA,
//    SONOS_TAG_CHANNEL, channel, path, 4, result, sizeof(result)) )
//    { // succesvolle read 
//      return constrain(atoi(result), 0, 100);
//    }

  // getvolume mislukt
//  return -1;
//}

uint8_t SonosUPnP::getVolume(IPAddress speakerIP, const char *channel)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetVolumeR, p_CurrentVolume };
  char result[5] = "0";
  upnpGetString(
    speakerIP, UPNP_RENDERING_CONTROL, p_GetVolumeA,
    SONOS_TAG_CHANNEL, channel, path, 4, result, sizeof(result));
  return constrain(atoi(result), 0, 100);
}


bool SonosUPnP::getOutputFixed(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetOutputFixedR, p_CurrentFixed };
  char result[3] = "0";
  upnpGetString(speakerIP, UPNP_RENDERING_CONTROL, p_GetOutputFixedA, "", "", path, 4, result, sizeof(result));
  return strcmp(result, "1") == 0;
}

int8_t SonosUPnP::getBass(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetBassR, p_CurrentBass };
  char result[5] = "0";
  upnpGetString(
    speakerIP, UPNP_RENDERING_CONTROL, p_GetBassA,
    SONOS_TAG_CHANNEL, SONOS_CHANNEL_MASTER, path, 4, result, sizeof(result));
  return constrain(atoi(result), -10, 10);
}

int8_t SonosUPnP::getTreble(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetTrebleR, p_CurrentTreble };
  char result[5] = "0";
  upnpGetString(
    speakerIP, UPNP_RENDERING_CONTROL, p_GetTrebleA,
    SONOS_TAG_CHANNEL, SONOS_CHANNEL_MASTER, path, 4, result, sizeof(result));
  return constrain(atoi(result), -10, 10);
}

bool SonosUPnP::getLoudness(IPAddress speakerIP)
{
  PGM_P path[] = { p_SoapEnvelope, p_SoapBody, p_GetLoudnessR, p_CurrentLoudness };
  char result[3] = "0";
  upnpGetString(
    speakerIP, UPNP_RENDERING_CONTROL, p_GetLoudnessA,
    SONOS_TAG_CHANNEL, SONOS_CHANNEL_MASTER, path, 4, result, sizeof(result));
  return strcmp(result, "1") == 0;
}

#endif


void SonosUPnP::seek(IPAddress speakerIP, const char *mode, const char *data)
{
  upnpSet(
    speakerIP, UPNP_AV_TRANSPORT, p_Seek,
    SONOS_TAG_TARGET, data, "", p_SeekModeTagStart, p_SeekModeTagEnd, mode);
}

void SonosUPnP::setAVTransportURI(IPAddress speakerIP, const char *scheme, const char *address, PGM_P metaStart_P, PGM_P metaEnd_P, const char *metaValue)
{
  // Info to show in player, in DIDL format, can be added as META data
  upnpSet(
    speakerIP, UPNP_AV_TRANSPORT, p_SetAVTransportURI,
    SONOS_TAG_CURRENT_URI, scheme, address, metaStart_P, metaEnd_P, metaValue);
}

void SonosUPnP::upnpSet(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P)
{
  upnpSet(ip, upnpMessageType, action_P, "", "");
}

void SonosUPnP::upnpSet(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value)
{
  upnpSet(ip, upnpMessageType, action_P, field, value, "", 0, 0, "");
}

void SonosUPnP::upnpSet(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *valueA, const char *valueB, PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue)
{
  upnpPost(ip, upnpMessageType, action_P, field, valueA, valueB, extraStart_P, extraEnd_P, extraValue);    
  ethClient_stop();
}

bool SonosUPnP::upnpGetzp(IPAddress ip) // JV new - simple GET status/zp command 
{ // tft.print("Getzp:");tft.println(ip);
  if (!ethClient.connect(ip, UPNP_PORT))
  { // tft.print("Return Getzp F");tft.println(ip);
    return false;
  }
  char buffer[50];
  ethClient_write("GET /status/zp HTTP/1.1\n");
  sprintf_P(buffer, p_HeaderHost, ip[0], ip[1], ip[2], ip[3], UPNP_PORT); // 29 bytes max
  ethClient_write(buffer);
  ethClient_write("Connection: close\n");
  ethClient_write("\n");

  //tft.print("WAIT:");tft.println(ip);
  uint32_t start = millis();
  while (!ethClient.available())
  {
    if (millis() > (start + UPNP_RESPONSE_TIMEOUT_MS))
    {
      //if (ethernetErrCallback) ethernetErrCallback();
      return false;
    }
  }
  return true;
}

 

bool SonosUPnP::upnpPost(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *valueA, const char *valueB, PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue)
{ 
  if (!ethClient.connect(ip, UPNP_PORT)) 
  { if (ethernetErrCallback) ethernetErrCallback();
    return false;
  }

Serial.print("*SONOS:");
  
  // Get UPnP service name
  PGM_P upnpService = getUpnpService(upnpMessageType);

  // Get HTTP content/body length
  uint16_t contentLength =
    sizeof(SOAP_ENVELOPE_START) - 1 +
    sizeof(SOAP_BODY_START) - 1 +
    SOAP_ACTION_TAG_LEN +
    (strlen_P(action_P) * 2) +
    sizeof(UPNP_URN_SCHEMA) - 1 +
    strlen_P(upnpService) +
    sizeof(SONOS_INSTANCE_ID_0_TAG) - 1 +
    sizeof(SOAP_BODY_END) - 1 +
    sizeof(SOAP_ENVELOPE_END) - 1;

  // Get length of field
  uint8_t fieldLength = strlen(field);
  if (fieldLength)
  {
    contentLength +=
      SOAP_TAG_LEN +
      (fieldLength * 2) +
      strlen(valueA) +
      strlen(valueB);
  }

  // Get length of extra field data (e.g. meta data fields)
  if (extraStart_P)
  {
    contentLength +=
      strlen_P(extraStart_P) +
      strlen(extraValue) +
      strlen_P(extraEnd_P);
  }

  char buffer[1400];

  // Write HTTP start
  ethClient_write("POST ");
  ethClient_write_P(getUpnpEndpoint(upnpMessageType), buffer, sizeof(buffer));
  ethClient_write_P(p_HttpVersion, buffer, sizeof(buffer));

  // Write HTTP header
  sprintf_P(buffer, p_HeaderHost, ip[0], ip[1], ip[2], ip[3], UPNP_PORT); // 29 bytes max
  ethClient_write(buffer);
  ethClient_write_P(p_HeaderContentType, buffer, sizeof(buffer));
  sprintf_P(buffer, p_HeaderContentLength, contentLength); // 23 bytes max
  ethClient_write(buffer);
  ethClient_write_P(p_HeaderSoapAction, buffer, sizeof(buffer));
  ethClient_write_P(p_UpnpUrnSchema, buffer, sizeof(buffer));
  ethClient_write_P(upnpService, buffer, sizeof(buffer));
  ethClient_write("#");
  ethClient_write_P(action_P, buffer, sizeof(buffer));
  ethClient_write(HEADER_SOAP_ACTION_END);
  ethClient_write_P(p_HeaderConnection, buffer, sizeof(buffer));
  ethClient_write("\n");
  

  // Write HTTP body
  ethClient_write_P(p_SoapEnvelopeStart, buffer, sizeof(buffer));
  ethClient_write_P(p_SoapBodyStart, buffer, sizeof(buffer));
  ethClient_write(SOAP_ACTION_START_TAG_START);
  ethClient_write_P(action_P, buffer, sizeof(buffer));
  ethClient_write(SOAP_ACTION_START_TAG_NS);
  ethClient_write_P(p_UpnpUrnSchema, buffer, sizeof(buffer));
  ethClient_write_P(upnpService, buffer, sizeof(buffer));
  ethClient_write(SOAP_ACTION_START_TAG_END);
  ethClient_write_P(p_InstenceId0Tag, buffer, sizeof(buffer));
  if (fieldLength)
  { 
    sprintf(buffer, SOAP_TAG_START, field); // 18 bytes
    ethClient_write(buffer);
    ethClient_write(valueA);
    ethClient_write(valueB);
    sprintf(buffer, SOAP_TAG_END, field); // 19 bytes
    ethClient_write(buffer);
  }
  if (extraStart_P)
  {
    ethClient_write_P(extraStart_P, buffer, sizeof(buffer)); // 390 bytes
    ethClient_write(extraValue);
    ethClient_write_P(extraEnd_P, buffer, sizeof(buffer)); // 271 bytes
  }
  ethClient_write(SOAP_ACTION_END_TAG_START);
  ethClient_write_P(action_P, buffer, sizeof(buffer)); // 35 bytes
  ethClient_write(SOAP_ACTION_END_TAG_END);
  ethClient_write_P(p_SoapBodyEnd, buffer, sizeof(buffer)); // 10 bytes
  ethClient_write_P(p_SoapEnvelopeEnd, buffer, sizeof(buffer)); // 14 bytes

Serial.println("");

  uint32_t start = millis();
  while (!ethClient.available())
  { //UpdateLCD(); // keep text scrolling
    if (millis() > (start + UPNP_RESPONSE_TIMEOUT_MS))
    {
      if (ethernetErrCallback) ethernetErrCallback();
      return false;
    }
  }
  return true;
}

PGM_P SonosUPnP::getUpnpService(uint8_t upnpMessageType)
{
  switch (upnpMessageType)
  {
    case UPNP_AV_TRANSPORT: return p_UpnpAvTransportService;
    case UPNP_RENDERING_CONTROL: return p_UpnpRenderingControlService;
    case UPNP_DEVICE_PROPERTIES: return p_UpnpDevicePropertiesService;
    case UPNP_CONTENT_DIRECTORY: return p_UpnpContentDirectoryService; 
    
  }
  
}

PGM_P SonosUPnP::getUpnpEndpoint(uint8_t upnpMessageType)
{
  switch (upnpMessageType)
  {
    case UPNP_AV_TRANSPORT: return p_UpnpAvTransportEndpoint;
    case UPNP_RENDERING_CONTROL: return p_UpnpRenderingControlEndpoint;
    case UPNP_DEVICE_PROPERTIES: return p_UpnpDevicePropertiesEndpoint;
    case UPNP_CONTENT_DIRECTORY: return p_UpnpContentDirectoryEndpoint;

  }
}

void SonosUPnP::ethClient_write(const char *data)
{
//  Serial.print("*SONOS: ");
  Serial.print(data);
  ethClient.print(data);
}


//ToDo ESP8266 brings its own write_P, we better use this one
void SonosUPnP::ethClient_write_P(PGM_P data_P, char *buffer, size_t bufferSize)
{
  uint16_t dataLen = strlen_P(data_P);
  uint16_t dataPos = 0;
//Serial.print("*SONOS: ");
  while (dataLen > dataPos)
  {  
  //  *((char *)mempcpy(dst, src, n)) = '\0'; 
   //https://en.wikibooks.org/wiki/C_Programming/C_Reference/nonstandard/strlcpy
   //memcpy_P(buffer, data_P + dataPos, bufferSize); 
   strncpy_P(buffer, data_P + dataPos, bufferSize);
    //strlcpy_P(buffer, data_P + dataPos, bufferSize);
    
Serial.print(buffer);
    ethClient.print(buffer);
    dataPos += bufferSize - 1;
  }
//Serial.println("");
  
}

void SonosUPnP::ethClient_stop()
{
  if (ethClient)
  { // FB kennelijk alles wat nog in de pijplijn zit maar lezen en niets mee doen?
    while (ethClient.available()) ethClient.read();
    // en waarom dit?  
    ethClient.stop();
  }
}


#ifndef SONOS_WRITE_ONLY_MODE

void SonosUPnP::ethClient_xPath(PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize)
{
  xPath.setPath(path, pathSize);
  while (ethClient.available() && !xPath.getValue(ethClient.read(), resultBuffer, resultBufferSize));
}


uint32_t SonosUPnP::getTimeInSeconds(const char *time)
{
  uint8_t len = strlen(time);
  uint32_t seconds = 0;
  uint8_t dPower = 0;
  uint8_t tPower = 0;
  for (int8_t i = len; i > 0; i--)
  {
    char character = time[i - 1];
    if (character == ':')
    {
      dPower = 0;
      tPower++;
    }
    else if(character >= '0' && character <= '9')
    {
      seconds += (character - '0') * uiPow(10, dPower) * uiPow(60, tPower);
      dPower++;
    }
  }  
  return seconds;
}

uint32_t SonosUPnP::uiPow(uint16_t base, uint16_t exponent)
{
  int result = 1;
  while (exponent)
  {
    if (exponent & 1) result *= base;
    exponent >>= 1;
    base *= base;
  }
  return result;
}

uint8_t SonosUPnP::convertState(const char *input)
{
  if (strcmp(input, SONOS_STATE_PLAYING_VALUE) == 0) return SONOS_STATE_PLAYING;
  if (strcmp(input, SONOS_STATE_PAUSED_VALUE) == 0)  return SONOS_STATE_PAUSED;
  return SONOS_STATE_STOPPED;
}

uint8_t SonosUPnP::convertPlayMode(const char *input)
{
  if (strcmp(input, SONOS_PLAY_MODE_NORMAL_VALUE) == 0)         return SONOS_PLAY_MODE_NORMAL;
  if (strcmp(input, SONOS_PLAY_MODE_REPEAT_VALUE) == 0)         return SONOS_PLAY_MODE_REPEAT;
  if (strcmp(input, SONOS_PLAY_MODE_SHUFFLE_REPEAT_VALUE) == 0) return SONOS_PLAY_MODE_SHUFFLE_REPEAT;
  if (strcmp(input, SONOS_PLAY_MODE_SHUFFLE_VALUE) == 0)        return SONOS_PLAY_MODE_SHUFFLE;
  return SONOS_PLAY_MODE_NORMAL;
}

MediaInfo SonosUPnP::getMediaInfo(IPAddress speakerIP, char *uriBuffer, size_t uriBufferSize, char *metaBuffer, size_t metaBufferSize)
{
  MediaInfo mediaInfo;
//  Serial.println("getMediaInfo:"); 
  if (upnpPost(speakerIP, UPNP_AV_TRANSPORT, p_GetMediaInfoA, "", "", "", 0, 0, ""))
  { // response ontvangen van de sonos    

    xPath.reset();
    char infoBuffer[320] = "";
    // Track number
    PGM_P npath[] = { p_SoapEnvelope, p_SoapBody, p_GetMediaInfoR, p_NrTracks };
    ethClient_xPath(npath, 4, infoBuffer, sizeof(infoBuffer));
mediaInfo.numberTracks = atoi(infoBuffer);
//    Serial.println(trackInfo.number); 

    // Media duration
//    PGM_P dpath[] = { p_SoapEnvelope, p_SoapBody, p_GetMediaInfoR, p_MediaDuration };
//    ethClient_xPath(dpath, 4, infoBuffer, sizeof(infoBuffer));
//    trackInfo.duration = getTimeInSeconds(infoBuffer);

    // Current URI String
//    Serial.println(p_CurrentURI);
    PGM_P cpath[] = { p_SoapEnvelope, p_SoapBody, p_GetMediaInfoR, p_CurrentURI };
// uit 8jun22    ethClient_xPath(cpath, 4, infoBuffer, sizeof(infoBuffer));
    ethClient_xPath(cpath, 4, uriBuffer, sizeof(uriBufferSize));
//    Serial.println(infoBuffer); 


//    Serial.println(p_CurrentURIMetaData);
    PGM_P mpath[] = { p_SoapEnvelope, p_SoapBody, p_GetMediaInfoR, p_CurrentURIMetaData };
    ethClient_xPath(mpath, 4, metaBuffer, metaBufferSize);
//    Serial.println(metaBuffer); 

    mediaInfo.uriMeta = metaBuffer;

// de overige velden geloof ik wel

  }
  ethClient_stop();
  return mediaInfo;
}

// JV
// New function to pass full treack info including Artist, Album and Songname. Uses modified Xpath parsing
// PArsed from SERVICE : AVTRAnsport -> GetPositionInfo -> XLM response incl TrackMetaData
FullTrackInfo SonosUPnP::getFullTrackInfo(IPAddress speakerIP)
{
  FullTrackInfo trackInfo;
  if (upnpPost(speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, ""))
  {
    xPath.reset();
    char infoBuffer[20] = "";
    // Track number
    PGM_P npath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_Track };
    ethClient_xPath(npath, 4, infoBuffer, sizeof(infoBuffer));
    trackInfo.number = atoi(infoBuffer);
    // Track duration
    trackInfo.duration=DURATION_BUFFER;
    PGM_P dpath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackDuration };
    ethClient_xPath(dpath, 4, DURATION_BUFFER, sizeof(DURATION_BUFFER));
      // Track position
    trackInfo.position = POSITION_BUFFER;
    PGM_P ppath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_RelTime };    
    ethClient_xPath(ppath, 4, POSITION_BUFFER, sizeof(POSITION_BUFFER));
  }
  ethClient_stop();
  if (upnpPost(speakerIP, UPNP_AV_TRANSPORT, p_GetPositionInfoA, "", "", "", 0, 0, ""))
  {
    xPath.reset(); 
    char infoBuffer[20] = "";
    // Track title
    TITLE_BUFFER[0]=0;
    PGM_P tpath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_TrackTitle };
    ethClient_xPath2(tpath, 5, TITLE_BUFFER, sizeof(TITLE_BUFFER));
    trackInfo.title=TITLE_BUFFER;
    // Track creator
    CREATOR_BUFFER[0]=0;
    PGM_P cpath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_TrackCreator };
    ethClient_xPath2(cpath, 5, CREATOR_BUFFER, sizeof(CREATOR_BUFFER));
    trackInfo.creator=CREATOR_BUFFER;
    // Track album
    ALBUM_BUFFER[0]=0;
      PGM_P apath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_TrackAlbum };
    ethClient_xPath2(apath, 5, ALBUM_BUFFER, sizeof(ALBUM_BUFFER));
    trackInfo.album=ALBUM_BUFFER;
    // Track artist
    ARTIST_BUFFER[0]=0;
    PGM_P aapath[] = { p_SoapEnvelope, p_SoapBody, p_GetPositionInfoR, p_TrackMeta, p_TrackArtist };
    ethClient_xPath2(aapath, 5, ARTIST_BUFFER, sizeof(ARTIST_BUFFER));
    trackInfo.artist=ARTIST_BUFFER;
//    if (ARTIST_BUFFER[0]!=0) trackInfo.creator=ARTIST_BUFFER; // otherwize keep creator attribute
  }
  ethClient_stop();

  return trackInfo;
}

// JV
// Modified version of xpath call: re-work XML special charactes '&lt;' to '<' and '&gt;' to '>' for correct parsing Attributes
// 
void SonosUPnP::ethClient_xPath2(PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize) 
{
  char c,d,e,f;
  xPath.setPath(path, pathSize);
  while (ethClient.available() ) {
    c=ethClient.read(); 
    if(c=='&'){
        d=ethClient.read(); 
        e=ethClient.read(); 
        f=ethClient.read(); 
        if (d=='l' && e=='t' && f==';'){
            c='<';
#if DEBUG_XPATH              
            Serial.print(c); /*****************/
#endif            
            if ( xPath.getValue(c, resultBuffer, resultBufferSize)) break;
            }
        else if (d=='g' && e=='t' && f==';'){
            c='>';
#if DEBUG_XPATH              
            Serial.print(c); /*****************/
#endif            
            if ( xPath.getValue(c, resultBuffer, resultBufferSize)) break; 
            }
        else {
#if DEBUG_XPATH           
            Serial.print(c); /*****************/
            Serial.print(d); /*****************/
            Serial.print(e); /*****************/
            Serial.print(f); /*****************/  
#endif                      
            if ( xPath.getValue(c, resultBuffer, resultBufferSize)) break;
            if ( xPath.getValue(d, resultBuffer, resultBufferSize)) break;
            if ( xPath.getValue(e, resultBuffer, resultBufferSize)) break;
            if ( xPath.getValue(f, resultBuffer, resultBufferSize)) break;
            }
    }
    else {
#if DEBUG_XPATH       
        Serial.print(c); /*****************/
#endif        
        if ( xPath.getValue(c, resultBuffer, resultBufferSize)) break;
      }
  }
#if DEBUG_XPATH  
Serial.println(""); /*****************/
#endif
convertMetaData(resultBuffer);   // Strip XML feedback from special charaters.
}


void SonosUPnP::upnpGetString(IPAddress speakerIP, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value, PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize)
{
  if (upnpPost(speakerIP, upnpMessageType, action_P, field, value, "", 0, 0, ""))
  {
    xPath.reset();
    ethClient_xPath(path, pathSize, resultBuffer, resultBufferSize);
  }
  ethClient_stop();
}



uint8_t SonosUPnP::convertMedium(const char *input)
{
  if (strcmp(input, SONOS_MEDIUM_LINEIN_VALUE) == 0) return SONOS_MEDIUM_LINEIN;
  if (strcmp(input, SONOS_MEDIUM_NETWORK_VALUE) == 0)  return SONOS_MEDIUM_NETWORK;
  return SONOS_MEDIUM_NONE;
}




/* 
Parse String with XML meta data on specia HTML characters

Converts:
&lt; = '<'  ascii d60
&gt; = '>' ascii d62 
&apos; '\'' ascii d44 
&quot; '\"' ascii d45
&amp;  '&' ascii 38 

&amp; is only converted to '&' if followed by &amp, otherwise its discarded
*/
uint8_t SonosUPnP::convertMetaData(char *input)
{
  int rp=0,wp=0; // read pointer, write pointer
  char c,d,e,f,amp=0;

while (input[rp]!=0)
    {
    c= input[rp];rp++;
    if (c=='&')
        {
#ifdef  DEBUG_XGEN
 Serial.print("* XML Metadata parser found &:");Serial.println(&input[rp-1]);
#endif          
        c= input[rp];rp++;
        d= input[rp];rp++;
        e= input[rp];rp++;
        if (c=='l' && d == 't' && e==';') {input[wp] = '<'; wp++;}
        if (c=='g' && d == 't' && e==';') {input[wp] = '>'; wp++;}
        if (c=='a' && d == 'p' && e=='o') { rp++;rp++;input[wp] = '\''; wp++;} // read s read ;
        if (c=='q' && d == 'u' && e=='o') { rp++;rp++;input[wp] = '\"'; wp++;} // read t read ;

        if (c=='a' && d == 'm' && e=='p') { 
          if(amp==0 ) {amp=1; input[rp]='&';}      // found &amp; first time, set next input ';' info another '&', and loop again
          else {rp++; input[wp]='&'; wp++;amp=0;}    // found &amp;amp second time, print '&'
          }
        else amp=0;
        }
    else
        {
        input[wp] = c; wp++;
        }
    if (rp>100) break;  // break loop if there is too much read    
    }
input[wp]=0;  // close character string
return(rp);
}




#endif


// JV : SSDP over UDP to scan for uPnP on port 1400 : Sonos, and load then into the IP list 
uint8_t SonosUPnP::CheckUPnP(IPAddress *List,int Listsize)
{
  #define SSDPUDP_TIMEOUT 40
// #define SSDPUDP_TIMEOUT 64000000 
  // dat is voor eeuwig bijna
  int u,n,n2,t=0, pc=1;
  uint8_t match,buffercounter,found=0;
  char c;
  char udpbuffer[33]; udpbuffer[32]=0;        // mark last buffer item as a zero
  WiFiUDP SSDP_UDP;                           // A UDP instance to let us send and receive packets over UDP  
  IPAddress tmpIP; 
  IPAddress remoteIP;
  char text[32];
  
  SSDP_UDP.begin(1900);
//     SSDP_UDP.beginPacket(IPAddress(239,255,255,250), 1900);        //SSDP request at port 1900
     SSDP_UDP.beginPacket(IPAddress(255,255,255,255), 1900);        //SSDP request at port 1900
     SSDP_UDP.write((const uint8_t*)p_UPnPBroadcast, sizeof(p_UPnPBroadcast));     // fb cast
     SSDP_UDP.endPacket();
     SSDP_UDP.beginPacket(IPAddress(239,255,255,250), 1900);        //SSDP request at port 1900
     SSDP_UDP.write((const uint8_t*)p_UPnPBroadcast, sizeof(p_UPnPBroadcast));     // fb cast
     SSDP_UDP.endPacket();

#if DEBUG_XGEN
    Serial.println("* SSDP : UDP packet send 239.255.255.250:1900");
    Serial.println(p_UPnPBroadcast);
#endif    
  while(t<SSDPUDP_TIMEOUT)
    {   // wait to see if a reply is available
//    t++;delay(500);
      t++;delay(100);
    while (n=SSDP_UDP.parsePacket())
       {  remoteIP = SSDP_UDP.remoteIP();
#if DEBUG_XPATH
    Serial.print("* Reading packet(");Serial.print(pc++);Serial.print(") of ");Serial.print(n);Serial.print(" bytes from IP:");Serial.println(remoteIP);
#endif 
       char string1[]=":1400"; match=0; buffercounter=0;
       for(u=0;u<n;++u)
           {           
           SSDP_UDP.read(&c, 1); // read the packet into the buffer
#if DEBUG_XGEN
           Serial.print(c);    
#endif               
           buffercounter = (buffercounter+1)%32; // store data in looping buffer of 32
           udpbuffer[buffercounter]=c;
           if (c==string1[match]) match++;
           else match=0;
           if ( match == sizeof(string1)-1) /// ! found one :1400
              {
#if DEBUG_XGEN
              Serial.print("* uPnP:1400 found ");
#endif               
              if (found<Listsize){
                  sprintf(text, "%d.%d.%d.%d", remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]);  
                  //readback_IP(&tmpIP,udpbuffer,buffercounter,32);
                  // check if it is really a new uPnP encountered here
                  if(found>0)
                  { // Serial.print("Found another Sonos:"); Serial.print(remoteIP);
                    for (n2=0;n2<found;n2++)
                    { if(remoteIP==List[n2])
                      {  // Serial.print(" which is the same as:"); Serial.print(List[n2]);
                         break;
                      }
                    }
                    if(n2==found)
                    {  // Serial.print(" and that is a new one:"); Serial.print(remoteIP);
                      List[found] = remoteIP;
                      TFT_line_print(found+1, text);
                      found++;
                    }
                  }
                  else
                  {  Serial.print("First Sonos found:"); Serial.print(remoteIP);
                    List[found]=remoteIP;
                    TFT_line_print(found+1, text);
                    found++;
                  }
                  // Serial.print("* IPAddress:");Serial.println(remoteIP);
                 }
              else {
#if DEBUG_XGEN
              Serial.println("* too many uPnP found ");
#endif               
                 }                            
              } // if found :1400              
          } // for-loop packet data
       } // while packet data is available 
    if (t>SSDPUDP_TIMEOUT) break;             // max <UDP_TIMEOUT> times to parse packed, otherwise skip
    } 
SSDP_UDP.stop(); 
if (found > 0) {
#if DEBUG_XGEN
    Serial.print("* SSDP Found ");Serial.print(found);Serial.println(" uPnP Devices on network");
#endif     
  }
  else{
#if DEBUG_XGEN
    Serial.print("* SSDP Found no uPnP Devices on network");
#endif     
}
for(t=found;t<Listsize;++t) List[t]= IPAddress(0,0,0,0); // Fill remaining list with 0.0.0.0-IP
return found;
}

// Parse IP in looped string "***www.xxx.yyy.zzz:1400***"
void SonosUPnP::readback_IP(IPAddress *IPa,char* buf,char pointer,char bufsize)
{
int t,digit,decimal;
IPAddress tmpIP(0,0,0,0);

for(t=0;t<5;++t){ // turn back pointer 5 places - looped
  if (pointer ==0) pointer=31;
  else pointer = pointer-1;
  }
digit=0;decimal=1;
for(t=0;t<4;++t) {
   while( buf[pointer]>47 && buf[pointer]<58){  // as long as digit : count on
      digit=digit+(buf[pointer]-48)*decimal;
      decimal=decimal*10;
         if (pointer ==0) pointer=31;  // loop pointer back
         else pointer = pointer-1; 
      }
  tmpIP[3-t]= digit;  // end of digit found
  decimal=1;
  digit=0;
     if (pointer ==0) pointer=31;  // loop pointer back
     else pointer = pointer-1;   
  }
*IPa=tmpIP; // copy Ipadress into pointer to give back
}



// dit vertelt het wel goed 
//https://svrooij.io/sonos-api-docs/services/content-directory.html#browse
//G_Sonos.getSonosPlayLists(ACTIVE_sonosIP, uri, sizeof(uri), metaBuffer, sizeof(metaBuffer));

JukeBoxSong JukeBoxSongs[101];

int SonosUPnP::getSonosPlayLists(IPAddress speakerIP)
{ char SQqueue[16]="?";
  int songcount = 0;
  char c;
  int n;
  char *p;
  
//"name": "Queue",
//      "serviceName": "QueueService",
//      "discoveryUri": "/xml/Queue1.xml",
//      "serviceId": "urn:sonos-com:serviceId:Queue",
//      "serviceType": "urn:schemas-sonos-com:service:Queue:1",
//      "controlURL": "/MediaRenderer/Queue/Control",
//      "eventSubURL": "/MediaRenderer/Queue/Event",


  MediaInfo mediaInfo;

  if (upnpPost2(speakerIP, UPNP_CONTENT_DIRECTORY, SONOS_TAG_BROWSE, 
  //"ObjectID", "SQ:1", // ge-wiresharked op de PC sonos app
  "ObjectID", "SQ:", 
  "BrowseFlag", "BrowseDirectChildren",
  //"BrowseFlag", "BrowseMetadata",    // krijg ik lege velden op, nada
  "Filter", "dc:title,res,dc:creator,upnp:artist,upnp:album,upnp:albumArtURI",          // ge-wiresharked op de PC sonos app
  "StartingIndex", "0", 
  "RequestedCount", "100", 
  "SortCriteria", ""))
  { // response ontvangen van de sonos    

    Serial.println("********************************************************"); 

// zoek de Jukebox playlist in de containers die op ons afkomen. Zo'n container ziet er ongeveer zo uit:
//&lt;container id=&quot;SQ:1&quot; parentID=&quot;SQ:&quot; restricted=&quot;true&quot;&gt;
//  &lt;dc:title&gt;Jukebox&lt;/dc:title&gt;
//  &lt;res protocolInfo=&quot;file:*:audio/mpegurl:*&quot;&gt;file:///jffs/settings/savedqueues.rsq#1&lt;/res&gt;
//  &lt;upnp:class&gt;object.container.playlistContainer&lt;/upnp:class&gt;
//  &lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fA08-So%2520Incredible.wav&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//  &lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fK09%2520Jo%2520Stafford%2520-%2520No%2520Other%2520Love.mp3&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//  &lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fB05-Manke%2520Nelis%2520-%2520Zo%2520Tussen%2520De%2520Mensen.wav&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//  &lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fH02%2520Caro%2520Emerald%2520-%2520A%2520Night%2520Like%2520This.mp3&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//&lt;/container&gt;



    char searchcontainer[16];
    char nameoftheplaylist[64];
    strcpy(searchcontainer, "&lt;container");
    int searchcontainerlength;
    searchcontainerlength = strlen(searchcontainer);

    char searchtitle[32];
    strcpy(searchtitle, "&lt;dc:title&gt;");
    int searchtitlelength;
    searchtitlelength = strlen(searchtitle);

    char thequeue[16]=""; 

    char searchsavedqueues[32];
    strcpy(searchsavedqueues, "savedqueues.rsq#");
    int searchsavedqueueslength;
    searchsavedqueueslength = strlen(searchsavedqueues);

    int match=0; 
    bool containerfound = false;
    bool titlefound = false;
    bool playlistfound = false;
    bool savedqueuesfound = false;
    int containercount = 0;
    int characterscopied = 0;
    
    while (ethClient.available() ) 
    { // chunked data
      // read (uint8_t *buf, size_t size)
      // read ()
      c=ethClient.read();
      if(!containerfound)
      { if (c==searchcontainer[match]) match++;
        else match=0;
        if ( match == searchcontainerlength)
        { containercount++;
          containerfound = true;
          titlefound = false;
          match=0;
          //Serial.println("");Serial.println("containerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfound");
        }
      }
      else
      { // container gevonden
        if(!titlefound)
        { if (c==searchtitle[match]) match++;
          else match=0;
          if ( match == searchtitlelength)
          { titlefound = true;
            characterscopied = 0;
            match=0;
            //Serial.println("");Serial.println("titlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefound");
          }
        }
        else
        { // title gevonden
          if(!playlistfound)
          { if(c!='&')
            { nameoftheplaylist[characterscopied++]=c;
              nameoftheplaylist[characterscopied]=0;
            }
            else
            { playlistfound = true;
              characterscopied = 0;
              match=0;
              //Serial.println("");Serial.println("titlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefound");
            }
          }
          else
          { if(!savedqueuesfound)
            { if (c==searchsavedqueues[match]) match++;
              else match=0;
              if ( match == searchsavedqueueslength)
              { //Serial.println("");Serial.println("savedqueuesfoundsavedqueuesfoundsavedqueuesfoundsavedqueuesfoundsavedqueuesfoundsavedqueuesfound");
                savedqueuesfound = true;
                characterscopied = 0;
                match=0;
              }
            }
            else // write down the Q number
            { if((c!='&') && (characterscopied<5))
              { thequeue[characterscopied++]=c;
                thequeue[characterscopied]=0;
              }
              else
              { containerfound = false;
                titlefound = false;
                playlistfound = false;
                savedqueuesfound = false;
                if(strlen(nameoftheplaylist)==strlen("Jukebox"))
                { if(strcmp(nameoftheplaylist, "Jukebox")==0)
                  sprintf(SQqueue, "SQ:%s", thequeue); 
  
// &lt;res protocolInfo=&quot;file:*:audio/mpegurl:*&quot;&gt;file:///jffs/settings/savedqueues.rsq#0  
//                                                                                             ^^^^^^^ dat is het nummer dat je moet hebben voor achter SQ:
//                  Serial.println("FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND");
//                  Serial.print(nameoftheplaylist);Serial.print("-");Serial.println(thequeue);
//                  Serial.println("FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND");
                }
              }    
            }
          }  
        }
      }
     Serial.print(c); /*****************/
    }        
  }
  ethClient_stop();

//SQqueue[0]='?';

  if(SQqueue[0]!='?') // a playlist named jukebox was found, now get the songs
  { if (upnpPost2(speakerIP, UPNP_CONTENT_DIRECTORY, SONOS_TAG_BROWSE, 
//    "ObjectID", "SQ:1", // ge-wiresharked op de PC sonos app   - is nummer van de queue, niet perse gelijk aan sortering in sonos app
//    "ObjectID", "SQ:2", // ge-wiresharked op de PC sonos app
      "ObjectID", SQqueue, // in voorgaande slag bepaald
    "BrowseFlag", "BrowseDirectChildren",
    //"BrowseFlag", "BrowseMetadata",    // krijg ik lege velden op, nada
//    "Filter", "dc:title,res,dc:creator,upnp:artist,upnp:album,upnp:albumArtURI",          // ge-wiresharked op de PC sonos app
    "Filter", "dc:title,res,dc:creator,upnp:artist,upnp:album,upnp:albumArtURI",          // ge-wiresharked op de PC sonos app
    "StartingIndex", "0", 
    "RequestedCount", "101", 
    "SortCriteria", ""))
    { // response ontvangen van de sonos    

//"ObjectID", "SQ:1", 
//"BrowseFlag", "BrowseDirectChildren",
//"Filter", "dc:title,res,dc:creator,upnp:artist,upnp:album,upnp:albumArtURI",
//&lt;item id=&quot;S://FILEHUB/SDCard_Volume1/Share/Jukebox1/B07-Sexual%20Healing.wav&quot; parentID=&quot;SQ:1&quot; restricted=&quot;true&quot;&gt;
//&lt;res protocolInfo=&quot;x-file-cifs:*:audio/wav:*&quot;&gt;x-file-cifs://FILEHUB/SDCard_Volume1/Share/Jukebox1/B07-Sexual%20Healing.wav&lt;/res&gt;
//&lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fB07-Sexual%2520Healing.wav&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//&lt;dc:title&gt;Sexual Healing&lt;/dc:title&gt;
//&lt;upnp:class&gt;object.item.audioItem.musicTrack&lt;/upnp:class&gt;
//&lt;dc:creator&gt;Monty Alexander&lt;/dc:creator&gt;
//&lt;upnp:album&gt;My America&lt;/upnp:album&gt;
//&lt;r:episodeNumber&gt;0&lt;/r:episodeNumber&gt;
//&lt;upnp:originalTrackNumber&gt;10&lt;/upnp:originalTrackNumber&gt;
//&lt;/item&gt;

      char searchitem[16];
      strcpy(searchitem, "&lt;item");
      int searchitemlength;
      searchitemlength = strlen(searchitem);

      char searchres[16];
      strcpy(searchres, "&lt;res");
      int searchreslength;
      searchreslength = strlen(searchres);

      char searchsong[16];
      strcpy(searchsong, "&quot;&gt;");
      int searchsonglength;
      searchsonglength = strlen(searchsong);


      int match=0; 
      bool itemfound = false;
      bool resfound = false;
      bool songfound = false;
      bool ltfound = false;
      int characterscopied = 0;
      char pathofthesong[256];
      int apostest = 0; 

      while (ethClient.available() ) 
      { // chunked data
        // read (uint8_t *buf, size_t size)
        // read ()
        c=ethClient.read();
        if(!itemfound)
        { if (c==searchitem[match]) match++;
          else match=0;
          if ( match == searchitemlength)
          { itemfound = true;
            match=0;
          }
        }
        else
        { // item found
          if(!resfound)
          { if (c==searchres[match]) match++;
            else match=0;
            if ( match == searchreslength)
            { resfound = true;
              match=0;
              apostest = 0;
            }
          }
          else
          { // res gevonden
            if(!songfound)
            { if (c==searchsong[match]) match++;
              else match=0;
              if ( match == searchsonglength)
              { songfound = true;
                characterscopied = 0;
                match=0;
              }
            }
            else // write down the name
            { if(!ltfound)
              { if(c!='&' && apostest==0)
                { if(characterscopied<250)pathofthesong[characterscopied++]= c;
                  pathofthesong[characterscopied]=0;
//Serial.println(pathofthesong);                  
                }
                else
                { // might be &lt; but also &apos; 
Serial.println(apostest);                  
                  if(apostest==0) // ignore first &
                  { apostest=1;
Serial.println(apostest);                  
                  }
                  else  // check wat comes after
                  { 
Serial.println(apostest);                  
                  if(apostest==1) // we skipped the &
                    { if(c!='l') // something else than &lt;
                      { apostest=2;   // apos or something
Serial.println(apostest);                  
                      }
                      else
                      { apostest=0;  // &lt found, were done
                        ltfound = true;
                      }
                    } 
                    else 
                    {
Serial.println(apostest);                  
                      if(apostest>=2) // // apos or something
                      { if(c==';') // ignore apos until ';'
                        { if(apostest==3)
                          { apostest = 0; // continue the search for &lt;
//                            pathofthesong[characterscopied++]= '\\';
                            pathofthesong[characterscopied++]= '\'';
                            pathofthesong[characterscopied]=0;
                          } 
                          else apostest++;              // &amp;apos;
                        }
                      }  
                    }
                  }
                }
              }
              else
              { itemfound = false;
                resfound = false;
                songfound = false;
                ltfound = false;
          Serial.println("");
          Serial.println(pathofthesong);
//          Serial.println("FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND");

                if(songcount<101)
                { // path of the song
                  if((p=strchr(pathofthesong,':'))!=NULL)
                  { *p++=0; 
                    strcpy(JukeBoxSongs[songcount].path, p);
                  }
                  else strcpy(JukeBoxSongs[songcount].path, "");

                  // scheme of the song, integer
                  JukeBoxSongs[songcount].scheme = SONOS_SOURCE_UNKNOWN; 
                  for(n=0;n<SONOS_MAXSOURCE;n++)
                  { if(memcmp(p_MediaSource[n], pathofthesong, strlen(pathofthesong))==0)
                    { JukeBoxSongs[songcount].scheme = n; 
                    }                
                  }
                  Serial.println(JukeBoxSongs[songcount].path);
                  Serial.println(JukeBoxSongs[songcount].scheme);
                  songcount++;
                }
              }
            }
          }
        }
//        Serial.print(c); /*****************/
      }  
      Serial.println("songcount="); Serial.println(songcount); /*****************/
    }
  }
  ethClient_stop();

  Serial.println("FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED"); 


  if(songcount==0)return -1; // did not find a 'playlist named 'jukebox' at all, not even an empty one
  return songcount;
}


bool SonosUPnP::upnpPost2(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field1, const char *valueA, const char *field2, const char *valueB, const char *field3, const char *valueC, const char *field4, const char *valueD, const char *field5, const char *valueE, const char *field6, const char *valueF)
{
  if (!ethClient.connect(ip, UPNP_PORT)) 
 {
  Serial.println("*SONOS: Unable to connect to the speaker :(");
  return false;
  }

Serial.print("upnp-POST2*SONOS:");
  
  // Get UPnP service name
  PGM_P upnpService = getUpnpService(upnpMessageType);

  // Get HTTP content/body length
  uint16_t contentLength =
    sizeof(SOAP_ENVELOPE_START) - 1 +
    sizeof(SOAP_BODY_START) - 1 +
    SOAP_ACTION_TAG_LEN +
    (strlen_P(action_P) * 2) +
    sizeof(UPNP_URN_SCHEMA) - 1 +
    strlen_P(upnpService) +
    sizeof(SONOS_INSTANCE_ID_0_TAG) - 1 +
    sizeof(SOAP_BODY_END) - 1 +
    sizeof(SOAP_ENVELOPE_END) - 1;


  // Get length of fields
  uint8_t fieldLength1 = strlen(field1);
  if (fieldLength1)
  {
    contentLength +=
      SOAP_TAG_LEN +
      (fieldLength1 * 2) +
      strlen(valueA);
  }
  // Get length of fields
  uint8_t fieldLength2 = strlen(field2);
  if (fieldLength2)
  {
    contentLength +=
      SOAP_TAG_LEN +
      (fieldLength2 * 2) +
      strlen(valueB);
  }
  // Get length of fields
  uint8_t fieldLength3 = strlen(field3);
  if (fieldLength3)
  {
    contentLength +=
      SOAP_TAG_LEN +
      (fieldLength3 * 2) +
      strlen(valueC);
  }
  // Get length of fields
  uint8_t fieldLength4 = strlen(field4);
  if (fieldLength4)
  {
    contentLength +=
      SOAP_TAG_LEN +
      (fieldLength4 * 2) +
      strlen(valueD);
  }
  // Get length of fields
  uint8_t fieldLength5 = strlen(field5);
  if (fieldLength5)
  {
    contentLength +=
      SOAP_TAG_LEN +
      (fieldLength5 * 2) +
      strlen(valueE);
  }
  // Get length of fields
  uint8_t fieldLength6 = strlen(field6);
  if (fieldLength6)
  {
    contentLength +=
      SOAP_TAG_LEN +
      (fieldLength6 * 2) +
      strlen(valueF);
  }

  char buffer[1400];

  // Write HTTP start
  ethClient_write("POST ");
  ethClient_write_P(getUpnpEndpoint(upnpMessageType), buffer, sizeof(buffer));
  ethClient_write_P(p_HttpVersion, buffer, sizeof(buffer));

  // Write HTTP header
  sprintf_P(buffer, p_HeaderHost, ip[0], ip[1], ip[2], ip[3], UPNP_PORT); // 29 bytes max
  ethClient_write(buffer);
  ethClient_write_P(p_HeaderContentType, buffer, sizeof(buffer));
  sprintf_P(buffer, p_HeaderContentLength, contentLength); // 23 bytes max
  ethClient_write(buffer);
  ethClient_write_P(p_HeaderSoapAction, buffer, sizeof(buffer));
  ethClient_write_P(p_UpnpUrnSchema, buffer, sizeof(buffer));
  ethClient_write_P(upnpService, buffer, sizeof(buffer));
  ethClient_write("#");
  ethClient_write_P(action_P, buffer, sizeof(buffer));
  ethClient_write(HEADER_SOAP_ACTION_END);
  ethClient_write_P(p_HeaderConnection, buffer, sizeof(buffer));
  ethClient_write("\n");
  

  // Write HTTP body
  ethClient_write_P(p_SoapEnvelopeStart, buffer, sizeof(buffer));
  ethClient_write_P(p_SoapBodyStart, buffer, sizeof(buffer));
  ethClient_write(SOAP_ACTION_START_TAG_START);
  ethClient_write_P(action_P, buffer, sizeof(buffer));
  ethClient_write(SOAP_ACTION_START_TAG_NS);
  ethClient_write_P(p_UpnpUrnSchema, buffer, sizeof(buffer));
  ethClient_write_P(upnpService, buffer, sizeof(buffer));
  ethClient_write(SOAP_ACTION_START_TAG_END);
  ethClient_write_P(p_InstenceId0Tag, buffer, sizeof(buffer));
  if(fieldLength1)
  { 
    sprintf(buffer, SOAP_TAG_START, field1); // 18 bytes
    ethClient_write(buffer);
    ethClient_write(valueA);
    sprintf(buffer, SOAP_TAG_END, field1); // 19 bytes
    ethClient_write(buffer);
  }
  if(fieldLength2)
  { 
    sprintf(buffer, SOAP_TAG_START, field2); // 18 bytes
    ethClient_write(buffer);
    ethClient_write(valueB);
    sprintf(buffer, SOAP_TAG_END, field2); // 19 bytes
    ethClient_write(buffer);
  }
  if(fieldLength3)
  { 
    sprintf(buffer, SOAP_TAG_START, field3); // 18 bytes
    ethClient_write(buffer);
    ethClient_write(valueC);
    sprintf(buffer, SOAP_TAG_END, field3); // 19 bytes
    ethClient_write(buffer);
  }
  if(fieldLength4)
  { 
    sprintf(buffer, SOAP_TAG_START, field4); // 18 bytes
    ethClient_write(buffer);
    ethClient_write(valueD);
    sprintf(buffer, SOAP_TAG_END, field4); // 19 bytes
    ethClient_write(buffer);
  }
  if(fieldLength5)
  { 
    sprintf(buffer, SOAP_TAG_START, field5); // 18 bytes
    ethClient_write(buffer);
    ethClient_write(valueE);
    sprintf(buffer, SOAP_TAG_END, field5); // 19 bytes
    ethClient_write(buffer);
  }
  if(fieldLength6)
  { 
    sprintf(buffer, SOAP_TAG_START, field6); // 18 bytes
    ethClient_write(buffer);
    ethClient_write(valueF);
    sprintf(buffer, SOAP_TAG_END, field6); // 19 bytes
    ethClient_write(buffer);
  }

  ethClient_write(SOAP_ACTION_END_TAG_START);
  ethClient_write_P(action_P, buffer, sizeof(buffer)); // 35 bytes
  ethClient_write(SOAP_ACTION_END_TAG_END);
  ethClient_write_P(p_SoapBodyEnd, buffer, sizeof(buffer)); // 10 bytes
  ethClient_write_P(p_SoapEnvelopeEnd, buffer, sizeof(buffer)); // 14 bytes

Serial.println("");

  uint32_t start = millis();
  while (!ethClient.available())
  {
    if (millis() > (start + UPNP_RESPONSE_TIMEOUT_MS))
    {
      if (ethernetErrCallback) ethernetErrCallback();
      return false;
    }
  }
  return true;
}



char BigBuffer[10000];
char Chop[4000];
char Message[1024];
int SonosUPnP::getSonosPlayLists2(IPAddress speakerIP)
{ char SQqueue[16]="?";
  int songcount = 0;
  char c;
  int n;
  char *p;
  char text[16];  
//"name": "Queue",
//      "serviceName": "QueueService",
//      "discoveryUri": "/xml/Queue1.xml",
//      "serviceId": "urn:sonos-com:serviceId:Queue",
//      "serviceType": "urn:schemas-sonos-com:service:Queue:1",
//      "controlURL": "/MediaRenderer/Queue/Control",
//      "eventSubURL": "/MediaRenderer/Queue/Event",


  MediaInfo mediaInfo;

  if (upnpPost2(speakerIP, UPNP_CONTENT_DIRECTORY, SONOS_TAG_BROWSE, 
  //"ObjectID", "SQ:1", // ge-wiresharked op de PC sonos app
  "ObjectID", "SQ:", 
  "BrowseFlag", "BrowseDirectChildren",
  //"BrowseFlag", "BrowseMetadata",    // krijg ik lege velden op, nada
  "Filter", "dc:title,res,dc:creator,upnp:artist,upnp:album,upnp:albumArtURI",          // ge-wiresharked op de PC sonos app
  "StartingIndex", "0", 
  "RequestedCount", "100", 
  "SortCriteria", ""))
  { // response ontvangen van de sonos    

    Serial.println("********************************************************"); 

// zoek de Jukebox playlist in de containers die op ons afkomen. Zo'n container ziet er ongeveer zo uit:
//&lt;container id=&quot;SQ:1&quot; parentID=&quot;SQ:&quot; restricted=&quot;true&quot;&gt;
//  &lt;dc:title&gt;Jukebox&lt;/dc:title&gt;
//  &lt;res protocolInfo=&quot;file:*:audio/mpegurl:*&quot;&gt;file:///jffs/settings/savedqueues.rsq#1&lt;/res&gt;
//  &lt;upnp:class&gt;object.container.playlistContainer&lt;/upnp:class&gt;
//  &lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fA08-So%2520Incredible.wav&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//  &lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fK09%2520Jo%2520Stafford%2520-%2520No%2520Other%2520Love.mp3&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//  &lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fB05-Manke%2520Nelis%2520-%2520Zo%2520Tussen%2520De%2520Mensen.wav&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//  &lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fH02%2520Caro%2520Emerald%2520-%2520A%2520Night%2520Like%2520This.mp3&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//&lt;/container&gt;



    char searchcontainer[16];
    char nameoftheplaylist[64];
    strcpy(searchcontainer, "&lt;container");
    int searchcontainerlength;
    searchcontainerlength = strlen(searchcontainer);

    char searchtitle[32];
    strcpy(searchtitle, "&lt;dc:title&gt;");
    int searchtitlelength;
    searchtitlelength = strlen(searchtitle);

    char thequeue[16]=""; 

    char searchsavedqueues[32];
    strcpy(searchsavedqueues, "savedqueues.rsq#");
    int searchsavedqueueslength;
    searchsavedqueueslength = strlen(searchsavedqueues);

    int match=0; 
    bool containerfound = false;
    bool titlefound = false;
    bool playlistfound = false;
    bool savedqueuesfound = false;
    int containercount = 0;
    int characterscopied = 0;
    
    while (ethClient.available() ) 
    { // chunked data
      // read (uint8_t *buf, size_t size)
      // read ()
      c=ethClient.read();
      if(!containerfound)
      { if (c==searchcontainer[match]) match++;
        else match=0;
        if ( match == searchcontainerlength)
        { containercount++;
          containerfound = true;
          titlefound = false;
          match=0;
          //Serial.println("");Serial.println("containerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfoundcontainerfound");
        }
      }
      else
      { // container gevonden
        if(!titlefound)
        { if (c==searchtitle[match]) match++;
          else match=0;
          if ( match == searchtitlelength)
          { titlefound = true;
            characterscopied = 0;
            match=0;
            //Serial.println("");Serial.println("titlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefound");
          }
        }
        else
        { // title gevonden
          if(!playlistfound)
          { if(c!='&')
            { nameoftheplaylist[characterscopied++]=c;
              nameoftheplaylist[characterscopied]=0;
            }                                 
            else
            { playlistfound = true;
              characterscopied = 0;
              match=0;
              //Serial.println("");Serial.println("titlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefoundtitlefound");
            }
          }
          else
          { if(!savedqueuesfound)
            { if (c==searchsavedqueues[match]) match++;
              else match=0;
              if ( match == searchsavedqueueslength)
              { //Serial.println("");Serial.println("savedqueuesfoundsavedqueuesfoundsavedqueuesfoundsavedqueuesfoundsavedqueuesfoundsavedqueuesfound");
                savedqueuesfound = true;
                characterscopied = 0;
                match=0;
              }
            }
            else // write down the Q number
            { if((c!='&') && (characterscopied<5))
              { thequeue[characterscopied++]=c;
                thequeue[characterscopied]=0;
              }
              else
              { containerfound = false;
                titlefound = false;
                playlistfound = false;
                savedqueuesfound = false;
                if(strlen(nameoftheplaylist)==strlen("Jukebox"))
                { if(strcmp(nameoftheplaylist, "Jukebox")==0)
                  sprintf(SQqueue, "SQ:%s", thequeue); 
  
// &lt;res protocolInfo=&quot;file:*:audio/mpegurl:*&quot;&gt;file:///jffs/settings/savedqueues.rsq#0  
//                                                                                             ^^^^^^^ dat is het nummer dat je moet hebben voor achter SQ:
//                  Serial.println("FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND");
//                  Serial.print(nameoftheplaylist);Serial.print("-");Serial.println(thequeue);
//                  Serial.println("FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND-FOUND");
                }
              }    
            }
          }  
        }
      }
     Serial.print(c); /*****************/
    }        
  }
  ethClient_stop();

  char StartingIndex[16];
  int Index;
  if(SQqueue[0]!='?') // a playlist named jukebox was found, now get the songs
  { for(Index=0;Index<101;Index++)
    {  sprintf(StartingIndex,"%d", Index);

// fetch one at the time    
    if (upnpPost2(speakerIP, UPNP_CONTENT_DIRECTORY, SONOS_TAG_BROWSE, 
//    "ObjectID", "SQ:1", // ge-wiresharked op de PC sonos app   - is nummer van de queue, niet perse gelijk aan sortering in sonos app
//    "ObjectID", "SQ:2", // ge-wiresharked op de PC sonos app
      "ObjectID", SQqueue, // in voorgaande slag bepaald
    "BrowseFlag", "BrowseDirectChildren",
    //"BrowseFlag", "BrowseMetadata",    // krijg ik lege velden op, nada
//    "Filter", "dc:title,res,dc:creator,upnp:artist,upnp:album,upnp:albumArtURI",          // ge-wiresharked op de PC sonos app
    "Filter", "dc:title,res,dc:creator",          // ge-wiresharked op de PC sonos app
    "StartingIndex", StartingIndex, 
    "RequestedCount", "1", 
    "SortCriteria", ""))
    { // response ontvangen van de sonos    

//"ObjectID", "SQ:1", 
//"BrowseFlag", "BrowseDirectChildren",
//"Filter", "dc:title,res,dc:creator,upnp:artist,upnp:album,upnp:albumArtURI",
//&lt;item id=&quot;S://FILEHUB/SDCard_Volume1/Share/Jukebox1/B07-Sexual%20Healing.wav&quot; parentID=&quot;SQ:1&quot; restricted=&quot;true&quot;&gt;
//&lt;res protocolInfo=&quot;x-file-cifs:*:audio/wav:*&quot;&gt;x-file-cifs://FILEHUB/SDCard_Volume1/Share/Jukebox1/B07-Sexual%20Healing.wav&lt;/res&gt;
//&lt;upnp:albumArtURI&gt;/getaa?u=x-file-cifs%3a%2f%2fFILEHUB%2fSDCard_Volume1%2fShare%2fJukebox1%2fB07-Sexual%2520Healing.wav&amp;amp;v=26&lt;/upnp:albumArtURI&gt;
//&lt;dc:title&gt;Sexual Healing&lt;/dc:title&gt;
//&lt;upnp:class&gt;object.item.audioItem.musicTrack&lt;/upnp:class&gt;
//&lt;dc:creator&gt;Monty Alexander&lt;/dc:creator&gt;
//&lt;upnp:album&gt;My America&lt;/upnp:album&gt;
//&lt;r:episodeNumber&gt;0&lt;/r:episodeNumber&gt;
//&lt;upnp:originalTrackNumber&gt;10&lt;/upnp:originalTrackNumber&gt;
//&lt;/item&gt;

      char searchitem[16];
      strcpy(searchitem, "&lt;item");
      int searchitemlength;
      searchitemlength = strlen(searchitem);

      char searchres[16];
      strcpy(searchres, "&lt;res");
      int searchreslength;
      searchreslength = strlen(searchres);

      char searchsong[16];
      strcpy(searchsong, "&quot;&gt;");
      int searchsonglength;
      searchsonglength = strlen(searchsong);


      int match=0; 
      bool itemfound = false;
      bool resfound = false;
      bool songfound = false;
      bool ltfound = false;
      int characterscopied = 0;
      char pathofthesong[256];
      int apostest = 0; 
      char *p2;

      while (ethClient.available() ) 
      { c=ethClient.read();
        if(characterscopied<15998)BigBuffer[characterscopied++]= c;
      }
      BigBuffer[characterscopied++]= 0;
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
      while((p=strstr(BigBuffer, "&quot;"))!=NULL)
      { *p++ ='\'';
        len = strlen(BigBuffer);
        len2 = len - (BigBuffer - p - 5);
        memmove(p, p+5, len2);
      }
//      while((p=strstr(BigBuffer, "&amp;apos;"))!=NULL)
//      { *p++ ='\'';
//        len = strlen(BigBuffer);
//        len2 = len - (BigBuffer - p - 9);
//        memmove(p, p+9, len2);
//      }
        // haalt alle &amps; weg, ruimt ook &amp;amp; op  
//      while((p=strstr(BigBuffer, "&amp;"))!=NULL)
//      { *p++ ='&';
//        len = strlen(BigBuffer);
//        len2 = len - (BigBuffer - p - 4);
//        memmove(p, p+4, len2);
//      }

        // haalt alle &amps; weg, maar 1x dus &amp;amp; wordt &amp; ruimt ook &amp;amp; op
        p = BigBuffer;  
        while((p=strstr(p, "&amp;"))!=NULL)
        { len = strlen(p);
          len2 = len - 4;
           p++;
           memmove(p, p+4, len2);
        }



//      while((p=strstr(BigBuffer, "%3a"))!=NULL)
//      { *p++ =':';
//        len = strlen(BigBuffer);
//        len2 = len - (BigBuffer - p - 3);
//        memmove(p, p+3, len2);
//      }
      //chopchop <>
      Serial.println(BigBuffer);
      p = BigBuffer;
      char *p3, *p4, *p5;
      while((p=strchr(p,'<'))!=NULL)
      { if((p2=strchr(p,'>'))!=NULL)
        { // <> found
          p2++;
          memmove(Chop, p, p2-p);
          Chop[p2-p]=0;
          Serial.println(Chop); // prints the found tag <....> 

          // series of tests on the tag found
                if((p3=strstr(Chop, "<item id=\'S:"))!=NULL)
                { p4 = p3+12; // start of path
                  //Serial.println(p4);

                  if((p3=strchr(p4, '\''))!=NULL)
                  { *p3=0;
                    strcpy(JukeBoxSongs[songcount].path, p4);                
                    Serial.println(JukeBoxSongs[songcount].path); 
                  }
                }  
                else if((p3=strstr(Chop, "<dc:title>"))!=NULL)
                 { // p2 = start of title
                 
                  if((p3=strstr(p2, "</dc:title>"))!=NULL)
                  { memmove(Message, p2, p3-p2);
                    Message[p3-p2]=0;
                    // splitten in geval van J03-Willeke Alberti-De Bruid.mp3" 
                    // volledige files komen nog met een <dc:.creator> enz.
                    if(GetSlotForMusicFile(Message)>=0) // file, mp3 or wav, from own libray on NAS, with a Jukebox designator   
                    { if((p5=strrchr(Message, '.'))!=NULL)*p5=0; // remove file extension
                      if((p5=strrchr(Message, '-'))!=NULL)
                      { strcpy(JukeBoxSongs[songcount].title, (p5+1));
                        *p5=0;
                        if((p5=strrchr(Message, '-'))!=NULL)
                        strcpy(JukeBoxSongs[songcount].artist, (p5+1));
                      } 
                    }
                    else // song from a sonos playlist, can be from own NAS library or apple music
                    { // file met tags - die zijn vaak lang
                      while(strlen(Message)>29)
                      { if((p5=strrchr(Message, '('))!=NULL)*p5=0; // get rid of additional (...) text in song title
                        else
                        { if((p5=strrchr(Message, ' '))!=NULL)*p5=0; // or trim after a space
                          else if(strlen(Message)>29)
                          { Message[29]=0; // or just cut it with an axe
                          }
                        }
                      }
                    
                      strcpy(JukeBoxSongs[songcount].title, Message);
                    }                
                    Serial.println(JukeBoxSongs[songcount].title); 
                  }
                }                                  
                else if((p3=strstr(Chop, "<dc:creator>"))!=NULL)
                 { // p2 = start of this tag
                  if((p3=strstr(p2, "</dc:creator>"))!=NULL)
                  { memmove(Message, p2, p3-p2);
                    Message[p3-p2]=0;
                    // some artist titles are just too long - shortening also makes them easier to fit on the jukebox strips
                    while(strlen(Message)>31)
                    { if((p5=strrchr(Message, '/'))!=NULL)*p5=0; // get rid of additional /.... text in artist name
                      else
                      { if((p5=strrchr(Message, ' '))!=NULL)*p5=0; // or trim after a space
                        else if(strlen(Message)>31)
                        { Message[31]=0; // or just cut it with an axe
                        }
                      }
                    }
                    strcpy(JukeBoxSongs[songcount].artist, Message);                
                    Serial.println(JukeBoxSongs[songcount].artist); 
                  }
                }
                else if((p3=strstr(Chop, "<res "))!=NULL)
                { // &lt;res protocolInfo=&quot;x-file-cifs:*:audio/wav:*&quot;&gt;x-file-cifs://FILEHUB/SDCard_Volume1/Share/Jukebox1/B07-Sexual%20Healing.wav&lt;/res&gt;
                  // <res protocolInfo='sonos.com-http:*:audio/mp4:*' duration='0:03:26'>x-sonos-http:librarytrack:.DVENxPQHeA3vvX.mp4?sid=204&flags=8224&sn=5</res>
                  // p2 = start of res
                  if((p3=strstr(p2, "</res>"))!=NULL)
                  { memmove(Message, p2, p3-p2);
                    // arrived at x-file-cifs://FILEHUB/SDCard_Volume1/Share/Jukebox1/B07-Sexual%20Healing.wav
                    // arrived at x-sonos-http:librarytrack%3a.DVENxPQHeA3vvX.mp4?sid=204&flags=8224&sn=5
                      Serial.println(Message);
                      // scheme of the song, convert to integer from table search
                      JukeBoxSongs[songcount].scheme = SONOS_SOURCE_UNKNOWN; 
                      for(n=0;n<SONOS_MAXSOURCE;n++)  // see list of defined schemes in SonosUPnp.h
                      { if(memcmp(p_MediaSource[n], Message, strlen(p_MediaSource[n]))==0)
                        { JukeBoxSongs[songcount].scheme = n; 
                          Serial.println(n);
                          break; // break out of the for loop
                        }                
                      }
                      //*(p5+1)='/'; // paste back
                      // get path
                      //if((p5=strrchr(Message, ':'))!=NULL)
                      if((p5=strstr(Message, "%3a"))!=NULL)
                      
                      {  strcpy(JukeBoxSongs[songcount].path, p5);
                      Serial.println("pppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp");
                      Serial.println(Message);
                      Serial.println(p5);
                      }
                  }
                }
                else if ((p3=strstr(Chop, "<NumberReturned>"))!=NULL)
                { // p2 = start of message
                  if((p3=strstr(p2, "</NumberReturned>"))!=NULL)
                  { memmove(Message, p2, p3-p2);
                    Message[p3-p2]=0;
                    if(Message[0]=='0')
                    { // apperently there are less than 101 songs in this playlist
                      // no need to fetch more empty responses
                      songcount--; // count as one less
                      Index = 9999; 
                    }
                    Serial.println(Message);
                  }
                }                                   
                 
           }
        p=p2;
//        Serial.println(p2);
      }  
      Serial.println("-------------------------------------------------"); 
      songcount++;
      // update progess counter on LCD 
      if(songcount<=100)
      { sprintf(text, "TRACK %d", songcount); 
        TFT_line_print(1, text);
      }  

    }
    }
  }
  ethClient_stop();

  Serial.println("FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED-FINISHED"); 

  if(songcount==0)return -1; // did not find a 'playlist named 'jukebox' at all, not even an empty one
  return songcount;
}
