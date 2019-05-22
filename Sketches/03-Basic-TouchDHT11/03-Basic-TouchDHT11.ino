#include <SimpleDHT.h>

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

void loop() {
  if(touchDetected){   
    ledState=!ledState;
    digitalWrite(pinLED,ledState);
    touchDetected = false;     
  }

  if(millis()-lastReading > 1500){ //Min interval between readings
    lastReading=millis();
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("DHT11 Reading Error: "); Serial.println(err);delay(1000);
      Serial.println("Last valid values: ");
    }
    Serial.print((int)temperature); Serial.print(" *C, "); 
    Serial.print((int)humidity); Serial.print(" H, ");
    if(ledState){Serial.println("ON");}else{ Serial.println("OFF");}
  }
}
