using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AI;
using LitJson;

public enum BossState
{
    Idle,
    Run,
    Attack,
    Die
}

public class Boss1 : MonoBehaviour
{
    public BossState BS;
    public float Hp;
    public float Speed;
    public float Damage;
    public TransportTCP transportTCP;
    public Animator Ani;
    public GameObject[] Users;
    public bool DieSend;

    [SerializeField]
    public BossData Data;

    // Start is called before the first frame update
    void Start()
    {
        transportTCP = GameObject.FindObjectOfType<TransportTCP>();
        Users = GameObject.FindGameObjectsWithTag("User");
        Ani = GetComponent<Animator>();
        Data.type = "BossState";
        Data.Damage = 10.0f;
        Data.RoomName = transportTCP.RoomName;
    }
    
    public void Move()
    {
       switch(Hp)
        {
            case 90:
                
                break;
        }
    }

    public void Pattern01()
    {
        
    }

    public void Pattern02()
    {

    }

    public void Pattern03()
    {

    }

    public void Animations()
    {
        if(BS == BossState.Die)
        {
            Ani.SetBool("Die", true);
           
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        if(other.gameObject.tag == "MyBullet")
        {
            if(Hp>0)
            {
                JsonData Dummy = JsonMapper.ToJson(Data);
                byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Dummy.ToString());
                transportTCP.Send(buffer, buffer.Length);
            }
            Destroy(other.gameObject);
        }
        if (other.gameObject.tag == "Bullet")
        {
            Destroy(other.gameObject);
        }
    }

   
    // Update is called once per frame
    void Update()
    {       
        if (Hp==0)
        {
            BS = BossState.Die;
            transportTCP.RoomPos.Clear();
        }
        else if(Hp>0)
        {
            Hp = transportTCP.Damage;
            Move();
        }
        
        Animations();
    }
}
