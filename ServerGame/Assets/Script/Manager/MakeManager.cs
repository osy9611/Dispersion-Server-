using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MakeManager : MonoBehaviour
{
    public TransportTCP transportTCP;
    public GameObject[] SpawnPoint;

    private void Start()
    {
        transportTCP = GameObject.FindObjectOfType<TransportTCP>();
        int Num = 0;
        if (transportTCP!=null)
        {
            foreach (KeyValuePair<string, PlayerInfo> pair in transportTCP.playerInfo)
            {
                PlayerInfo Info = pair.Value;
                if (Info.Charactor == null)
                {
                    if (Info.ID == transportTCP.MyInfo.ID)
                    {
                        for(int i =0;i< transportTCP.RoomPos.Count;i++)
                        {
                            if(Info.ID == transportTCP.RoomPos[i].ID)
                            {
                                Num = i;
                            }
                        }
                        GameObject Dummy = Instantiate(transportTCP.Charactors[0], SpawnPoint[Num].transform.position, Quaternion.identity) as GameObject;
                        //Dummy.transform.position = new Vector3((float)Info.x, 0, (float)Info.z);
                        //Info.Charactor.transform.position = transform.position;
                        Info.Charactor = Dummy;
                    }
                }
                
            }
        }
        CreateUser();
    }

    public void CreateUser()
    {
        int Num = 0;
        foreach (KeyValuePair<string, PlayerInfo> pair in transportTCP.InGamePlayers)
        {
            transportTCP.CreateChar = false;
            PlayerInfo Info = pair.Value;
            if (Info.Charactor == null)
            {
                if (Info.ID != transportTCP.MyInfo.ID)
                {
                    for (int i = 0; i < transportTCP.RoomPos.Count; i++)
                    {
                        if (Info.ID == transportTCP.RoomPos[i].ID)
                        {
                            Num = i;
                        }
                    }
                    Info.Charactor = Instantiate(transportTCP.Charactors[1], SpawnPoint[Num].transform.position, Quaternion.identity) as GameObject;                   
                    ServerPlayer server = Info.Charactor.GetComponent<ServerPlayer>();
                    server.ID = Info.ID;
                    transportTCP.serverplayers.Add(Info.ID, server);
                }
            }
        }
        
    }
}
