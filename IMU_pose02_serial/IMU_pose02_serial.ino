#include <M5StickCPlus.h>
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
        Serial.printf("Calibration Init!!!!! %f\n", gyro);
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
          Serial.printf("Calibration Finish!!!!!\n");
          gyroXOffset = gyroXSum / calibrationCount;
          gyroYOffset = gyroYSum / calibrationCount;
          gyroZOffset = gyroZSum / calibrationCount;
          pitchOffset = pitchSum / calibrationCount;
          rollOffset = rollSum / calibrationCount;
          yawOffset = yawSum / calibrationCount;
//          Serial.printf(" gyroXSum = %f\n", gyroXSum);
//          Serial.printf(" gyroYSum = %f\n", gyroYSum);
//          Serial.printf(" gyroZSum = %f\n", gyroZSum);
//          Serial.printf(" gyroXOffset = %f\n", gyroXOffset);
//          Serial.printf(" gyroYOffset = %f\n", gyroYOffset);
//          Serial.printf(" gyroZOffset = %f\n", gyroZOffset);
//          Serial.printf(" pitchSum = %f\n", pitchSum);
//          Serial.printf(" rollSum = %f\n", rollSum);
//          Serial.printf(" yawSum = %f\n", yawSum);
//          Serial.printf(" pitchOffset = %f\n", pitchOffset);
//          Serial.printf(" rollOffset = %f\n", rollOffset);
//          Serial.printf(" yawOffset = %f\n", yawOffset);

          // 組み込みライブラリは25Hz動作なので実際のサンプリングレートとの比で調整する
          gyroGain = DEG_TO_RAD / (sampleFrequency / 25);
        }
      }
    } else {
      // 出力
//      Serial.printf(" %f,%f,%f\n", pitch, roll, yaw);
      Serial.print(pitch);
      Serial.print("\t");
      Serial.print(roll);
      Serial.print("\t");
      Serial.println(yaw);
      Serial.flush();
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  M5.begin();
  M5.IMU.Init();

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
