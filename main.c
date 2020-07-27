#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <linux/limits.h>
#include <errno.h>

#include "cmds.h"
#include "errors.h"

#define BUFSIZE 256


void print_err();

void console_loop();
int handle_cmds(char *user_input);

static pid_t child;
static int status;

int main(int argc, char *argv[]){
	struct stat path_stat;
	long err;
	
	if (argc < 2){
		fprintf(stderr, "ERROR: Please provide the executable file\n");
		goto exit;
	}
	else if (argc > 2){
		fprintf(stderr, "ERROR: Too many arguments specified\n");	
		goto exit;
	}

	if (access(argv[1], F_OK) != 0){
		print_err();
		goto exit;
	}

	lstat(argv[1], &path_stat);
    if(!S_ISREG(path_stat.st_mode)){
    	fprintf(stderr, "ERROR: Argument provided is not a file\n");
    	goto exit;
    }

    child = fork();
    if(child == 0){
		//we are in the child process
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		kill(getpid(), SIGSTOP);
		execl(argv[1], argv[1], NULL);
	}
	else{
		wait(&status);
		if(WIFSTOPPED(status)){
			//run console
			console_loop();		
		}
	}

exit:
	return 0;
}

void console_loop(){
	char *user_input = (char *)malloc(BUFSIZE);
	int err;
	while(1){
		printf("(cdb) >> ");
		fgets(user_input, BUFSIZE, stdin);
		user_input[strlen(user_input)-1] = '\0';
		err = handle_cmds(user_input);
		if(err == ERR_KILLED){
			break;
		}
	}

exit:
	if(user_input){
		free(user_input);
	}
}

int handle_cmds(char *user_input){
	//handle commands

	//TODO: Fix and improve error handling
	long err = 0;
	if (!strcmp(RUN_CMD, user_input)){
		err = ptrace(PTRACE_CONT, child, 0, 0);
		if(err == -1)
			print_err();
	}
	else if(!strcmp(CONT_CMD, user_input)){
		err = ptrace(PTRACE_CONT, child, 0, 0);
		if(err == -1)
			print_err();
	}
	else if(!strcmp(QUIT_CMD, user_input)){
		err = ptrace(PTRACE_KILL, child, 0, 0);
		if(err == -1)
			print_err();
		else
			err = ERR_KILLED;
	}
	else if(!strcmp(BP_CMD, user_input)){
		//TODO: Implement after disassembler
	}
	else if(!strcmp(DISAS_CMD, user_input)){
		//TODO: Use 3rd party disassembler
	}
	else{
		err = ERR_UNSUPPORTED_CMD;
		fprintf(stderr, "CmdError: Command not supported\n");
	}
	return err;
}

void print_err(){
	fprintf(stderr, "ERROR: %s\n", strerror(errno));
}