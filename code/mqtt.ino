#include <ArduinoHA.h>


unsigned long startMillis;  
unsigned long currentMillis; 







#define BROKER_ADDR IPAddress(192,168,1,121)
// byte mac[] = {0x3C, 0x71, 0xBF, 0xFE, 0x4D, 0x78}; 
// byte mac[] = "ESP32-JUKEBOX"; 
unsigned long lastSentAt = millis();
// HADevice device(mac, sizeof(mac));
HADevice device(WiFi.getHostname());
WiFiClient HAclient; 
HAMqtt mqtt(HAclient, device);

void onMqttMessage(const char* topic, const uint8_t* payload, uint16_t length) {
    // This callback is called when message from MQTT broker is received.
    // Please note that you should always verify if the message's topic is the one you expect.
    // For example: if (memcmp(topic, "myCustomTopic") == 0) { ... }

//Serial.print("New message on topic: ");
//Serial.println(topic);
//Serial.print("Data: ");
//Serial.println((const char*)payload);
//Serial.println(topic);
//Serial.println((const char*)payload);

   int key = 0;

   if(memcmp(topic, "JukeSetVolume", 13) == 0) 
   { sscanf((const char*)payload, "%d", &NewVolumeForSonos);
     if(NewVolumeForSonos>90)NewVolumeForSonos=90;
     // Serial.println(NewVolumeForSonos);
     NewVolumeForSonos = rotary_loop(NewVolumeForSonos); 
     AsyncVolumeForSonos = NewVolumeForSonos;
   }  
   else if(memcmp(topic, "JukeSwitchToRadio", 17) == 0) 
   { if(DeviceMode != SELECTRADIO)cancel_button_long_pressed = 200;
   }  
   else if(memcmp(topic, "JukeSwitchToSongs", 17) == 0) 
   { if(DeviceMode != SELECTSONG)cancel_button_long_pressed = 200;
   }  
   else if(memcmp(topic, "JukeSelectRadio", 15) == 0) 
   { sscanf((const char*)payload, "%d", &key);
     if(key>0 && key<21)NewRadioStation=key;
   }  
   else if(memcmp(topic, "JukeSelectSong", 14) == 0) 
   { sscanf((const char*)payload, "%d", &key);
     if(key>0 && key<101)selectedsong = key;
   }  
}

void onMqttConnected() {
    //tft.println("Connected to the broker!");

    // You can subscribe to custom topic if you need
    mqtt.subscribe("JukeSetVolume"); // volume send from  HA
    mqtt.subscribe("JukeSwitchToRadio"); // switch to radio from HA
    mqtt.subscribe("JukeSwitchToSongs"); // switch to jukebox from HA
//    mqtt.subscribe("JukeToggleModeSwitchToRadio"); 
    mqtt.subscribe("JukeSelectRadio"); // choose radio station 1-20 from HA HA
    mqtt.subscribe("JukeSelectSong"); // choose song 1-100 from HA
//    mqtt.subscribe("JukeWhatMode"); // mode status request from HA
//    mqtt.subscribe("JukeReset"); // remote reboot jukebox from HA

}

void onMqttConnectionFailed() {
  //Serial.println("Failed to connect to the broker!");
}


HASensor AdcLeft("AdcLeftJukebox"); // " " is unique ID of the sensor. You should define your own ID.
HASensor AdcRight("AdcRightJukebox"); // " " is unique ID of the sensor. You should define your own ID.
HASensor MQTTEncoderValue("JukeVolumePosition"); // " " is unique ID of the sensor. You should define your own ID.
HASensor MQTTDeviceMode("JukeDeviceMode"); // " " is unique ID of the sensor. You should define your own ID.


void setupMQTT() {

  mqtt.onMessage(onMqttMessage);
  mqtt.onConnected(onMqttConnected);
  mqtt.onConnectionFailed(onMqttConnectionFailed);

 // configure sensor (optional)
 //AdcLeft.setUnitOfMeasurement("mV");
 //AdcLeft.setDeviceClass("temperature");
 //AdcLeft.setIcon("mdi:thermometer");
 //AdcLeft.setName("Juke ADC Left Row");
 // configure sensor (optional)
 //AdcRight.setUnitOfMeasurement("mV");
 //AdcRight.setDeviceClass("temperature");
 //AdcRight.setIcon("mdi:thermometer");
 //AdcRight.setName("Juke ADC Right Row");


// configure sensor (optional)
MQTTDeviceMode.setName("Jukebox DeviceMode");

// configure sensor (optional)
MQTTEncoderValue.setUnitOfMeasurement("dB");
MQTTEncoderValue.setDeviceClass("temperature");
MQTTEncoderValue.setIcon("mdi:volume-medium");
MQTTEncoderValue.setName("JukeBox Volume Setting");

// set device's details (optional)
  device.setName(WiFi.getHostname());
  device.setSoftwareVersion("by Frank");
  device.setModel("ESP32");

  mqtt.begin(BROKER_ADDR, "mqtt1", "mqtt1"); // user and password for the mqtt broker

}

void loopMQTT() {

  mqtt.loop();

// every second, update values using MQTT (for Home Assistant)
  if ((millis() - lastSentAt) >= 1000) 
  { lastSentAt = millis();
//    AdcLeft.setValue(analogRead(ANALOG_LEFT));
//    AdcRight.setValue(analogRead(ANALOG_RIGHT));
    MQTTEncoderValue.setValue(rotary_loop(-1));
    MQTTDeviceMode.setValue(DeviceMode);
   }
   
}
