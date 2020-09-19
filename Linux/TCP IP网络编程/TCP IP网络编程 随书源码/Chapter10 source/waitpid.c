#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int status;
	pid_t pid=fork();
	
	if(pid==0)
	{
		sleep(15);
		return 24;   	
	}
	else
	{
		while(!waitpid(-1, &status, WNOHANG))
		{
			sleep(3);
			puts("sleep 3sec.");
		}

		if(WIFEXITED(status))
			printf("Child send %d \n", WEXITSTATUS(status));
	}
	return 0;
}

/*
root@my_linux:/home/swyoon/tcpip# gcc waitpid.c -o waitpid
root@my_linux:/home/swyoon/tcpip# ./waitpid
sleep 3sec.
sleep 3sec.
sleep 3sec.
sleep 3sec.
sleep 3sec.
Child send 24 
*/
