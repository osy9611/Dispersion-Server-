using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Bullet : MonoBehaviour
{
    public float Speed;
    public float Power;
    public float Life = 2f;


    // Update is called once per frame
    void Update()
    {
        Life -= Time.deltaTime;
        if(Life<=0f)
        {
            Destroy(this.gameObject);
        }
        transform.Translate(Vector3.forward * Speed * Time.deltaTime);
    }
}
