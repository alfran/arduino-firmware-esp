#include <ESP8266WiFi.h>


const char* ssid = "DHLabs";
const char* password = "dhlabsrfid01";
const char* http_username = "";
const char* http_password = "";
const char* host_name = "arduinoino";

void setup() {
  initSerial(9600);
  initWebFS();
  initArduinoOTA();
  initWiFi(WIFI_AP_STA, host_name, ssid, password);
}

void loop() { 
  manageOTA();
}
