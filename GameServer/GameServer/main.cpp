#include"SockData.h"

int main(int argc, char *argv[])
{
	int retval;

	MsgType Type;
	
	SOCKETTYPE * sockType;

	//���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	//socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	//bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	//listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	//�κ� ������ ����
	HANDLE Lobby;
	Lobby = (HANDLE)_beginthreadex(NULL,
		0, &ThreadLobby,
		NULL,
		0,
		0);

	CloseHandle(Lobby);

	//������ ��ſ� ����� ����(����)
	FD_SET rset;
	SOCKET client_sock;
	int addrlen, i, j;
	//������ ��ſ� ����� ����
	SOCKADDR_IN clientaddr;

	//�߰� ������ ����� �� ����ϱ� ���Ͽ� ���
	//�κ� �����ʹ� �ٸ��� ���Ӽ����� ���� �Ľ��Ͽ� ����Ұ�
	Json::Reader reader;
	Json::Value Message;

	while (1)
	{
		//���� �� �ʱ�ȭ
		FD_ZERO(&rset);
		FD_SET(listen_sock, &rset);
		for (i = 0; i < nTotalSockets; i++)
		{
			FD_SET(SocketInfoArray[i]->sock, &rset);
		}

		//select()
		retval = select(0, &rset, NULL, NULL, NULL);
		if (retval == SOCKET_ERROR)
		{
			err_display("select()");
			break;
		}

		//���� �� �˻�(1) : Ŭ���̾�Ʈ ���� ����
		if (FD_ISSET(listen_sock, &rset))
		{
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET)
			{
				err_display("accept()");
				break;
			}
			else
			{
				//������ Ŭ���̾�Ʈ ���� ���
				printf("[TCPv4 ����] Ŭ���̾�Ʈ ����: [%s]:%d\n",
					inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

				//���� ���� �߰�
				AddSocketInfo(client_sock);
			}
		}

		//���� �� �˻�(2) : ������ ���
		for (i = 0; i < nTotalSockets; i++)
		{
			SOCKETINFO * ptr = SocketInfoArray[i];
			ptr->buf[0] = '\0';			
			if (FD_ISSET(ptr->sock, &rset))
			{
				//������ �ޱ�
				retval = recv(ptr->sock, (char *)&ptr->buf,
					BUFSIZE, 0);
				
				if (retval == 0 || retval == SOCKET_ERROR)
				{
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
						if (Message["type"] == "Move")
						{
							for (int i = 0; i < Room[Message["RoomName"].asString()].Data.Total; ++i)
							{
								if (!strcmp(Message["ID"].asString().c_str(), Room[Message["RoomName"].asString()].Data.UserData[i].ID))
								{
									Room[Message["RoomName"].asString()].userPos[i].Pos.x= stof(Message["nx"].asString());
									Room[Message["RoomName"].asString()].userPos[i].Pos.y = stof(Message["ny"].asString());
									Room[Message["RoomName"].asString()].userPos[i].Pos.z = stof(Message["nz"].asString());
								}
							}
						}
						if (Message["type"] == "Rotation")
						{
							for (int i = 0; i < Room[Message["RoomName"].asString()].Data.Total; ++i)
							{
								if (!strcmp(Message["ID"].asString().c_str(), Room[Message["RoomName"].asString()].Data.UserData[i].ID))
								{
									Room[Message["RoomName"].asString()].userPos[i].Rot = stof(Message["y"].asString());
								
									cout << "Rot : " << Room[Message["RoomName"].asString()].userPos[i].Rot;
									cout << endl;
								}
							}
						}

						if (Message["type"] == "BossState")
						{
							cout << ptr->buf << endl;

							if (Room.find(Message["RoomName"].asString()) != Room.end())
							{
								if (Room[Message["RoomName"].asString()].BossHP != 0)
								{
									Room[Message["RoomName"].asString()].BossHP -= 10;
									Json::Value root;
									root["type"] = "BossHP";
									root["HP"] = Room[Message["RoomName"].asString()].BossHP;
									root["RoomName"] = Message["RoomName"].asString();
									Json::StyledWriter writer;
									string str = writer.write(root);
									strcpy(ptr->buf, str.c_str());
									cout << ptr->buf << endl;
									retval = send(ptr->sock, (char*)&ptr->buf, sizeof(ptr->buf), 0);
									cout << Message["RoomName"].asString() << ":HP:" << Room[Message["RoomName"].asString()].BossHP << endl;

									if (Room[Message["RoomName"].asString()].BossHP == 0)
									{
										Json::Value root;
										root["type"] = "ClearMessage";
										root["Message"] = u8"Clear";
										root["RoomName"] = Room[Message["RoomName"].asString()].Data.RoomName;
										Json::StyledWriter writer;
										string str = writer.write(root);
										strcpy(ptr->buf, str.c_str());
										cout << ptr->buf << endl;
										retval = send(ptr->sock, (char*)&ptr->buf, sizeof(ptr->buf), 0);	//�߰輭���� ����

										DestroyRoom Data;
										Data.type = GAMEEND;
										strcpy(Data.RoomName, Message["RoomName"].asString().c_str());
										send(lobbysock, (char *)&Data, sizeof(Data), 0);	//�κ� ������ �����͸� ���� ���� �����ϱ� ����
										cout << Room[Message["RoomName"].asString()].Data.RoomName << " ���� ������ ����Ǿ����ϴ�" << endl;

										Room.erase(Room.find(Message["RoomName"].asString()));
									}
								} 
							}
						}

					}
					catch (exception &e)
					{
						cout << "���� �߻� type : " << ptr->buf << endl;
					};
				}
				// ���� ����Ʈ �� ����
				ptr->recvbytes += retval;
			}
		}
	}
	return 0;
}

