#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	struct sockaddr_in addr1, addr2;
	char *str_ptr;
	char str_arr[20];
   
	addr1.sin_addr.s_addr=htonl(0x1020304);
	addr2.sin_addr.s_addr=htonl(0x1010101);
	
	str_ptr=inet_ntoa(addr1.sin_addr);
	strcpy(str_arr, str_ptr);
	printf("Dotted-Decimal notation1: %s \n", str_ptr);
	
	inet_ntoa(addr2.sin_addr);
	printf("Dotted-Decimal notation2: %s \n", str_ptr);
	printf("Dotted-Decimal notation3: %s \n", str_arr);
	return 0;
}

/*
root@com:/home/swyoon/tcpip# gcc inet_ntoa.c -o ntoa
root@com:/home/swyoon/tcpip# ./ntoa
Dotted-Decimal notation1: 1.2.3.4 
Dotted-Decimal notation2: 1.1.1.1 
Dotted-Decimal notation3: 1.2.3.4 

*/
