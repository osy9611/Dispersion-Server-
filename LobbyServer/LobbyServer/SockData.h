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

#define MSGSIZE     (BUFSIZE-sizeof(int))  // 채팅 메시지 최대 길이
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

//서버와 연결될때 어떤 소켓 타입인지를 확인하는 함수
#define SOCKTYPE 2011

//DB 서버와 통신
#define CHECKLOGIN 2012
#define LOGIN 2013
#define LOGINFAIL 2014
#define SIGNUP 2017
#define SIGNOK 2018
#define SIGNFAIL 2019

//게임 시작할때 게임 서버와 통신
#define GAMESATRT 2015
//게임이 끝날때 게임 서버와 통신
#define GAMEEND 2016

#define MAXROOMCOUNT 4	//최대 방 접속인원

//소켓 타입들 
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

//소켓 타입 1.중계 2.게임 3.DB
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

//유저가 접속을 종료할때
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

//방 정보
struct RoomInfo
{
	int type;
	int Total;
	char ID[60];
	char RoomName[30];
};

//방 생성과 유저들이 들어있는 정보들
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

//유저가 접속하려는 방정보
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

//소켓 관리 함수
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);

//오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);

//사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags);

queue<string> ChatLog;

//유저의 포트번호 및 유저 데이터를 넣음
map<string, UserData> Users;
//방을 관리하기 위한 데이터
map<string, RoomData> Rooms;