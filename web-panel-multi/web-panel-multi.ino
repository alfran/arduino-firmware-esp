#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>

const char* ssid = "DHLabs";
const char* password = "dhlabsrfid01";
const char* http_username = "";
const char* http_password = "";

void setup() {
  initSerial(9600);
  initWebFS();
  initArduinoOTA();
  initWiFi(WIFI_AP_STA, "pippowifi", ssid, password);
}

void loop() { 
  manageOTA();
}
