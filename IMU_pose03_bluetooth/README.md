# IMU_pose03_bluetooth

IMUを使った姿勢測定。BluetoothSerialに対応。M5ボタン押下でキャリブレーション。

System requirements
-------------------
- M5StickPlus

How to run
----------
- bluetoothの接続は、M5StickPlusをと以下のUnityAppもしくはSerialMonitorを起動した状態で `M5StickCPlusBT` というDeviceIdに接続（Macの場合、M5StickPlusを起動しただけではダメで、受け手のApp等を起動しないと接続できない）
- [/SampleUnityApp/M5StickPlus](https://github.com/seintoseiya/m5StickPlus/tree/main/IMU_pose03_bluetooth/SampleUnityApp/M5StickPlus) : 出力のサンプルとしてCubeが回転するだけのUnityApp
- M5StickPlusの液晶を上面右側に来る様にして設置して、M5ボタンを押下することでキャリブレーションを開始

TODO
----------
- SerialPlotterで確認するとわかりやすいが、yaw値の方向に回し続けると少しずつずれていく問題。。。
- M5StickPlusでは6軸センサーしかないので、9軸あるM5Stackで試す

Reference article
----------
- [M5StickCのIMU(AHRS)研究 その2 パラメータ調整](https://lang-ship.com/blog/work/m5stickc-imu-ahrs-2/)
- [M5StickCPlusとBluetooth接続でシリアル通信](https://modalsoul.hatenablog.com/entry/2021/01/13/070000)
