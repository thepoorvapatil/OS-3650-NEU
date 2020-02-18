#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include "svec.h"
#include "ast.h"
#include "parse.h"

int
isOperator(const char* text, int ii){
	
	if(text[ii] == '<' || text[ii] == '>' || text[ii] == ';')
		return 1;

	if(text[ii] == '|'){
		if(text[ii+1] == '|')
			return 2;
		else 
			return 1;
	}

	if(text[ii] == '&'){
		if(text[ii+1] == '&')
			return 2;
		else 
			return 1;
	}
	

	return 0; 
}


char*
read_token(const char* text, int ii){
	int nn = 0;

	while (!isspace(text[ii+nn]) && isOperator(text, ii+nn) == 0)
		++nn;

	char* token = malloc (nn + 1);
	memcpy(token, text + ii, nn);
	token[nn] = 0;
	
	return token; 
}

svec* 
tokenize (const char* text) {
	
	svec* vector = make_svec();
	int nn = strlen(text);
	int ii = 0;
	char *token;

	while (ii < nn) {
	
		if(isspace(text[ii])){
			ii++; //if it is an space we skip it. 

		} else {
			int opLength = isOperator(text, ii);

			if (opLength == 0) {
				//it is not an operator
				token = read_token(text, ii);
				svec_push_back(vector, token);
				ii += strlen(token);	
			} else {
				//it is an operator
				token = malloc(opLength + 1);
				memcpy(token, text + ii, opLength);
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
	//Checking for built in commands.
	if( ast->isOperator = 0 ){

		if( strcmp(ast->cmd, "cd") == 0){
		//Change directory. 
		chdir(svec_get(ast->args, 0));
		}

		if(strcmp(ast->cmd, "exit") == 0) {
		//Exit built in.
		exit(0);
		}

	} else {
    		int cpid;

    		if ((cpid = fork())) {
        		// Parent process.
	
        		//printf("Parent pid: %d\n", getpid());
        		//printf("Parent knows child pid: %d\n", cpid);

        		// Child may still be running until we wait.

        		int status;
        		waitpid(cpid, &status, 0);

        		//printf("== executed program complete ==\n");

        		//printf("child returned with wait code %d\n", status);
        		if (WIFEXITED(status)) {
            		//printf("child exited with exit code (or main returned) %d\n", WEXITSTATUS(status));
        		}
   		 } else {
        		//Child process.
        		//printf("Child pid: %d\n", getpid());
        		//printf("Child knows parent pid: %d\n", getppid());

	
        		//printf("== executed program's output: ==\n");
	
			//Evaluating the abstract sintax tree to execute the command. 
        		ast_eval(ast);

        		//printf("Can't get here, exec only returns on error.\n");
    		}
	}
}

void processInput(char* cmd){
	
	//Converting command froms tring to resizable array of strings.
	svec *vector = tokenize(cmd);

	//Converting the array of strings into an Abstract Sintax Tree.
	shell_ast* ast = parse(vector);
	
	//Freeing array of strings because we don't need it anymore. 
	free(vector);
	
	//Executing command. 
	execute(ast);
}


int
main(int argc, char* argv[])
{

    		char cmd[256];

    		if (argc == 1) {
		//No script file introduced.

			while(1) {

				//Prompting.
        			printf("nush$ ");

				fflush(stdout);
			
				//Reading command.
        			char* rtrn = fgets(cmd, 256, stdin);
				if (!rtrn){
					//Breaking in case user wants to end. 
					printf("\n");
					break;
				}


				if(strlen(cmd) == 1){
					//Nothing introduced. 
					continue;
				}

				//Processing input.
				processInput(cmd);
			}

    		} else {
		//Script file introduced. 
			
			//Reading from the input file and storing it in buffer. 
			
			//Creating buffer for lines and opening file. 
        		char buffer[100];
			FILE* input = fopen(argv[1], "r");
			//Error check in fopen
			if (input == NULL){
				printf("Error in fopen %s\n", strerror(errno));
				exit(0);
			}

			//While there is a line to read
			while (fgets(buffer, 100, input) != NULL){
				//We process the command that each line repesents.
				processInput(buffer);
			}	
			
			//Closing file. 
			fclose(input);
			

    		}		
	
    return 0;
}
