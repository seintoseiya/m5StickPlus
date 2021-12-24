#include <M5StickCPlus.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

hw_timer_t *timer;
QueueHandle_t xQueue;
TaskHandle_t taskHandle;

const int sampleFrequency = 100;
uint8_t calibration = 0;        // キャリブレーションの状態(0:初期化直後, 1:データ取得中, 2:完了)

// タイマー割り込み
void IRAM_ATTR onTimer() {
  int8_t data;

  // キューを送信
  xQueueSendFromISR(xQueue, &data, 0);
}

// 実際のタイマー処理用タスク
void task(void *pvParameters) {  
  uint16_t calibrationCount = 0;  // データ取得数
  float gyroXSum = 0;             // ジャイロX軸の累計数
  float gyroYSum = 0;             // ジャイロY軸の累計数
  float gyroZSum = 0;             // ジャイロZ軸の累計数
  float gyroXOffset = 0;          // ジャイロX軸のオフセット
  float gyroYOffset = 0;          // ジャイロY軸のオフセット
  float gyroZOffset = 0;          // ジャイロZ軸のオフセット
  float pitchSum = 0;             // ピッチの累計数
  float rollSum = 0;              // ロールの累計数
  float yawSum = 0;               // ヨーの累計数
  float pitchOffset = 0;          // ピッチのオフセット
  float rollOffset = 0;           // ロールのオフセット
  float yawOffset = 0;            // ヨーのオフセット
  float gyroGain = 0;             // キャリブレーションが終わるまでは0

  while (1) {
    int8_t data;
    float accX;
    float accY;
    float accZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    float pitch;
    float roll;
    float yaw;

    M5.update(); // Buttonの状態更新のため必要

    if(M5.BtnA.wasPressed()){
      calibration = 0;
      calibrationCount = 0;
      gyroXSum = 0;
      gyroYSum = 0;
      gyroZSum = 0;
      gyroXOffset = 0;
      gyroYOffset = 0;
      gyroZOffset = 0;
      pitchSum = 0;
      rollSum = 0;
      yawSum = 0;
      pitchOffset = 0;
      rollOffset = 0;
      yawOffset = 0;
      gyroGain = 0;
    }

    // タイマー割り込みがあるまで待機する
    xQueueReceive(xQueue, &data, portMAX_DELAY);

    // 加速度、ジャイロ取得
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);

    // ジャイロ補正
    gyroX -= gyroXOffset;
    gyroY -= gyroYOffset;
    gyroZ -= gyroZOffset;

    // AHRS計算
    MahonyAHRSupdateIMU(gyroX * gyroGain, gyroY * gyroGain, gyroZ * gyroGain, accX, accY, accZ, &pitch, &roll, &yaw);

    // AHRS補正
    pitch -= pitchOffset;
    roll -= rollOffset;
    yaw -= yawOffset;

    // キャリブレーション
    if (calibration == 0) {
      // 最初の200個は読み捨てる
      calibrationCount++;
      if (200 <= calibrationCount) {
        calibration = 1;
        calibrationCount = 0;
      }
    } else if (calibration == 1) {
      // 一定時間データを取得してオフセットを計算する
      float gyro = abs(gyroX) + abs(gyroY) + abs(gyroZ);
      if (30 < gyro) {
        // 振動があった場合には再度キャリブレーション
        calibrationCount = 0;
        gyroXSum = 0;
        gyroYSum = 0;
        gyroZSum = 0;
        pitchSum = 0;
        rollSum = 0;
        yawSum = 0;     
        SerialBT.println("Calibration Init!!!!!");
      
      } else {
        // 累計を保存
        gyroXSum += gyroX;
        gyroYSum += gyroY;
        gyroZSum += gyroZ;
        pitchSum += pitch;
        rollSum += roll;
        yawSum += yaw;
        calibrationCount++;
        if (500 <= calibrationCount) {
          // 一定数溜まったらオフセット計算
          calibration = 2;
          SerialBT.println("Calibration Finish!!!!!");
          
          gyroXOffset = gyroXSum / calibrationCount;
          gyroYOffset = gyroYSum / calibrationCount;
          gyroZOffset = gyroZSum / calibrationCount;
          pitchOffset = pitchSum / calibrationCount;
          rollOffset = rollSum / calibrationCount;
          yawOffset = yawSum / calibrationCount;

          // 組み込みライブラリは25Hz動作なので実際のサンプリングレートとの比で調整する
          gyroGain = DEG_TO_RAD / (sampleFrequency / 25);
        }
      }
    } else {
        // 出力
        SerialBT.print(pitch);
        SerialBT.print("\t");
        SerialBT.print(roll);
        SerialBT.print("\t");
        SerialBT.println(yaw);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  M5.begin();
  M5.IMU.Init();

  M5.Lcd.setTextFont(1);
  M5.Lcd.setCursor(0, 0, 2); 
  M5.Lcd.println("BlueTooth Sample.");
  SerialBT.begin("M5StickCPlusBT");

  // キュー作成
  xQueue = xQueueCreate(1, sizeof(int8_t));

  // Core1の優先度5でタスク起動
  xTaskCreateUniversal(
    task,           // タスク関数
    "task",         // タスク名(あまり意味はない)
    8192,           // スタックサイズ
    NULL,           // 引数
    5,              // 優先度(大きい方が高い)
    &taskHandle,    // タスクハンドル
    APP_CPU_NUM     // 実行するCPU(PRO_CPU_NUM or APP_CPU_NUM)
  );

  // 4つあるタイマーの1つめを利用
  // 1マイクロ秒ごとにカウント(どの周波数でも)
  // true:カウントアップ
  timer = timerBegin(0, getApbFrequency() / 1000000, true);

  // タイマー割り込み設定
  timerAttachInterrupt(timer, &onTimer, true);

  // マイクロ秒単位でタイマーセット
  timerAlarmWrite(timer, 1000 * 1000 / sampleFrequency, true);

  // タイマー開始
  timerAlarmEnable(timer);
}

void loop() {
  delay(1);
}
