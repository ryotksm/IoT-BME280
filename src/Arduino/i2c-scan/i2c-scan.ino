/*
 I2Cアドレススキャナー(I2C address scanner)
 
 I2Cのアドレスをスキャンしてデバイスが存在するアドレス
 をシリアルモニタに表示する。
 2014/12/23　ラジオペンチ
 http://radiopench.blog96.fc2.com/
 2016/04/27 ESP-WROOM-02 (ESP8266) 用にちょっと修正
*/
 
#include <Wire.h>
 
void setup() {
  Wire.begin(12,14);                       // 引数無しは (4, 5)と同じ (SDA,SCL)
  Serial.begin(115200);
  Serial.println(); Serial.println("Start I2C address scanner"); 
  }
 
void loop() {
  Serial.println("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
  for (byte n = 0; n <= 0x7F; n++) {  // I2Cアドレスを指定
    adrCheck(n);                      // デバイス有無を調べ結果を出力
  }
  Serial.println();                  // 1行空けて
  delay(2000);                        // 2秒待って繰り返す
}
 
void adrCheck(byte adr) {
  byte    dummy;                      // Dummy
  if ((adr & 0x0F) == 0) {            // 行の先頭なら
    print_hex2chr(adr);               // アドレスを出力
    Serial.print(":");
  }
  Serial.print(" ");                 // 区切り用に空白を入れる
 
  if (adr < 8 || adr > 0x77) {        // 予約アドレスの範囲は
    Serial.print("xx");              // xx 表示
  }
  else {                              // アドレスにデバイスがあるか？
    Wire.beginTransmission(adr);
    Wire.write(&dummy, 0);
    if (Wire.endTransmission() == 0) { // 書き込みが正常終了していたら
      print_hex2chr(adr);             // そのアドレスを表示
    }
    else {
      Serial.print("--");            // 無効表示（--）
    }
  }
  if ( (adr & 0x0F) == 0x0F) {        // 行末なら改行
    Serial.println();
  }
}
 
void print_hex2chr(byte x) {          // 16進2桁で出力
  Serial.print((x >> 4), HEX);       // 上位
  Serial.print((x & 0x0F), HEX);     // 下位
}
