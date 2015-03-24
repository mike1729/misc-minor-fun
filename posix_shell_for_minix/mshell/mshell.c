#define _POSIX_SOURCE 1

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "config.h"
#include "commands.h"
#include "cparse.h"
#include "mshell.h"


int main(argc, argv)
int argc;
char* argv[];
{
	int i, commands_count, N=0, in_bgrd, slen, dnt_wr_prpt=0;
	command_s * commands;
	struct stat st;

	set_handler();

	/*check out what is hiding in 0*/
	if ( fstat(0, &st)==-1 ) 	write( 1, err, 12);
	if ( !S_ISCHR(st.st_mode) ) 	slen=0;
	else				slen=strlen(prompt);
		
	while(1)
	{
		write_prompt_stats(slen, &dnt_wr_prpt);
				
		if ((commands_count=read_format_commands(&N)) == -1) break;
		if (commands_count==0) dnt_wr_prpt=1;
		for (i=0; i<commands_count; i++)
		{
			/*check if it is background job*/
			in_bgrd=in_background(input_buf+cmd_ind[i]);
			/*split commands*/
			commands = split_commands(input_buf+cmd_ind[i]);

			if( commands == NULL)
			{
				write(1, "commands=NULL\n", 14);
				break;
			}
			/*do the job*/
			dispatch_commands(commands,in_bgrd);
		}
	}
	/*say goodbay*/
	printf("Exit.\n");
	return 0;
}

int write_prompt_stats(int slen, int *dnt_wr_prpt)
{
	int i,n;
	char s[100];
	if (slen)
		for (i=0; i<fin_childs_number; i++)
		{
			n=0;
			/*print child status*/
			if (WIFEXITED(dead_child[i].status))
				n=sprintf(s, "Process (%d) ended with status (%d).\n", dead_child[i].pid, WEXITSTATUS(dead_child[i].status));
			else if (WIFSIGNALED(dead_child[i].status))
				n=sprintf(s, "Process (%d) ended killed by signal (%d).\n",dead_child[i].pid,WTERMSIG(dead_child[i].status));
			s[n]=0;
			write(1,s,n);
		}
	/*reset finished childs count*/
	fin_childs_number=0;
	/*manage prompt displaying*/
	if (*dnt_wr_prpt)
		*dnt_wr_prpt=0;
	else
		write(1, prompt, slen);
}

int read_format_commands(int* N)
{
	int i,k,p=0,commands_count;
	
	/*rewrite old trailng input*/
	for (i=0; i<*N; i++)  input_buf[i] = tmp_buf[i];
		
	/*read new input*/
	do k = read(0, input_buf+*N, BUFF_SIZE-*N);
	while (k==-1 && errno==EINTR);
	if (k == -1)
	{
		write(1, "k=-1\n", 5);
		return -1;
	} 
	if (k == 0)
		return -1;
			

	/*read unnecessary white space*/
	for (i=0; i<k+*N; i++) 
		if (input_buf[i]!='\n')
			break;
	/*split input into commands*/
	commands_count=0;
	cmd_ind[0]=0;			
	while (i<k+*N) 
	{
		if (input_buf[i]=='\n')  
		{
			commands_count++;
			while (i<k+*N && input_buf[i]=='\n')
				input_buf[i++]=0;
			p = i;
			cmd_ind[commands_count]=i;
		}
		else i++;
	}
	/*remember tail of possibly not full command*/
	if (p<k+*N)
		for (i=p;i<k+*N;i++)
		{
			tmp_buf[i-p] = input_buf[i];
			input_buf[i] = 0;
		}
	*N += k-p;
	
	return commands_count;
}

