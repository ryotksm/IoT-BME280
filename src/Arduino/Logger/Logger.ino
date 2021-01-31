#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <FS.h>

const char* DEFAULTSSID = "your-ssid";
const char* DEFAULTPASS = "your-password";

ESP8266WiFiMulti wifiMulti;
const char* settings = "/wifi_settings.txt";
ESP8266WebServer server(80);
boolean cn = false;

/**
   設定画面
*/
void handleRootGet() {
  String html = "";
  html += F( "<h1>WiFi Setting</h1>");
  html += F( "<table><tr><th>SSID</th></tr>");
  File f = SPIFFS.open(settings, "r");
  while (f.available()) {
    String ssid = f.readStringUntil('\n');
    String pass = f.readStringUntil('\n');
    html += F( "<tr><td>");
    html += ssid;
    html += F("</td></tr>");
  }
  f.close();
  html += F("</table><br>");
  html += F("<form method='post'>");
  html += F("<input type='text' name='ssid' placeholder='ssid'><br>");
  html += F("<input type='password' name='pass' placeholder='pass'><br>");
  html += F("<input type='submit'><br>");
  html += F("</form>");
  server.send(200, "text/html", html);
}

void handleRootPost() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  ssid.trim();
  pass.trim();
  String bkf = String(settings) + ".bak";
  SPIFFS.remove(bkf);
  SPIFFS.rename(settings, bkf);
  File fr = SPIFFS.open(bkf, "r");
  File fw = SPIFFS.open(settings, "w");
  // 入力されたSSID/Passを書き込み
  fw.println(ssid);
  fw.println(pass);
  while (fr.available()) {
    // ファイルからSSID/Passを読み込み
    String ssidr = fr.readStringUntil('\n');
    String passr = fr.readStringUntil('\n');
    ssidr.trim();
    passr.trim();
    if (ssidr != ssid) {
      // 入力されたものと違う場合はファイルに書き込む
      // 二重登録の防止と順番入れ替え対応
      fw.println(ssidr);
      fw.println(passr);
    }
  }
  fw.close();
  fr.close();
  String html = "";
  html += F("<h1>WiFi Setting</h1>");
  html += ssid + "<br>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();

  server.on("/", HTTP_GET, handleRootGet);
  server.on("/", HTTP_POST, handleRootPost);
  server.begin();
  Serial.println("HTTP server started.");

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  File f = SPIFFS.open(settings, "r");
  while (f.available()) {
    String ssid = f.readStringUntil('\n');
    String pass = f.readStringUntil('\n');
    wifiMulti.addAP(ssid.c_str(), pass.c_str());
    Serial.println(ssid.c_str());
    Serial.println(pass.c_str());
  }
  f.close();
  wifiMulti.addAP(DEFAULTSSID, DEFAULTPASS);

  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Access point: ");
    Serial.println(WiFi.SSID());
  }
}

void loop() {
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    cn = false;
    delay(1000);
  } else {
    if (!cn) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("Access point: ");
      Serial.println(WiFi.SSID());
      cn = true;
    }
    server.handleClient();
  }
}
