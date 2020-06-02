#pragma once

#pragma comment(lib, "ws2_32")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib , "lib_json")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include<iostream>
#include"json/json.h"
#include <conio.h>
#include<ctime>
#include<queue>
#include<map>
#include<random>
#include<process.h>


#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define LOBBYPORT 9001
#define GAMEPORT 9002

#define MSGSIZE     (BUFSIZE-sizeof(int))  // ä�� �޽��� �ִ� ����
#define BUFSIZE 512

#define User 2001
#define CHAT 2002
#define USERDELETE 2003
#define CREATEROOM 2004
#define ENTERROOM 2005
#define SENDROOM 2006
#define ROOMMASSAGE 2007
#define READYMESSAGE 2008
#define DESTROYNAME 2009
#define DESTROYROOM 2010

#define LOGIN 2013
#define LOGINFAIL 2014

#define SIGNUP 2017
#define SIGNOK 2018
#define SIGNFAIL 2019

//���� �����ϴ� �Լ�
#define GAMESATRT 2015
//������ � Ÿ�������� �˷��ִ� �Լ�
#define SOCKTYPE 2011
#define TYPE 1;	//�߰輭���� 1��
using namespace std;

struct SOCKETINFO
{
	SOCKET sock;
	char   buf[BUFSIZE];
	int    recvbytes;
	char   ID[60];
	string str;
};

struct MsgType
{
	int type;
	char dummy[MSGSIZE];
};

struct SOCKETTYPE
{
	int type;
	int SockType;
};

struct UserData
{
	int type;
	char ID[60];
	char PASSWORD[60];
};

struct UserInfo
{
	int type;
	char ID[60];
};

struct UserOut
{
	int type;
	char ID[60];
};

struct ChatMessage
{
	int type;
	char buf[MSGSIZE];
};

//�� ������ �������� ����ִ� ������
struct RoomData
{
	int type;
	int Total = 0;
	int CountNum = 0;
	char RoomName[30];
	UserData userData[4];
	BOOL Ready[4];
};

struct RoomMessage
{
	int type;
	int Message;
	char ID[60];
};

struct RoomInfo
{
	int type;
	int Total;
	char ID[60];
	char RoomName[30];
};

//������ ������ ���� �Լ���
struct RoomUserInfo
{
	RoomData Data;
	SOCKET sockInfo[4];
};

struct EnterInfo
{
	int type;
	char ID[60];
	char RoomName[30];
};

struct DestroyRoomInfo
{
	int type;
	char ID[60];
	char RoomName[30];
};

struct Ready
{
	int type;
	char ID[60];
	char RoomName[30];
	BOOL Ready;
};

int nTotalSockets = 0;
SOCKETINFO *SocketInfoArray[FD_SETSIZE];

// ���� ���� �Լ�
BOOL AddSocketInfo(SOCKET sock, int port);
void RemoveSocketInfo(int nIndex);

// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);

unsigned __stdcall ThreadLobby(void* pArguments); // ������ �Լ�
unsigned __stdcall ThreadGame (void* pArguments); 
static SOCKET lobbysock;
static SOCKET gamesock;
//���̽� ������ ť���ٰ� ����
queue<string> Receive;

map<string, RoomUserInfo> Rooms;

static UserData Users;
static UserOut out;
static RoomInfo Room;
static EnterInfo Enter;
static Ready ready;
static SOCKETTYPE sockType;

BOOL SendRoom=FALSE;