// ���� ���� �߰�
BOOL AddSocketInfo(SOCKET sock)
{
	if (nTotalSockets >= FD_SETSIZE)
	{
		cout << "[����] ���� ������ �߰��� �� �����ϴ�!\n";
		return FALSE;
	}

	SOCKETINFO *ptr = new SOCKETINFO;
	if (ptr == NULL)
	{
		cout << "[����] �޸𸮰� �����մϴ�!\n";
		return FALSE;
	}

	ptr->sock = sock;
	ptr->recvbytes = 0;
	SocketInfoArray[nTotalSockets++] = ptr;

	return TRUE;
}

// ���� ���� ����
void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO *ptr = SocketInfoArray[nIndex];

	// ������ Ŭ���̾�Ʈ ���� ���
	SOCKADDR_IN clientaddrv;
	int addrlen = sizeof(clientaddrv);
	getpeername(ptr->sock, (SOCKADDR *)&clientaddrv, &addrlen);
	printf("[TCPv4 ����] Ŭ���̾�Ʈ ����: [%s]:%d\n",
		inet_ntoa(clientaddrv.sin_addr), ntohs(clientaddrv.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];

	--nTotalSockets;
}


// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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
	RoomData *roomData;

	//random �Լ����� ������ �������� �����ϱ� ����
	random_device randDevice;	//���� ����̽�
	mt19937 mt(randDevice());
	uniform_int_distribution<int> distribution(0, 2);

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

	
	while (1)
	{
		retval = recv(lobbysock, (char *)&Type, BUFSIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR) {
			break;
		}

		if (Type.type == GAMESATRT)
		{
			roomData = (RoomData*)&Type;
			RoomUserInfo roomUserInfo;
			roomUserInfo.BossHP = 100;
			roomUserInfo.Data = *roomData;
			Room.insert(pair<string, RoomUserInfo>(roomData->RoomName, roomUserInfo));
			cout << Room[roomData->RoomName].Data.RoomName << "�濡�� ������ �����߽��ϴ�" << endl;
			cout << Room[roomData->RoomName].Data.CountNum << endl;
			
			for (int i = 0; i < Room[roomData->RoomName].Data.CountNum; i++)
			{
				Json::Value root;
				Json::StyledWriter writer;
				root["type"] = "SetPosition";
				root["ID"] = Room[roomData->RoomName].Data.UserData[i].ID;
				root["Pos"] = i;
				root["On"] = "Start";
				root["RoomName"] = roomData->RoomName;
				string str = writer.write(root);
				char buf[1024];
				strcpy(buf, str.c_str());
				cout << buf << endl;
				send(SocketInfoArray[0]->sock, buf, sizeof(buf), 0);
			}

			Json::Value Boss;
			Json::StyledWriter writer;
			Boss["type"] = "Boss";
			Boss["Num"] = distribution(randDevice);
			Boss["Message"] = u8"System �޽��� : �ʵ忡 ������ �����Ͽ����ϴ�";
			Boss["Pos"] = distribution(randDevice);
			Boss["RoomName"] = roomData->RoomName;
			string Bossstr = writer.write(Boss);
			char Bossbuf[1024];
			strcpy(Bossbuf, Bossstr.c_str());
			send(SocketInfoArray[0]->sock, Bossbuf, sizeof(Bossbuf), 0);
		}
	}

	closesocket(lobbysock);
	return 0;
}