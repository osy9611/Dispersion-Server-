using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;
public class UIManager : MonoBehaviour
{
    public Text SystemMessage;
    public TransportTCP transportTCP;
    public GameObject SignUpUI;

    public GameObject ClearUI;
    
    // Start is called before the first frame update
    void Start()
    {
        transportTCP = GameObject.FindObjectOfType<TransportTCP>();

    }

    // Update is called once per frame
    void Update()
    {
        if(SceneManager.GetActiveScene().name == "Demo 1")
        {
            if(transportTCP.SeverMesssage =="Clear")
            {
                ClearUI.SetActive(true);
            }
            else
            {
                StartCoroutine(MessageOn());
            }          
        }
        else if (SceneManager.GetActiveScene().name == "MainMenu")
        {
            if (transportTCP.SeverMesssage == "LogInFail")
            {
                transportTCP.m_state = ChatState.NOT_CONNECTED;
                TextOn("아이디 또는 비밀번호가 틀렸습니다");
            }
            if(transportTCP.SeverMesssage == "EmptyInfo")
            {
                transportTCP.m_state = ChatState.NOT_CONNECTED;
                TextOn("아이디와 비밀번호를 정확하게 적어주세요");
            }
            if(transportTCP.SeverMesssage == "SignOk")
            {
                transportTCP.m_state = ChatState.NOT_CONNECTED;
                SignUpUI.SetActive(false);
                TextOn("회원가입 성공");
            }
            if (transportTCP.SeverMesssage == "SignFail")
            {
                transportTCP.m_state = ChatState.NOT_CONNECTED;
                TextOn("회원가입 실패");
            }
        }
    }

    void TextOn(string Message)
    {
        SystemMessage.gameObject.SetActive(true);
        SystemMessage.text = Message;
        Invoke("TextOff", 1.0f);
    }

    void TextOff()
    {
        transportTCP.SeverMesssage = "";
        SystemMessage.gameObject.SetActive(false);
    }

    public void SignUpUIOn()
    {
        if(SignUpUI.activeSelf==true)
        {
            transportTCP.SignInfo.ID = "";
            transportTCP.SignInfo.Password = "";
            SignUpUI.SetActive(false);
        }
        else
        {
            SignUpUI.SetActive(true);
        }
      
    }

    IEnumerator MessageOn()
    {
        if (SystemMessage.gameObject.activeSelf==false)
        {
            SystemMessage.text = transportTCP.SeverMesssage;
            SystemMessage.gameObject.SetActive(true);
        }
        yield return new WaitForSeconds(2.0f);
        transportTCP.SeverMesssage = "";
        SystemMessage.gameObject.SetActive(false);
    }
}
