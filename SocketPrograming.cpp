// SocketPrograming.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <iostream>
#include <conio.h>
#include <process.h>
//#include <unistd>
using namespace std;
#define MAXLINE 128

int getmax(int);
int num_chat = 0;
int client_s[5];
HANDLE hThread;
HANDLE hMutex;
unsigned WINAPI HandleClient(void* arg);//쓰레드 함수
void SendMsg(char* msg, int len);//메시지 보내는 함수

int main(int argc, char* argv[])
{
	int s = 0;
	char line[MAXLINE] = { 0, };
	char message[MAXLINE + 1] = { 0, };
	u_long iMode = 1;
	int port_no = 1;
	fd_set  read_fds;//읽기를 감지할 소켓번호 구조체
	unsigned int set = 1;
	WSADATA wasData;
	//생성
	///param1 : 버전
	///param2 : wasdata를 전달하여 윈속 구조체 정보를 받아옴
	if (WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
		cout << "실패" << endl;
	//param = 주소 체계 , 소켓타입 , 프로토콜
	///소켓 타입 = SOCK_STREAM(연결지향) , SOCK_DGRAM(비연결지향) 으로 구분됨
	///프로토콜 = IPPROTO_TCP, IPPROTO_UDP로 구분(소켓과 쌍맞춰주면됨)
	hMutex = CreateMutex(NULL, false, NULL);
	s = socket(AF_INET, SOCK_STREAM, 0);
	//s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		cout << "실패2" << endl;

	//소캣 정보 입력
	sockaddr_in sockinfo;
	sockinfo.sin_family = AF_INET;
	sockinfo.sin_port = htons(1234);
	sockinfo.sin_addr.s_addr = htonl(
	                               INADDR_ANY);//컴퓨터 마다 고유의 자신의 IP주소를 할당.
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&set, sizeof(set));


	// bind(인터넷에 연결)
	if (bind(s, (SOCKADDR*)&sockinfo, sizeof(sockinfo)) == SOCKET_ERROR)
		cout << "실패3" << endl;

	// listen(5개의 통로가 개설)
	if (listen(s, 5) == SOCKET_ERROR)
		cout << "대기열 실패" << endl;


	//accept(클라이언트가 접속시도)

	///FD(파일디스크립터) : 소켓이 가지는 고유 정숫값
	SOCKET clientsock;
	struct sockaddr_in clientinfo;
	int clientsize = sizeof(clientinfo);
	printf("클라이언트로부터 접속을 기다리고 있습니다...\n");

	int maxfdp = s + 1; //+1 해줘야만함

	while (1)
	{
		///fd_set를 초기화할때 사용
		FD_ZERO(&read_fds);
		FD_SET(s, &read_fds);
		//FD_SET(0, &read_fds); //select가 0번까지 감시
		///read_fds 비트의 client_s[i]번째 비트가 1로 저장됨
		for (int i = 0; i < num_chat; i++)
		{
			//cout << "fd_set 호출" << endl;
			FD_SET(client_s[i], &read_fds);
		}
		///select함수 : fd_set 구조체에 할당된 FD의 이벤트가 발생하면 어떤 FD에 이벤트가 발생했는지 알ㄹ줌
		///maxfdp미만의 FD만 체크하며. maxfdp가 2일경우 read_fds는 010000~~으로 1이 한개만 존재하게됨
		maxfdp = getmax(s) + 1;
		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0)
		{
			printf("select error<=0\n");
			exit(0);
		}

		//cout << "select end" << endl;

		//_beginthread
		///read_fds의 s FD에 이벤트가 발생하였다면
		if (FD_ISSET(s, &read_fds))
		{
			clientsock = accept(s, (SOCKADDR*)&clientinfo, &clientsize);
			if (clientsock == INVALID_SOCKET)
				cout << "소켓 연결 실패" << endl;
		}

		//write()
		bool bExist = false;
		for (int i = 0; i < num_chat; i++)
		{
			if (client_s[i] == clientsock)
			{
				bExist = true;
				//break;
			}
		}
		if (!bExist)
		{
			client_s[num_chat] = clientsock;
			//cout << num_chat + 1 << "번째 사용자 추가" << endl;
			num_chat++;
		}
		WaitForSingleObject(hMutex, INFINITE);
		ReleaseMutex(hMutex);
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (VOID*)&clientsock, 0,
		                                 NULL);
	}
	//연결해제
	closesocket(s);
	closesocket(clientsock);
	WSACleanup();
	return 0;
}


int getmax(int k)
{
	int max = k;
	int r;
	for (r = 0; r < num_chat; r++)
	{
		if (client_s[r] > max) max = client_s[r];
	}
	return max;
}
unsigned WINAPI HandleClient(void* arg)
{
	//cout << "HandleClient 진입" << endl;
	SOCKET clientSock = *((SOCKET*)
	                      arg); //매개변수로받은 클라이언트 소켓을 전달
	int strLen = 0, i;
	char msg[100];

	while ((strLen = recv(clientSock, msg, sizeof(msg),
	                      0)) != 0) //클라이언트로부터 메시지를 받을때까지 기다린다.
		SendMsg(msg, strLen);//SendMsg에 받은 메시지를 전달한다.

	//이 줄을 실행한다는 것은 해당 클라이언트가 나갔다는 사실임 따라서 해당 클라이언트를 배열에서 제거해줘야함
	(hMutex, INFINITE);//뮤텍스 실행
	for (i = 0; i < num_chat; i++) //배열의 갯수만큼
	{
		if (clientSock ==
		    client_s[i])  //만약 현재 clientSock값이 배열의 값과 같다면
		{
			while (i++ < num_chat - 1) //클라이언트 개수 만큼
				client_s[i] = client_s[i + 1];//앞으로 땡긴다.
			break;
		}
	}
	ReleaseMutex(hMutex);//뮤텍스 중지
	//cout << "HandleClient 나감" << endl;
	return 0;
}
void SendMsg(char* msg, int
             len)   //메시지를 모든 클라이언트에게 보낸다.
{
	//cout << "SendMsg 진입" << endl;

	int i;
	WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
	for (i = 0; i < num_chat; i++) //클라이언트 개수만큼
		send(client_s[i], msg, len,
		     0);//클라이언트들에게 메시지를 전달한다.
	ReleaseMutex(hMutex);//뮤텍스 중지
	//cout << "SendMsg 나감" << endl;
}