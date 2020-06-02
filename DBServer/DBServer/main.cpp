#include"SockData.h"

int main(int argc, char *argv[])
{
	::CoInitialize(NULL);	//컴포넌트 오브젝트로 만든 모델을 사용하겠다
	Database db;
	Table tbl;

	if (!db.Open("", "", CnnStr))
	{
		db.GetErrorErrStr(ErrStr);
		cout << ErrStr << endl;
	}
	else
	{
		cout << "Database Connect Success!!" << endl;
	}
	::CoUninitialize();

	//Select 문을 사용했을때
	/*if (!db.Execute("SELECT * FROM User", tbl))
	{
		db.GetErrorErrStr(ErrStr);
		cout << ErrStr << endl;
	}

	char id[100];
	if (!tbl.ISEOF())
	{
		tbl.MoveFirst();
	}

	while (!tbl.ISEOF())
	{
		if (tbl.Get("ID", id))
		{
			cout << "\nid:" << id;
		}
		else
		{
			tbl.GetErrorErrStr(ErrStr);
			cout << "\n" << ErrStr << "\n";
			break;
		}

		tbl.MoveNext();
	}*/

	MsgType Type;
	UserData *User;

	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.2");
	serveraddr.sin_port = htons(LOBBYPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	fd_set rset;

	sockType.type = SOCKTYPE;
	sockType.SockType = TYPE;
	send(sock, (char*)&sockType, BUFSIZE, 0);
	string Query;	//쿼리문

	while (1)
	{
		retval = recv(sock, (char *)&Type, BUFSIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR) {
			break;
		}

		if (Type.type == CHECKLOGIN)
		{
			User = (UserData *)&Type;
			string ID = User->ID;
			string PASSWORD = User->PASSWORD;
			Query = "SELECT ID,PASSWORD FROM User WHERE ID = '" + ID + "' AND PASSWORD = '" + PASSWORD + "'";
			const char *ch = Query.c_str();
			if (db.Execute(ch, tbl))
			{
				if (tbl.ISEOF())
				{
					cout << "ID, PASSWORD가 일치하는게 없습니다" << endl;
					UserInfo Data;
					Data.type = LOGINFAIL;
					strcpy(Data.ID, User->ID);
					send(sock, (char *)&Data, sizeof(Data), 0);
				}
				else
				{
					char id[60];
					char pwd[60];
					tbl.Get("ID", id);
					tbl.Get("PASSWORD", pwd);
					UserInfo Data;
					Data.type = LOGIN;
					strcpy(Data.ID, id);
					send(sock, (char *)&Data, sizeof(Data), 0);
				}
				
			}
		}

		if (Type.type == SIGNUP)
		{
			User = (UserData *)&Type;
			string ID = User->ID;
			string PASSWORD = User->PASSWORD;
			Query = "SELECT ID FROM User WHERE ID = '" + ID + "'";
			const char *ch = Query.c_str();
			if (db.Execute(ch, tbl))
			{
				if (tbl.ISEOF())
				{
					cout << "새로운 유저가 들어왔습니다 ID :"<< User->ID << endl;
					Query = "INSERT INTO User(ID,PASSWORD) VALUES ('" + ID + "' , '" +PASSWORD + "')";
					const char *ch = Query.c_str();
					if (db.Execute(ch, tbl))
					{
						cout << "Insert Success" << endl;
					}
					UserInfo Data;
					Data.type = SIGNOK;
					strcpy(Data.ID, User->ID);
					send(sock, (char *)&Data, sizeof(Data), 0);
				}
				else
				{
					char id[60];
					tbl.Get("ID", id);
					UserInfo Data;
					Data.type = SIGNFAIL;
					strcpy(Data.ID, id);
					send(sock, (char *)&Data, sizeof(Data), 0);
				}

			}
		}
	}

	closesocket(sock);
	return 0;

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