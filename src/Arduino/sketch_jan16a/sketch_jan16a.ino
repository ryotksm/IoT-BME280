#include <FS.h>
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("");
   
  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    Serial.print(dir.fileName());
    File f = dir.openFile("r");
    Serial.println(String(" ") + f.size());
  }
}
 
void loop() {
  // put your main code here, to run repeatedly:
  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    Serial.print(dir.fileName());
    File f = dir.openFile("r");
    Serial.println(String(" ") + f.size());
  }
}
