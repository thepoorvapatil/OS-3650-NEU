#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "ast.h"
#include "parse.h"
#include "svec.h"

int 
streq(const char* aa, const char* bb){
	
	return strcmp(aa,bb) == 0;
}

int find_first_index(svec* toks, const char* tt){

		for (int ii=0; ii < toks->size; ++ii){
			char* token = svec_get(toks, ii);
			if( streq(token, tt) ){
				return ii;
			}		
		}

		return -1;
}


int contains(svec* toks, const char* tt){
	return find_first_index(toks, tt) >= 0;
}


svec* 
slice(svec* toks, int first, int last){
		
	svec* vector = make_svec();

	for(int xx = first; xx < last; ++xx){
		char* token = svec_get(toks, xx);
		svec_push_back(vector, token);
	}
	return vector;	

}


shell_ast* 
parse(svec* toks){
	
	const char* ops[] = {";", "|", "||", "&", "&&", "<", ">"};

	int containedOperator = 0;
	for (int ii= 0; ii < 7; ++ii){
		const char* operator = ops[ii];
		
		if(contains(toks, operator)){

			//Gettint the index of the first operator 
			int opIndex = find_first_index(toks, operator);

			//Creating operator type node
			shell_ast* ast = make_shell_ast(); //making node
			ast->op = svec_get(toks, opIndex); //adding opeator
			ast->isOperator = 1; //classifying node 

			if(strcmp(operator, ";") == 0 || strcmp(operator, "&&") == 0 || strcmp(operator, "||") == 0){
				//If the operator is a ; or and AND or and OR we create a left child for the left command and a right child for the right command.
				//Dividing toks into the right and left part of the operator.
				svec* left = slice(toks, 0, opIndex);
				svec* right = slice(toks, opIndex + 1, toks->size);
				ast->left = parse(left); //adding leftchild 
				ast->right = parse(right); //adding rightchild
				free(left); //freeing left child
				free(right); //freeing right child 
			
			} else if ( strcmp(operator, "<") == 0 ){
				//If the operator is redirect input. 
				ast->cmd = svec_get(toks, 0); //get the command
				ast->file = svec_get(toks, opIndex + 1); //add the input file
				ast->args = make_svec(); //creating the argumemtns
				svec_push_back(ast->args, ast->file); //adding the input file to the arguments 
				//Know this is not space efficient but will focus on functionality first. 

			} else if ( strcmp(operator, ">") == 0) {
				//If operator is redirect output. 
				ast->cmd = svec_get(toks, 0); //get the command
				ast->file = svec_get(toks, opIndex + 1); //add the input file
				ast->args = make_svec(); //creating arguments
				for(int ii = 1; ii < toks->size; ++ii){ //adding arguments 
					char* token = svec_get(toks, ii);
					if ( strcmp(token, ">") == 0) break;
					svec_push_back(ast->args, token);
				}
				//Know this is not space efficient but will focus on functionality first. 

			} else {

			}
			return ast;
		}
	}

	if(containedOperator == 0){
		
		//Creatting command type node
		shell_ast* ast = make_shell_ast(); //making node
		ast->cmd = svec_get(toks, 0); //adding command
		ast->args = make_svec(); //adding command arguments
		for(int ii = 1; ii < toks->size; ++ii){
			char* token = svec_get(toks, ii);
			svec_push_back(ast->args, token);
		}

		return ast;
	}

	fprintf(stderr, "Invalid token stream");
	exit(1); 
		

}


