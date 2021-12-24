using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CollisionScript : MonoBehaviour
{
    void OnTriggerEnter(Collider t)
    {
        if (gameObject.name == "CheckPoint1")
        {
            Store.CheckPoint1 = true;
            if (Store.CheckPoint2)
            {
                Store.CheckPoint1 = false;
                Store.CheckPoint2 = false;
                Store.Cnt++;
            }
        }
        else
        {
            Store.CheckPoint2 = true;
        }
    }
}
