#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>     // for struct ip_mreq

#define BUF_SIZE 30
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hRecvSock;
	SOCKADDR_IN adr;
	struct ip_mreq joinAdr;
	char buf[BUF_SIZE];
	int strLen;


	if(argc!=3) {
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
  
	hRecvSock=socket(PF_INET, SOCK_DGRAM, 0);
 	memset(&adr, 0, sizeof(adr));
	adr.sin_family=AF_INET;
	adr.sin_addr.s_addr=htonl(INADDR_ANY);	
	adr.sin_port=htons(atoi(argv[2]));
	
	if(bind(hRecvSock, (SOCKADDR*) &adr, sizeof(adr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	
	joinAdr.imr_multiaddr.s_addr=inet_addr(argv[1]);
	joinAdr.imr_interface.s_addr=htonl(INADDR_ANY);
  	
	if(setsockopt(hRecvSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
			(void*)&joinAdr, sizeof(joinAdr))==SOCKET_ERROR)
		ErrorHandling("setsock() error");
  
	while(1)
	{
		strLen=recvfrom(hRecvSock, buf, BUF_SIZE-1, 0, NULL, 0);
		if(strLen<0) 
			break;
		buf[strLen]=0;
		fputs(buf, stdout);
	}
	closesocket(hRecvSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}