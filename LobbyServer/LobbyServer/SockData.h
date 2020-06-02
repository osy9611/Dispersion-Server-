#pragma warning( disable : 4996)
#pragma warning( disable : 4330)
#pragma once

#pragma comment(lib,"ws2_32")
#pragma comment(lib, "winmm.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include<iostream>
#include <conio.h>
#include<ctime>
#include<queue>
#include<map>
#include<random>
#include<process.h>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9001

#define MSGSIZE     (BUFSIZE-sizeof(int))  // ä�� �޽��� �ִ� ����
#define BUFSIZE 512
#define MAXCHAT 20

#define USER 2001
#define CHAT 2002
#define USERDELETE 2003
#define CREATEROOM 2004
#define ENTERROOM 2005
#define SENDROOM 2006
#define ROOMMASSAGE 2007
#define READYMESSAGE 2008
#define DESTROYNAME 2009
#define DESTROYROOM 2010

//������ ����ɶ� � ���� Ÿ�������� Ȯ���ϴ� �Լ�
#define SOCKTYPE 2011

//DB ������ ���
#define CHECKLOGIN 2012
#define LOGIN 2013
#define LOGINFAIL 2014
#define SIGNUP 2017
#define SIGNOK 2018
#define SIGNFAIL 2019

//���� �����Ҷ� ���� ������ ���
#define GAMESATRT 2015
//������ ������ ���� ������ ���
#define GAMEEND 2016

#define MAXROOMCOUNT 4	//�ִ� �� �����ο�

//���� Ÿ�Ե� 
#define RELAY 1
#define INGAME 2
#define DATABASE 3

using namespace std;

struct SOCKETINFO
{
	SOCKET sock;
	char buf[MSGSIZE];
	int recvbytes;
	int Type;
};

struct MsgType
{
	int type;
	char dummy[MSGSIZE];
};

//���� Ÿ�� 1.�߰� 2.���� 3.DB
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

//������ ������ �����Ҷ�
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

//�� ����
struct RoomInfo
{
	int type;
	int Total;
	char ID[60];
	char RoomName[30];
};

//�� ������ �������� ����ִ� ������
struct RoomData
{
	int type;
	int Total = 0;
	int CountNum = 0;
	char RoomName[30];
	UserData UserData[MAXROOMCOUNT];
	BOOL Ready[MAXROOMCOUNT];
};

struct RoomaMessage
{
	int type;
	int Message;
	char ID[60];
};

//������ �����Ϸ��� ������
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

struct DestroyRoom
{
	int type;
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

//���� ���� �Լ�
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);

//���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);

//����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags);

queue<string> ChatLog;

//������ ��Ʈ��ȣ �� ���� �����͸� ����
map<string, UserData> Users;
//���� �����ϱ� ���� ������
map<string, RoomData> Rooms;