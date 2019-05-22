#include <SimpleDHT.h>
#include "EspMQTTClient.h"

#define TOPIC_LED "esp32-led"
#define TOPIC_TEMP "esp32-temp"
#define TOPIC_HUM "esp32-hum"

int pinDHT11 = 23;
int pinLED = 2;
int threshold = 40;
long lastTouch=0;
long lastReading=0;
bool touchDetected = false;
bool ledState=LOW;
byte temperature = 0;
byte humidity = 0;
SimpleDHT11 dht11(pinDHT11);

EspMQTTClient client(
  "LabSE",
  "labse0987",
  "gisexp.icomp.ufam.edu.br",  // MQTT Broker server ip
  "esp32",                    // Can be omitted if not needed
  "123123",               // Can be omitted if not needed
  "esp32client",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void gotTouch(){ 
  if((millis()-lastTouch) > 300){
    lastTouch = millis();
    touchDetected = true; 
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(pinLED,OUTPUT);
  digitalWrite(pinLED,ledState);
  touchAttachInterrupt(T3, gotTouch, threshold);
}

void onConnectionEstablished()
{
  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe(TOPIC_LED, [](const String & payload) {
    if (payload.equals("A")){ ledState = 1; } 
    else{ if(payload.equals("B")) { ledState=0; } }
    digitalWrite(pinLED,ledState);
  }); 
}


void loop() {
  if(touchDetected){   
    ledState=!ledState;
    digitalWrite(pinLED,ledState);
    touchDetected = false;     
    
    String letterState = "A"; //default=on
    if (ledState == 0){ letterState = "B"; } // change to off
    client.publish(TOPIC_LED, String(letterState)); //send mqtt on/off
  }
  

  if(millis()-lastReading > 1500){ //Min interval between readings
    lastReading=millis();
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("DHT11 Reading Error: "); Serial.println(err);delay(1000);
      Serial.println("Last valid values: ");
    }
    client.publish(TOPIC_TEMP, String(temperature));    
    client.publish(TOPIC_HUM, String(humidity));    
  }
  client.loop(); 
}
