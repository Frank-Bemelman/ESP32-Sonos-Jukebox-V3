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


struct JukeBoxSong
{
  char artist[48];
  char title[32];
  char path[128]; 
  int scheme; // sonos scheme
}; 

int GetSlotForMusicFile(char *musicfilename);

//# define DEBUG_XGEN 1

#ifndef SonosUPnP_h
#define SonosUPnP_h

//#define SONOS_WRITE_ONLY_MODE

#include "Arduino.h"
//#include "avr/pgmspace.h"
#include "pgmspace.h"
#ifndef SONOS_WRITE_ONLY_MODE
#include "MicroXPath_P.h"
#endif
#if defined(__AVR__)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else              
#include <WiFi.h>
#endif

// HTTP:
#define HTTP_VERSION " HTTP/1.1\n"
#define HEADER_HOST "Host: %d.%d.%d.%d:%d\n"
#define HEADER_CONTENT_TYPE "Content-Type: text/xml; charset=\"utf-8\"\n"
#define HEADER_CONTENT_LENGTH "Content-Length: %d\n"
#define HEADER_SOAP_ACTION "SOAPAction: \"urn:"
#define HEADER_SOAP_ACTION_END "\"\n"
#define HEADER_CONNECTION "Connection: close\n"

// SOAP tag data:
#define SOAP_ENVELOPE_START "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
#define SOAP_ENVELOPE_END "</s:Envelope>"
#define SOAP_BODY_START "<s:Body>"
#define SOAP_BODY_END "</s:Body>"
#define SOAP_TAG_START "<%s>"
#define SOAP_TAG_END "</%s>"
#define SOAP_TAG_LEN 5
#define SOAP_TAG_ENVELOPE "s:Envelope"
#define SOAP_TAG_BODY "s:Body"

// UPnP config:
#define UPNP_PORT 1400
#define UPNP_MULTICAST_IP (byte[]) {239,255,255,250}
#define UPNP_MULTICAST_PORT 1900
#define UPNP_MULTICAST_TIMEOUT_S 2
#define UPNP_RESPONSE_TIMEOUT_MS 3000
#define UPNP_DEVICE_SCAN "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\nST: urn:schemas-upnp-org:device:ZonePlayer:1\r\n\0"
//#define UPNP_DEVICE_SCAN "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: 2\r\nST: ssdp:all\r\n\0"


// UPnP tag data:
#define SOAP_ACTION_START_TAG_START "<u:"
#define SOAP_ACTION_START_TAG_NS " xmlns:u=\"urn:"
#define SOAP_ACTION_START_TAG_END "\">"
#define SOAP_ACTION_END_TAG_START "</u:"
#define SOAP_ACTION_END_TAG_END ">"
#define SOAP_ACTION_TAG_LEN 24

// UPnP service data:
#define UPNP_URN_SCHEMA "schemas-upnp-org:service:"
#define UPNP_AV_TRANSPORT 1
#define UPNP_AV_TRANSPORT_SERVICE "AVTransport:1"
#define UPNP_AV_TRANSPORT_ENDPOINT "/MediaRenderer/AVTransport/Control"
#define UPNP_RENDERING_CONTROL 2
#define UPNP_RENDERING_CONTROL_SERVICE "RenderingControl:1"
#define UPNP_RENDERING_CONTROL_ENDPOINT "/MediaRenderer/RenderingControl/Control"
#define UPNP_DEVICE_PROPERTIES 3
#define UPNP_DEVICE_PROPERTIES_SERVICE "DeviceProperties:1"
#define UPNP_DEVICE_PROPERTIES_ENDPOINT "/DeviceProperties/Control"

// UPnP service data: added by frank
#define UPNP_CONTENT_DIRECTORY 4
#define UPNP_CONTENT_DIRECTORY_SERVICE "ContentDirectory:1"
#define UPNP_CONTENT_DIRECTORY_ENDPOINT "/MediaServer/ContentDirectory/Control"
#define SONOS_TAG_BROWSE "Browse"
#define SONOS_TAG_BROWSE_RESPONSE "u:BrowseResponse"
#define SONOS_TAG_BROWSE_FLAG "BrowseFlag"


