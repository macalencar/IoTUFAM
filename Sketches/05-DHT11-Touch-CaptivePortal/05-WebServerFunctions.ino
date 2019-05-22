//************************ WEB SERVER FUNCTIONS *****************************//
//Scans WiFi networks and creates a selection list in HTML 
String scanAP() {
  String formTag = "<form action=# method=get><select name=ssid>";
  int n = WiFi.scanNetworks();
  if (n == 0) {
    formTag = "<h2>No Networks Found</h2>";
  }
  else {
    Serial.print(n); Serial.println(" networks found");
    // Append SSID's to HTML Selection List
    for (int i = 0; i < n; ++i) {
      formTag += "<option value=\""; formTag += WiFi.SSID(i); formTag += "\">";
      if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
        formTag += "*";
      }
      formTag += WiFi.SSID(i); formTag += "("; formTag += WiFi.RSSI(i);
      formTag += ")</option>";
      delay(10);
    }
    formTag += "<input type=password name=password placeholder=password><br><input type=submit value=Conectar></form>";
  }
  formTag += "<br> <a href=/update>update wifi list</a>";
  return formTag;
}

// Convert UTF-8 chars code
unsigned char h2int(char c){
  if (c >= '0' && c <= '9') {return ((unsigned char)c - '0'); }
  if (c >= 'a' && c <= 'f') {return ((unsigned char)c - 'a' + 10); }
  if (c >= 'A' && c <= 'F') {return ((unsigned char)c - 'A' + 10);}
  return (0);
}

// Decode URL in UTF-8 format
String urldecode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == '+') { encodedString += ' '; }
    else if (c == '%') {
      i++;
      code0 = str.charAt(i);
      i++;
      code1 = str.charAt(i);
      c = (h2int(code0) << 4) | h2int(code1);
      encodedString += c;
    }
    else { encodedString += c; }
    yield();
  }
  return encodedString;
}
