#include "svec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

int
check_special(char string){
  int counter = 0;
  if (string == '<'){counter+=1;}
  if (string == '>'){counter+=1;}
  if (string == ';'){counter+=1;}
  if (string == '&'){counter+=1;}
  if (string == '|'){counter+=1;}
  if (counter > 0){
    return 1;
  }
  return 0;
}

void trim_spcaes(char *string){

  int counter = 0;
  while(string[counter] == ' ' || string[counter] == '\t' || string[counter] == '\n')
  {
      counter++;
  }

  int i = 0;
  while(string[i + counter] != '\0')
  {
      string[i] = string[i + counter];
      i++;
  }
  string[i] = '\0';

  i = 0;
  counter = -1;
  while(string[i] != '\0')
  {
      if(string[i] != ' ' && string[i] != '\t' && string[i] != '\n')
      {
          counter = i;
      }

      i++;
  }
  string[counter + 1] = '\0';
}

void
reverse(svec* aa){
  for(int i = aa->size-1; i>=0; i--){
    char *result = svec_get(aa, i);
    printf("%s\n", result);
  }
}

void
print_list(svec* aa){
  printf("Result: ");
  for(int i = 0; i<aa->size; i++){
    char *result = svec_get(aa, i);
    printf("%s, ", result);
  }
  printf("\n");
}

void
chomp(char* text)
{
  // remove trailing newline from string
  strtok(text, "\n");
}

void append(char* s, char c) {
        // // printf("Before append %s\n", s);
        // int len = strlen(s);
        // s[len] = c;
        // s[len+1] = '\0';
        // // printf("After append %s\n", s);
        strncat(s, &c, 1); 
}

void push_tokenise(svec* aa , char* bb){
  // printf("Getting this: %s\n", bb);
 char push_token[128] = "";
 chomp(bb);
  // printf("After chomping: %s\n", bb);
 for(int ii = 0; ii<strlen(bb); ii++){
    char character = bb[ii];
    // print_list(aa);
    // printf("Character: %c\n", character);
    if(character ==' '){
       trim_spcaes(push_token);
       int length = strlen(push_token);
       if(length !=0){
         svec_push_back(aa , push_token);
       }
       memset(push_token,0,strlen(push_token));
    }

    else if(check_special(character)){
       trim_spcaes(push_token);
       int length = strlen(push_token);
       if(length !=0){
         svec_push_back(aa , push_token);
       }
       memset(push_token,0,strlen(push_token));

       if(bb[ii+1] == character && ii<strlen(bb) - 1 ){  //
          append( push_token, character);
          append( push_token, character);
          ii++; //Incremenet by 1 because we have found that it is a special character afer hthis special character

          trim_spcaes(push_token);
          int length = strlen(push_token);
          if(length !=0){
            svec_push_back(aa , push_token);
          }
          memset(push_token,0,strlen(push_token));
       }
       else{   //Foud that the next oene is not the same characeter
          append( push_token, character);
          trim_spcaes(push_token);
          int length = strlen(push_token);
          if(length !=0){
            svec_push_back(aa , push_token);
          }
          memset(push_token,0,strlen(push_token));
       }
    }
    else{
       append(push_token, character);
    }
 }
//  printf("Pushable %s\n", push_token);
 trim_spcaes(push_token);
 int length = strlen(push_token);
 if(length !=0){
   svec_push_back(aa , push_token);
 }
 memset(push_token,0,strlen(push_token));
}

int main(int argc, char* argv[]){
   while(1){
      svec* aa  = make_svec();
      printf("tokens$ ");
      char str[256];
      char* bb = fgets(str, 256, stdin);
      if (!bb){
         printf("\n");
         exit(0);
      }
      push_tokenise(aa , bb);
      reverse(aa );
      free_svec(aa );
   }
   return 0;
}
