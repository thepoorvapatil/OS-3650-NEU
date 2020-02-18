#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "svec.h"
#include "ast.h"
#include "parse.h"

int 
streq(const char* aa, const char* bb){
	
	return strcmp(aa,bb) == 0;
}

int 
find_first_index(svec* toks, const char* tt){

		for (int ii=0; ii < toks->size; ii++){
			char* token = svec_get(toks, ii);
			if( streq(token, tt) ){
				return ii;
			}		
		}
		return -1;
}

int 
contains(svec* toks, const char* tt){
	return find_first_index(toks, tt) >= 0;
}


svec* 
slice(svec* toks, int start, int end){
		
	svec* sv = make_svec();

	for(int ii = start; ii < end; ++ii){
		char* token = svec_get(toks, ii);
		svec_push_back(sv, token);
	}
	return sv;	

}


shell_ast* 
parse(svec* toks){
	
	const char* operators[] = {";", "|", "||", "&", "&&", "<", ">"};

	int opp = 0;
	for (int ii= 0; ii < sizeof(operators); ++ii){
		const char* operator = operators[ii];
		
		if(contains(toks, operator)){

			//Get index of first operator 
			int operatorIndex = find_first_index(toks, operator);

			//Create operator node

			//node
			shell_ast* ast = make_shell_ast(); 
			//add operator
			ast->op = svec_get(toks, operatorIndex); 
			ast->isOperator = 1; 

			//separating commands
			if(strcmp(operator, ";") == 0 || strcmp(operator, "||") == 0 || strcmp(operator, "&&") == 0){
				
				//Divide toks into the right and left branch
				svec* left = slice(toks, 0, operatorIndex);
				svec* right = slice(toks, operatorIndex + 1, toks->size);
				//left child 
				ast->left = parse(left); 
				//right child
				ast->right = parse(right); 

				free(left); 
				free(right);
			
			} 

			//redirect input
			else if ( strcmp(operator, "<") == 0 ){
				ast->cmd = svec_get(toks, 0); 
				ast->file = svec_get(toks, operatorIndex + 1); 
				//create arguments
				ast->args = make_svec(); 
				//push back with input file and args 
				svec_push_back(ast->args, ast->file);
			} 

			//redirect output
			else if ( strcmp(operator, ">") == 0) {
				ast->cmd = svec_get(toks, 0); 
				ast->file = svec_get(toks, operatorIndex + 1); 
				//create arguments
				ast->args = make_svec();
				//add arguments
				for(int ii = 1; ii < toks->size; ii++){  
					char* token = svec_get(toks, ii);
					if ( strcmp(token, ">") == 0)
						break;
					svec_push_back(ast->args, token);
				}

			} 

			return ast;
		}
	}

	if(opp == 0){
		
		//Create node
		shell_ast* ast = make_shell_ast(); 
		ast->cmd = svec_get(toks, 0); 
		ast->args = make_svec(); 
		for(int ii = 1; ii < toks->size; ii++){
			char* token = svec_get(toks, ii);
			svec_push_back(ast->args, token);
		}

		return ast;
	}

	fprintf(stderr, "Invalid Token Stream");
	exit(1); 
		

}


