#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

int tot;

bool en = false;
bool conflag = false;
String staticIP_param ;
String netmask_param;
String gateway_param;
String dhcp = "on";

int pMillis = 0;
int intervall = 30000;

String debug_log = "";
int debug_counter = 0;

bool reboot = false;

int c_status = WL_IDLE_STATUS;
char newSSID[50];
char newPASSWORD[50];
bool cflag = false;

String readS;
int siz = 0;

// WEB HANDLER IMPLEMENTATION
class SPIFFSEditor: public AsyncWebHandler {
  private:
    String _username;
    String _password;
    bool _uploadAuthenticated;
  public:
    SPIFFSEditor(String username = String(), String password = String()): _username(username), _password(password), _uploadAuthenticated(false) {}
    bool canHandle(AsyncWebServerRequest *request) {
      if (request->method() == HTTP_GET && request->url() == "/edit" && (SPIFFS.exists("/edit.htm") || SPIFFS.exists("/edit.htm.gz")))
        return true;
      else if (request->method() == HTTP_GET && request->url() == "/list")
        return true;
      else if (request->method() == HTTP_GET && (request->url().endsWith("/") || SPIFFS.exists(request->url()) || (!request->hasParam("download") && SPIFFS.exists(request->url() + ".gz"))))
        return true;
      else if (request->method() == HTTP_POST && request->url() == "/edit")
        return true;
      else if (request->method() == HTTP_DELETE && request->url() == "/edit")
        return true;
      else if (request->method() == HTTP_PUT && request->url() == "/edit")
        return true;
      return false;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      if (_username.length() && (request->method() != HTTP_GET || request->url() == "/edit" || request->url() == "/list") && !request->authenticate(_username.c_str(), _password.c_str()))
        return request->requestAuthentication();

      if (request->method() == HTTP_GET && request->url() == "/edit") {
        request->send(SPIFFS, "/edit.htm");
      } else if (request->method() == HTTP_GET && request->url() == "/list") {
        if (request->hasParam("dir")) {
          String path = request->getParam("dir")->value();
          Dir dir = SPIFFS.openDir(path);
          path = String();
          String output = "[";
          while (dir.next()) {
            File entry = dir.openFile("r");
            if (output != "[") output += ',';
            bool isDir = false;
            output += "{\"type\":\"";
            output += (isDir) ? "dir" : "file";
            output += "\",\"name\":\"";
            output += String(entry.name()).substring(1);
            output += "\"}";
            entry.close();
          }
          output += "]";
          request->send(200, "text/json", output);
          output = String();
        }
        else
          request->send(400);
      } else if (request->method() == HTTP_GET) {
        String path = request->url();
        if (path.endsWith("/"))
          path += "index.html";
        request->send(SPIFFS, path, String(), request->hasParam("download"));
      } else if (request->method() == HTTP_DELETE) {
        if (request->hasParam("path", true)) {
          ESP.wdtDisable(); SPIFFS.remove(request->getParam("path", true)->value()); ESP.wdtEnable(10);
          request->send(200, "", "DELETE: " + request->getParam("path", true)->value());
        } else
          request->send(404);
      } else if (request->method() == HTTP_POST) {
        if (request->hasParam("data", true, true) && SPIFFS.exists(request->getParam("data", true, true)->value()))
          request->send(200, "", "UPLOADED: " + request->getParam("data", true, true)->value());
        else
          request->send(500);
      } else if (request->method() == HTTP_PUT) {
        if (request->hasParam("path", true)) {
          String filename = request->getParam("path", true)->value();
          if (SPIFFS.exists(filename)) {
            request->send(200);
          } else {
            File f = SPIFFS.open(filename, "w");
            if (f) {
              f.write(0x00);
              f.close();
              request->send(200, "", "CREATE: " + filename);
            } else {
              request->send(500);
            }
          }
        } else
          request->send(400);
      }
    }

    void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      if (!index) {
        if (!_username.length() || request->authenticate(_username.c_str(), _password.c_str()))
          _uploadAuthenticated = true;
        request->_tempFile = SPIFFS.open(filename, "w");
      }
      if (_uploadAuthenticated && request->_tempFile && len) {
        ESP.wdtDisable(); request->_tempFile.write(data, len); ESP.wdtEnable(10);
      }
      if (_uploadAuthenticated && final)
        if (request->_tempFile) request->_tempFile.close();
    }
};


// SKETCH BEGIN
AsyncWebServer server(80);

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

