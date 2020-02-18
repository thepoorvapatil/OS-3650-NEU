#ifndef CALC_AST_H
#define CALC_AST_H

#include <stdlib.h>
#include "svec.h"


typedef struct shell_ast {

	//NODE TYPE OPERATOR
	char* op; //operator of the command
	struct shell_ast* left; //left side of operator
	struct shell_ast* right; //right side of operator

	//NODE TYPE COMMAND
	char* cmd;
	svec* args;

	//NODE TYPE FILE
	char* file;

	//DIFFERENTIATOR 
	int isOperator; 

} shell_ast;

shell_ast* make_shell_ast();
void free_shell_ast(shell_ast* ast);
void print_ast(shell_ast* ast);
void ast_eval(shell_ast* ast);

#endif
