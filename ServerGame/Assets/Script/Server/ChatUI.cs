using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using UnityEngine.UI;
using LitJson;
using System.Text;
using UnityEngine.SceneManagement;

public enum ChatState
{
    NOT_CONNECTED = 0,   //시작상태(접속 안됨)
    CONNECTED,          //접속됨
    CLOSING,            //접속 종료
    ERROR,              //오류 상태
}

public struct Chatt
{
    public string type;
    public string Message;
}

public class ChatUI : MonoBehaviour
{
    public TransportTCP m_transport;
    
    private string m_sendComment = "";
    private List<string>[] m_message;

    private static int MESSAGE_LINE = 18;
    private static int CHAT_MEMBER_NUM = 2;

    public InputField ChatField;
    public Text ErrorMessage;   //에러 메시지를 사용하기 위함 함수
    public Text ChatMessage;    //채팅 문장을 표시
    public ScrollRect Scroll;
    private bool isSent = false;    //채팅을 보내기위함

    [SerializeField]
    public List<string> m;

    Chatt chatt;    //채팅 관련 구조체

    public RoomManager roomManager;
    private void Start()
    {
        m_transport = GameObject.FindObjectOfType<TransportTCP>();
        m_transport.RegisterEventHandler(OnEventHandling);

        m_message = new List<string>[CHAT_MEMBER_NUM];
        for (int i = 0; i < CHAT_MEMBER_NUM; ++i)
        {
            m_message[i] = new List<string>();
        }
    }

    private void Update()
    {
        switch (m_transport.m_state)
        {
            case ChatState.NOT_CONNECTED:
                for (int i = 0; i < CHAT_MEMBER_NUM; ++i)
                {
                    m_message[i].Clear();
                }
                break;
            case ChatState.CONNECTED:
                {
                    UpdateChatting();
                    SendChatting();
                }
                break;
            case ChatState.CLOSING:
                {
                    UpdateLeave();
                }
                break;
            case ChatState.ERROR:
                {
                    Error();
                }
                break;
        }
    }

