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
        if(ch == ' '){
            trim(token);
            if(strlen(token)!=0){
                svec_push_back(sv, token);
            }
            memset(token,0,strlen(token));
        }
        else if(ch == '&' || ch == '|' || ch == '<' || ch == '>' || ch == ';'){  
            
            trim(token);
            if(strlen(token)!=0)
                svec_push_back(sv, token);
            memset(token,0,strlen(token));
            if(i!=strlen(line) - 1 && line[i+1] == ch){ 
                // printf("Repeated!\n");
                strncat(token, &ch, 1);
                strncat(token, &ch, 1);
                i++;
                trim(token);
                if(strlen(token)!=0)
                svec_push_back(sv, token);
                memset(token,0,strlen(token));
            }
            else{ 
                // printf("Non repeated!\n");
                strncat(token, &ch, 1);
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
   trim(token);
   if(strlen(token)!=0)
      svec_push_back(sv, token);
   memset(token,0,strlen(token));
}

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
        } 
        else {
            //Child process.
            ast_eval(ast);
        }
	}
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

            if(strlen(cmd) == 1){
                continue;
            }

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
        //file open error check
        if (f == NULL){
            printf("Error in fopen %s\n", strerror(errno));
            exit(0);
        }

        while (fgets(buffer, 100, f) != NULL){
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
