using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using LitJson;

public class RoomManager : MonoBehaviour
{
    public Button RoomButton;
    public Transform MakePos;
    public GameObject SetRoomButton;
    public TransportTCP transportTCP;

    public InputField RoomName;

    public List<GameObject> Rooms;

    public Text CountNum;
    public int Count;

    public RoomData Data;
    public EnterData enterData;
    public ReadyCheck ready;

    public Canvas canvas;
    private GraphicRaycaster gr;
    private PointerEventData ped;

    public Text[] UserName;
    public Text[] Readys;
    public int EnterCount;
    public GameObject ReadyRoom;

    public bool StartGame;
    private void Start()
    {
        transportTCP = GameObject.FindObjectOfType<TransportTCP>();
       
        Data.type = "RoomInfo";
        Data.ID = transportTCP.MyInfo.ID;

        enterData.type = "EnterRoomInfo";
        enterData.ID = transportTCP.MyInfo.ID;

        ready.type = "ReadyMessage";
        ready.ID = transportTCP.MyInfo.ID;

        gr = canvas.GetComponent<GraphicRaycaster>();
        ped = new PointerEventData(null);   
    }

    public void MakeRoom()
    {
        if (SetRoomButton.activeSelf == false)
        {
            SetRoomButton.SetActive(true);
        }
    }

    public void MakeDone()
    {
        if(Count>0)
        {
            Data.RoomName = RoomName.text;
            ready.RoomName = RoomName.text;
            Data.Total = Count;
            JsonData data = JsonMapper.ToJson(Data);
            byte[] buffer = System.Text.Encoding.UTF8.GetBytes(data.ToString());
            transportTCP.Send(buffer, buffer.Length);
            SetRoomButton.SetActive(false);
            ReadyRoom.SetActive(true);
            UserName[EnterCount].text = transportTCP.MyInfo.ID;
            EnterCount++;
        }
    }

    public void ReadyOn()
    {
        if(ready.Ready==false)
        {
            ready.Ready = true;
            JsonData data = JsonMapper.ToJson(ready);
            byte[] buffer = System.Text.Encoding.UTF8.GetBytes(data.ToString());
            transportTCP.Send(buffer, buffer.Length);

            for(int i=0;i<UserName.Length;i++)
            {
                if(UserName[i].text == Data.ID)
                {
                    Readys[i].text = "Ready";
                }
            }
        }
    }

    public void CountUp()
    {
        if(Count<4)
        {
            Count += 1;
            CountNum.text = Count.ToString();
        }
    }

    public void CountDonw()
    {
        if(Count>0)
        {
            Count -= 1;
            CountNum.text = Count.ToString();
        }
    }
 
    public void MakeOtherRoom(string RoomName,string UserName)
    {
        Vector3 Pos = new Vector3(RoomButton.transform.position.x, RoomButton.transform.position.y - (28 * Rooms.Count), RoomButton.transform.position.z);
        GameObject obj = Instantiate(RoomButton.gameObject, Pos, Quaternion.identity);
        obj.GetComponent<Button>().transform.GetChild(0).GetComponent<Text>().text = RoomName;
        obj.GetComponent<Button>().transform.GetChild(1).GetComponent<Text>().text = UserName;
        obj.transform.parent = MakePos;
        obj.SetActive(true);
        Rooms.Add(obj);
    }

    public void EnterRoom()
    {
        ped.position = Input.mousePosition;
        List<RaycastResult> results = new List<RaycastResult>();
        gr.Raycast(ped, results);
        if(results.Count !=0)
        {
            GameObject obj = results[0].gameObject;
            if (obj.CompareTag("Room"))
            {
                UserName[0].text = transportTCP.MyInfo.ID;
                EnterCount++;
                enterData.RoomName = obj.GetComponent<Button>().transform.GetChild(0).GetComponent<Text>().text;
                ready.RoomName = enterData.RoomName;
                JsonData data = JsonMapper.ToJson(enterData);
                byte[] buffer = System.Text.Encoding.UTF8.GetBytes(data.ToString());
                transportTCP.Send(buffer, buffer.Length);               
            }
        }
    }
    
    public void CancelRoom()
    {
        SetRoomButton.SetActive(false);
    }


}
