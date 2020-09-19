#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char *message);

typedef struct
{
	SOCKET hClntSock;
	char buf[BUF_SIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;	
	SOCKADDR_IN lisnAdr, recvAdr;
	LPWSAOVERLAPPED lpOvLp;
	DWORD recvBytes;
	LPPER_IO_DATA hbInfo;
	int mode=1, recvAdrSz, flagInfo=0;
	
	if(argc!=2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!"); 

	hLisnSock=WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	ioctlsocket(hLisnSock, FIONBIO, &mode);   // for non-blocking socket

	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family=AF_INET;
	lisnAdr.sin_addr.s_addr=htonl(INADDR_ANY);
	lisnAdr.sin_port=htons(atoi(argv[1]));

	if(bind(hLisnSock, (SOCKADDR*) &lisnAdr, sizeof(lisnAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hLisnSock, 5)==SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAdrSz=sizeof(recvAdr);    

	while(1)
	{
		SleepEx(100, TRUE);    // for alertable wait state
		hRecvSock=accept(hLisnSock, (SOCKADDR*)&recvAdr,&recvAdrSz);
		if(hRecvSock==INVALID_SOCKET)
		{
			if(WSAGetLastError()==WSAEWOULDBLOCK)
				continue;
			else
				ErrorHandling("accept() error");
		}
		puts("Client connected.....");

		lpOvLp=(LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		hbInfo=(LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		hbInfo->hClntSock=(DWORD)hRecvSock;
		(hbInfo->wsaBuf).buf=hbInfo->buf;
		(hbInfo->wsaBuf).len=BUF_SIZE;

		lpOvLp->hEvent=(HANDLE)hbInfo;
		WSARecv(hRecvSock, &(hbInfo->wsaBuf), 
			1, &recvBytes, &flagInfo, lpOvLp, ReadCompRoutine);
	}
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

void CALLBACK ReadCompRoutine(
	DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo=(LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock=hbInfo->hClntSock;
	LPWSABUF bufInfo=&(hbInfo->wsaBuf);
	DWORD sentBytes;

	if(szRecvBytes==0)
	{
		closesocket(hSock);
		free(lpOverlapped->hEvent); free(lpOverlapped);
		puts("Client disconnected.....");
	}
	else    // echo!
	{
		bufInfo->len=szRecvBytes;
		WSASend(hSock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine);
	}
}

void CALLBACK WriteCompRoutine(
	DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo=(LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock=hbInfo->hClntSock;
	LPWSABUF bufInfo=&(hbInfo->wsaBuf);
	DWORD recvBytes;
	int flagInfo=0;
	WSARecv(hSock, bufInfo,
		1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
