#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include "parse.h"
#include "svec.h"
#include "ast.h"

int
isOperator(const char* str, int ii){
	
	if(str[ii] == '<' || str[ii] == '>' || str[ii] == ';')
		return 1;

	if(str[ii] == '|'){
		if(str[ii+1] == '|')
			return 2;
		else 
			return 1;
	}

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
	if( ast->isOperator = 0 ){

		if( strcmp(ast->cmd, "cd") == 0){
            //built in function to cd
            chdir(svec_get(ast->args, 0));
		}
		if(strcmp(ast->cmd, "exit") == 0) {
		    exit(0);
		}

	} else {
    	int cpid;
        if ((cpid = fork())) {
            // Parent process.

            int status;
            waitpid(cpid, &status, 0);

            // if (WIFEXITED(status)) {
            // 	printf("child exited with exit code (or main returned) %d\n", WEXITSTATUS(status));
            // }
        } 
        else {
            //Child process.
            ast_eval(ast);
        }
	}
}

void processInput(char* cmd){
	
	svec *vector = tokenize(cmd);
	shell_ast* ast = parse(vector);
	free(vector);
	execute(ast);
}


int
main(int argc, char* argv[])
{

    		char cmd[256];

    		if (argc == 1) {

			while(1) {

        		printf("nush$ ");
				fflush(stdout);
			    fgets(cmd, 256, stdin);

				// if (!rtrn){
				// 	//Breaking in case user wants to end. 
				// 	printf("\n");
				// 	break;
				// }

				if(strlen(cmd) == 1){
					continue;
				}

				svec *vector = tokenize(cmd);
                shell_ast* ast = parse(vector);
                free(vector);
                execute(ast);
			}

    		} 
            else {
                //file input
            
        		char buffer[100];
			    FILE* f = fopen(argv[1], "r");
			    //Error check in fopen
			    if (f == NULL){
				printf("Error in fopen %s\n", strerror(errno));
				exit(0);
			}

			while (fgets(buffer, 100, f) != NULL){
				svec *vector = tokenize(buffer);
                shell_ast* ast = parse(vector);
                free(vector);
                execute(ast);
			}
			
			//Close file 
			fclose(f);
			
    		}		
	
    return 0;
}
