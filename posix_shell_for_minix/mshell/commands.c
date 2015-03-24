#define _POSIX_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "commands.h"

extern char **environ;

int echo(char*[]);
int mexit(char*[]);
int mcd(char*[]);
int mkill(char*[]);
int lenv(char*[]);
int ls(char*[]);

command_pair dispatch_table[]={
	{"echo", &echo},
	{"exit", &mexit},
	{"cd", &mcd},
	{"kill", &mkill},
	{"lenv", &lenv},
	{"lls", &ls},
	{NULL,NULL}
};
int ls(argv)
char * argv[];
{
	char cwd[1024];
	DIR *dir;
	struct dirent *dp;
	
	if (getcwd(cwd,sizeof(cwd))==NULL)
	{
		printf("getcwd error\n");
	}
	if ( (dir = opendir(cwd)) == NULL)
	{
		printf("opendir error\n");
	}
	while ((dp = readdir(dir)) !=NULL)
		printf("%s ", dp->d_name);

	putchar('\n');
	return 0;
}

int lenv(argv)
char * argv[];
{
	char **env;
	int flag = O_WRONLY | O_CREAT, fd;
	mode_t mode = S_IRUSR | S_IWUSR;
		
	if ( argv[1] && strcmp(argv[1],">>")==0 )
		flag |= O_APPEND;
	if ( argv[1] && strcmp(argv[1],">")==0 )
		flag |= O_TRUNC;
	if (argv[2])
		fd=open(argv[2], flag, mode);
	else 
		fd=1;

	for (env = environ; *env; ++env)
	{
		write(fd, *env,strlen(*env));
		write(fd,"\n",1);
	}
	if (argv[2])
		close(fd);
	return 0;
}

int mkill(argv)
char * argv[];
{
	int signal;
	if ( !argv[2] )
		return kill( strtol(argv[1],NULL,10) ,SIGTERM);	
	
	return kill( strtol(argv[2],NULL,10), -strtol(argv[1],NULL,10)); 
}

int mcd(argv)
char * argv[];
{
	if (chdir(argv[1]))
	{
		puts("chdir error");
	}
	return 0;
}

int mexit(argv)
char * argv[];
{
	exit(1);
	return 0;
}

int echo(argv)
char * argv[];
{
	int i =1;
	if (argv[i]) printf("%s", argv[i++]);
	while  (argv[i])
		printf(" %s", argv[i++]);
	puts("");
	fflush(stdout);
	return 0;
}

