using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using LitJson;
public class ServerPlayer : MonoBehaviour
{

    public PlayerState PS;
    public string ID;
    [SerializeField]
    public MoveKey PlayerKey;

    [SerializeField]
    public MoveCamera PlayerRot;

    [SerializeField]
    public InputKey Keys;

    public float Speed;
    public float MinSpeed;
    public float MaxSpeed;

    public Vector3 move;
    public Vector3 moveset;
    public Vector3 UpdatePos;
    Vector3 Rot;

    public Animator Ani;
    public bool Sent;

    public TransportTCP transportTCP;

    public float RotationX;
    public float SpeedX;
    public float SpeedY;

    public Text text;

    //행동 관련 함수
    bool NowMove;
    bool AttackOn;

    //총알 관련 함수
    public GameObject Bullet;
    public GameObject ShotPoint;

    private void Start()
    {
        Speed = MinSpeed;
        transportTCP = GameObject.FindObjectOfType<TransportTCP>();
        Ani = GetComponent<Animator>();
        text = GameObject.Find("SeverPlayer").GetComponent<Text>();
    }

    public void Move()
    {
        byte[] buffer = new byte[2048];

        int recvSize = transportTCP.Receive(ref buffer, buffer.Length);
        if (recvSize > 0)
        {
            string message = System.Text.Encoding.UTF8.GetString(buffer);   //버퍼에 있는 데이터를 가져온다
            string[] DummyMessage = message.Split('\0');
          
            JsonData Message = JsonMapper.ToObject(DummyMessage[0]);
            if (Message["type"].ToString() == "Move")
            {
                if (transportTCP.serverplayers.ContainsKey(Message["ID"].ToString()))
                {
                    ServerPlayer Info = transportTCP.serverplayers[Message["ID"].ToString()];
                    Info.move.x = float.Parse(Message["x"].ToString());
                    Info.move.z = float.Parse(Message["z"].ToString());
                    
                    if(Info.move.x==0 && Info.move.z==0)
                    {
                        moveset.x = float.Parse(Message["nx"].ToString());
                        moveset.y = transform.position.y;
                        moveset.z = float.Parse(Message["nz"].ToString());
                    }

                }
            }

            if (Message["type"].ToString() == "NowMove")
            {
                if (transportTCP.serverplayers.ContainsKey(Message["ID"].ToString()))
                {
                    PlayerRot.y = float.Parse(Message["Rot"].ToString());
                    UpdatePos.x = float.Parse(Message["nx"].ToString());
                    UpdatePos.z = float.Parse(Message["nz"].ToString());

                    if(Vector3.Distance(transform.position,UpdatePos)>=1.0f)
                    {
                        transform.position = Vector3.Lerp(transform.position, UpdatePos, Speed * Time.deltaTime);
                    }
                }
            }
            
            if(Message["type"].ToString()=="KeyDown")
            {
                if (transportTCP.serverplayers.ContainsKey(Message["ID"].ToString()))
                {
                    Keys.Run = bool.Parse(Message["Run"].ToString());
                    Keys.Attack = bool.Parse(Message["Attack"].ToString());
                }
            }
        }

        if(move.x == 0 && move.z == 0)
        {
            PS = PlayerState.Idle;
        }

        if (move.x != 0 || move.z != 0)
        {
            PS = PlayerState.Walk;
        }

      

        transform.Translate(move * Speed * Time.deltaTime);
        UpdatePos.y = transform.position.y;
        text.text = "X :" + transform.position.x.ToString() + "Z :" + transform.position.z.ToString();

        //transform.position = Vector3.Lerp(transform.position, UpdatePos, Speed * Time.deltaTime);
        transform.rotation = Quaternion.Slerp(transform.rotation, Quaternion.Euler(transform.eulerAngles.x,
             (float)PlayerRot.y, transform.eulerAngles.z),10.0f*Time.deltaTime);
    }

    public void KeyCheck()
    {

        if (Keys.Run == true)
        {
            Speed = MaxSpeed;
        }
        else if (Keys.Run == false)
        {
            Speed = MinSpeed;
        }

        if(Keys.Attack==true)
        {
            if(AttackOn ==false)
            {
                AttackOn = true;
                StartCoroutine(Shot());
            }
          
        }
        if(Keys.Attack==false)
        {
            AttackOn = false;
        }
    }


    IEnumerator Shot()
    {
        AttackOn = true;
        Instantiate(Bullet, ShotPoint.transform.position, transform.rotation);
        yield return new WaitForSeconds(0.15f);
        AttackOn = false;
    }

    public void Animations()
    {
        if (PS == PlayerState.Idle)
        {
            Ani.SetBool("Walk", false);
            Ani.SetBool("Idle", true);
        }
        if (PS == PlayerState.Walk)
        {
            Ani.SetBool("Idle", false);
            Ani.SetBool("Walk", true);
        }
    }

   
    private void Update()
    {
        if(PS!=PlayerState.Die)
        {
            Move();
            Animations();
            KeyCheck();
        }
    }
}
