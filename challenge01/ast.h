#ifndef CALC_AST_H
#define CALC_AST_H

#include <stdlib.h>
#include "svec.h"


typedef struct shell_ast {

	//operator node
	char* op; 
	struct shell_ast* left; 
	struct shell_ast* right; 

	//command node
	char* cmd;
	svec* args;

	//file
	char* file;
	
	//separtor
	int isOperator; 

} shell_ast;

shell_ast* make_shell_ast();
void free_shell_ast(shell_ast* ast);
void print_ast(shell_ast* ast);
void ast_eval(shell_ast* ast);

#endif
