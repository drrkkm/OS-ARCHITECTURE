#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
	pid_t p = fork();
	if (p == -1) { perror("fork"); return 1;}
	pid_t pid = getpid();
	pid_t ppid = getppid();

	if (p > 0){
	printf("PARENT: me: %d\n parent: %d\n", pid, ppid);
	wait(NULL);}
	else if  ( p == 0)
		{ 
		printf("me: %d\n parent: %d\n", pid, ppid);
		execlp("ls", "ls", "-l", "-a", NULL);}
	//pid_t p = fork();
	printf("fork: %d\n", p);
	return 0;
}
