#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "svec.h"
#include "ast.h"
#include "parse.h"


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

void chomp(char* text)
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

void
execute(char* cmd)
{
    int cpid;

    if ((cpid = fork())) {
        // parent process
        // printf("Parent pid: %d\n", getpid());
        // printf("Parent knows child pid: %d\n", cpid);

        // Child may still be running until we wait.

        int status;
        waitpid(cpid, &status, 0);

        // printf("== executed program complete ==\n");
        //
        // printf("child returned with wait code %d\n", status);
        if (WIFEXITED(status)) {
            // printf("child exited with exit code (or main returned) %d\n", WEXITSTATUS(status));
        }
    }
    else {
        // child process
        // printf("Child pid: %d\n", getpid());
        // printf("Child knows parent pid: %d\n", getppid());
        //
        // for (int ii = 0; ii < strlen(cmd); ++ii) {
        //     if (cmd[ii] == ' ') {
        //         cmd[ii] = 0;
        //         break;
        //     }
        // }
        //
        // // The argv array for the child.
        // // Terminated by a null pointer.
        // char* args[] = {cmd, "one", 0};
        //
        // printf("== executed program's output: ==\n");
        //
        // execvp(cmd, args);
        // printf("Can't get here, exec only returns on error.");
        ast_eval(ast);
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
        printf("nush$ ");
        fflush(stdout);
        char* output = fgets(cmd, 256, stdin);
        if (!output){
          print("\n");
        }
    }
    else {
      memcpy(cmd, "echo", 5);
    }

    execute(cmd);

    return 0;
}