#define SONOS_TAG_GET_ZONE_ATTR "GetZoneAttributes"
#define SONOS_TAG_GET_ZONE_ATTR_RESPONSE "u:GetZoneAttributesResponse"
#define SONOS_TAG_ZONENAME "CurrentZoneName"
#define SONOS_TAG_GET_ZONE_INFO "GetZoneInfo"
#define SONOS_TAG_GET_ZONE_INFO_RESPONSE "u:GetZoneInfoResponse"
#define SONOS_TAG_SERIAL "ExtraInfo"

#define SONOS_GET_ZPSUPPORTINFO "ZPSupportInfo"
#define SONOS_GET_ZPINFO "ZPInfo"
#define SONOS_GET_ZPZONE "ZoneName"
#define SONOS_GET_ZPLOCALUID "LocalUID"
#define SONOS_GET_ZPSERIAL "SerialNumber"
#define SONOS_GET_ZPSERIESID "SeriesID"


// Sonos speaker state control:
/*
<u:Play>
  <InstanceID>0</InstanceID>
  <Speed>1</Speed>
</u:Play>
<u:Seek>
  <InstanceID>0</InstanceID>
  <Unit>REL_TIME</Unit>
  <Target>0:01:02</Target>
</u:Seek>
*/
#define SONOS_TAG_PLAY "Play"
#define SONOS_SOURCE_RINCON_TEMPLATE "RINCON_%s0%d%s"
#define SONOS_TAG_SPEED "Speed"
#define SONOS_TAG_STOP "Stop"
#define SONOS_TAG_PAUSE "Pause"
#define SONOS_TAG_PREVIOUS "Previous"
#define SONOS_TAG_NEXT "Next"
#define SONOS_DIRECTION_BACKWARD 0
#define SONOS_DIRECTION_FORWARD 1
#define SONOS_INSTANCE_ID_0_TAG "<InstanceID>0</InstanceID>"

#define SONOS_TAG_SEEK "Seek"
#define SONOS_TAG_TARGET "Target"
#define SONOS_SEEK_MODE_TAG_START "<Unit>"
#define SONOS_SEEK_MODE_TAG_END "</Unit>"
#define SONOS_SEEK_MODE_TRACK_NR "TRACK_NR"
#define SONOS_SEEK_MODE_REL_TIME "REL_TIME"
#define SONOS_TIME_FORMAT_TEMPLATE "%d:%02d:%02d"

#define SONOS_TAG_SET_AV_TRANSPORT_URI "SetAVTransportURI"
#define SONOS_TAG_CURRENT_URI "CurrentURI"
#define SONOS_URI_META_LIGHT_START "<CurrentURIMetaData>"
#define SONOS_URI_META_LIGHT_END "</CurrentURIMetaData>"
#define SONOS_RADIO_META_FULL_START "<CurrentURIMetaData>&lt;DIDL-Lite xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot; xmlns:r=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot; xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot;&gt;&lt;item id=&quot;R:0/0/46&quot; parentID=&quot;R:0/0&quot; restricted=&quot;true&quot;&gt;&lt;dc:title&gt;"
#define SONOS_RADIO_META_FULL_END "&lt;/dc:title&gt;&lt;upnp:class&gt;object.item.audioItem.audioBroadcast&lt;/upnp:class&gt;&lt;desc id=&quot;cdudn&quot; nameSpace=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot;&gt;SA_RINCON65031_&lt;/desc&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;</CurrentURIMetaData>"

#define SONOS_TAG_BECOME_COORDINATOR_OF_STANDALONE_GROUP "BecomeCoordinatorOfStandaloneGroup"

#define SONOS_TAG_SET_LED_STATE "SetLEDState"
#define SONOS_TAG_DESIRED_LED_STATE "DesiredLEDState"

