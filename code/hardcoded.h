
// list of hard coded radio stations with url and friendly name for quick initial display purposes
// the logos of the radio stations are stored on the SD card, in \art114\ and have names formatted as radio-114-xx.jpg
// these are all small 114x144 pixel jpg files, stored in baseline format. Don't use progressive jpg because the 
// jpg loader can't decode that spohisticated format
// there is a fallback image for missing artwork, stored in radio11400.h which can be used if no image has been
// prepared or if you don't have an SD card installed

const char *RadioStations[20][2]={
{ "//playerservices.streamtheworld.com/api/livestream-redirect/SRGSTR03.mp3", "Sky LoveSongs" },  // A (1)
{ "//open.live.bbc.co.uk/mediaselector/5/select/version/2.0/mediaset/http-icy-mp3-a/vpid/bbc_radio_one/format/pls.pls", "BBC1" },  // B (2)
{ "//stream.bollenstreekomroep.nl/live-mp3-192-stereo", "BOLLENSTREEK" },  // C (3)
{ "//icecast.vrtcdn.be/radio1-high.mp3", "VRT 1" },  // D (4)
{ "//crtve--di--crtve-ice--01--cdn.cast.addradio.de/crtve/rne1/main/mp3/high", "Radio Nacion" },  // E (5)
{ "//icecast.radiofrance.fr/francemusiquelajazz-midfi.mp3", "Jazz Frans" },  // F (6)
{ "//allsoundradio.torontocast.stream/allsound.pls", "Allsound Radio" },  // G (7)
{ "//icecast.vrtcdn.be/radio1_classics_high.mp3", "VRT 1 Classic" },  // H (8)
{ "//playerservices.streamtheworld.com/api/livestream-redirect/TLPSTR15.mp3", "RADIO10 Non-Stop" },  // J (9)
{ "//playerservices.streamtheworld.com/api/livestream-redirect/TLPSTR19.mp3", "538 Ibiza" },  // K (10)

{ "//icecast.omroep.nl/radio1-bb-mp3", "NPO1" },  // 1 (11)
{ "//icecast.omroep.nl/radio2-bb-mp3", "NPO2" },  // 2 (12)
{ "//icecast.omroep.nl/3fm-bb-mp3", "3FM" },  // 3 (13)
{ "//icecast.omroep.nl/radio4-bb-mp3", "NPO4" },  // 4 (14)
{ "//icecast.omroep.nl/radio5-bb-mp3", "NPO5" },  // 5 (15) 
{ "//19993.live.streamtheworld.com/SKYRADIO.mp3", "SKY 101 FM" },  // 6 (16)
{ "//stream.100p.nl/100pctnl.mp3", "100%NL" },  // 7 (17)
{ "//playerservices.streamtheworld.com/api/livestream-redirect/RADIO538.mp3", "Radio 538" },  // 8 (18)
{ "//stream.radio10.nl/radio10", "RADIO 10" }, // 9 (19)
{ "//stream.sublimefm.nl/SublimeFM_mp3", "Sublime FM" }  // 0 (20)
};

// we try to get a list of songs by trying to find a 'Jukebox' (case sensitive) playlist on the Sonos.
// first create a 'Jukebox' playlist on the Sonos, using local music files from a samba share, such as a NAS or shared folder on windows PC.
// reboot ESP32 and load the playlist in ESP32

// If you want a last resort fallback (in case samba share disconnected, no internet connection) then copy all music files to the SD card
// rename Sonos playlist to something other than 'Jukebox' and reboot ESP32 - that will force the use of the SD card
// directory will be read, songs sorted and put under the right keys of the jukebox

// recommended filename format for SD to provide some info about artist and name of song
// also add the keys under which the song will be stored
// sonos will try to figure it out anyway, but is not always reliable
// in such case we may use the filename to display something sensible

/* your SD directory of 100 songs may look something like these examples
A02-J.J.Cale-City Girls.wav
A09-Caro Emerald-Stuck.wav
B03-Billy Ocean-Get Outta My Dreams.wav
G10-Ray Charles-Fever.wav
H01-Madonna-La Isla Bonita.wav
H10-Melody Gardot-Your Heart Is As Black As Night.mp3
K10-Luis Fonsi-Despacito.mp3
*/
// end of file
