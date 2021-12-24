#include <M5StickCPlus.h>

void setup() {
  // initialize the M5StickC object
  M5.begin();
  M5.Axp.ScreenBreath(10); // 画面の明るさ7〜１2
  M5.Lcd.setRotation(3); // 画面を横向きにする
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setCursor(5, 10);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("Hello World!");
}

void loop(){
}