    //데이터를 받고 패팅에 갱신해줌
    void UpdateChatting()
    {
        byte[] buffer = new byte[2048];

        int recvSize = m_transport.Receive(ref buffer, buffer.Length);

        if (recvSize > 0)
        {
            string message = System.Text.Encoding.UTF8.GetString(buffer);   //버퍼에 있는 데이터를 가져온다
            string[] DummyMessage = message.Split('\0');
            JsonData Message = JsonMapper.ToObject(DummyMessage[0]);
            Debug.Log(Message["type"].ToString());
            if (Message["type"].ToString() == "ChattMessage")
            {
                ChatMessage.text += Message["Message"].ToString() + "\n";
                Scroll.verticalNormalizedPosition = 1.0f;
                AddMessage(ref m_message[1], message);
            }
            if (Message["type"].ToString() == "EnterMessage")
            {
                ChatMessage.text += Message["Message"].ToString() + "\n";
                Scroll.verticalNormalizedPosition = 1.0f;
                AddMessage(ref m_message[1], message);
            }
            if (Message["type"].ToString() == "RoomInfo")
            {
                string RoomName = Message["RoomName"].ToString();
                string UserName = Message["ID"].ToString();
                roomManager.MakeOtherRoom(RoomName, UserName);
            }
            if (Message["type"].ToString() == "EnterUser")
            {
                if (roomManager.ReadyRoom.activeSelf == false)
                {
                    roomManager.ReadyRoom.SetActive(true);
                }


                if (Message["ID"].ToString() != m_transport.MyInfo.ID)
                {
                    roomManager.UserName[roomManager.EnterCount].text = Message["ID"].ToString();
                    roomManager.EnterCount++;
                }
            }

            if (Message["type"].ToString() == "DestroyRoom")
            {
                if (m_transport.playerInfo.ContainsKey(Message["ID"].ToString()))
                {
                    m_transport.InGamePlayers.Remove(Message["ID"].ToString());
                    m_transport.playerInfo.Remove(Message["ID"].ToString());
                }
                for (int i = 0; i < roomManager.Rooms.Count; i++)
                {
                    if (roomManager.Rooms[i].GetComponent<Button>()
                        .transform.GetChild(0).GetComponent<Text>().text == Message["RoomName"].ToString())
                    {
                        Destroy(roomManager.Rooms[i]);
                        roomManager.Rooms.RemoveAt(i);
                        for (int j = 0; j < roomManager.Rooms.Count; j++)
                        {
                            roomManager.Rooms[j].transform.position = new Vector3(roomManager.Rooms[i].transform.position.x,
                            roomManager.Rooms[j].transform.position.y + 28f
                          , roomManager.Rooms[j].transform.position.z);
                        }
                    }
                }
            }
            if (Message["type"].ToString() == "DestroyName")
            {
                if (m_transport.playerInfo.ContainsKey(Message["ID"].ToString()))
                {
                    m_transport.InGamePlayers.Remove(Message["ID"].ToString());
                    m_transport.playerInfo.Remove(Message["ID"].ToString());
                }
                for (int i = 0; i < roomManager.UserName.Length; i++)
                {
                    if (roomManager.UserName[i].text == Message["ID"].ToString())
                    {
                        roomManager.UserName[i].text = "";
                        roomManager.Readys[i].text = "";
                        roomManager.EnterCount--;
                    }
                }
            }

            if (Message["type"].ToString() == "ReadyMessage")
            {
                Debug.Log("일단 들어옴");
                for (int i = 0; i < roomManager.EnterCount; i++)
                {
                    if (roomManager.UserName[i].text == Message["ID"].ToString())
                    {
                        roomManager.Readys[i].text = "Ready";
                    }
                }
            }

            if (Message["type"].ToString() == "SetPosition")
            {
                if (Message["On"].ToString() == "Start")
                {
                    roomManager.StartGame = true;
                    SceneManager.LoadScene("Demo 1");
                }
            }
        }
    }

    //채팅의 상태가 닫혀있으면 메시지를 삭제한다
    void UpdateLeave()
    {
        m_transport.Disconnect();
        //메시지 삭제
        for (int i = 0; i < 2; ++i)
        {
            m_message[i].Clear();
        }
        m_transport.m_state = ChatState.NOT_CONNECTED;
    }
    
    void SendChatting()
    {
        if (Input.GetKeyDown(KeyCode.Return))
        {
            if (ChatField.gameObject.activeSelf == true)
            {
                isSent = true; m_sendComment = ChatField.text;
                string message = "[" + DateTime.Now.ToString("HH:mm:ss") + "] "+m_transport.MyInfo.ID+" : " + m_sendComment;
                chatt.type = "ChattMessage";
                chatt.Message = message;
                JsonData Data = JsonMapper.ToJson(chatt);
                byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Data.ToString());
                m_transport.Send(buffer, buffer.Length);
            }
            else if (ChatField.gameObject.activeSelf == false)
            {
                ChatField.gameObject.SetActive(true);
            }
        }


    }

    public void ExitChatting()
    {
        m_transport.m_state = ChatState.CLOSING;
    }

    void Error()
    {
        ErrorMessage.gameObject.SetActive(true);
    }
    void AddMessage(ref List<string> messages, string str)
    {
        while (messages.Count >= MESSAGE_LINE)
        {
            messages.RemoveAt(0);
            m.RemoveAt(0);
        }
        messages.Add(str);

        m = messages;
    }

    private void OnApplicationQuit()
    {
        if (m_transport != null)
        {
            m_transport.StopServer();
        }
    }

    public void OnEventHandling(NetEventState state)
    {
        switch (state.type)
        {
            case NetEventType.Connect:
                break;
            case NetEventType.Disconnect:
                break;
        }

    }
}