// Playlist & Queue
/*
<u:AddURIToQueueResponse xmlns:u="urn:schemas-upnp-org:service:AVTransport:1">
  <FirstTrackNumberEnqueued>21</FirstTrackNumberEnqueued>
  <NumTracksAdded>36</NumTracksAdded>
  <NewQueueLength>56</NewQueueLength>
</u:AddURIToQueueResponse>
*/
#define SONOS_TAG_ADD_URI_TO_QUEUE "AddURIToQueue"
#define SONOS_TAG_ENQUEUED_URI "EnqueuedURI"
#define SONOS_SAVED_QUEUES "file:///jffs/settings/savedqueues.rsq#%d"
#define SONOS_TAG_REMOVE_ALL_TRACKS_FROM_QUEUE "RemoveAllTracksFromQueue"
#define SONOS_PLAYLIST_META_LIGHT_START "<EnqueuedURIMetaData></EnqueuedURIMetaData><DesiredFirstTrackNumberEnqueued>"
#define SONOS_PLAYLIST_META_LIGHT_END "0</DesiredFirstTrackNumberEnqueued><EnqueueAsNext>1</EnqueueAsNext>"
//#define SONOS_PLAYLIST_META_FULL_START "<EnqueuedURIMetaData>&lt;DIDL-Lite xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot; xmlns:r=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot; xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot;&gt;&lt;item id=&quot;SQ:0&quot; parentID=&quot;SQ:&quot; restricted=&quot;true&quot;&gt;&lt;dc:title&gt;"
//#define SONOS_PLAYLIST_META_FULL_END "&lt;/dc:title&gt;&lt;upnp:class&gt;object.container.playlistContainer&lt;/upnp:class&gt;&lt;desc id=&quot;cdudn&quot; nameSpace=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot;&gt;RINCON_AssociatedZPUDN&lt;/desc&gt;&lt;/item&gt;&lt;/DIDL-Lite&gt;</EnqueuedURIMetaData><DesiredFirstTrackNumberEnqueued>0</DesiredFirstTrackNumberEnqueued><EnqueueAsNext>1</EnqueueAsNext>"

// Track & source:
/*
<u:GetPositionInfoResponse>
  <Track>1</Track>
  <TrackDuration>0:03:21</TrackDuration>
  <TrackMetaData>[Meta data in DIDL-Lite]</TrackMetaData>
  <TrackURI></TrackURI>
  <RelTime>0:01:23</RelTime>
  <AbsTime>NOT_IMPLEMENTED</AbsTime>
  <RelCount>2147483647</RelCount>
  <AbsCount>2147483647</AbsCount>
</u:GetPositionInfoResponse>
*/
#define SONOS_TAG_GET_POSITION_INFO "GetPositionInfo"
#define SONOS_TAG_GET_POSITION_INFO_RESPONSE "u:GetPositionInfoResponse"
#define SONOS_TAG_TRACK "Track"
#define SONOS_TAG_NRTRACKS "NrTracks"
#define SONOS_TAG_TRACK_DURATION "TrackDuration"
#define SONOS_TAG_MEDIA_DURATION "MediaDuration"
#define SONOS_TAG_TRACK_URI "TrackURI"
#define SONOS_TAG_CURRENT_URI "CurrentURI"
#define SONOS_TAG_REL_TIME "RelTime"
#define SONOS_TAG_TRACK_METADATA "TrackMetaData"

#define SONOS_TAG_TRACKMETA "TrackMetaData" 


#define SONOS_ATTRIB_TITLE "dc:title"
#define SONOS_ATTRIB_CREATOR "dc:creator"
#define SONOS_ATTRIB_ALBUM "upnp:album"
#define SONOS_ATTRIB_ARTIST "r:albumArtist"


#define SONOS_MAXSOURCE 14
                                  
#define SONOS_SOURCE_UNKNOWN_SCHEME "z-sonos-unknown:\0"
#define SONOS_SOURCE_SPOTIFY_SCHEME "x-sonos-spotify:\0"
#define SONOS_SOURCE_FILE_SCHEME "x-file-cifs:\0"
#define SONOS_SOURCE_LIBRARY_SCHEME "x-sonos-http:librarytrack\0"  // apple afspeellijsten
#define SONOS_SOURCE_HTTP_SCHEME "x-sonos-http:\0"
#define SONOS_SOURCE_RADIO_SCHEME "x-rincon-mp3radio:\0"
#define SONOS_SOURCE_RADIO_AAC_SCHEME "aac:\0"
#define SONOS_SOURCE_LINEIN_SCHEME "x-rincon-stream:\0"
#define SONOS_SOURCE_MASTER_SCHEME "x-rincon:\0"
#define SONOS_SOURCE_QUEUE_SCHEME "x-rincon-queue:\0"
#define SONOS_SOURCE_SPOTIFYSTATION_SCHEME "x-sonosprog-spotify:\0"
#define SONOS_SOURCE_LOCALHTTP_SCHEME "http:\0"
#define SONOS_SOURCE_LOCALHTTPS_SCHEME "https:\0"
#define SONOS_SOURCE_SPOTIFY_RADIO_SCHEME "x-sonos-vli:\0"  // apple afspeellijsten