String toStringWifiMode(int mod) {
  String mode;
  switch (mod) {
    case 0:
      mode = "OFF";
      break;
    case 1:
      mode = "STA";
      break;
    case 2:
      mode = "AP";
      break;
    case 3:
      mode = "AP+STA";
      // statements
      break;
    case 4:
      mode = "----";
      break;
    default:
      // statements
      break;
  }
  return mode;
}

WiFiMode intToWifiMode(int mod) {
  WiFiMode mode;
  switch (mod) {
    case 0:
      mode = WIFI_OFF;
      break;
    case 1:
      mode = WIFI_STA;
      break;
    case 2:
      mode = WIFI_AP;
      break;
    case 3:
      mode = WIFI_AP_STA;
      break;
    case 4:
      break;
    default:
      break;
  }
  return mode;
}

String toStringWifiStatus(int state) {
  String status;
  switch (state) {
    case 0:
      status = "connecting";
      break;
    case 1:
      status = "unknown status";
      break;
    case 2:
      status = "wifi scan completed";
      break;
    case 3:
      status = "got IP address";
      // statements
      break;
    case 4:
      status = "connection failed";
      break;
    default:
      // statements
      break;
  }
  return status;
}

String toStringEncryptionType(int thisType) {
  String eType;
  switch (thisType) {
    case ENC_TYPE_WEP:
      eType = "WEP";
      break;
    case ENC_TYPE_TKIP:
      eType = "WPA";
      break;
    case ENC_TYPE_CCMP:
      eType = "WPA2";
      break;
    case ENC_TYPE_NONE:
      eType = "None";
      break;
    case ENC_TYPE_AUTO:
      eType = "Auto";
      break;
  }
  return eType;
}



IPAddress stringToIP(String address) {
  int p1 = address.indexOf('.'), p2 = address.indexOf('.', p1 + 1), p3 = address.indexOf('.', p2 + 1); //, 4p = address.indexOf(3p+1);
  String ip1 = address.substring(0, p1), ip2 = address.substring(p1 + 1, p2), ip3 = address.substring(p2 + 1, p3), ip4 = address.substring(p3 + 1);

  return IPAddress(ip1.toInt(), ip2.toInt(), ip3.toInt(), ip4.toInt());
}



const char* ssid = "YOUR_SSID_HERE";
const char* password = "YOUR_PASSWORD_HERE";
const char* http_username = "";
const char* http_password = "";


extern "C" void system_set_os_print(uint8 onoff);
extern "C" void ets_install_putc1(void* routine);

//Use the internal hardware buffer
static void _u0_putc(char c) {
  while (((U0S >> USTXC) & 0x7F) == 0x7F);
  U0F = c;
}

void initSerial() {
  Serial.begin(9600);
  ets_install_putc1((void *) &_u0_putc);
  system_set_os_print(1);
}

