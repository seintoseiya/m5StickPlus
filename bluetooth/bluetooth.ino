// 参考サイト：https://modalsoul.hatenablog.com/entry/2021/01/13/070000

#include <M5StickCPlus.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() { 
  Serial.begin(115200);
  
  M5.begin();
  M5.Lcd.setTextFont(1);
  M5.Lcd.setCursor(0, 0, 2); 
  M5.Lcd.println("BlueTooth Sample!");
  SerialBT.begin("ESP32test"); //Bluetooth device name
}

void loop() {
  if(SerialBT.available()) {
    SerialBT.println("get any date!");
  }
  else{
    SerialBT.println("SerialBT! send");
  }
  delay(500);
}
