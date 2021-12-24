// 参考サイト：https://lang-ship.com/blog/work/m5stickc-iphone/

#include <M5StickCPlus.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("M5StickC BLE ScreenShot");

// Battery update time
unsigned long nextVbatCheck = 0;

// get Battery Lebel
int getVlevel() {
  float vbat = M5.Axp.GetBatVoltage();
  int vlevel = ( vbat - 3.2 ) / 0.8 * 100;
  if ( vlevel < 0 ) {
    vlevel = 0;
  }
  if ( 100 < vlevel ) {
    vlevel = 100;
  }

  return vlevel;
}

void setup() {
  M5.begin();
  M5.Axp.ScreenBreath(9);
  setCpuFrequencyMhz(80);
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 16);
  M5.Lcd.println("NimBLE");
  M5.Lcd.println("ScreenShot");
  M5.Lcd.println();
  M5.Lcd.println(" Press BtnA");

  bleKeyboard.setBatteryLevel(getVlevel());
  bleKeyboard.begin();
}

void loop() {
  // Button Update
  M5.update();

  if (bleKeyboard.isConnected()) {
    if ( M5.BtnA.wasPressed() ) {
      // ScreenShot(COMMAND + SHIFT + 3)
      bleKeyboard.press(KEY_LEFT_GUI);
      bleKeyboard.press(KEY_LEFT_SHIFT);
      bleKeyboard.press('3');
      delay(100);
      bleKeyboard.releaseAll();
    }
  }

  // Battery Lebel Update
  if (nextVbatCheck < millis()) {
    M5.Lcd.setCursor(112, 0);
    M5.Lcd.printf("%3d%%", getVlevel());

    nextVbatCheck = millis() + 60000;
  }

  // Wait
  delay(1);
}
