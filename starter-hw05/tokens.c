#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "svec.h"
#include <assert.h>
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
make_word(const char *text, int ii)
{
   int nn = 0;
   while (!isblank(text[ii + nn]) && !(text[ii + nn] == '&' || text[ii + nn] == '|' || text[ii + nn] == '<' || text[ii + nn] == '>' || text[ii + nn] == ';'))
   {
      nn++;
   }

   char *word = malloc(nn + 1);
   memcpy(word, text + ii, nn);
   word[nn] = 0;
   return word;
}

svec *
tokenize(char *text)
{
   svec *sv = make_svec();
   //Length of the text
   long len = strlen(text);
   long ii = 0;

   while (ii < len)
   {
      if (isspace(text[ii]))
      {
         ++ii;
         continue;
      }

      if ((text[ii] == '|' && text[ii + 1] == '|') || (text[ii] == '&' && text[ii + 1] == '&'))
      {
         char str[3];
         str[0] = text[ii];
         str[1] = text[ii + 1];
         str[2] = 0;
         svec_push_back(sv, str);
         ii += 2;
         continue;
      }

      if (text[ii] == '<' || text[ii] == '>' || text[ii] == ';' || text[ii] == '|' || text[ii] == '&')
      {
         char str[2];
         str[0] = text[ii];
         str[1] = 0;
         svec_push_back(sv, str);
         ++ii;
         continue;
      }

         char *word = make_word(text, ii);
         chomp(word);
         svec_push_back(sv, word);
         ii += strlen(word);
         free(word);
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

//         //check for strial character
//         else if(ch == '&' || ch == '|' || ch == '<' || ch == '>' || ch == ';'){ 
            
//             trim(token);
//             //if not empty
//             if(strlen(token)!=0)
//                 svec_push_back(sv, token);
//             memset(token,0,strlen(token));

//             //check for repeated strial character
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
//             //if its not repeated strial character
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
//         //non strial non space character
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
main(int wordc, char* const wordv[]){

    while (1) {
        // svec* tokens = make_svec();
        printf("tokens$ ");
        fflush(stdout);
        char text[1024];
        char* line = read_line(text);
        if (!line) {
            exit(0);
        }
        //tokenize
        svec* tokens = tokenize(line);
        //print in reverse
        print_in_reverse(tokens);
        free_svec(tokens);
    }
    return 0;
}
   





