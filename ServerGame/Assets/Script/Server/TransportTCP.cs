using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using LitJson;
using System.Text;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using System.Text;

public class TransportTCP : MonoBehaviour
{
    /*
     * 소켓 접속 관련
     */

    //리스닝 소켓
    private Socket m_listner = null;

    //클라이언트와의 접속용 소켓
    private Socket m_socket = null;

    //송신 버퍼
    private PacketQueue m_sendQueue;

    //수신 버퍼
    private PacketQueue m_recvQueue;

    //접속 플래그
    private bool m_isConnected = false;

    /*
     * 이벤트 관련 맴버 변수
     */

    //이벤트 통지의 델리게이트
    public delegate void EventHandler(NetEventState state);

    private EventHandler m_handler;

    /*
     * 스레드 관련 멤버 변수
     */

    //스레드 실행 플레그
    protected bool m_threadLoop = false;
    protected Thread m_thread = null;
    private static int s_mtu = 1024;

    //접속 관련 변수
    public ChatState m_state = ChatState.NOT_CONNECTED;

    public string m_hostAddress;
    public int m_port = 9000;
    public string StoreMessage;

    public MyData MyInfo;
    public MyData SignInfo;

    //DB에서 로그인 정보가 일치하면 로그온이 됨을 알려줌
    public bool LogIn;
    [SerializeField]
    public Dictionary<string, PlayerInfo> playerInfo;   //서버에 접속한 플레이어 정보들
    public Dictionary<string, ServerPlayer> serverplayers;  //인게임에서 사용할것
    public Dictionary<string, PlayerInfo> InGamePlayers;    //대기 방에서 사용할것

    //캐릭터 오브젝트들
    public GameObject[] Charactors;

    //캐릭터를 만들기
    public bool CreateChar;

    //보스 관련
    public bool BossMake;
    public int  BossNum;
    public int  BossPos;
    public string SeverMesssage;

    //룸 이름과 스폰 위치
    public string RoomName;

    [SerializeField]
    public List<SpawnPoint> RoomPos;

    public float Damage;

    private void Start()
    {
        MyInfo.type = "UserInfo";
        SignInfo.type = "SignInfo";

        //송수신 버퍼를 작성한다
        m_sendQueue = new PacketQueue();
        m_recvQueue = new PacketQueue();
        playerInfo = new Dictionary<string, PlayerInfo>();
        serverplayers = new Dictionary<string, ServerPlayer>();
        InGamePlayers = new Dictionary<string, PlayerInfo>();
        DontDestroyOnLoad(gameObject);
    }

   
    //대기 종료
    public void StopServer()
    {
        m_threadLoop = false;
        if (m_thread != null)
        {
            m_thread.Join();
            m_thread = null;
        }

        Disconnect();

        if (m_listner != null)
        {
            m_listner.Close();
            m_listner = null;
        }
    }