#define SONOS_SOURCE_UNKNOWN 0
#define SONOS_SOURCE_SPOTIFY 1
#define SONOS_SOURCE_FILE 2
#define SONOS_SOURCE_LIBRARY 3
#define SONOS_SOURCE_HTTP 4
#define SONOS_SOURCE_RADIO 5
#define SONOS_SOURCE_RADIO_AAC 6
#define SONOS_SOURCE_LINEIN 7
#define SONOS_SOURCE_MASTER 8
#define SONOS_SOURCE_QUEUE 9
#define SONOS_SOURCE_SPOTIFYSTATION 10
#define SONOS_SOURCE_LOCALHTTP 11
#define SONOS_SOURCE_LOCALHTTPS 12
#define SONOS_SOURCE_SPOTIFY_RADIO 13


#define UNKNOWN_SCHEME "Unknown"
#define SPOTIFY_SCHEME "Spotify"
#define FILE_SCHEME "File"
#define LIBRARY_SCHEME "Libray"
#define HTTP_SCHEME "Http"      // sonos thema radio     , apple thema radio
#define RADIO_SCHEME "Radio"
#define RADIO_AAC_SCHEME "Radio AAC"
#define LINEIN_SCHEME "Line-In"
#define MASTER_SCHEME "Master"
#define QUEUE_SCHEME "Queue"
#define SPOTIFYSTATION_SCHEME "Spotify Station"
#define LOCALHTTP_SCHEME "Local Http"
#define LOCALHTTPS_SCHEME "Local Https"
#define SPOTIFY_RADIO_SCHEME "Spotify Radio"  



#define SONOS_TAG_GET_MEDIA_INFO "GetMediaInfo"
#define SONOS_TAG_GET_MEDIA_INFO_RESPONSE "u:GetMediaInfoResponse"
#define SONOS_TAG_CURRENTURIMETADATA "CurrentURIMetaData"


// Volume, bass & treble:
/*
<u:GetVolume>
  <InstanceID>0</InstanceID>
  <Channel>Master</Channel>
</u:GetVolume>
<u:GetVolumeResponse>
  <CurrentVolume>[0-100]</CurrentVolume>
</u:GetVolumeResponse>
*/
#define SONOS_TAG_CHANNEL "Channel"
#define SONOS_CHANNEL_MASTER "Master"
//#define SONOS_CHANNEL_LEFT "LF"
//#define SONOS_CHANNEL_RIGHT "RF"
#define SONOS_TAG_GET_MUTE "GetMute"
#define SONOS_TAG_GET_MUTE_RESPONSE "u:GetMuteResponse"
#define SONOS_TAG_CURRENT_MUTE "CurrentMute"
#define SONOS_TAG_GET_VOLUME "GetVolume"
#define SONOS_TAG_GET_VOLUME_RESPONSE "u:GetVolumeResponse"
#define SONOS_TAG_CURRENT_VOLUME "CurrentVolume"
#define SONOS_TAG_GET_OUTPUT_FIXED "GetOutputFixed"
#define SONOS_TAG_GET_FIXED_RESPONSE "u:GetOutputFixedResponse"
#define SONOS_TAG_CURRENT_FIXED "CurrentFixed"
#define SONOS_TAG_GET_BASS "GetBass"
#define SONOS_TAG_GET_BASS_RESPONSE "u:GetBassResponse"
#define SONOS_TAG_CURRENT_BASS "CurrentBass"
#define SONOS_TAG_GET_TREBLE "GetTreble"
#define SONOS_TAG_GET_TREBLE_RESPONSE "u:GetTrebleResponse"
#define SONOS_TAG_CURRENT_TREBLE "CurrentTreble"
#define SONOS_TAG_GET_LOUDNESS "GetLoudness"
#define SONOS_TAG_GET_LOUDNESS_RESPONSE "u:GetLoudnessResponse"
#define SONOS_TAG_CURRENT_LOUDNESS "CurrentLoudness"

