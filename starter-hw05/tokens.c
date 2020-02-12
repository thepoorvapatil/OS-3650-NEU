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

void 
trim(char *str) 
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
print_in_reverse(svec* sv){
   for (int ii = sv->size - 1; ii >= 0; --ii) {
      char* line = svec_get(sv, ii);
      printf("%s\n", line);
   }
}

char *
read_argument(const char *text, int ii)
{
   int nn = 0;
   while (!isblank(text[ii + nn]) && !(text[ii + nn] == '&' || text[ii + nn] == '|' || text[ii + nn] == '<' || text[ii + nn] == '>' || text[ii + nn] == ';'))
   {
      nn++;
   }

   char *arg = malloc(nn + 1);
   memcpy(arg, text + ii, nn);
   arg[nn] = 0;
   return arg;
}

svec*
tokenize(char *line)
{
   svec *sv = make_svec();
   //Length of the line
   int index = 0;
   int len = strlen(line);

   while (index < len)
   {
      if (isspace(line[index]) || line[index] == 0 || line[index] == '\n')
      {
         index++;
         continue;
      }

      if ((line[index] == '|' && line[index + 1] == '|') || (line[index] == '&' && line[index + 1] == '&'))
      {
         char spec[] = "xx";
         spec[0] = line[index];
         spec[1] = line[index + 1];
         svec_push_back(sv, spec);
         index += 2;
         continue;
      }

      if (line[index] == '<' || line[index] == '>' || line[index] == ';' || line[index] == '|' || line[index] == '&')
      {
         char spec[] = "x";
         spec[0] = line[index];
         svec_push_back(sv, spec);
         index++;
         continue;
      }

      if (line[index] != 0)
      {
         char *arg = read_argument(line, index);
         chomp(arg);
         svec_push_back(sv, arg);
         index += strlen(arg);
         free(arg);

         continue;
      }
   }
   return sv;
}

// void tokenize(svec* sv, char*)
//     char token[256] = "";
//     chomp(line);
//     for(int i = 0; i<strlen(line); i++){
//         char ch = line[i];

//         //space. pushback into sv
//         if(ch == " "){ 
//             trim(token);
//             //if not empty
//             if(strlen(token)!=0){
//                 svec_push_back(sv, token);
//             }
//             memset(token,0,strlen(token));
//         }

//         //check for special character
//         else if(ch == '&' || ch == '|' || ch == '<' || ch == '>' || ch == ';'){ 
            
//             trim(token);
//             //if not empty
//             if(strlen(token)!=0)
//                 svec_push_back(sv, token);
//             memset(token,0,strlen(token));

//             //check for repeated special character
//             if(i<strlen(line)-1 && ch == line[i+1]){ 

//                 strncat(token, &ch, 1);
//                 strncat(token, &ch, 1);
//                 i++;
//                 //trim
//                 trim(token);
//                 //if not empty
//                 if(strlen(token)!=0){
//                     svec_push_back(sv, token);
//                 }
//                 memset(token,0,strlen(token));
//             }
//             //if its not repeated special character
//             else{ 
//                 strncat(token, &ch, 1);
//                 trim(token);
//                 //if not empty
//                 if(strlen(token)!=0){
//                     svec_push_back(sv, token);
//                 }
//                 memset(token,0,strlen(token));
//             }
//         }
//         //non special non space character
//         else{ 
//             strncat(token, &ch, 1);
//         }
//     }
//     //trim
//     trim(token);

//     //if not empty
//     if(strlen(token)!=0){
//         svec_push_back(sv, token);
//     }
//     memset(token,0,strlen(token));
// }


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
        //tokenize
        tokens = tokenize(line);
        //print in reverse
        print_in_reverse(tokens);
        free_svec(tokens);
    }
    return 0;
}
   





