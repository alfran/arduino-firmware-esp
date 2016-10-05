
void initWiFi(int wifiMode, const char* hostname, const char* ssid, const char* password){
 if(wifiMode==WIFI_AP_STA){
  WiFi.mode(WIFI_AP_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
 } else {
  WiFi.softAP(ssid,password);
 }
 while ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
  delay ( 5000 );
  Serial.print ( "." );
  ESP.restart();
 }
}


