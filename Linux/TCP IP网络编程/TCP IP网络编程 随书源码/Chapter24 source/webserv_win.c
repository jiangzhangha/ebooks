#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#define BUF_SIZE  2048
#define BUF_SMALL  100

unsigned  WINAPI RequestHandler(void* arg);
char* ContentType(char* file);
void SendData(SOCKET sock, char* ct, char* fileName);
void SendErrorMSG(SOCKET sock);
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;

	HANDLE hThread;
	DWORD dwThreadID;
	int clntAdrSize;

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!");
	
	hServSock=socket(PF_INET, SOCK_STREAM, 0);   
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAdr.sin_port=htons(atoi(argv[1]));
	
	if(bind(hServSock, (SOCKADDR*) &servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hServSock, 5)==SOCKET_ERROR)
		ErrorHandling("listen() error");

	/* 요청 및 응답 */
	while(1)
	{
		clntAdrSize=sizeof(clntAdr);
		hClntSock=accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSize);
		printf("Connection Request : %s:%d\n", 
			inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));
		hThread=(HANDLE)_beginthreadex(
			NULL, 0, RequestHandler, (void*)hClntSock, 0, (unsigned *)&dwThreadID);
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI RequestHandler(void *arg)
{
	SOCKET hClntSock=(SOCKET)arg;
	char buf[BUF_SIZE];
	char method[BUF_SMALL];
	char ct[BUF_SMALL];
	char fileName[BUF_SMALL];
	
	recv(hClntSock, buf, BUF_SIZE, 0);
	if(strstr(buf, "HTTP/")==NULL)    // HTTP에 의한 요청인지 확인
	{
		SendErrorMSG(hClntSock);
		closesocket(hClntSock);
		return 1;
	}
	
	strcpy(method, strtok(buf, " /"));
	if(strcmp(method, "GET"))    // GET 방식 요청인지 확인 
		SendErrorMSG(hClntSock);

	strcpy(fileName, strtok(NULL, " /"));    // 요청 파일이름 확인
	strcpy(ct, ContentType(fileName));    // Content-type 확인	
	SendData(hClntSock, ct, fileName);    // 응  답
	return 0;
}

void SendData(SOCKET sock, char* ct, char* fileName) 
{	
	char protocol[]="HTTP/1.0 200 OK\r\n";
	char servName[]="Server:simple web server\r\n";
	char cntLen[]="Content-length:2048\r\n";
	char cntType[BUF_SMALL];
	char buf[BUF_SIZE];
	FILE* sendFile;

	sprintf(cntType, "Content-type:%s\r\n\r\n", ct);
	if((sendFile=fopen(fileName, "r"))==NULL)
	{
		SendErrorMSG(sock);
		return;
	}

	/* 헤더 정보 전송 */
	send(sock, protocol, strlen(protocol), 0);
	send(sock, servName, strlen(servName), 0);
	send(sock, cntLen, strlen(cntLen), 0); 
	send(sock, cntType, strlen(cntType), 0);

	/* 요청 데이터 전송 */
	while(fgets(buf, BUF_SIZE, sendFile)!=NULL) 
		send(sock, buf, strlen(buf), 0);

	closesocket(sock);   // HTTP 프로토콜에 의해서 응답 후 종료
}

void SendErrorMSG(SOCKET sock)   // 오류 발생시 메시지 전달
{	
	char protocol[]="HTTP/1.0 400 Bad Request\r\n";
	char servName[]="Server:simple web server\r\n";
	char cntLen[]="Content-length:2048\r\n";
	char cntType[]="Content-type:text/html\r\n\r\n";
	char content[]="<html><head><title>NETWORK</title></head>"
	       "<body><font size=+5><br>오류 발생! 요청 파일명 및 요청 방식 확인!"
		   "</font></body></html>";

	send(sock, protocol, strlen(protocol), 0);
	send(sock, servName, strlen(servName), 0);
	send(sock, cntLen, strlen(cntLen), 0); 
	send(sock, cntType, strlen(cntType), 0);
	send(sock, content, strlen(content), 0);
	closesocket(sock);
}

char* ContentType(char* file)    // Content-Type 구분
{
	char extension[BUF_SMALL];
	char fileName[BUF_SMALL];
	strcpy(fileName, file);
	strtok(fileName, ".");
	strcpy(extension, strtok(NULL, "."));
	if(!strcmp(extension, "html")||!strcmp(extension, "htm")) 
		return "text/html";
	else
		return "text/plain";
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}