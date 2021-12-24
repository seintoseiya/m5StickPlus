using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RecieveSerial : MonoBehaviour
{
    //先ほど作成したクラス
    public SerialHandler serialHandler;
    public GameObject obj;

    void Start()
    {
        //信号を受信したときに、そのメッセージの処理を行う
        serialHandler.OnDataReceived += OnDataReceived;
    }

    void Update()
    {
        //文字列を送信
        //serialHandler.Write("hogehoge");
    }

    //受信した信号(message)に対する処理
    void OnDataReceived(string message)
    {
        var data = message.Split(
                new string[] { "\t" }, System.StringSplitOptions.None);
        //if (data.Length < 2) return;

        try
        {
            if(data[0].Contains("Calibration Init!!!!!"))
            {
                Debug.Log(data[0]);
            }
            else if (data[0].Contains("Calibration Finish!!!!!"))
            {
                Debug.Log(data[0]);
            }
            else
            {
                obj.transform.rotation = Quaternion.Euler(float.Parse(data[0]) * -1.0f, float.Parse(data[2]) * -1.0f, float.Parse(data[1]));
            }
        }
        catch (System.Exception e)
        {
            Debug.LogWarning(e.Message);
        }
    }
}