using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BossManager : MonoBehaviour
{
    public TransportTCP transportTCP;
    public GameObject[] Boss;
    public Transform[] BossSpawn;
    // Start is called before the first frame update
    void Start()
    {
        transportTCP = GameObject.FindObjectOfType<TransportTCP>();
    }

    public void Make()
    {
        if (transportTCP.BossMake == true)
        {
            Instantiate(Boss[transportTCP.BossNum], BossSpawn[transportTCP.BossPos].position, BossSpawn[transportTCP.BossPos].rotation);
            transportTCP.BossMake = false;
        }
    }
    // Update is called once per frame
    void Update()
    {
        Make();
    }
}
