#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <linux/limits.h>

int main(int argc, char *argv[]){
	struct stat path_stat;
	pid_t child;
	int status;
	char *quit_str = (char *)malloc(20);
	// long err;

	if (argc < 2){
		fprintf(stderr, "ERROR: Please provide the executable file\n");
		goto exit;
	}
	else if (argc > 2){
		fprintf(stderr, "ERROR: Too many arguments specified\n");	
		goto exit;
	}

    lstat(argv[1], &path_stat);
    if(!S_ISREG(path_stat.st_mode)){
    	fprintf(stderr, "ERROR: Argument provided is not a file\n");
    	goto exit;
    }
	if (access(argv[1], F_OK) == 0){
		
		child = fork();
		if(child == 0){
			//we are in the child process
			ptrace(PTRACE_TRACEME, 0, NULL, NULL);
			kill(getpid(), SIGSTOP);
			execl(argv[1], argv[1], NULL);
		}
		else{
			waitpid(child, &status, 0);
			printf("Child has stopped!\n");
			fgets(quit_str, 20, stdin);
			ptrace(PTRACE_CONT, child, 0, 0);


		}

	}
	else{
		fprintf(stderr, "File does not exist in the current directory!\n");
	}

exit:
	if (quit_str){
		free(quit_str);
	}
	return 0;
}