//************************ SPIFFS FUNCTIONS *****************************//
bool readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return false;
  }
  Serial.println("- read from file:");
  int n = 0; //1-wifi,2-ssid
  String line = "";
  while (file.available()) {
    char c = file.read();
    if (c != '\n') {
      if (n == 0) {
        ssid += c;
      }
      if (n == 1) {
        password += c;
      }
    } else {
      n++;
    }
  }
  file.close();
  return true;
}

bool writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return false;
  }
  if (!file.print(message)) {
    Serial.println("- fail to write");
    return false;
  }
  Serial.println("- file written");
  file.close();
  return true;
}

bool deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("- file deleted");
    return true;
  }
  Serial.println("- delete failed");
  return false;
}
