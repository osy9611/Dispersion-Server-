#pragma warning( disable : 4996)
#pragma warning( disable : 4330)
#pragma once

#pragma comment(lib,"ws2_32")
#pragma comment(lib, "winmm.lib")

/*
Ado�� ������ �ͽ������ �Բ� ������ �Ǿ���
*/
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include<ctime>
#include<queue>
#include<map>
#include<random>
#include<process.h>
#include <stdio.h>
#include <iostream>
#include <comdef.h>
#include <conio.h>
#include "Database.h"

#define LOBBYPORT 9001

#define MSGSIZE     (BUFSIZE-sizeof(int))  // ä�� �޽��� �ִ� ����
#define BUFSIZE 512

#define SOCKTYPE 2011

//DB ���
#define CHECKLOGIN 2012
#define LOGIN 2013
#define LOGINFAIL 2014

#define SIGNUP 2017
#define SIGNOK 2018
#define SIGNFAIL 2019

#define NEWUSER 2015

#define TYPE 3;
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

void err_quit(char *msg);
void err_display(char *msg);
// ����
// https://www.connectionstrings.com/access/

/*
MySql��
"DRIVER={MySQL ODBC 8.0 ANSI Driver};\
				   SERVER=localhost;\
				   DATABASE=db�̸�;\
				   USER=���̵�;\
				   PASSWORD=��й�ȣ;"
				   */

char CnnStr[200] = "Driver={Microsoft Access Driver (*.mdb)};Dbq=C:\\Users\\osy96\\OneDrive\\���� ȭ��\\3�г�\\3�г� 2�б�\\�ý��� ���α׷���\\��Ƽ�̵�����۵���2(��3)\\��Ʈ��ũ ����(�л꼭��)\\DBServer\\User.mdb;";
char ErrStr[200];

using namespace std;

static SOCKETTYPE sockType;
