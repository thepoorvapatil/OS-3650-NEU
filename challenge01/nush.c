#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "ast.h"
#include "parse.h"
#include "svec.h"

int
isOperator(const char* str, int ii){
	
	if(str[ii] == '<' || str[ii] == '>' || str[ii] == ';')
		return 1;

    //check for ||
	if(str[ii] == '|'){
		if(str[ii+1] == '|')
			return 2;
		else 
			return 1;
	}

    //check for &&
	if(str[ii] == '&'){
		if(str[ii+1] == '&')
			return 2;
		else 
			return 1;
	}
	return 0; 
}

char*
read_token(const char* str, int ii){
	int nn = 0;

	while (!isspace(str[ii+nn]) && isOperator(str, ii+nn) == 0)
		++nn;

	char* token = malloc (nn + 1);
	memcpy(token, str + ii, nn);
	token[nn] = 0;
	
	return token; 
}

svec* 
tokenize (const char* str) {
	
	svec* vector = make_svec();
	int nn = strlen(str);
	int ii = 0;
	char *token;

	while (ii < nn) {
	
		if(isspace(str[ii])){
			ii++; //if it is an space we skip it. 

		} else {
			int opLength = isOperator(str, ii);

			if (opLength == 0) {
				//it is not an operator
				token = read_token(str, ii);
				svec_push_back(vector, token);
				ii += strlen(token);	
			} else {
				//it is an operator
				token = malloc(opLength + 1);
				memcpy(token, str + ii, opLength);
				token[opLength] = 0;
				svec_push_back(vector, token);
				ii += opLength;		
			}
			free(token);

		}

	}

	return vector;
}

void
execute(shell_ast* ast)
{
    if( ast->isOperator = 0 ){

		if( strcmp(ast->cmd, "cd") == 0){
		//inbuilt cd
		chdir(svec_get(ast->args, 0));
		}

		if(strcmp(ast->cmd, "exit") == 0) {
		exit(0);
		}
    }
    else{
        int cpid;

        if ((cpid = fork())) {
            // parent process
            printf("Parent pid: %d\n", getpid());
            printf("Parent knows child pid: %d\n", cpid);

            // Child may still be running until we wait.

            int status;
            waitpid(cpid, &status, 0);

            printf("== executed program complete ==\n");

            printf("child returned with wait code %d\n", status);
            if (WIFEXITED(status)) {
                printf("child exited with exit code (or main returned) %d\n", WEXITSTATUS(status));
            }
        }
        else {
            // child process
            printf("Child pid: %d\n", getpid());
            printf("Child knows parent pid: %d\n", getppid());

            for (int ii = 0; ii < strlen(cmd); ++ii) {
                if (cmd[ii] == ' ') {
                    cmd[ii] = 0;
                    break;
                }
            }

            // The argv array for the child.
            // Terminated by a null pointer.
            char* args[] = {cmd, "one", 0};

            printf("== executed program's output: ==\n");

            execvp(cmd, args);
            printf("Can't get here, exec only returns on error.");
        }
    }
}

int
main(int argc, char* argv[])
{
    char cmd[256];

    if (argc == 1) {
        while (1)
        {
            printf("nush$ ");
            fflush(stdout);
            fgets(cmd, 256, stdin);

            if (!cmd){
                printf("\n");
                break;
            }

            if(strlen(cmd)==1){
                continue;
            }

            svec *vector = tokenize(cmd);

            //Converting the array of strings into an Abstract Sintax Tree.
            shell_ast* ast = parse(vector);
            
            //Freeing array of strings because we don't need it anymore. 
            free(vector);
            
            //Executing command. 
            execute(ast);
        }
        
        
    }
    //read from file given as argument.
    else {
        // memcpy(cmd, "echo", 5);
        char buffer[100];
        FILE* f = fopen(argv[1], "r");
        //check for file error
        if (f == NULL){
			printf("Error in fopen %s\n", strerror(errno));
			exit(0);
		}
        //read  file

        while (fgets(buffer, 100, f) != NULL){
            svec *vector = tokenize(cmd);

            //Converting the array of strings into an Abstract Sintax Tree.
            shell_ast* ast = parse(vector);
            
            //Freeing array of strings because we don't need it anymore. 
            free(vector);
            
            //Executing command. 
            execute(ast);
			
		}	

        //close file
        fclose(f);

    }

    execute(cmd);

    return 0;
}