void dispatch_commands(command_s * commands,int in_bgrd)
{
	command_s cmd;
	int filedes[2][2];
	int i=0, j, last_flag=0, f, skip=0, stat_lock;
	sigset_t mask;

	/*init mask*/
	if(sigemptyset(&mask)) write(1,"sigemptyset error\n",50);
	if(sigaddset(&mask,SIGCHLD)) write(1,"sigaddset error\n",50);
	if(sigprocmask(SIG_BLOCK, &mask, NULL)) write(1,"sigprocmask error\n",50);
	/*run commands*/
	while( (cmd=commands[i]).argv != NULL)
	{
		skip=0;
		/*create new pipe to connect to next process*/
		if( commands[i+1].argv!=NULL )
		{
			if( pipe(filedes[i&1]) <0)
			{
				write(1, "pipe function error\n",23);
				exit(1);	
			}
		}
		else	last_flag=1;
		
		/*check if job is a shell command*/
		for (j=0;j<6;j++)
			if( strcmp( cmd.argv[0], dispatch_table[j].name) ==0 )
			{
				dispatch_table[j].fun(cmd.argv);
				skip=1;
			}
		
		if ((f=fork())<0)
			write(1,"start panic! fork has failed\n",33);
		if( !f ) /*child process*/
		{
			/*change and/or close descriptors*/
			manage_pipes(i, filedes, last_flag);		
			/*change in*/	
			if (cmd.in_file_name != NULL)
				open_in_file_name(cmd);
			/*change out*/
			if (cmd.out_file_name != NULL)
				open_out_file_name(cmd);	
			/*background job*/
			if (in_bgrd)
				setsid();
			/*skip for a shell command*/
			if (!skip){
				execvp(cmd.argv[0], cmd.argv); 
			}
			exit(1);
		}
		else /*parrent process*/
		{
			/*remember child process id*/
			if (!in_bgrd)
				child_pids[i]=f;
			
			/*free unused descriptors*/
			if (i!=0)
			{
				close(filedes[1-i&1][0]);
				close(filedes[1-i&1][1]);
			}
			i++;
		}	 
	}
	/*clear mask*/
	cur_child_count = childs_number = i;
	if (sigemptyset(&mask)) write(1,"sigemptyset error\n",50);
	if (sigfillset(&mask)) write(1,"sigfillset error\n",50);
	if (sigdelset(&mask, SIGCHLD)) write(1,"sigdelset error\n",50);
	/*wait for children from foreground*/
	if (!in_bgrd)
		while (cur_child_count>0) sigsuspend(&mask);
	if (sigemptyset(&mask)) write(1,"sigemptyset error'\n",50);
	if (sigaddset(&mask,SIGCHLD)) write(1,"sigaddset error'\n",50);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL)) write(1,"sigprocmask error'\n",50);
}

void manage_pipes(int i, int filedes[2][2], int last_flag)
{
	int a=i&1;
	if (i!=0)
	{
		close(0);
		/*make last process out a curent process in*/
		if (fcntl( filedes[1-a][0], F_DUPFD,0) != 0)
		{
			write(2,"error in connecting last process out with current process in (in pipe)\n", 80);
			exit(1);
		}
		close( filedes[1-a][0] );	
		close( filedes[1-a][1] );
	}
	if (!last_flag)
	{
		close(1);
		/*make current process out a next process in*/ 
		if (fcntl( filedes[a][1], F_DUPFD,1) != 1)
		{
			write(2,"error in connecting current process out with next process in (in pipe)\n",80);
			exit(1);
		}
		close ( filedes[a][0] );
		close ( filedes[a][1] );
	}
}

void open_in_file_name(command_s cmd)
{
	/*change input stream*/
	close(0);
	if( open(cmd.in_file_name, O_RDONLY) != 0)
	{
		write(2,"error in opening in_file_name\n",35);
		exit(1);
	}
}

void open_out_file_name(command_s cmd)
{
	/*change output stream*/
	int flag = O_WRONLY | O_CREAT;
	mode_t mode = S_IRUSR | S_IWUSR;
	close(1);
	if ( cmd.append_mode ) 	flag |= O_APPEND;
	else 			flag |= O_TRUNC;
	if ( open(cmd.out_file_name , flag, mode) != 1)
	{
		write(2,"error in opening out_file_name\n",35);
		exit(1);
	}
}

void handler(int x)
{
	pid_t child;
	int stat_loc;
	/*wait for childs and remember finished childs pids*/
	while (1) 
	{
		child=waitpid(-1,&stat_loc,WNOHANG);
		if (child<=0) break;
		if (in_child_pids(child)) cur_child_count--;
		else if (fin_childs_number < DEAD_CHILDREN)
		{
			dead_child[fin_childs_number].pid=child;
			dead_child[fin_childs_number++].status=stat_loc;
		}
	}
}

int in_child_pids(int child)
{
	int i=-1;
	while (++i<childs_number)
		if (child_pids[i]==child)
		{
			child_pids[i]=-1;
			return 1;
		}
	return 0;
}

void prosthesis_handler(int x)
{
}

void set_handler()
{
	struct sigaction act;
	act.sa_handler=handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGCHLD, &act, NULL);
	act.sa_handler=prosthesis_handler;
	sigaction(SIGINT, &act, NULL);
	
}
