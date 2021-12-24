#include <M5StickCPlus.h>

void setup() {
  Serial.begin(9600);
  // initialize the M5StickC object
  M5.begin();
}

void loop(){
  M5.update(); // Buttonの状態更新のため必要

  if(M5.BtnA.wasPressed()){
    Serial.println("Pressed A button.");
  }
  
  if(M5.BtnB.wasPressed()){
    Serial.println("Pressed B button.");  
  }
}
