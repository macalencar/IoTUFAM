#include <SimpleDHT.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "FS.h"
#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true
#define CONF_FILE "/wifi.conf"
#define PIN_DHT11 23
#define PIN_LED 2
#define DNS_PORT  53
#define HTTP_PORT 80
#define touchTHRESHOLD 40


// DHT11
long lastReading = 0;
byte temperature = 0;
byte humidity = 0;
SimpleDHT11 dht11(PIN_DHT11);

// TOUCH SENSOR
bool touchState = false;
long lastTouch = 0;
bool ledState = false;


// CONNECTIVITY
String ssid = "";
String password = "";
bool storeCredentials = false;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

// WEB SERVER
WiFiServer server(HTTP_PORT);
String responseHTML = "<!DOCTYPE html><html><head><title>CaptivePortal</title></head>";
String formHTML = "";
String adminpassword="YWRtaW4u";


//************************ OTHER FUNCTIONS *****************************//
//Reboot System 
void(* resetFunc) (void) = 0;

//Touch sensor
void gotTouch() {
  lastTouch = millis();
  touchState = true;
}


void setup() {
  Serial.begin(115200);
  
  //Configure LedPin and Touch Interruption pin(T3 = 15)
  
  pinMode(PIN_LED, OUTPUT);                                                     
  touchAttachInterrupt(T3, gotTouch, touchTHRESHOLD);

  //Check if file system is available and if there's a config file
  if((SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) && (readFile(SPIFFS, CONF_FILE))){
    Serial.println("STAMODE");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    int count = 0;
    
    //Trying to connect
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      count++;
      //wait 60 seconds before clear config and reboot
      if (count == 120) {     
        deleteFile(SPIFFS, CONF_FILE);
        Serial.println("Can't connect to" + ssid + " removing config ");
        resetFunc();
        break;
      }
    }
    //Connected
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("APMODE");
    WiFi.mode(WIFI_AP_STA);
    formHTML = scanAP();
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("Esp32-CaptivePortal");
    dnsServer.start(DNS_PORT, "*", apIP);
  }
  //Starts Web Server
  server.begin();
  
}

void loop() {
  dnsServer.processNextRequest();
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
          if (c == '\n') {
            int auth = currentLine.indexOf("Authorization: Basic ")+1;
            String httpauthentication=currentLine.substring(auth+21, currentLine.length());
            Serial.println(httpauthentication);
            Serial.println(adminpassword);
            if(adminpassword!=httpauthentication){ 
              client.println("HTTP/1.1 401 Unauthorized");
              client.println("WWW-Authenticate: Basic realm=\"Secure Page\"");
              client.println("Content-Type: text/html");
              client.println();
              break;
            }
/*            client.println("<html>Text to send if user hits Cancel button</html>"); // really need this for the popup!
            client.println("HTTP/1.1 401 Authorization Required ");
            client.println("Content-type:text/html");
            client.println("WWW-Authorization: Basic realm=\"Pagina segura\"");
            Serial.println("Authorization: Basic "+adminpassword);
            client.println();
            client.print("<body>");*/

          //CHECK CONNECTIVITY
          if (currentLine.indexOf("?ssid") != -1) {
            client.print("<h3>Credentials saved, reboot your device and check in your serial port</h3>");
            int first = currentLine.indexOf("ssid=");
            int last = currentLine.indexOf("&");
            ssid = urldecode(currentLine.substring(first + 5, last));
            first = currentLine.indexOf("password=");
            last = currentLine.indexOf(" HTTP");
            password = urldecode(currentLine.substring(first + 9, last));
            Serial.println(ssid + "\n" + password + "\n");
            storeCredentials = true;
          }
          else{if(currentLine.startsWith("GET /disconnect")) {
              deleteFile(SPIFFS, CONF_FILE);
              client.print("<h3>Your device is still connected. Reboot your device and reconfigure the WiFi connection</h3>");}
          else{if(WiFi.status() == WL_CONNECTED) {
                String ip = WiFi.localIP().toString();
                client.print("<h3>Connected to: " + WiFi.SSID() + " - "); client.print(ip); client.print("</h3> <a href=/disconnect>Disconnect</a> "); }
          else{ client.print(formHTML); }
          }}

          //OTHERS ELEMENTS
          if(currentLine.startsWith("GET /led/on")){ ledState = 1; digitalWrite(PIN_LED, ledState); }
          else{if(currentLine.startsWith("GET /led/off")){ ledState = 0; digitalWrite(PIN_LED, ledState);}
          else {if (currentLine.startsWith("GET /update")) { formHTML = scanAP(); }
          }}

          //TEMPERATURE, HUMIDITY
          client.print("<hr><br><b>Temperature: </b>");
          client.print(String(temperature));
          client.print("<br><b>Humidity: </b>");
          client.print(String(humidity));

          //LED STATE / CONTROL
          client.print("<br><b>LedState: </b>");
          if(ledState == 0){ client.print("OFF <a href=/led/on>Turn On</a>"); }
          else{ client.print("ON <a href=/led/off>Turn Off</a>");}

          //END HTML PAGE
          client.print("<hr></body></html>");
          currentLine = "";
          break;
        } 
        else if (c != '\r') { currentLine += c; }
      }
    }
    client.stop();
  }

  //UPDATE /  CHECK VALUES FROM TOUCH AND DHT11
  if (touchState && millis() - lastTouch > 100) {
    touchState = false;
    ledState = !ledState;
    digitalWrite(PIN_LED, ledState);
  }
  if (millis() - lastReading > 2000) { //Min interval between readings
    lastReading = millis();
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("DHT11 Reading Error: "); Serial.println(err); delay(1000);
    }
  }

  //CHECK IF SHOULD STORE CREDENTIALS
  if (storeCredentials) {
    String fileContent = ssid + "\n" + password;
    if (writeFile(SPIFFS, CONF_FILE, fileContent.c_str())) { Serial.print("Credentials stored in "); Serial.println(CONF_FILE); }
    else { Serial.println("Fail to store credentials"); }
    storeCredentials = false;
  }
}
