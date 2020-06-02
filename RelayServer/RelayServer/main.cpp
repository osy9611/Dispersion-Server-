#include "SockData.h"

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	/*----- IPv4 소켓 초기화 시작 -----*/
	// socket()
	SOCKET listen_sockv4 = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sockv4 == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddrv4;
	ZeroMemory(&serveraddrv4, sizeof(serveraddrv4));
	serveraddrv4.sin_family = AF_INET;
	serveraddrv4.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddrv4.sin_port = htons(SERVERPORT);
	retval = bind(listen_sockv4, (SOCKADDR *)&serveraddrv4, sizeof(serveraddrv4));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sockv4, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");
	/*----- IPv4 소켓 초기화 끝 -----*/

	//로비 서버와 연결
	HANDLE Lobby;
	Lobby = (HANDLE)_beginthreadex(NULL,
		0, &ThreadLobby,
		NULL,
		0,
		0);

	CloseHandle(Lobby);

	//게임 서버와 연결
	HANDLE Game;
	Game = (HANDLE)_beginthreadex(NULL,
		0, &ThreadGame,
		NULL,
		0,
		0);

	CloseHandle(Game);

	// 데이터 통신에 사용할 변수(공통)
	FD_SET rset;
	SOCKET client_sock;
	int addrlen, i, j;
	// 데이터 통신에 사용할 변수(IPv4)
	SOCKADDR_IN clientaddrv4;

	//Json 통신하려고 만듬
	Json::Reader reader;
	Json::Value Message;

	while (1)
	{
		// 소켓 셋 초기화
		FD_ZERO(&rset);
		FD_SET(listen_sockv4, &rset);
		for (i = 0; i < nTotalSockets; i++)
		{
			FD_SET(SocketInfoArray[i]->sock, &rset);
		}

		// select()
		retval = select(0, &rset, NULL, NULL, NULL);
		if (retval == SOCKET_ERROR)
		{
			err_display("select()");
			break;
		}

		// 소켓 셋 검사(1): 클라이언트 접속 수용
		if (FD_ISSET(listen_sockv4, &rset))
		{
			addrlen = sizeof(clientaddrv4);
			client_sock = accept(listen_sockv4, (SOCKADDR *)&clientaddrv4, &addrlen);
			if (client_sock == INVALID_SOCKET)
			{
				err_display("accept()");
				break;
			}
			else
			{
				// 접속한 클라이언트 정보 출력
				printf("[TCPv4 서버] 클라이언트 접속: [%s]:%d\n",
					inet_ntoa(clientaddrv4.sin_addr), ntohs(clientaddrv4.sin_port));
				// 소켓 정보 추가
				AddSocketInfo(client_sock, clientaddrv4.sin_port);
			}
		}

		// 소켓 셋 검사(2): 데이터 통신
		for (i = 0; i < nTotalSockets; i++)
		{
			SOCKETINFO *ptr = SocketInfoArray[i];
			ptr->buf[0] = '\0';
			if (FD_ISSET(ptr->sock, &rset))
			{
				// 데이터 받기
				ptr->recvbytes = 0;
				retval = recv(ptr->sock, ptr->buf + ptr->recvbytes,
					BUFSIZE, 0);
				if (retval == 0 || retval == SOCKET_ERROR) {
					RemoveSocketInfo(i);
					continue;
				}
				Receive.push(ptr->buf);
				bool parsingRet = reader.parse(Receive.front(), Message);
				Receive.pop();
				if (parsingRet)
				{
					try
					{
						if (Message["type"] == "UserInfo")
						{
							Users.type = User;							
							strcpy(Users.ID, Message["ID"].asString().c_str());		
							strcpy(Users.PASSWORD, Message["Password"].asString().c_str());
							strcpy(ptr->ID, Message["ID"].asString().c_str());
							send(lobbysock, (char *)&Users, sizeof(Users), 0);
						}

						if (Message["type"] == "SignInfo")
						{
							Users.type = SIGNUP;
							strcpy(Users.ID, Message["ID"].asString().c_str());
							strcpy(Users.PASSWORD, Message["Password"].asString().c_str());
							strcpy(ptr->ID, Message["ID"].asString().c_str());
							send(lobbysock, (char *)&Users, sizeof(Users), 0);
						}

						if (Message["type"] == "ChattMessage")
						{
							ptr->recvbytes += retval;

							// 현재 접속한 모든 클라이언트에게 데이터를 보냄!
							for (j = 0; j < nTotalSockets; j++) {
								SOCKETINFO *ptr2 = SocketInfoArray[j];
								retval = send(ptr2->sock, ptr->buf, ptr->recvbytes, 0);
								//cout << retval << endl;
								if (retval == SOCKET_ERROR) {
									err_display("send()");
									RemoveSocketInfo(j);
									--j; // 루프 인덱스 보정
									continue;
								}
							}
						}

						if (Message["type"] == "RoomInfo")
						{
							Room.type = CREATEROOM;
							strcpy(Room.RoomName, Message["RoomName"].asString().c_str());
							strcpy(Room.ID, ptr->ID);
							Room.Total = stoi(Message["Total"].asString());
							send(lobbysock, (char *)&Room, sizeof(Room), 0);

							ptr->recvbytes += retval;

							// 현재 접속한 모든 클라이언트에게 데이터를 보냄!
							for (j = 0; j < nTotalSockets; j++) {
								SOCKETINFO *ptr2 = SocketInfoArray[j];
								retval = send(ptr2->sock, ptr->buf, ptr->recvbytes, 0);
								//cout << retval << endl;
								if (retval == SOCKET_ERROR) {
									err_display("send()");
									RemoveSocketInfo(j);
									--j; // 루프 인덱스 보정
									continue;
								}
							}
						}
						
						if (Message["type"] == "EnterRoomInfo")
						{
							Enter.type = ENTERROOM;
							strcpy(Enter.ID , ptr->ID);							
							strcpy(Enter.RoomName, Message["RoomName"].asString().c_str());
							send(lobbysock, (char *)&Enter, sizeof(Enter), 0);							
						}

						if (Message["type"] == "ReadyMessage")
						{
							ready.type = READYMESSAGE;
							ready.Ready = TRUE;
							strcpy(ready.ID, Message["ID"].asString().c_str());
							strcpy(ready.RoomName, Message["RoomName"].asString().c_str());
							cout << "ReadyMessage " << ready.ID << " " << ready.RoomName << endl;
							send(lobbysock, (char *)&ready, sizeof(ready), 0);
						}

						if (Message["type"] == "Move")
						{
							ptr->recvbytes += retval;
							retval = send(gamesock, (char *)&ptr->buf, ptr->recvbytes, 0);
							SendRoom = TRUE;
						}

						if (Message["type"] == "NowMove")
						{
							ptr->recvbytes += retval;
							retval = send(gamesock, (char *)&ptr->buf, ptr->recvbytes, 0);
							SendRoom = TRUE;
						}

						if (Message["type"] == "Rotation")
						{
							ptr->recvbytes += retval;
							retval = send(gamesock, (char *)&ptr->buf, ptr->recvbytes, 0);
							SendRoom = TRUE;
						}

						if (Message["type"] == "KeyDown")
						{
							ptr->recvbytes += retval;
							retval = send(gamesock, (char *)&ptr->buf, ptr->recvbytes, 0);
							SendRoom = TRUE;
						}

						if (Message["type"] == "BossState")
						{
							cout << ptr->buf << endl;
							cout << "크기" << retval<<endl;
							ptr->recvbytes += retval;
							retval = send(gamesock, (char *)&ptr->buf, ptr->recvbytes, 0);
						}

						if (SendRoom ==TRUE)
						{
							for (int i = 0; i < Rooms[Message["RoomName"].asString()].Data.Total; i++)
							{
								if (strcmp(Rooms[Message["RoomName"].asString()].Data.userData[i].ID, Message["ID"].asString().c_str()))
								{
									send(Rooms[Message["RoomName"].asString()].sockInfo[i], (char*)&ptr->buf, ptr->recvbytes, 0);
								}
							}
							SendRoom = FALSE;
						}
					}

					catch (exception &e)
					{
						cout << "에러 발생 type : " << ptr->buf << endl;
					}
				}
			}
		}
	}

	return 0;
}

