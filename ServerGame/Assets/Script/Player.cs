using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using LitJson;
public enum PlayerState
{
    Idle,
    Walk,
    Run,
    Attack,
    Die
}



public class Player : MonoBehaviour
{
    public PlayerState PS;

    public MoveKey PlayerKey;
    public NowPosition NowPos;
    public MoveCamera PlayerRot;
    public InputKey Keys;
    public float Speed;
    public float MinSpeed;
    public float MaxSpeed;

    public Vector3 move;
    Vector3 Rot;
    float oldRot;
    public Animator Ani;
    public bool Sent;

    public TransportTCP transportTCP;

    public CameraMove cameraMove;

    public bool RotStop =true;
    public bool MoveSend = false;
    public float Dummyx, Dummyz, Dummyy;

    float timeSpan; //경과 시간을 갖는 변수
    float checkTime;

    float deltaTIme = 0.5f;
    float sendtime;

    //총알 관련 함수
    public GameObject Bullet;
    public GameObject ShotPoint;
    bool AttackOn;

    public Text text;

    void Start()
    {
        checkTime = 0.1f;

        transportTCP = GameObject.FindObjectOfType<TransportTCP>();
        Ani = GetComponent<Animator>();

        PlayerKey.type = "Move";
        PlayerKey.ID = transportTCP.MyInfo.ID;
        PlayerKey.RoomName = transportTCP.RoomName;

        PlayerRot.type = "Rotation";
        PlayerRot.ID = transportTCP.MyInfo.ID;
        PlayerRot.RoomName = transportTCP.RoomName;

        NowPos.type = "NowMove";
        NowPos.ID = transportTCP.MyInfo.ID;
        NowPos.RoomName = transportTCP.RoomName;

        Keys.type = "KeyDown";
        Keys.ID = transportTCP.MyInfo.ID;
        Keys.RoomName = transportTCP.RoomName;

        text = GameObject.Find("Player").GetComponent<Text>();
    }

    public void Move()
    {
       
        move.x = Input.GetAxisRaw("Horizontal");
        move.z = Input.GetAxisRaw("Vertical");
        PlayerKey.x = move.x;
        PlayerKey.z = move.z;
        PlayerKey.nx = transform.position.x;
        PlayerKey.ny = transform.position.y;
        PlayerKey.nz = transform.position.z;

        NowPos.nx = transform.position.x;
        NowPos.ny = transform.position.y;
        NowPos.nz = transform.position.z;
        NowPos.Rot = transform.eulerAngles.y;

        if (Input.GetKey(KeyCode.W)|| Input.GetKey(KeyCode.S) || Input.GetKey(KeyCode.A) || Input.GetKey(KeyCode.D))
        {
            if(PlayerRot.stop == 1)
            {
                PlayerRot.stop = 0;
            }
            MoveSend = true;
            PS = PlayerState.Walk;
            Speed = MinSpeed;


            if (Input.GetKey(KeyCode.LeftShift))
            {
                Keys.Run = true;
                Speed = MaxSpeed;
            }
        }

        if (move.x==0&&move.z==0&&PS!=PlayerState.Idle)
        {
            PlayerRot.stop = 1;
            Speed = 0;
            PS = PlayerState.Idle;
        }

        if(Input.GetMouseButton(0))
        {
            if(AttackOn==false)
            {
                StartCoroutine(Shot());
            }
           
        }
        transform.Translate(move * Speed * Time.deltaTime);
        text.text = "X :" + transform.position.x.ToString() + "Z :" + transform.position.z.ToString();
    }

    public void MoveCheck()
    {
        string[] nowtime = System.DateTime.Now.ToString("HH:mm:ss").Split(':');
        NowPos.NowTime = (float.Parse(nowtime[0]) * 60 * 60) +
            (float.Parse(nowtime[1]) * 60) +
            float.Parse(nowtime[2]);

        if (Input.GetKeyDown(KeyCode.W) || Input.GetKeyDown(KeyCode.S) 
            || Input.GetKeyDown(KeyCode.A) || Input.GetKeyDown(KeyCode.D))
        {
            JsonData Data = JsonMapper.ToJson(PlayerKey);
            byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Data.ToString());
            transportTCP.Send(buffer, buffer.Length);            
        }
        if (Input.GetKeyUp(KeyCode.W) || Input.GetKeyUp(KeyCode.S)
            || Input.GetKeyUp(KeyCode.A) || Input.GetKeyUp(KeyCode.D))
        {
            JsonData Data = JsonMapper.ToJson(PlayerKey);
            byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Data.ToString());
            transportTCP.Send(buffer, buffer.Length);
            MoveSend = false;
        }

        if (Input.GetKeyDown(KeyCode.LeftShift))
        {            
            Keys.Run = true;
            Speed = MaxSpeed;
            SendKey();
        }

        if (Input.GetKeyUp(KeyCode.LeftShift))
        {
            Keys.Run = false;
            Speed = MaxSpeed;
            SendKey();
        }

        if(Input.GetMouseButtonDown(0))
        {
            Keys.Attack = true;
            SendKey();
        }

        if (Input.GetMouseButtonUp(0))
        {
            Keys.Attack = false;
            SendKey();
        }

        timeSpan += Time.deltaTime;
       
        //캐릭터가 이동하면 매 시간마다 이동 위치를 계산한다
        if (timeSpan > checkTime)
        {
            if (MoveSend == true)
            {
                JsonData Data = JsonMapper.ToJson(NowPos);
                byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Data.ToString());
                transportTCP.Send(buffer, buffer.Length);
                timeSpan = 0;
            }
        }
    }

    public void RotCheck()
    {
        if(move.x==0&&move.z==0)
        {
            if (cameraMove.RotationX != 0)
            {
                MoveSend = true;
            }
            if (cameraMove.RotationX == 0)
            {
                MoveSend = false;   
            }
        }
       
    }

    public void SendKey()
    {
        JsonData Data = JsonMapper.ToJson(Keys);
        byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Data.ToString());        
        transportTCP.Send(buffer, buffer.Length);
    }
    
    public void Animations()
    {
        if(PS == PlayerState.Idle)
        {
            Ani.SetBool("Walk", false);
            Ani.SetBool("Idle", true);
        }
        if(PS==PlayerState.Walk)
        {
            Ani.SetBool("Idle", false);
            Ani.SetBool("Walk", true);
        }
    }


    IEnumerator Shot()
    {
        AttackOn = true;
        Instantiate(Bullet, ShotPoint.transform.position, transform.rotation);
        yield return new WaitForSeconds(0.15f);
        AttackOn = false;
    }

    /*가급적이면 Update 문을 사용하자 
     FixedUpdate는 통신할때 전송이 불규칙적으로 전송되는것을 발견*/
    void Update()
    {
        if (PS != PlayerState.Die)
        {
            Move();
            MoveCheck();
            RotCheck();

            Animations();
        }
    }
}
