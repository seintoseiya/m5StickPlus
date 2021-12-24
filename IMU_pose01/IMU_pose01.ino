#include <M5StickCPlus.h>

#define MODE_A 0 // 数値表示
#define MODE_B 1 // 姿勢表示
uint8_t disp_mode = MODE_A;

#define BTN_A_PIN  37
#define BTN_ON  LOW
#define BTN_OFF HIGH
uint8_t prev_btn_a = BTN_OFF;
uint8_t btn_a      = BTN_OFF;

#define POSE_P_X 0
#define POSE_M_X 1
#define POSE_P_Y 2
#define POSE_M_Y 3
#define POSE_P_Z 4
#define POSE_M_Z 5
uint8_t pose = POSE_P_X;
uint8_t prev_pose = POSE_P_X;

// 加速度。センサで取得できる値の単位は[g]なので、通常の[m/s^2]単位で考えるなら9.8倍する
float accX_g = 0;
float accY_g = 0;
float accZ_g = 0;
float accX_mpss = 0;
float accY_mpss = 0;
float accZ_mpss = 0;

// 角速度。センサで取得できる値の単位は[dps, degree per second]
float gyroX_dps = 0;
float gyroY_dps = 0;
float gyroZ_dps = 0;

boolean near_p_g(float value){
  if(8.0 < value && value < 12.0){
    return true;
  }else{
    return false;
  }
}

boolean near_m_g(float value){
  if(-12.0 < value && value < -8.0){
    return true;
  }else{
    return false;
  }
}

boolean near_zero(float value){
  if(-2.0 < value && value < 2.0){
    return true;
  }else{
    return false;
  }
}

///////////////////////////////////////////////////////////////

void setup() {
  // Initialize the M5StickC object
  M5.begin();
  pinMode(BTN_A_PIN,  INPUT_PULLUP);
  // 6軸センサ初期化
  M5.Imu.Init();
  // LCD display
  M5.Lcd.setRotation(1);  // ボタンBが上になる向き
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
}

void loop() {
  btn_a = digitalRead(BTN_A_PIN);

  if(prev_btn_a == BTN_OFF && btn_a == BTN_ON){
    M5.Lcd.fillScreen(BLACK);
    if(disp_mode == MODE_A){
      disp_mode = MODE_B;
      M5.Lcd.setTextSize(3);
    }else{
      disp_mode = MODE_A;
      M5.Lcd.setTextSize(1);
      M5.Lcd.setRotation(1);  // ボタンBが上になる向き
    }
  }

  prev_btn_a = btn_a;

  // 加速度取得
  M5.Imu.getAccelData(&accX_g,&accY_g,&accZ_g);
  accX_mpss = accX_g * 9.8;
  accY_mpss = accY_g * 9.8;
  accZ_mpss = accZ_g * 9.8;
  // 角速度取得
  M5.Imu.getGyroData(&gyroX_dps,&gyroY_dps,&gyroZ_dps);

  if(disp_mode == MODE_A){
    // 取得した値を表示する
    M5.Lcd.setCursor(0, 30);
    M5.Lcd.printf("Acc : %.2f  %.2f  %.2f   ", accX_mpss, accY_mpss, accZ_mpss);
    M5.Lcd.setCursor(0, 45);
    M5.Lcd.printf("Gyro: %.2f  %.2f  %.2f    ", gyroX_dps, gyroY_dps, gyroZ_dps);
  }else{
    // 現在の姿勢を検出する
    if(near_zero(accX_mpss) && near_p_g(accY_mpss) && near_zero(accZ_mpss)){
      pose = POSE_P_Y;
    }else if(near_p_g(accX_mpss) && near_zero(accY_mpss) && near_zero(accZ_mpss)){
      pose = POSE_P_X;
    }else if(near_zero(accX_mpss) && near_zero(accY_mpss) && near_p_g(accZ_mpss)){
      pose = POSE_P_Z;
    }else if(near_zero(accX_mpss) && near_m_g(accY_mpss) && near_zero(accZ_mpss)){
      pose = POSE_M_Y;
    }else if(near_m_g(accX_mpss) && near_zero(accY_mpss) && near_zero(accZ_mpss)){
      pose = POSE_M_X;
    }else if(near_zero(accX_mpss) && near_zero(accY_mpss) && near_m_g(accZ_mpss)){
      pose = POSE_M_Z;
    }

    // 姿勢に変化があった場合にのみ描画する
    if(prev_pose != pose){
      M5.Lcd.fillScreen(BLACK);
      switch(pose){
      case POSE_P_X:
        M5.Lcd.setRotation(1);
        M5.Lcd.setCursor(56, 16);
        M5.Lcd.print("+X");
        break;
      case POSE_M_X:
        M5.Lcd.setRotation(3);
        M5.Lcd.setCursor(56, 16);
        M5.Lcd.print("-X");
        break;
      case POSE_P_Y:
        M5.Lcd.setRotation(0);
        M5.Lcd.setCursor(16, 56);
        M5.Lcd.print("+Y");
        break;
      case POSE_M_Y:
        M5.Lcd.setRotation(2);
        M5.Lcd.setCursor(16,56);
        M5.Lcd.print("-Y");
        break;
      case POSE_P_Z:
        M5.Lcd.setRotation(1);
        M5.Lcd.setCursor(56, 16);
        M5.Lcd.print("+Z");
        break;
      case POSE_M_Z:
        M5.Lcd.setRotation(1);
        M5.Lcd.setCursor(56, 16);
        M5.Lcd.print("-Z");
        break;
      default:
        ;
      }
    }

    prev_pose = pose;
  }

  delay(100);
}
