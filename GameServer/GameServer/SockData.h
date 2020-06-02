#pragma once

#pragma comment(lib, "ws2_32")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib , "lib_json")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include<iostream>
#include"json/json.h"
#include <conio.h>
#include <math.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include<ctime>
#include<queue>
#include<map>
#include<random>
#include<process.h>

#define SERVERIP "127.0.0.3"
#define SERVERPORT 9002
#define LOBBYPORT 9001

#define MSGSIZE     (BUFSIZE-sizeof(int))  // 채팅 메시지 최대 길이
#define BUFSIZE 512


#define GAMESATRT 2015
#define GAMEEND 2016

//소켓이 어떤 타입인지를 알려주는 함수
#define SOCKTYPE 2011
#define TYPE 2	//게임 서버는 2번

using namespace std;
using namespace DirectX;

struct SOCKETINFO
{
	SOCKET sock;
	char buf[BUFSIZE];
	int recvbytes;	
};

struct SOCKETTYPE
{
	int type;
	int SockType;
};

struct MsgType
{
	int type;
	char dummy[MSGSIZE];
};

struct UserData
{
	int type;
	char ID[60];
	char PASSWORD[60];
};

struct UserPos
{
	float Rot = 0;
	XMFLOAT3 Pos = XMFLOAT3(0, 0, 0);
};

struct DestroyRoom
{
	int type;
	char RoomName[30];
};

//방 생성과 유저들이 들어있는 정보들
struct RoomData
{
	int type;
	int Total = 0;
	int CountNum = 0;
	char RoomName[30];
	UserData UserData[4];
	BOOL Ready[4];
};

//방정보들
struct RoomUserInfo
{
	RoomData Data;
	UserPos userPos[4];
	float BossHP;
};


int nTotalSockets = 0;
SOCKETINFO *SocketInfoArray[FD_SETSIZE];

//소켓 관리 함수
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);

//오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);

map<string,RoomUserInfo> Room;


unsigned __stdcall ThreadLobby(void* pArguments);

static SOCKET lobbysock;

static queue<string> Receive;


static SOCKETTYPE sockType;