// Set volume, bass & treble:
/*
<u:SetVolume>
  <InstanceID>0</InstanceID>
  <Channel>Master</Channel>
  <DesiredVolume>[0-100]</DesiredVolume>
</u:SetVolume>
*/
#define SONOS_TAG_SET_MUTE "SetMute"
#define SONOS_TAG_DESIRED_MUTE "DesiredMute"
#define SONOS_TAG_SET_VOLUME "SetVolume"
#define SONOS_TAG_DESIRED_VOLUME "DesiredVolume"
#define SONOS_TAG_SET_BASS "SetBass"
#define SONOS_TAG_DESIRED_BASS "DesiredBass"
#define SONOS_TAG_SET_TREBLE "SetTreble"
#define SONOS_TAG_DESIRED_TREBLE "DesiredTreble"
#define SONOS_TAG_SET_LOUDNESS "SetLoudness"
#define SONOS_TAG_DESIRED_LOUDNESS "DesiredLoudness"
#define SONOS_CHANNEL_TAG_START "<Channel>"
#define SONOS_CHANNEL_TAG_END "</Channel>"

// Play Mode:
/*
<u:GetTransportSettingsResponse>
  <PlayMode>[NORMAL/REPEAT_ALL/SHUFFLE/SHUFFLE_NOREPEAT]</PlayMode>
  <RecQualityMode>NOT_IMPLEMENTED</RecQualityMode>
</u:GetTransportSettingsResponse>
*/
#define SONOS_TAG_GET_TRANSPORT_SETTINGS "GetTransportSettings"
#define SONOS_TAG_GET_TRANSPORT_SETTINGS_RESPONSE "u:GetTransportSettingsResponse"
#define SONOS_TAG_PLAY_MODE "PlayMode"
#define SONOS_PLAY_MODE_NORMAL B00
#define SONOS_PLAY_MODE_NORMAL_VALUE "NORMAL"
#define SONOS_PLAY_MODE_REPEAT B01
#define SONOS_PLAY_MODE_REPEAT_VALUE "REPEAT_ALL"
#define SONOS_PLAY_MODE_SHUFFLE B10
#define SONOS_PLAY_MODE_SHUFFLE_VALUE "SHUFFLE_NOREPEAT"
#define SONOS_PLAY_MODE_SHUFFLE_REPEAT B11
#define SONOS_PLAY_MODE_SHUFFLE_REPEAT_VALUE "SHUFFLE"
// Set Play Mode:
#define SONOS_TAG_SET_PLAY_MODE "SetPlayMode"
#define SONOS_TAG_NEW_PLAY_MODE "NewPlayMode"

// State:
/*
<u:GetTransportInfoResponse>
  <CurrentTransportState>[PLAYING/PAUSED_PLAYBACK/STOPPED]</CurrentTransportState>
  <CurrentTransportStatus>[OK/ERROR]</CurrentTransportStatus>
  <CurrentSpeed>1</CurrentSpeed>
</u:GetTransportInfoResponse>
*/

#define SONOS_TAG_GET_MEDIA_INFO "GetMediaInfo"
#define SONOS_TAG_GET_MEDIA_INFO_RESPONSE "u:GetMediaInfoResponse"
#define SONOS_TAG_MEDIUM_STATUS "PlayMedium"
#define SONOS_MEDIUM_NONE 1
#define SONOS_MEDIUM_NONE_VALUE "NONE"
#define SONOS_MEDIUM_LINEIN 2
#define SONOS_MEDIUM_LINEIN_VALUE "LINE-IN"
#define SONOS_MEDIUM_NETWORK 3
#define SONOS_MEDIUM_NETWORK_VALUE "NETWORK"
#define SONOS_TAG_ARTIST_STATUS "CurrentURIMetaData"

#define SONOS_TAG_GET_TRANSPORT_INFO "GetTransportInfo"
#define SONOS_TAG_GET_TRANSPORT_INFO_RESPONSE "u:GetTransportInfoResponse"
#define SONOS_TAG_CURRENT_TRANSPORT_STATE "CurrentTransportState"
#define SONOS_STATE_PLAYING 1
#define SONOS_STATE_PLAYING_VALUE "PLAYING"
#define SONOS_STATE_PAUSED 2
#define SONOS_STATE_PAUSED_VALUE "PAUSED_PLAYBACK"
#define SONOS_STATE_STOPPED 3
#define SONOS_STATE_STOPPED_VALUE "STOPPED"

struct TrackInfo
{
  uint16_t number;
  uint32_t duration;
  uint32_t position;
  char *uri;
  char *trackmetadata; // hier komt de artiest en track titel terecht - frank - 21mei2020
};

struct FullTrackInfo // JV new, pass text-info as char string
{
  uint16_t number;
  char *duration;
  char *position;
  char *creator;
  char *title;
  char *album;
  char *artist;
};


