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
	//Everything gets initialized and attributes are modified depending on the type of node.
	shell_ast* node = malloc(sizeof(shell_ast));
	node->op = 0;
	node->left = 0;
	node->right = 0;
	node->cmd = 0;
	node->args = 0;
	node->file =0;
	node->isOperator = 0;
	return node;

	//Aware this is space inefficient. Will look for efficiency after funtionality. 
}

char*
get_command(shell_ast* ast){
	//Helper funtion that allows us to get the command out of an ast node. 
	char* c = malloc(16);
	sprintf(c, "%s", ast->cmd);
	return c;
}

void
get_arguments(shell_ast* ast, char** arguments){
	//Helper funtion that allows us to get the arguments out of an ast node.
	
	//First element is the command.
	arguments[0] = ast->cmd;

	//Last argument is null.
	arguments[ast->args->size + 1] = 0;

	//Filling the rest of the arguments
	for (int ii = 1; ii < ast->args->size + 1; ++ii){
		arguments[ii] = svec_get(ast->args, ii - 1);
	}	
}

void 
redirect_input(char* newinput){
	
	//Changing the input fd. 
	close(0);

	//Opening the input file so we can read from it. 
	int fd = open(newinput, O_RDONLY);
	//Error check of open.
	if (fd < 0){
		printf("Error in open: %s\n", strerror(errno));
		exit(0);
	}

	//Creating a copy of the file descriptor for new input to stdin for execvp. 
	dup2(fd, 0);

	//Closing input file. 
	close(fd);

}

void 
redirect_output(char* newoutput){
	

	//Opening input file so we can write in it. If it does not exist we create it. 
	int fd1 = open(newoutput, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	//Error check of open. 
	if (fd1 < 0) {
		printf("Error in open: %s\n", strerror(errno));
		exit(0);
	}

	close(1);
	
	//Creating a copy of the file descriptor for new otput to stdout for execvp.
	dup(fd1);
	
	//Closing output file.
	close(fd1);
		
}

void
eval_simplecommand(shell_ast* ast){
	
	//Evaluating a simple command
	
	//Getting the command
	char* command = get_command(ast);

	//Getting the arguments
	int num_of_args = ast->args->size;
	char* arguments[num_of_args + 2]; //adding the command and the last null value.
	get_arguments(ast, arguments);

	//Freeing ast
	free_shell_ast(ast);

	//Executing command with arguments.
	execvp(command, arguments);
	
	//Error control on execvp
	printf("Error in execvp: %s\n", strerror(errno));
}


void 
eval_redir(shell_ast* ast){
	

	//Evaluating redirection operator.
	printf("%s ", ast->file);

	if ( strcmp(ast->op, "<") == 0 ){
		//Redirecting input for execvp.
		redirect_input(ast->file);

	} else {
		//Redirectin output for execvp.
		redirect_output(ast->file);	
	}
	
	//Getting command.
	char* command = get_command(ast);

	//Getting the arguments.
	int num_of_args = ast->args->size;
	char* arguments[num_of_args + 2];
	get_arguments(ast, arguments);

	//Freeing ast
	free_shell_ast(ast);

	execvp(command, arguments);

	//Error check on execvp
	printf("Error in execvp: %s\n", strerror(errno));



}




void 
ast_eval (shell_ast* ast){

	//Differentiate wether the node is a command or an operator. 
	
	if(ast->isOperator == 0) {
	//It is a simple command.
		eval_simplecommand(ast);		
		
	} else {
	//It is a command with an operator.
	//Commands are going to be treated differently depending in the operator they contain. 
	
		if (strcmp(ast->op, ";") == 0){
			//Evaluating semicolon operator.
			if (ast->left){
				ast_eval(ast->left);
			}

			if (ast->right){
				ast_eval(ast->right);
			}

		} else if (strcmp(ast->op, "<") == 0 || strcmp(ast->op, ">") == 0) {
			printf("%s ", ast->file);
			//Evaluating redirect input operator.
			eval_redir(ast);

		} else {
			fprintf(stderr, "Operator not implemented\n");
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
		//just getting commands for the moment
		//
		//BE CAREFUL THIS CAN BE A FILE 
		char* tmp = malloc(16);
		sprintf(tmp, "%s", ast->cmd);
		for(int ii=0; ii < ast->args->size; ++ii){
			printf("%s ", svec_get(ast->args, ii));
		}
		return tmp;
	} else {
		//it is an operand
		char* ll = 0;
		char* rr = 0;
		if (ast->left){
			ll = ast_string(ast->left);	
		}

		if (ast->right){
			rr = ast_string(ast->right);
		}

		char* full = malloc(128);
		sprintf(full, "(%s %s %s)", ll, ast->op, rr);
		free(ll);
		free(rr);
		return full;
	}
}



void
print_ast(shell_ast* ast){
	char* text = ast_string(ast);
	printf("%s\n", text);
	free(text);
}