// 소켓 정보 추가
BOOL AddSocketInfo(SOCKET sock,int port)
{
	if (nTotalSockets >= FD_SETSIZE)
	{
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return FALSE;
	}

	SOCKETINFO *ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[오류] 메모리가 부족합니다!\n");
		return FALSE;
	}

	ptr->sock = sock;
	ptr->recvbytes = 0;
	SocketInfoArray[nTotalSockets++] = ptr;

	return TRUE;
}

// 소켓 정보 삭제
void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO *ptr = SocketInfoArray[nIndex];

	//유저가 나갔을 때 포트 번호를 보내줘서 지워버림
	out.type = USERDELETE;
	strcpy(out.ID, ptr->ID);
	send(lobbysock, (char *)&out, sizeof(out), 0);

	// 종료한 클라이언트 정보 출력
	SOCKADDR_IN clientaddrv;
	int addrlen = sizeof(clientaddrv);
	getpeername(ptr->sock, (SOCKADDR *)&clientaddrv, &addrlen);
	printf("[TCPv4 서버] 클라이언트 종료: [%s]:%d\n",
		inet_ntoa(clientaddrv.sin_addr), ntohs(clientaddrv.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];

	--nTotalSockets;
}

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


unsigned __stdcall ThreadLobby(void* pArguments)
{
	MsgType Type;
	UserInfo *userInfo;
	RoomData *Data;
	RoomMessage *roomMessage;
	DestroyRoomInfo *destroyRoom;

	int retval;

	lobbysock = socket(AF_INET, SOCK_STREAM, 0);
	if (lobbysock == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.2");
	serveraddr.sin_port = htons(LOBBYPORT);
	retval = connect(lobbysock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	fd_set rset;

	sockType.type = SOCKTYPE;
	sockType.SockType = TYPE;
	send(lobbysock, (char*)&sockType, BUFSIZE, 0);

	int sockNum[4];
	int cnt = 0;
	while (1)
	{
		retval = recv(lobbysock, (char *)&Type, BUFSIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR) {
			break;
		}

		if (Type.type == LOGIN)
		{
			userInfo = (UserInfo *)&Type;
			Json::Value root;
			root["type"] = "LogIn";
			Json::StyledWriter writer;
			string str = writer.write(root);
			char buf[BUFSIZE];
			strcpy(buf, str.c_str());
			for (int i = 0; i < nTotalSockets; i++)
			{
				if (strcmp(userInfo->ID, SocketInfoArray[i]->ID) == 0)
				{
					send(SocketInfoArray[i]->sock,buf, sizeof(buf), 0);
				}
			}
		}
		if (Type.type == LOGINFAIL)
		{
			userInfo = (UserInfo *)&Type;
			Json::Value root;
			root["type"] = "LogInFail";
			Json::StyledWriter writer;
			string str = writer.write(root);
			char buf[BUFSIZE];
			strcpy(buf, str.c_str());
			for (int i = 0; i < nTotalSockets; i++)
			{
				if (strcmp(userInfo->ID, SocketInfoArray[i]->ID) == 0)
				{
					send(SocketInfoArray[i]->sock, buf, sizeof(buf), 0);
				}
			}
		}

		if (Type.type == SIGNOK)
		{
			userInfo = (UserInfo *)&Type;
			Json::Value root;
			root["type"] = "SignOk";
			Json::StyledWriter writer;
			string str = writer.write(root);
			char buf[BUFSIZE];
			strcpy(buf, str.c_str());
			for (int i = 0; i < nTotalSockets; i++)
			{
				if (strcmp(userInfo->ID, SocketInfoArray[i]->ID) == 0)
				{
					send(SocketInfoArray[i]->sock, buf, sizeof(buf), 0);
				}
			}
		}

		if (Type.type == SIGNFAIL)
		{
			userInfo = (UserInfo *)&Type;
			Json::Value root;
			root["type"] = "SignFail";
			Json::StyledWriter writer;
			string str = writer.write(root);
			char buf[BUFSIZE];
			strcpy(buf, str.c_str());
			for (int i = 0; i < nTotalSockets; i++)
			{
				if (strcmp(userInfo->ID, SocketInfoArray[i]->ID) == 0)
				{
					send(SocketInfoArray[i]->sock, buf, sizeof(buf), 0);
				}
			}
		}

		//방에 들어온 유저 정보를 받아서 보내줌
		if (Type.type == SENDROOM)
		{
			cnt = 0;
			Data = (RoomData *)&Type;

			//현제 중계 서버에 들어와있는 해당 유저를 검색한다
			for (int i = 0; i < Data->CountNum; i++)
			{
				for (int j = 0; j < nTotalSockets; j++)
				{
					if (strcmp(Data->userData[i].ID, SocketInfoArray[j]->ID) == 0)
					{
						sockNum[cnt] =j;
						cnt++;
					}
				}
			}

			for (int i = 0; i < Data->CountNum; i++)
			{
				Json::Value root;
				root["type"] = "EnterUser";
				root["ID"] = SocketInfoArray[sockNum[i]]->ID;
				root["Total"] = Data->Total;
				Json::StyledWriter writer;
				string str = writer.write(root);
				char buf[BUFSIZE];
				strcpy(buf, str.c_str());

				for (int j = 0; j < Data->CountNum; j++)
				{
					send(SocketInfoArray[sockNum[j]]->sock, (char *)&buf, sizeof(buf), 0);
				}
			}
		}
		if (Type.type == ROOMMASSAGE)
		{
			roomMessage = (RoomMessage *)&Type;

			for (int i = 0; i < nTotalSockets; i++)
			{
				if (!strcmp(roomMessage->ID, SocketInfoArray[i]->ID))
				{
					Json::Value root;
					Json::StyledWriter writer;
					root["type"] = "EnterMessage";
					root["Message"] = u8"방이 없습니다";
					string str = writer.write(root);
					char buf[BUFSIZE];
					strcpy(buf, str.c_str());
					retval =send(SocketInfoArray[i]->sock, buf, strlen(buf), 0);
					break;
				}
			}
		}

		if (Type.type == READYMESSAGE)
		{
			cnt = 0;
			Data = (RoomData *)&Type;

			//현제 중계 서버에 들어와있는 해당 유저를 검색한다
			for (int i = 0; i < Data->CountNum; i++)
			{
				for (int j = 0; j < nTotalSockets; j++)
				{
					if (strcmp(Data->userData[i].ID, SocketInfoArray[j]->ID) == 0)
					{
						sockNum[cnt] = j;
						cnt++;
					}
				}
			}

			for (int i = 0; i < cnt; i++)
			{
				if (Data->Ready[i] == TRUE)
				{
					Json::Value root;
					root["type"] = "ReadyMessage";
					root["ID"] = SocketInfoArray[sockNum[i]]->ID;
					root["RoomName"] = Data->RoomName;
					root["Ready"] = true;

					Json::StyledWriter writer;
					string str = writer.write(root);
					char buf[BUFSIZE];
					strcpy(buf, str.c_str());

					for (int j = 0; j < Data->CountNum; j++)
					{
						send(SocketInfoArray[sockNum[j]]->sock,buf, sizeof(buf), 0);
					}
				}			
			
			}
		}

		if (Type.type == DESTROYNAME)
		{
			destroyRoom = (DestroyRoomInfo *)&Type;

			Json::Value root;
			root["type"] = "DestroyName";
			root["RoomName"] = destroyRoom->RoomName;
			root["ID"] = destroyRoom->ID;
			Json::StyledWriter writer;
			string str = writer.write(root);
			char buf[BUFSIZE];
			strcpy(buf, str.c_str());

			for (int i = 0; i < nTotalSockets; i++)
			{
				send(SocketInfoArray[i]->sock, buf, sizeof(buf), 0);
			}
		}

		if (Type.type == DESTROYROOM)
		{
			destroyRoom = (DestroyRoomInfo *)&Type;

			Json::Value root;
			root["type"] = "DestroyRoom";
			root["RoomName"] = destroyRoom->RoomName;
			root["ID"] = destroyRoom->ID;
			Json::StyledWriter writer;
			string str = writer.write(root);
			char buf[BUFSIZE];
			strcpy(buf, str.c_str());

			for (int i = 0; i < nTotalSockets; i++)
			{
				send(SocketInfoArray[i]->sock, buf, sizeof(buf), 0);
			}
		}

		if (Type.type == GAMESATRT)
		{
			Data = (RoomData *)&Type;
			RoomUserInfo roomUserInfo;
			roomUserInfo.Data = *Data;
			for (int i = 0; i < Data->Total; i++)
			{
				for (int j = 0; j < nTotalSockets; j++)
				{
					if (!strcmp(SocketInfoArray[j]->ID, Data->userData[i].ID))
					{
						roomUserInfo.sockInfo[i] = SocketInfoArray[j]->sock;
					}
				}
			}

			Rooms.insert(pair<string, RoomUserInfo>(Data->RoomName, roomUserInfo));
			cout<< Rooms[Data->RoomName].Data.RoomName << "게임 방 게임을 시작했습니다" << endl;
		}

	}

	closesocket(lobbysock);
	return 0;
}


unsigned __stdcall ThreadGame(void* pArguments)
{
	MsgType Type;
	RoomData *roomData;

	int retval;

	gamesock = socket(AF_INET, SOCK_STREAM, 0);
	if (lobbysock == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.2");
	serveraddr.sin_port = htons(GAMEPORT);
	retval = connect(gamesock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	fd_set rset;
	sockType.type = SOCKTYPE;
	sockType.SockType = TYPE;
	send(gamesock, (char*)&sockType, BUFSIZE, 0);

	SOCKETINFO ptr;
	ptr.sock = gamesock;
	ptr.recvbytes = 0;
	
	//Json 통신하려고 만듬
	Json::Reader reader;
	Json::Value Message;

	while (1)
	{
		ptr.buf[0] = '\0';
		retval = recv(gamesock, (char *)&ptr.buf,
			BUFSIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR) {
			break;
		}
		Receive.push(ptr.buf);
		cout << "받은 : " << Receive.front() << endl;
		bool parsingRet = reader.parse(Receive.front(), Message);
		Receive.pop();
		
		if (parsingRet)
		{
			try
			{
				if (Message["type"] == "ClearMessage")
				{
					Json::StyledWriter writer;
					string str = writer.write(Message);
					char Data[BUFSIZE];
					strcpy(Data, str.c_str());
					cout << "보낸 : " << Data << endl;

					for (int i = 0; i < Rooms[Message["RoomName"].asString()].Data.CountNum; i++)
					{
						retval = send(Rooms[Message["RoomName"].asString()].sockInfo[i], (char *)&Data, sizeof(Data), 0);
					}
					Rooms.erase(Rooms.find(Message["RoomName"].asString()));
				}
				else
				{
					Json::StyledWriter writer;
					string str = writer.write(Message);
					char Data[BUFSIZE];
					strcpy(Data, str.c_str());
					cout << "보낸 : " << Data << endl;

					for (int i = 0; i < Rooms[Message["RoomName"].asString()].Data.CountNum; i++)
					{
						retval = send(Rooms[Message["RoomName"].asString()].sockInfo[i], (char *)&Data, sizeof(Data), 0);
					}
				}
			}
			catch (exception &e)
			{
				cout << "에러 발생 type :" << ptr.buf << endl;
			}
		}
	}

	closesocket(gamesock);
	return 0;
}

