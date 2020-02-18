#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "ast.h"
#include "svec.h"

shell_ast*
make_shell_ast(){
	//initialise
	shell_ast* node = malloc(sizeof(shell_ast));
	node->op = 0;
	node->left = 0;
	node->right = 0;
	node->cmd = 0;
	node->args = 0;
	node->file =0;
	node->isOperator = 0;
	return node;

}

char*
get_command(shell_ast* ast){
	//Helper funtion to get the command of ast node 
	char* cm = malloc(16);
	sprintf(cm, "%s", ast->cmd);
	return cm;
}

void
get_arguments(shell_ast* ast, char** Args){
	//Helper funtion to get arguments of ast node.
	
	//command
	Args[0] = ast->cmd;

	//null
	Args[ast->args->size + 1] = 0;

	//other args
	for (int ii = 1; ii < ast->args->size + 1; ii++){
		Args[ii] = svec_get(ast->args, ii - 1);
	}	
}

void 
redirect_input(char* newinput){
	
	//Changing the input fd.
	close(0);

	//open file 
	int fd = open(newinput, O_RDONLY);

	//file open error check
	if (fd < 0){
		printf("Error in open: %s\n", strerror(errno));
		exit(0);
	}

	//copy of the fd for input to stdin for execvp 
	dup2(fd, 0);

	//Close file 
	close(fd);

}

void 
redirect_output(char* newoutput){
	

	//Opening input file so we can write in it. If it does not exist we create it. 
	int fd = open(newoutput, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	//Error check of open. 
	if (fd < 0) {
		printf("Error in open: %s\n", strerror(errno));
		exit(0);
	}

	close(1);
	
	//Creating a copy of the file descriptor for new otput to stdout for execvp.
	dup(fd);
	
	//Closing output file.
	close(fd);
		
}

//eval_simplecommand
void
eval_commandsimple(shell_ast* ast){
		
	char* command = get_command(ast);

	int num_of_args = ast->args->size;
	char* arguments[num_of_args + 2]; 
	get_arguments(ast, arguments);

	free_shell_ast(ast);
	execvp(command, arguments);
	
	//error check
	printf("Error in execvp: %s\n", strerror(errno));
}


void 
eval_redirect(shell_ast* ast){
	
	printf("%s ", ast->file);

	if ( strcmp(ast->op, "<") == 0 ){
		redirect_input(ast->file);

	} else {
		redirect_output(ast->file);	
	}
	
	char* command = get_command(ast);

	int num_of_args = ast->args->size;
	char* arguments[num_of_args + 2];
	get_arguments(ast, arguments);

	free_shell_ast(ast);

	execvp(command, arguments);

	//error check
	printf("Error in execvp: %s\n", strerror(errno));

}


void 
ast_eval (shell_ast* ast){

	//check if node is a command or operator
	
	if(ast->isOperator == 0) {
		//simple command execution
		eval_commandsimple(ast);		
		
	} else {
		//command with an operator.
		if (strcmp(ast->op, ";") == 0){
			if (ast->left){
				ast_eval(ast->left);
			}

			if (ast->right){
				ast_eval(ast->right);
			}

		} else if (strcmp(ast->op, "<") == 0 || strcmp(ast->op, ">") == 0) {
			printf("%s ", ast->file);
			//redirect input
			eval_redirect(ast);

		} else {
			fprintf(stderr, "Operator not recognized\n");
			exit(0);
		}	
			
	}	
}


void 
free_shell_ast(shell_ast* ast){
	
	if(ast) {
		if (ast->left){
			free(ast->left);
		}

		if (ast->right) {
			free(ast->right);
		}

		free(ast);
	}
}

char*
ast_string(shell_ast* ast){

	if (ast->op == 0){

		char* tmp = malloc(16);
		sprintf(tmp, "%s", ast->cmd);
		for(int ii=0; ii < ast->args->size; ii++){
			printf("%s ", svec_get(ast->args, ii));
		}
		return tmp;

	} 
	//operand
	else {
		char* aa = 0;
		char* bb = 0;
		if (ast->left){
			aa = ast_string(ast->left);	
		}

		if (ast->right){
			bb = ast_string(ast->right);
		}

		char* str = malloc(128);
		sprintf(str, "(%s %s %s)", aa, ast->op, bb);
		free(aa);
		free(bb);
		return str;
	}
}



void
print_ast(shell_ast* ast){
	char* text = ast_string(ast);
	printf("%s\n", text);
	free(text);
}







