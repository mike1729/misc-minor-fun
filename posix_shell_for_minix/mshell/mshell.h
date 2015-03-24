/*connects output stream with input stream between processes*/
void manage_pipes(int, int[2][2],int);

/*reads commands from input, splits and format them*/
int read_format_commands(int*);

/*run commands*/
void dispatch_commands(command_s*,int);

/*manages prompt and dead childs info displaying*/
int write_prompt_stats(int, int *);

/*changes process input stream*/
void open_in_file_name(command_s);

/*changes process output stream*/
void open_out_file_name(command_s);

/*chandler for SIGCHLD, waits for foreground childs and collects info about background childs*/
void handler(int);

/*checks if pid is a foreground job*/
int in_child_pids(int);

/*initializes hendlar*/
void set_handler(void);

/*finished background children info*/
struct {
	pid_t pid;
	int status;
} dead_child[DEAD_CHILDREN];

/*foreground children pids*/
int child_pids[BUFF_SIZE+1];

/*some ugly global vars*/
int childs_number, cur_child_count, fin_childs_number=0;
char prompt[10]=PROMPT, input_buf[BUFF_SIZE+1], tmp_buf[BUFF_SIZE+1], err[13]=RERROR_COM;
int cmd_ind[BUFF_SIZE>>1];

