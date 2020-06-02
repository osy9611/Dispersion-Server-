using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using LitJson;

public class Boss2 : MonoBehaviour
{
    public BossState BS;
    public float Hp;
    public float Speed;
    public float Damage;
    public TransportTCP transportTCP;
    public Animator Ani;
    public GameObject[] Users;
    public bool DieSend;
    BossData Data;
    // Start is called before the first frame update
    void Start()
    {
        transportTCP = GameObject.FindObjectOfType<TransportTCP>();
        Users = GameObject.FindGameObjectsWithTag("User");
        Ani = GetComponent<Animator>();
        Data.type = "BossState";
    }
    public void Move()
    {

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
        if (BS == BossState.Die)
        {
            Ani.SetBool("Die", true);
           
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.gameObject.tag == "Bullet")
        {
            if (Hp > 0)
            {
                Hp -= 10;
            }
            Destroy(other.gameObject);
        }
    }


    // Update is called once per frame
    void Update()
    {       
        if (Hp == 0)
        {
            if (DieSend == false)
            {
                DieSend = true;
                JsonData data = JsonMapper.ToJson(Data);
                byte[] buffer = System.Text.Encoding.UTF8.GetBytes(data.ToString());
                transportTCP.Send(buffer, buffer.Length);
            }
          
            BS = BossState.Die;
        }
        else if (Hp > 0)
        {
            Move();
        }
        Animations();
    }
}