    public void ConnectOnline()
    {
        if (m_state != ChatState.CONNECTED)
        {
            if(!m_isConnected)
            {
                bool ret = Connect(m_hostAddress, m_port);
                if (ret)
                {
                    m_state = ChatState.CONNECTED;
                    //연결이 되면 씬을 넘기고 유저 데이터를 서버에 넘겨준다
                    //SceneManager.LoadScene("Demo 1");
                    JsonData Data = JsonMapper.ToJson(MyInfo);
                    byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Data.ToString());
                    Send(buffer, buffer.Length);
                }
                else
                {
                    m_state = ChatState.ERROR;
                }
            }
            else
            {
                m_state = ChatState.CONNECTED;
                //연결이 되면 씬을 넘기고 유저 데이터를 서버에 넘겨준다
                //SceneManager.LoadScene("Demo 1");
                JsonData Data = JsonMapper.ToJson(MyInfo);
                byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Data.ToString());
                Send(buffer, buffer.Length);
            }
            
        }
    }

    public void ConnectSignUp()
    {
        if (m_state != ChatState.CONNECTED)
        {
            if (!m_isConnected)
            {
                bool ret = Connect(m_hostAddress, m_port);

                if (ret)
                {
                    m_state = ChatState.CONNECTED;
                    if (SignInfo.ID == "" && SignInfo.Password == "")
                    {
                        SeverMesssage = "EmptyInfo";
                    }
                    else
                    {
                        JsonData Data = JsonMapper.ToJson(SignInfo);
                        byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Data.ToString());
                        Send(buffer, buffer.Length);
                    }
                }
                else
                {
                    m_state = ChatState.ERROR;
                }
            }
            else
            {
                m_state = ChatState.CONNECTED;
                if (SignInfo.ID == "" && SignInfo.Password == "")
                {
                    SeverMesssage = "EmptyInfo";
                }
                else
                {
                    JsonData Data = JsonMapper.ToJson(SignInfo);
                    byte[] buffer = System.Text.Encoding.UTF8.GetBytes(Data.ToString());
                    Send(buffer, buffer.Length);
                }
            }
        }
    }
    

    //연결이 되면 아이디와 패스워드를 받아온다
    public void StoreInfoID(Text text)
    {
        MyInfo.ID = text.text;
    }
    public void StoreInfoPassword(Text text)
    {
        MyInfo.Password = text.text;
    }

    public void StoreInfoSignID(Text text)
    {
        SignInfo.ID = text.text;
    }
    public void StoreInfoSignPassword(Text text)
    {
        SignInfo.Password = text.text;
    }

    public bool Connect(string address,int port)
    {
        Debug.Log("TransportTCp connect called.");
        if (m_listner !=null)
        {
            return false;
        }

        bool ret = false;


        try
        {
            //소켓 객체를 생성한다(TCP 소켓)
            m_socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            //딜레이 없이 진행할것인지를 물어보는 용도
            m_socket.NoDelay = true;
            //서버에 연결한다
            m_socket.Connect(address, port);
            ret = LaunchThread();
        }
        catch
        {
            m_socket = null;
        }

        if (ret == true)
        {
            m_isConnected = true;
            Debug.Log("Connection success.");
        }
        else
        {
            m_isConnected = false;
            Debug.Log("Connect fail");
        }

        if(m_handler !=null)
        {
            //접속 결과를 통지 
            NetEventState state = new NetEventState();
            state.type = NetEventType.Connect;
            state.result = (m_isConnected == true) ? NetEventResult.Success : NetEventResult.Failure;
            m_handler(state);
            Debug.Log("Evnet Handler Called");
        }

        return m_isConnected;
    }

    //close
    public void Disconnect()
    {
        m_isConnected = false;

        if (m_socket != null)
        {
            //소켓 클로즈
            m_socket.Shutdown(SocketShutdown.Both);  //소켓에서 보내기 및 받기를 사용할 수 없도록 설정한다
            m_socket.Close();
            m_socket = null;
        }

        //끊김을 통지
        if (m_handler != null)
        {
            NetEventState state = new NetEventState();
            state.type = NetEventType.Disconnect;
            state.result = NetEventResult.Success;
            m_handler(state);
        }
    }

    //송신 처리
    public int Send(byte[] data, int size)
    {
        if (m_sendQueue == null)
        {
            return 0;
        }
        return m_sendQueue.Enqueue(data, size);
        //m_socket.Send(data, size, SocketFlags.None);
    }

    //수신 처리
    public int Receive(ref byte[] buffer, int size)
    {
        if (m_recvQueue == null)
        {
            return 0;
        }

        return m_recvQueue.Dequeue(ref buffer, size);
    }

    //이벤트 통지 함수 등록
    public void RegisterEventHandler(EventHandler handler)
    {
        m_handler += handler;
    }

    //이벤트 통지 함수 삭제
    public void UnregisterEventHandler(EventHandler handler)
    {
        m_handler -= handler;
    }

    //스레드 시작 함수
    bool LaunchThread()
    {
        try
        {
            //Dispatch용 스레드 시작
            m_threadLoop = true;
            m_thread = new Thread(new ThreadStart(Dispatch));
            m_thread.Start();
        }
        catch
        {
            Debug.Log("Cannot launch thread");
            return false;
        }

        return true;
    }

    //스레드 측 송수신 처리
    public void Dispatch()
    {
        Debug.Log("Dispatch thread started");

        while (m_threadLoop)
        {
            //클라이언트로부터의 접속을 기다린다
            AcceptClient();

            //클라이언트와의 송수신 처리를 한다
            if (m_socket != null && m_isConnected == true)
            {
                //송신처리
                DispatchSend();

                //수신 처리
                DispatchReceive();
            }

            //Thread.Sleep(5);  //스레드를 5초 동안 정지한다 
        }

        Debug.Log("Dispatch tread ended");
    }

    //클라이언트 접속
    void AcceptClient()
    {
        //Socket.Poll : 소켓의 상태를 결정한다
        //현재는 0초 동안 응답을 기다리고 SelectMode는 SelectRead라는 상태로 설정한다
        if (m_listner != null && m_listner.Poll(0, SelectMode.SelectRead))
        {
            //클라이언트가 접속함
            m_socket = m_listner.Accept();
            m_isConnected = true;
            NetEventState state = new NetEventState();
            state.type = NetEventType.Connect;
            state.result = NetEventResult.Success;
            m_handler(state);
            Debug.Log("Connected from client.");
        }
    }

    //스레드측 송신처리
    void DispatchSend()
    {
        try
        {
            //송신처리
            if (m_socket.Poll(0, SelectMode.SelectWrite))
            {
                byte[] buffer = new byte[s_mtu];

                int sendSize = m_sendQueue.Dequeue(ref buffer, buffer.Length);
                while (sendSize > 0)
                {
                    //서버에 데이터를 전송한다
                    m_socket.Send(buffer, sendSize, SocketFlags.None);
                    sendSize = m_sendQueue.Dequeue(ref buffer, buffer.Length);
                }
            }
        }
        catch
        {
            return;
        }
    }

    //스레드 측 수신처리
    void DispatchReceive()
    {
        //수신 처리
        try
        {
            while (m_socket.Poll(0, SelectMode.SelectRead))
            {
                byte[] buffer = new byte[s_mtu];

                int recvSize = m_socket.Receive(buffer, buffer.Length, SocketFlags.None);
               
                if (recvSize == 0)
                {
                    //끊기
                    Debug.Log("Disconnect recv from server.");
                    Dispatch();
                }
                else if (recvSize > 0)
                {
                    string message = System.Text.Encoding.UTF8.GetString(buffer);   //버퍼에 있는 데이터를 가져온다
                    Debug.Log(message);
                    string[] DummyMessage = message.Split('\0');
                    JsonData Message = JsonMapper.ToObject(DummyMessage[0]);                    
                    if (Message["type"].ToString() == "LogIn")
                    { 
                        LogIn = true;
                    }
                    if (Message["type"].ToString() == "LogInFail")
                    {
                        SeverMesssage = "LogInFail";
                    }
                    if(Message["type"].ToString() == "SignOk")
                    {
                        SeverMesssage = "SignOk";
                    }
                    if(Message["type"].ToString() == "SignFail")
                    {
                        SeverMesssage = "SignFail";
                    }
                    if (Message["type"].ToString() == "ChattMessage")
                    {
                        StoreMessage += Message["Message"].ToString() + "\n";
                    }
                    if (Message["type"].ToString() == "EnterMessage")
                    {
                        StoreMessage += Message["Message"].ToString() + "\n";
                    }
                    if (Message["type"].ToString() == "NowUserInfo")
                    {
                        PlayerInfo data;
                        data.ID = Message["ID"].ToString();
                        data.Rot = float.Parse(Message["Rot"].ToString());
                        data.Charactor = null;
                        playerInfo.Add(Message["ID"].ToString(), data);
                    }

                    if (Message["type"].ToString() == "Boss")
                    {
                        Damage = 100;
                        BossNum = int.Parse(Message["Num"].ToString());
                        SeverMesssage = Message["Message"].ToString();
                        BossPos = int.Parse(Message["Pos"].ToString());

                        BossMake = true;
                    }

                    if (Message["type"].ToString() == "BossHP")
                    {
                        Damage = float.Parse(Message["HP"].ToString());
                    }

                    if (Message["type"].ToString() == "SetPosition")
                    {
                        RoomName = Message["RoomName"].ToString();
                        if (playerInfo.ContainsKey(Message["ID"].ToString()))
                        {
                            SpawnPoint roomPosData;
                            roomPosData.Pos = int.Parse(Message["Pos"].ToString());
                            roomPosData.ID = Message["ID"].ToString();
                            RoomPos.Add(roomPosData);                           
                        }
                    }

                    if(Message["type"].ToString() == "EnterUser")
                    {
                        PlayerInfo data;
                        data.ID = Message["ID"].ToString();
                        data.Rot = 0;
                        data.Charactor = null;
                        if (!playerInfo.ContainsKey(Message["ID"].ToString()))
                        {
                            playerInfo.Add(Message["ID"].ToString(), data);
                        }

                        if (playerInfo.ContainsKey(Message["ID"].ToString()))
                        {
                            PlayerInfo Info = playerInfo[Message["ID"].ToString()];
                            if (!InGamePlayers.ContainsKey(Message["ID"].ToString()))
                            {
                                 InGamePlayers.Add(Message["ID"].ToString(), Info);
                            }
                        }
                    }
                  
                    if(Message["type"].ToString()=="ClearMessage")
                    {
                        Debug.Log(message);
                        SeverMesssage = Message["Message"].ToString();

                    }
               
                    m_recvQueue.Enqueue(buffer, recvSize);
                }
            }
        }
        catch
        {
            return;
        }
    }

    public bool IsConnected()
    {
        return m_isConnected;
    }

    private void OnApplicationQuit()
    {
        m_thread.Interrupt();
        //m_thread.Abort();
    }

}
