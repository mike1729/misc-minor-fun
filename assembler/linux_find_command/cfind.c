#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE  512

extern int errno;

char full_path[4096];

struct linux_dirent {
  long           d_ino;
  off_t          d_off;
  unsigned short d_reclen;
  char           d_name[];
};

void process_path(char*, unsigned int);

int main(int argc, char** argv) {
  int fd;

  if (argc != 2) {
  	printf("ELOOP=%d\nENOENT=%d\nENOTDIR=%d\n",ELOOP ,ENOENT,ENOTDIR); 
  	return; 
  }
  //printf("O_RDONLY=%d\nO_NOFOLLOW=%d\n", O_RDONLY, O_NOFOLLOW);
  fd = open(argv[1], O_RDONLY | O_NOFOLLOW);
  
  if (fd==-1) 
  	switch (errno) {
  /*40*/case ELOOP:   	return;
  /*2 */case ENOENT:  	printf("find: %s: No such file or directory\n", argv[1]);
  			return;
  /*20*/case ENOTDIR:  	printf("find: %s: Not a directory\n", argv[1]);
  			return;
  	}
  close(fd);
  process_path(argv[1],0);
  return 0;
}

void process_path(char* path, unsigned int full_path_offset) {
  char name[256], buf[BUF_SIZE];
  int fd,num_read,bpos=0, old_full_path_offset=full_path_offset;
  struct linux_dirent *dir;
  
  strcpy(full_path+full_path_offset, path);
  
  full_path_offset += strlen(path);
  printf("%s\n",full_path);


  fd=open(full_path, O_RDONLY | O_NOFOLLOW);
  
  if (full_path_offset!=1 && full_path[full_path_offset-1]!='/')
  	full_path[full_path_offset++]='/';
  do {
  	num_read = syscall(SYS_getdents, fd, buf, BUF_SIZE);
  	//printf(" - %d   ||   ",num_read);
	for (bpos=0; bpos<num_read; ) {

		dir = (struct linux_dirent *) (buf + bpos);
		//printf("size =%d        ",dir->d_reclen);
		bpos += dir->d_reclen;
		if ( (strcmp(dir->d_name,".") !=0)  && ( strcmp(dir->d_name,"..")!=0) )
			process_path(dir->d_name, full_path_offset);
	}
  } while (num_read>0);
  full_path[old_full_path_offset]=0;
  close(fd);
}