void setup() {
  initSerial();
  SPIFFS.begin();

  //Enable to start in AP mode
  char * softApssid;
  char buff [17];
  WiFi.macAddress().toCharArray(buff, 17);
  delay(500);
  String tmp_buff =  String(buff);
  tmp_buff.replace(":","");
  String tmp_ssid = "Arduino-Primo-" + tmp_buff;
  tmp_ssid.toCharArray(softApssid , tmp_ssid.length());
  delay(500);
  WiFi.softAP(softApssid);

  //Enable to start in STA mode
  /*WiFi.mode(WIFI_STA);
    WiFi.hostname("ARDUINOWiFi");
    WiFi.begin(ssid, password);
    while ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
    delay ( 5000 );
    Serial.print ( "." );
    ESP.restart();
    }*/

  server.serveStatic("/fs", SPIFFS, "/");

  //"wifi/info" information
  server.on("/wifi/info", HTTP_GET, [](AsyncWebServerRequest * request) {
    String ipadd = (WiFi.getMode() == 1 || WiFi.getMode() == 3) ? toStringIp(WiFi.localIP()) : toStringIp(WiFi.softAPIP());
    String staticadd = dhcp.equals("on") ? "0.0.0.0" : staticIP_param;
    int change = WiFi.getMode() == 1 ? 3 : 1;

    debug_log += "[" + String(debug_counter) + "] GET wifi/info ";
    debug_log += String( "{\"ssid\":\"" + WiFi.SSID() + "\",\"hostname\":\"" + WiFi.hostname() + "\",\"ip\":\"" + ipadd + "\",\"mode\":\"" + toStringWifiMode(WiFi.getMode()) + "\",\"chan\":\""
                         + WiFi.channel() + "\",\"status\":\"" + toStringWifiStatus(WiFi.status()) + "\", \"gateway\":\"" + toStringIp(WiFi.gatewayIP()) + "\", \"netmask\":\"" + toStringIp(WiFi.subnetMask()) + "\",\"rssi\":\""
                         + WiFi.RSSI() + "\",\"mac\":\"" + WiFi.macAddress() + "\",\"phy\":\"" + WiFi.getPhyMode() + "\", \"dhcp\": \"" + dhcp + "\", \"staticip\":\"" + staticadd +
                         + "\n" );
    debug_counter++;

    request->send(200, "text/plain", String("{\"ssid\":\"" + WiFi.SSID() + "\",\"hostname\":\"" + WiFi.hostname() + "\",\"ip\":\"" + ipadd + "\",\"mode\":\"" + toStringWifiMode(WiFi.getMode()) + "\",\"chan\":\""
                                            + WiFi.channel() + "\",\"status\":\"" + toStringWifiStatus(WiFi.status()) + "\", \"gateway\":\"" + toStringIp(WiFi.gatewayIP()) + "\", \"netmask\":\"" + toStringIp(WiFi.subnetMask()) + "\",\"rssi\":\""
                                            + WiFi.RSSI() + "\",\"mac\":\"" + WiFi.macAddress() + "\",\"phy\":\"" + WiFi.getPhyMode() + "\", \"dhcp\": \"" + dhcp + "\", \"staticip\":\"" + staticadd +
                                            + "\", \"warn\": \"" + "<a href='#' class='pure-button button-primary button-larger-margin' onclick='changeWifiMode(" + change + ")'>Switch to " + toStringWifiMode(change) + " mode</a>\""
                                            + "}" ));
  });

  //"system/info" information
  server.on("/system/info", HTTP_GET, [](AsyncWebServerRequest * request) {

    request->send(200, "text/plain", String("{\"id\":\"" + String(ESP.getFlashChipId()) + "\",\"size\":\"" + (ESP.getFlashChipSize() / 1024 / 1024) + " MB\",\"baud\":\"9600\"}"));
    //    request->send(200, "text/plain", String("{\"id\":\"0x00\",\"size\":\"4MB\",\"baud\":\"9600\"}"));
  });
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/console/send", HTTP_POST, [](AsyncWebServerRequest * request) {
    Serial.print( request->getParam("text")->value());
    request->send(200, "text/plain", String("1"));
  });

  server.on("/console/text", HTTP_GET, [](AsyncWebServerRequest * request) {
    en = true;
    //Serial.println(readS);
    //request->send(200, "text/plain", String("{\"text\":\""+readS+"\", \"start\":\"0\", \"len\":\""+String(readS.length())+"\"}"));
    request->send(200, "text/plain", String(readS));
    readS = "";
    siz = 0;
  });

  server.on("/console/reset", HTTP_POST, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "1");
    pinMode(12, OUTPUT);
    digitalWrite(12, LOW);
    delay(10);
    digitalWrite(12, HIGH);
  });

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/system/update", HTTP_POST, [](AsyncWebServerRequest * request) {
    String newhostname = request->getParam("name")->value();
    //    Serial.println("New Hostname "+newhostname);
    WiFi.hostname(newhostname);
    debug_log += "[" + String(debug_counter) + "] POST  Hostname updated to : " + newhostname + "\n" ;
    debug_counter++;
    request->send(200, "text/plain", newhostname);
  });

  server.on("/log/reset", HTTP_POST, [](AsyncWebServerRequest * request) {
    reboot = true;
    request->send(200, "text/plain",  "1");
  });

  server.on("/wifi/scan", HTTP_GET, [](AsyncWebServerRequest * request) {
    //    Serial.println("wifi scan");
    String scanResp = "";

    //      Serial.print("Scanned : "); Serial.println(tot);

    debug_log += String("[" + String(debug_counter) + "] GET  WiFi scan : " + String(tot) + " found\n");
    debug_counter++;

    if (tot == 0) {
      request->send(200, "text/plain", "No networks found");
    }
    if (tot == -1 ) {
      request->send(500, "text/plain", "Error during scanning");
    }

    scanResp += "{\"result\": { \"APs\" : [ ";

    for (int netIndex = 0; netIndex < tot; netIndex++) {
      scanResp += "{\"enc\" : \"";
      scanResp += toStringEncryptionType (WiFi.encryptionType(netIndex));
      scanResp += "\",";
      scanResp += "\"essid\":\"";
      scanResp += WiFi.SSID(netIndex);
      scanResp += "\",";
      scanResp += "\"rssi\" :\"";
      scanResp += WiFi.RSSI(netIndex);
      scanResp += "\"}";

      debug_log +=  String("bss" + String(netIndex) + ": " + WiFi.SSID(netIndex) + "\t( " + WiFi.RSSI(netIndex) + " )\n" );
      if (netIndex != tot - 1)
        scanResp += ",";
    }

    scanResp += "]}}";
    request->send(200, "text/plain", scanResp);
  });

  server.on("/connect", HTTP_POST, [](AsyncWebServerRequest * request) {
    String newSSID_param = request->getParam("essid")->value();
    String newPASSWORD_param = request->getParam("passwd")->value();

    newSSID_param.toCharArray(newSSID, newSSID_param.length() + 1);
    newPASSWORD_param.toCharArray(newPASSWORD, newPASSWORD_param.length() + 1);

    cflag = true;
    delay(1000);

    debug_log += String ("[" + String(debug_counter) + "] POST  Connect to : " + newSSID_param + "\n" );
    debug_counter++;
    request->send(200, "text/plain", "1");
  });

  server.on("/setmode", HTTP_POST, [](AsyncWebServerRequest * request) {
    int newMode = request->getParam("mode")->value().toInt();

    debug_log += "[" + String(debug_counter) + "] POST  Mode change from " + toStringWifiMode(WiFi.getMode()) + "to " + toStringWifiMode(newMode) + "\n";
    debug_counter++;
    switch (newMode) {
      case 1 :
      case 3 :
        request->send(200, "text/plain", String("Mode changed " + toStringWifiMode(WiFi.getMode())));
        WiFi.mode( intToWifiMode(newMode) );
        //        Serial.println("ok change mode " + toStringWifiMode(WiFi.getMode()));
        break;
      case 2 :
        //        Serial.println("ok change mode " + toStringWifiMode(WiFi.getMode()));
        request->send(200, "text/plain", String("Mode changed " + toStringWifiMode(WiFi.getMode())));
        WiFi.mode( WIFI_AP );
        break;
    }
  });

  server.on("/special", HTTP_POST, [](AsyncWebServerRequest * request) {
    dhcp = request->getParam("dhcp")->value();
    staticIP_param = request->getParam("staticip")->value();
    netmask_param = request->getParam("netmask")->value();
    gateway_param = request->getParam("gateway")->value();

    if (dhcp == "off") {
      debug_log += "[" + String(debug_counter) + "] POST  STATIC IP on | " + "StaticIp : " + staticIP_param + ", NetMask : " + netmask_param + ", Gateway : " + gateway_param + "\n";
      debug_counter++;
      conflag = true;
      delay(5000);
      request->send(200, "text/plain", String("{\"url\":\"" + staticIP_param + "\"}"));
    }
    else {
      debug_log += "[" + String(debug_counter) + "] POST  DHCP on \n";
      debug_counter++;
      conflag = false;
      request->send(200, "text/plain",  "1");
      WiFi.begin(ssid, password);
      while ( WiFi.waitForConnectResult() != WL_CONNECTED ) {
        delay ( 5000 );
        Serial.print ( "." );
        ESP.restart();
      }
    }
  });

  server.on("/log/text", HTTP_GET, [](AsyncWebServerRequest * request) {
    String tmp_resp = debug_log;
    debug_log = "";
    request->send(200, "text/plain",  tmp_resp);
  });

  server.on("/log/dbg", HTTP_GET,  [](AsyncWebServerRequest * request) {
    //TODO
    //resp   :    {auto/off/on0/on1}
  });



  server.addHandler(new SPIFFSEditor(http_username, http_password));

  server.onNotFound([](AsyncWebServerRequest * request) {

    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++) {
      AsyncWebHeader* h = request->getHeader(i);
      //os_printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for (i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (p->isFile()) {
        //os_printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if (p->isPost()) {
        //os_printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        //os_printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  server.onFileUpload([](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  });
  server.onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

  });

  server.begin();
  ArduinoOTA.begin();
}

void loop() {

  ArduinoOTA.handle();
  if (conflag) {
    conflag = false;
    WiFi.config( stringToIP(staticIP_param) , stringToIP(gateway_param), stringToIP(netmask_param));
  }

  int cMillis = millis();
  if (pMillis == 0 || cMillis - pMillis > intervall) {
    Serial.println("Scanning");
    tot = WiFi.scanNetworks();
    pMillis = cMillis;
  }


  if (cflag)
  {
    WiFi.disconnect();
    WiFi.begin(newSSID, newPASSWORD);
    while ( WiFi.status() != WL_CONNECTED) {
      delay(1000);
    }
    cflag = false;
  }

  if (reboot)
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
}
