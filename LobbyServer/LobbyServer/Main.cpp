#include "SockData.h"

int main(int argc, char *argv[])
{
	int retval;

	MsgType Type;
	UserData *User;
	UserInfo *userInfo;
	ChatMessage *Chat;
	UserOut *userOut;
	RoomInfo *roomInfo;
	RoomData *roomData;
	EnterInfo *enterInfo;
	Ready *ready;
	DestroyRoom *desytroyRoom;

	SOCKETTYPE * sockType;

	//윈속 초기화
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

	//데이터 통신에 사용할 변수(공통)
	FD_SET rset;
	SOCKET client_sock;
	int addrlen, i, j;
	//데이터 통신에 사용할 변수
	SOCKADDR_IN clientaddr;

	while (1)
	{
		//소켓 셋 초기화
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

		//소켓 셋 검사(1) : 클라이언트 접속 수용
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
				//접속한 클라이언트 정보 출력
				printf("[TCPv4 서버] 클라이언트 접속: [%s]:%d\n",
					inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

				//소켓 정보 추가
				AddSocketInfo(client_sock);
			}
		}

		//소켓 셋 검사(2) : 데이터 통신
		for (i = 0; i < nTotalSockets; i++)
		{
			SOCKETINFO * ptr = SocketInfoArray[i];
			if (FD_ISSET(ptr->sock, &rset))
			{
				//데이터 받기
				retval = recv(ptr->sock, (char*)&Type, BUFSIZE, 0);
				if (Type.type==SOCKTYPE)
				{
					sockType = (SOCKETTYPE *)&Type;
					ptr->Type = sockType->SockType;
					cout << "SocketType : " << ptr->Type << endl;
				}
				if (Type.type == USER)
				{
					User = (UserData *)&Type; 
					for (int i = 0; i < nTotalSockets; i++)
					{
						if (SocketInfoArray[i]->Type == DATABASE)
						{
							UserData Data = *User;
							Data.type = CHECKLOGIN;
							send(SocketInfoArray[i]->sock, (char*)&Data, sizeof(Data), 0);
						}
					}
				}

				if (Type.type == LOGIN)
				{
					userInfo = (UserInfo*)&Type;
					cout << "Login User ID :" << userInfo->ID<< endl;
					Users.insert(pair<string, UserData>(userInfo->ID, *User));
					for (int i = 0; i < nTotalSockets; i++)
					{
						if (SocketInfoArray[i]->Type == RELAY)
						{
							send(SocketInfoArray[i]->sock, (char*)&Type, sizeof(Type), 0);
						}
					}
				}

				if (Type.type == LOGINFAIL)
				{
					userInfo = (UserInfo*)&Type;
					cout << "Login Fail ID :" << userInfo->ID << endl;
					for (int i = 0; i < nTotalSockets; i++)
					{
						if (SocketInfoArray[i]->Type == RELAY)
						{
							send(SocketInfoArray[i]->sock, (char*)&Type, sizeof(Type), 0);
						}
					}
				}

				if (Type.type == SIGNUP)
				{
					User = (UserData *)&Type;
					for (int i = 0; i < nTotalSockets; i++)
					{
						if (SocketInfoArray[i]->Type == DATABASE)
						{
							UserData Data = *User;
							Data.type = SIGNUP;
							send(SocketInfoArray[i]->sock, (char*)&Data, sizeof(Data), 0);
						}
					}
				}

				if (Type.type == SIGNOK)
				{
					userInfo = (UserInfo*)&Type;
					cout << "SignOK User ID :" << userInfo->ID << endl;
					for (int i = 0; i < nTotalSockets; i++)
					{
						if (SocketInfoArray[i]->Type == RELAY)
						{
							send(SocketInfoArray[i]->sock, (char*)&Type, sizeof(Type), 0);
						}
					}
				}

				if (Type.type == SIGNFAIL)
				{
					userInfo = (UserInfo*)&Type;
					cout << "SignFail ID :" << userInfo->ID << endl;
					for (int i = 0; i < nTotalSockets; i++)
					{
						if (SocketInfoArray[i]->Type == RELAY)
						{
							send(SocketInfoArray[i]->sock, (char*)&Type, sizeof(Type), 0);
						}
					}
				}

				if (Type.type == USERDELETE)
				{
					userOut = (UserOut *)&Type;
					if (Users.find(User->ID) != Users.end())
					{
						cout << "LogOut User ID :"<< Users[userOut->ID].ID << endl;
						Users.erase(Users.find(userOut->ID));
						for (auto iter = Rooms.begin(); iter != Rooms.end(); iter++)
						{
							for (int i = 0; i < iter->second.CountNum; i++)
							{
								if (!strcmp(iter->second.UserData[i].ID, User->ID))
								{
									if (Rooms[iter->first].CountNum != 1)
									{
										DestroyRoomInfo DestroyData;
										DestroyData.type = DESTROYNAME;
										strcpy(DestroyData.ID, Rooms[iter->first].UserData[i].ID);
										strcpy(DestroyData.RoomName, Rooms[iter->first].RoomName);
										send(ptr->sock, (char *)&DestroyData, sizeof(DestroyData), 0);
										memmove(Rooms[iter->first].UserData - 1, Rooms[iter->first].UserData, sizeof(UserData)*MAXROOMCOUNT);
										memmove(Rooms[iter->first].Ready - 1, Rooms[iter->first].Ready, sizeof(BOOL)*MAXROOMCOUNT);
										Rooms[iter->first].CountNum--;
									}
									else
									{
										DestroyRoomInfo DestroyData;
										DestroyData.type = DESTROYROOM;
										strcpy(DestroyData.ID, Rooms[iter->first].UserData[i].ID);
										strcpy(DestroyData.RoomName, Rooms[iter->first].RoomName);
										send(ptr->sock, (char *)&DestroyData, sizeof(DestroyData), 0);
										Rooms.erase(Rooms.find(Rooms[iter->first].RoomName));
										cout << "방이 삭제되었습니다" << endl;
										break;
									}
								}
							}
							break;
						}
					}
				}
				if (Type.type == CREATEROOM)
				{
					roomInfo = (RoomInfo *)&Type;
					RoomData *Data = new RoomData;
					Data->Total = roomInfo->Total;
					Data->CountNum = 1;
					Data->UserData[0] = Users[roomInfo->ID];
					strcpy(Data->RoomName, roomInfo->RoomName);
					//방 이름과 룸 데이터를 맵에다가 넣는다
					Rooms.insert(pair<string, RoomData>(roomInfo->RoomName, *Data));
				}

				if (Type.type == ENTERROOM)
				{
					enterInfo = (EnterInfo *)&Type;
					//들어오는 유저가 있으면 방이 있는지 검색한다
					if (Rooms.find(enterInfo->RoomName) != Rooms.end())
					{
						//인원수가 초과가 아니라면 데이터를 넣어주고 중계서버로 보내준다
						if (Rooms[enterInfo->RoomName].CountNum != Rooms[enterInfo->RoomName].Total)
						{
							cout << "Room Enter! User ID :" << Users[enterInfo->ID].ID << endl;
							Rooms[enterInfo->RoomName].UserData[Rooms[enterInfo->RoomName].CountNum] = Users[enterInfo->ID];
							Rooms[enterInfo->RoomName].CountNum++;
							RoomData Data = Rooms[enterInfo->RoomName];

							Data.type = SENDROOM;
							send(ptr->sock, (char *)&Data, sizeof(Data), 0);
						}
						else
						{
							cout << enterInfo->RoomName << "Is Full" << endl;
							RoomaMessage roomMessage;
							roomMessage.type = ROOMMASSAGE;
							roomMessage.Message = 0;
							strcpy(roomMessage.ID, enterInfo->ID);
							send(ptr->sock, (char *)&roomMessage, sizeof(roomMessage), 0);
						}
					}
				}


				if (Type.type == READYMESSAGE)
				{
					int ReadyCount =0;
					ready = (Ready *)&Type;
					if (Rooms.find(ready->RoomName) != Rooms.end())
					{
						for (int i = 0; i < Rooms[ready->RoomName].Total; i++)
						{
							if (Rooms[ready->RoomName].Ready[i] == TRUE)
							{
								ReadyCount++;
							}
							//유저를 찾아서 Ready를 TRUE 값으로 바꾼다
							if (!strcmp(Rooms[ready->RoomName].UserData[i].ID, Users[ready->ID].ID))
							{
								Rooms[ready->RoomName].Ready[i] = TRUE;
								RoomData Data = Rooms[ready->RoomName];
								Data.type = READYMESSAGE;
								strcpy(Data.RoomName, ready->RoomName);
								cout << "RoomName :" << ready->RoomName << " ID :"
									<< Rooms[ready->RoomName].UserData[i].ID << " Is Ready" << endl;
								send(ptr->sock, (char *)&Data, sizeof(Data), 0);
								ReadyCount++;
							}
						}

						if (ReadyCount == Rooms[ready->RoomName].Total)
						{
							cout << "모든 유저들 준비완료" << endl;

							RoomData Data = Rooms[ready->RoomName];
							for (int i = 0; i < nTotalSockets; i++)
							{
								if (SocketInfoArray[i]->Type == INGAME)
								{
									Data.type = GAMESATRT;
									send(SocketInfoArray[i]->sock, (char*)&Data, sizeof(Data), 0);
								}
								if (SocketInfoArray[i]->Type == RELAY)
								{
									Data.type = GAMESATRT;
									send(SocketInfoArray[i]->sock, (char*)&Data, sizeof(Data), 0);
								}
							}

						}
					}
				}

				if (Type.type == GAMEEND)
				{
					desytroyRoom = (DestroyRoom *)&Type;
					cout << desytroyRoom->RoomName << " 방에서 게임이 종료되었습니다" << endl;
					if (Rooms.find(desytroyRoom->RoomName) != Rooms.end())
					{
						cout << desytroyRoom->RoomName << " 방에서 게임이 종료되었습니다" << endl;
						Rooms.erase(Rooms.find(desytroyRoom->RoomName));
					}
					
				}
				
				if (retval == 0 || retval == SOCKET_ERROR)
				{
					RemoveSocketInfo(i);
					continue;
				}


				// 받은 바이트 수 누적
				ptr->recvbytes += retval;

			}
		}
	}

	return 0;
}

// 소켓 정보 추가
BOOL AddSocketInfo(SOCKET sock)
{
	if (nTotalSockets >= FD_SETSIZE)
	{
		cout << "[오류] 소켓 정보를 추가할 수 없습니다!\n";
		return FALSE;
	}

	SOCKETINFO *ptr = new SOCKETINFO;
	if (ptr == NULL)
	{
		cout << "[오류] 메모리가 부족합니다!\n";
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

//사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}