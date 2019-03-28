#include <stdio.h>
#include <windows.h>
#include <process.h>

#define NUM_THREAD	50
unsigned WINAPI threadInc(void * arg);
unsigned WINAPI threadDes(void * arg);
long long num=0;

int main(int argc, char *argv[]) 
{
	HANDLE tHandles[NUM_THREAD];
	int i;

	printf("sizeof long long: %d \n", sizeof(long long));
	for(i=0; i<NUM_THREAD; i++)
	{
		if(i%2)
		    tHandles[i]=(HANDLE)_beginthreadex(NULL, 0, threadInc, NULL, 0, NULL);
		else
		    tHandles[i]=(HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
	}

	WaitForMultipleObjects(NUM_THREAD, tHandles, TRUE, INFINITE);
	printf("result: %lld \n", num);
	return 0;
}

unsigned WINAPI threadInc(void * arg) 
{
	int i;
	for(i=0; i<50000000; i++)
		num+=1;
	return 0;
}
unsigned WINAPI threadDes(void * arg)
{
	int i;
	for(i=0; i<50000000; i++)
		num-=1;
	return 0;
}