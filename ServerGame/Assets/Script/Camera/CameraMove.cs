using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraMove : MonoBehaviour
{
    [Range(0, 300)]
    public float SpeedX;
    [Range(0, 300)]
    public float SpeedY;

    public Vector3 Rot;
    public float RotationX;
    public float RotationY;

    Vector3 Move;
    float MoveX;
    float MoveY;
    public float MoveSpeed;

    public float Distance;
    public float Height;
    public Player player;
    public GameObject Target;
    Vector3 PlayerLock;
    Vector3 CameraPos;
    

    public void MoveUpdate()
    {
        RotationX = Input.GetAxis("Mouse X") * SpeedX;
        RotationY = Input.GetAxis("Mouse Y") * SpeedY;

        Target.transform.Rotate(0, RotationX, 0);
    }

    // Update is called once per frame
    void Update()
    {
        MoveUpdate();
    }
}