struct SonosInfo // JV new, pass text info as Char string
{
  uint16_t number;
  char *uid;        // Rincon-xxxx 32 bytes
  char *serial;      // 16 bytes serialnumber short - no '-'
  char *seriesid;   // Series ID or Sonos Type - 16bytes
  char *zone;        // Zone name - 32 bytes
  char *medium;      // medium - network, linein etc
  char *status;      // Status - play/stop/pause etc
  char *playmode;    // playmode, see SONOS_PLAY_MODE definitions
  char *source;      // source, defined in URI , see  SONOS_SOURCE definitions
  };



struct MediaInfo
{
  uint16_t numberTracks;
  char *uri;
  char *uriMeta;
};

// https://developer.sonos.com/reference/sonos-music-api/getmediametadata/
// http://blog.travelmarx.com/2010/06/exploring-sonos-via-upnp.html
#define SONOS_TAG_GET_MEDIAMETADATA "GetMediaMetadata"        // frank 24JUL20
#define SONOS_TAG_GET_MEDIAMETADATARESPONSE "GetMediaMetadataResponse"  // frank 24JUL20
#define SONOS_TAG_GET_MEDIAMETADATARESULT "GetMediaMetadataResult"  // frank 24JUL20
#define SONOS_TAG_ID "Id"  // frank 24JUL20
#define SONOS_TAG_TITLE "Title"  // frank 24JUL20

class SonosUPnP
{

  public:

    SonosUPnP(WiFiClient client, void (*ethernetErrCallback)(void));

    void setAVTransportURI(IPAddress speakerIP, const char *scheme, const char *address);
    void seekTrack(IPAddress speakerIP, uint16_t index);
    void seekTime(IPAddress speakerIP, uint8_t hour, uint8_t minute, uint8_t second);
    void setPlayMode(IPAddress speakerIP, uint8_t playMode);
    void play(IPAddress speakerIP);
    void playFile(IPAddress speakerIP, const char *path);
    void playHttp(IPAddress speakerIP, const char *address);
    void playRadio(IPAddress speakerIP, const char *address, const char *title);
    void playLineIn(IPAddress speakerIP, const char *speakerID);
    void playQueue(IPAddress speakerIP, const char *speakerID);
    void playConnectToMaster(IPAddress speakerIP, const char *masterSpeakerID);
    void disconnectFromMaster(IPAddress speakerIP);
    void stop(IPAddress speakerIP);
    void pause(IPAddress speakerIP);
    void skip(IPAddress speakerIP, uint8_t direction);
    void setMute(IPAddress speakerIP, bool state);
    void setVolume(IPAddress speakerIP, uint8_t volume);
    void setVolume(IPAddress speakerIP, uint8_t volume, const char *channel);
    void setBass(IPAddress speakerIP, int8_t bass);
    void setTreble(IPAddress speakerIP, int8_t treble);
    void setLoudness(IPAddress speakerIP, bool state);
    void setStatusLight(IPAddress speakerIP, bool state);
    void addPlaylistToQueue(IPAddress speakerIP, uint16_t playlistIndex);
    void addTrackToQueue(IPAddress speakerIP, const char *scheme, const char *address);
    void removeAllTracksFromQueue(IPAddress speakerIP);
    uint8_t CheckUPnP(IPAddress *List,int Listsize); // new JV uPnP IPlist check by SSDP over UDP

    
    #ifndef SONOS_WRITE_ONLY_MODE
    
