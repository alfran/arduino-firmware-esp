#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>


int interval = 30000;
bool en = false;
bool conflag = false;
int pMillis = 0;
int tot;
bool reboot = false;
String readS;
bool cflag = false;
String staticIP_param ;
String netmask_param;
String gateway_param;
String dhcp = "on";
char newSSID[50];
char newPASSWORD[50];




void initArduinoOTA(){
  ArduinoOTA.setHostname(host_name);
  ArduinoOTA.begin();
}

void manageOTA(){
    ArduinoOTA.handle();
    if(conflag){
    conflag = false;
    WiFi.config( stringToIP(staticIP_param) , stringToIP(gateway_param), stringToIP(netmask_param));
  }

  int cMillis = millis();
  if(pMillis == 0 || cMillis-pMillis > interval){
    Serial.println("Scanning");
    tot = WiFi.scanNetworks();
    pMillis = cMillis;
  }

 
  if(cflag)
  {
    WiFi.disconnect();
    WiFi.begin(newSSID, newPASSWORD);
    while( WiFi.status() != WL_CONNECTED){
      delay(1000);
    }
    cflag = false;
  }

  if(reboot)
  {
    reboot = false;
    ESP.restart();
   }

  if (en) {
    while (Serial.available() > 0) {
      char x = Serial.read();
      if (int(x) != -1)
        readS = readS + x;
    }
  }   
  yield();
}

