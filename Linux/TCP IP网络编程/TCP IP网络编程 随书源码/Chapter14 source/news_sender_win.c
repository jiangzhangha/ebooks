#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>     // for IP_MULTICAST_TTL option

#define TTL 64
#define BUF_SIZE 30
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hSendSock;
	SOCKADDR_IN mulAdr;
	int timeLive=TTL;
	FILE *fp;
	char buf[BUF_SIZE];

	if(argc!=3) {
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
  	
	hSendSock=socket(PF_INET, SOCK_DGRAM, 0);
	memset(&mulAdr, 0, sizeof(mulAdr));
	mulAdr.sin_family=AF_INET;
	mulAdr.sin_addr.s_addr=inet_addr(argv[1]); 
	mulAdr.sin_port=htons(atoi(argv[2]));      
	
	setsockopt(hSendSock, IPPROTO_IP, 
		IP_MULTICAST_TTL, (void*)&timeLive, sizeof(timeLive));
	
	if((fp=fopen("news.txt", "r"))==NULL)
		ErrorHandling("fopen() error");

	while(!feof(fp))
	{
		fgets(buf, BUF_SIZE, fp);
		sendto(hSendSock, buf, strlen(buf), 
			0, (SOCKADDR*)&mulAdr, sizeof(mulAdr));
		Sleep(2000);
	}
	closesocket(hSendSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}