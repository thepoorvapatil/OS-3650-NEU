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

// svec* 
// tokenize (const char* text) {
	
// 	svec* vector = make_svec();
// 	int nn = strlen(text);
// 	int ii = 0;
// 	char *token;

// 	while (ii < nn) {
	
// 		if(isspace(text[ii])){
// 			ii++; //if it is an space we skip it. 

// 		} else {
// 			int opLength = isOperator(text, ii);

// 			if (opLength == 0) {
// 				//it is not an operator
// 				token = read_token(text, ii);
// 				svec_push_back(vector, token);
// 				ii += strlen(token);	
// 			} else {
// 				//it is an operator
// 				token = malloc(opLength + 1);
// 				memcpy(token, text + ii, opLength);
// 				token[opLength] = 0;
// 				svec_push_back(vector, token);
// 				ii += opLength;		
// 			}
// 			free(token);

// 		}

// 	}

// 	return vector;
// }

void
chomp(char* text) // reused from length-sort.c from HW04
{
    // TODO: Modify input string to remove trailing newline ('\n')
    int l=strlen(text)-1;
    if( text[l] == '\n'){
        text[l] = '\0';
    }
}

void trim(char *str) 
{
    // check for the leading white spaces and trim
    int index = 0;
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n')
    {
        index++;
    }

    // move all characters to its left replacing the leading white spaces
    int i = 0;
    while(str[i + index] != '\0')
    {
        str[i] = str[i + index];
        i++;
    }
    // string ends with NULL
    str[i] = '\0'; 

    //check till end of string for trailing white spaces and trim
    index = -1;
    i = 0;
    while(str[i] != '\0')
    {
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
        {
            index = i;
        }
        i++;
    }
    // reset the character after last character as NULL */
    str[index + 1] = '\0';
}

void 
tokenize(svec* sv, char* line){
    char token[256] = "";
    chomp(line);
    //go character by character throughout line
    for(int i = 0; i<strlen(line); i++){
        char ch = line[i];
        //space is delimiter 
        //If we reach a space, it's a delimiter. Push and continue. 
        if(ch == ' '){
            trim(token);
            if(strlen(token)!=0){
                svec_push_back(sv, token);
            }
            memset(token,0,strlen(token));
        }
        else if(ch == '&' || ch == '|' || ch == '<' || ch == '>' || ch == ';'){ //If we reach a special character, push the current string to the list and empty it. 
            // printf("Putting 2: %s\n", token);
            trim(token);
            if(strlen(token)!=0)
                svec_push_back(sv, token);
            memset(token,0,strlen(token));
            if(i!=strlen(line) - 1 && line[i+1] == ch){ //Encountered repeated special
                // printf("Repeated!\n");
                strncat(token, &ch, 1);
                strncat(token, &ch, 1);
                i++;
                // printf("Putting 3: %s\n", token);
                trim(token);
                if(strlen(token)!=0)
                svec_push_back(sv, token);
                memset(token,0,strlen(token));
            }
            else{ //Encountered non-repeat special
                // printf("Non repeated!\n");
                strncat(token, &ch, 1);
                // printf("Putting 4: %s\n", token);
                trim(token);
                if(strlen(token)!=0)
                svec_push_back(sv, token);
                memset(token,0,strlen(token));
            }
        }
      else{ //It's not a special character, or a space.
         strncat(token, &ch, 1);
      }
      // printf("Character is: %c and is special character? %d\n", br[i], is_special((cpy)));
   }
   // printf("Putting 5: %s\n", token);
   trim(token);
   if(strlen(token)!=0)
      svec_push_back(sv, token);
   memset(token,0,strlen(token));
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

// void processInput(char* cmd){
	
// 	svec *vector = tokenize(cmd);
// 	shell_ast* ast = parse(vector);
// 	free(vector);
// 	execute(ast);
// }


int
main(int argc, char* argv[])
{

            // svec* tokens = make_svec();

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

                    // svec *vector = tokenize(cmd);
                    svec* tokens = make_svec();
                    tokenize(tokens, cmd);
                    shell_ast* ast = parse(tokens);
                    free(tokens);
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
				// svec *vector = tokenize(buffer);
                svec* tokens = make_svec();
                tokenize(tokens, buffer);
                shell_ast* ast = parse(tokens);
                free(tokens);
                execute(ast);
			}
			
			//Close file 
			fclose(f);
			
    		}		
	
    return 0;
}
