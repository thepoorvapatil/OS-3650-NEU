#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "svec.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

char*
read_line(char* text)
{
    
    char* line = fgets(text, 1024, stdin);
    return line;
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

void print_in_reverse(svec* sv){
   for (int ii = sv->size - 1; ii >= 0; --ii) {
      char* line = svec_get(sv, ii);
      printf("%s\n", line);
   }
}

void tokenize(svec* sv, char* line){
    char token[256] = "";
    chomp(line);
    //go character by character throughout line
    for(int i = 0; i<strlen(line); i++){
        char ch = line[i];
        //space is delimiter 
        //If we reach a space, it's a delimiter. Push and continue. 
        if(ch == " "){
            trim(token);
            if(strlen(token)!=0){
                svec_push_back(sv, token);
            }
            memset(token,0,strlen(token));
        }
        else if(ch == '&' || ch == '|' || ch == '<' || ch == '>' || ch == ';'){ //If we reach a special character, push the current string to the list and empty it. 
            trim(token);
            if(strlen(token)!=0){
                svec_push_back(sv, token);
            }
            
            memset(token,0,strlen(token));
            // check for double special characters
            if(i!=strlen(line) - 1 && line[i+1] == ch){ 
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
    if(strlen(token)!=0){
        svec_push_back(sv, token);
    }
    memset(token,0,strlen(token));
}


int 
main(int argc, char* const argv[]){

    while (1) {
        svec* tokens = make_svec();
        printf("tokens$ ");
        fflush(stdout);
        char text[1024];
        char* line = read_line(text);
        if (!line) {
            exit(0);
        }
        //tokenize line and store in tokens
        tokenize(tokens, line);
        //print in reverse
        print_in_reverse(tokens);
        free_svec(tokens);
    }
    return 0;
}
   





