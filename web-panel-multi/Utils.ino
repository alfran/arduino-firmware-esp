extern "C" void system_set_os_print(uint8 onoff);
extern "C" void ets_install_putc1(void* routine);

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



IPAddress stringToIP(String address){
  int p1 = address.indexOf('.'), p2 = address.indexOf('.', p1+1), p3 = address.indexOf('.', p2+1);//, 4p = address.indexOf(3p+1);
  String ip1 = address.substring(0, p1), ip2 = address.substring(p1+1, p2), ip3 = address.substring(p2+1, p3), ip4 = address.substring(p3+1);
  
  return IPAddress(ip1.toInt(), ip2.toInt(), ip3.toInt(), ip4.toInt());
  }


static void _u0_putc(char c) {
  while (((U0S >> USTXC) & 0x7F) == 0x7F);
  U0F = c;
}

void initSerial(int spd) {
  Serial.begin(spd);
  ets_install_putc1((void *) &_u0_putc);
  system_set_os_print(1);
}


