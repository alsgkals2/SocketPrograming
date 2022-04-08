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
unsigned WINAPI HandleClient(void* arg);//������ �Լ�
void SendMsg(char* msg, int len);//�޽��� ������ �Լ�

int main(int argc, char* argv[])
{
	int s = 0;
	char line[MAXLINE] = { 0, };
	char message[MAXLINE + 1] = { 0, };
	u_long iMode = 1;
	int port_no = 1;
	fd_set  read_fds;//�б⸦ ������ ���Ϲ�ȣ ����ü
	unsigned int set = 1;
	WSADATA wasData;
	//����
	///param1 : ����
	///param2 : wasdata�� �����Ͽ� ���� ����ü ������ �޾ƿ�
	if (WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
		cout << "����" << endl;
	//param = �ּ� ü�� , ����Ÿ�� , ��������
	///���� Ÿ�� = SOCK_STREAM(��������) , SOCK_DGRAM(�񿬰�����) ���� ���е�
	///�������� = IPPROTO_TCP, IPPROTO_UDP�� ����(���ϰ� �ָ����ָ��)
	hMutex = CreateMutex(NULL, false, NULL);
	s = socket(AF_INET, SOCK_STREAM, 0);
	//s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		cout << "����2" << endl;

	//��Ĺ ���� �Է�
	sockaddr_in sockinfo;
	sockinfo.sin_family = AF_INET;
	sockinfo.sin_port = htons(1234);
	sockinfo.sin_addr.s_addr = htonl(
	                               INADDR_ANY);//��ǻ�� ���� ������ �ڽ��� IP�ּҸ� �Ҵ�.
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&set, sizeof(set));


	// bind(���ͳݿ� ����)
	if (bind(s, (SOCKADDR*)&sockinfo, sizeof(sockinfo)) == SOCKET_ERROR)
		cout << "����3" << endl;

	// listen(5���� ��ΰ� ����)
	if (listen(s, 5) == SOCKET_ERROR)
		cout << "��⿭ ����" << endl;


	//accept(Ŭ���̾�Ʈ�� ���ӽõ�)

	///FD(���ϵ�ũ����) : ������ ������ ���� ������
	SOCKET clientsock;
	struct sockaddr_in clientinfo;
	int clientsize = sizeof(clientinfo);
	printf("Ŭ���̾�Ʈ�κ��� ������ ��ٸ��� �ֽ��ϴ�...\n");

	int maxfdp = s + 1; //+1 ����߸���

	while (1)
	{
		///fd_set�� �ʱ�ȭ�Ҷ� ���
		FD_ZERO(&read_fds);
		FD_SET(s, &read_fds);
		//FD_SET(0, &read_fds); //select�� 0������ ����
		///read_fds ��Ʈ�� client_s[i]��° ��Ʈ�� 1�� �����
		for (int i = 0; i < num_chat; i++)
		{
			//cout << "fd_set ȣ��" << endl;
			FD_SET(client_s[i], &read_fds);
		}
		///select�Լ� : fd_set ����ü�� �Ҵ�� FD�� �̺�Ʈ�� �߻��ϸ� � FD�� �̺�Ʈ�� �߻��ߴ��� �ˤ���
		///maxfdp�̸��� FD�� üũ�ϸ�. maxfdp�� 2�ϰ�� read_fds�� 010000~~���� 1�� �Ѱ��� �����ϰԵ�
		maxfdp = getmax(s) + 1;
		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0)
		{
			printf("select error<=0\n");
			exit(0);
		}

		//cout << "select end" << endl;

		//_beginthread
		///read_fds�� s FD�� �̺�Ʈ�� �߻��Ͽ��ٸ�
		if (FD_ISSET(s, &read_fds))
		{
			clientsock = accept(s, (SOCKADDR*)&clientinfo, &clientsize);
			if (clientsock == INVALID_SOCKET)
				cout << "���� ���� ����" << endl;
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
			//cout << num_chat + 1 << "��° ����� �߰�" << endl;
			num_chat++;
		}
		WaitForSingleObject(hMutex, INFINITE);
		ReleaseMutex(hMutex);
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (VOID*)&clientsock, 0,
		                                 NULL);
	}
	//��������
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
	//cout << "HandleClient ����" << endl;
	SOCKET clientSock = *((SOCKET*)
	                      arg); //�Ű������ι��� Ŭ���̾�Ʈ ������ ����
	int strLen = 0, i;
	char msg[100];

	while ((strLen = recv(clientSock, msg, sizeof(msg),
	                      0)) != 0) //Ŭ���̾�Ʈ�κ��� �޽����� ���������� ��ٸ���.
		SendMsg(msg, strLen);//SendMsg�� ���� �޽����� �����Ѵ�.

	//�� ���� �����Ѵٴ� ���� �ش� Ŭ���̾�Ʈ�� �����ٴ� ����� ���� �ش� Ŭ���̾�Ʈ�� �迭���� �����������
	(hMutex, INFINITE);//���ؽ� ����
	for (i = 0; i < num_chat; i++) //�迭�� ������ŭ
	{
		if (clientSock ==
		    client_s[i])  //���� ���� clientSock���� �迭�� ���� ���ٸ�
		{
			while (i++ < num_chat - 1) //Ŭ���̾�Ʈ ���� ��ŭ
				client_s[i] = client_s[i + 1];//������ �����.
			break;
		}
	}
	ReleaseMutex(hMutex);//���ؽ� ����
	//cout << "HandleClient ����" << endl;
	return 0;
}
void SendMsg(char* msg, int
             len)   //�޽����� ��� Ŭ���̾�Ʈ���� ������.
{
	//cout << "SendMsg ����" << endl;

	int i;
	WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
	for (i = 0; i < num_chat; i++) //Ŭ���̾�Ʈ ������ŭ
		send(client_s[i], msg, len,
		     0);//Ŭ���̾�Ʈ�鿡�� �޽����� �����Ѵ�.
	ReleaseMutex(hMutex);//���ؽ� ����
	//cout << "SendMsg ����" << endl;
}