    void setRepeat(IPAddress speakerIP, bool repeat);
    void setShuffle(IPAddress speakerIP, bool shuffle);
    void toggleRepeat(IPAddress speakerIP);
    void toggleShuffle(IPAddress speakerIP);
    void togglePause(IPAddress speakerIP);
    void toggleMute(IPAddress speakerIP);
    void toggleLoudness(IPAddress speakerIP);
    uint8_t getState(IPAddress speakerIP,char *buf); // new JV : string passthrough
    uint8_t getState(IPAddress speakerIP);
    uint8_t getMedium(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    uint8_t getMedium(IPAddress speakerIP); // new JV 
    uint8_t getPlayMode(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    bool getZone(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    bool getSerial(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    uint8_t getSource(IPAddress speakerIP,char *buf); // new JV  : string passthrough
    uint8_t getSource(IPAddress speakerIP);

    uint8_t getPlayMode(IPAddress speakerIP);
    bool getRepeat(IPAddress speakerIP);
    bool getShuffle(IPAddress speakerIP);
//    TrackInfo getTrackInfo(IPAddress speakerIP, char *uriBuffer, size_t uriBufferSize);
    TrackInfo getTrackInfo(IPAddress speakerIP, char *uriBuffer, size_t uriBufferSize, char *metaBuffer, size_t metaBufferSize);

// FB 7okt2020
    MediaInfo getMediaInfo(IPAddress speakerIP, char *uriBuffer, size_t uriBufferSize, char *metaBuffer, size_t metaBufferSize);

    uint16_t getTrackNumber(IPAddress speakerIP);
    void getTrackURI(IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize);
    void getTrackCreator(IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize); // new JV - parse XML Metadata attribute Creator
    void getTrackTitle(IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize); // new JV - parse XML Metadata attribute Title
    void getTrackAlbum(IPAddress speakerIP, char *resultBuffer, size_t resultBufferSize); // new JV - parse XML Metadata attribute Album
    uint8_t getSourceFromURI(const char *uri);
    uint32_t getTrackDurationInSeconds(IPAddress speakerIP);
    uint32_t getTrackPositionInSeconds(IPAddress speakerIP);
    uint16_t getTrackPositionPerMille(IPAddress speakerIP);
    bool getMute(IPAddress speakerIP);
    uint8_t getVolume(IPAddress speakerIP);
    uint8_t getVolume(IPAddress speakerIP, const char *channel);
    bool getOutputFixed(IPAddress speakerIP);
    int8_t getBass(IPAddress speakerIP);
    int8_t getTreble(IPAddress speakerIP);
    bool getLoudness(IPAddress speakerIP);
    
    FullTrackInfo getFullTrackInfo(IPAddress speakerIP);       // 18 mei nu starten alles ombouwen naar handiger data ophalen
    SonosInfo getSonosInfo(IPAddress speakerIP); // new JV - parse Get status/zp XLM

// frank 30-jun-22
    int getSonosPlayLists(IPAddress speakerIP);
    int getSonosPlayLists2(IPAddress speakerIP);
    
    #endif

  private:

    WiFiClient ethClient;      

    void (*ethernetErrCallback)(void);
    void seek(IPAddress speakerIP, const char *mode, const char *data);
    void setAVTransportURI(IPAddress speakerIP, const char *scheme, const char *address, PGM_P metaStart_P, PGM_P metaEnd_P, const char *metaValue);
    void upnpSet(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P);
    void upnpSet(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value);
    void upnpSet(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *valueA, const char *valueB, PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue);
    bool upnpPost(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *valueA, const char *valueB, PGM_P extraStart_P, PGM_P extraEnd_P, const char *extraValue);

// frank
    bool upnpPost2(IPAddress ip, uint8_t upnpMessageType, PGM_P action_P, const char *field1, const char *valueA, const char *field2, const char *valueB, const char *field3, const char *valueC, const char *field4, const char *valueD, const char *field5, const char *valueE, const char *field6, const char *valueF);

    const char *getUpnpService(uint8_t upnpMessageType);
    const char *getUpnpEndpoint(uint8_t upnpMessageType);
    void ethClient_write(const char *data);
    void ethClient_write_P(PGM_P data_P, char *buffer, size_t bufferSize);
    void ethClient_stop();
    void readback_IP(IPAddress *IPa,char* buf,char pointer,char bufsize); // New JV : readback IP from (UDP)buffer
    bool upnpGetzp(IPAddress ip); // New JV GET command status/zp  


    #ifndef SONOS_WRITE_ONLY_MODE

    MicroXPath_P xPath;
    void ethClient_xPath(PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize);
    void ethClient_xPath2(PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize); // modified version to search/parse in Metadata
    void upnpGetString(IPAddress speakerIP, uint8_t upnpMessageType, PGM_P action_P, const char *field, const char *value, PGM_P *path, uint8_t pathSize, char *resultBuffer, size_t resultBufferSize);
    uint32_t getTimeInSeconds(const char *time);
    uint32_t uiPow(uint16_t base, uint16_t exp);
    uint8_t convertMedium(const char *input);
    uint8_t convertState(const char *input);
    uint8_t convertPlayMode(const char *input);
    uint8_t convertMetaData(char *input ); 


    #endif
};

#endif

//#ifdef ESP8266  //has no strlcpy_P

/* size_t ICACHE_FLASH_ATTR strlcpy_P(char* dest, const char* src, size_t size) {
    const char* read = src;
    char* write = dest;

*/
