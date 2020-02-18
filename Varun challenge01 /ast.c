#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

calc_ast*
make_ast_value(int vv)
{
    //initialising the values, in a effective but space inefficientg way
    calc_ast* ast = malloc(sizeof(calc_ast));
    ast->op = '=';
    ast->arg0 = 0;
    ast->arg1 = 0;
    ast->value = vv;
    ast->cmd = 0;
    ast->args = 0;
    ast->file = 0;
    return ast;
}

char*
command_get(calc_ast* ast){
    char* m = malloc(16);
    sprintf(m, "%s", ast->cmd);
    return m;
}

void
argument_get(calc_ast* ast, char** arguments){
    // command will always be the first element and last element will be null
    argument[0] = ast->cmd;
    arguments[ast->args->size + 1] = 0;
    //while loop to fill all other arguments
    int j = 1;
    while (j < ast->args->size){
        arguments[j] = svec_get(ast->args, ii - 1);
        ++j;
    }
}

void
rinput(char* ninput){
    
    close(0);
    //open input file and check error
    int in = open(ninput, O_RDONLY);
    if (0>in){
        printf("Error: %s\n", strerror(errno));
        exit(0);
    }
    // duplicate and close file
    dup2(in, 0);
    close(in);
}
void
routput(char* noutput){
    //open input file with write access and check error
    int out = open(noutput, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (0>in){
        printf("Error: %s\n", strerror(errno));
        exit(0);
    }
    // duplicate and close file
    dup(out);
    close(out);
}

void
evaluate_command(calc_ast* ast){
    char* c = get_command(ast);

    int count = ast->args->size;
    char* arguments[count +2];
    argument_get(ast, arguments);
    free_calc_ast(ast);
    execvp(c, arguments);

}

void
evaluate_redir(calc_ast* ast){
    printf("%s", ast->file);

    if (strcmp(ast->op, "<") != 0){
        routput(ast->file);
    }
    else{
        rinput(ast->file);
    }
    char* c = get_command(ast);

    int count = ast->args->size;
    char* arguments[count +2];
    argument_get(ast, arguments);
    free_calc_ast(ast);
    execvp(c, arguments);
}

void
free_ast(calc_ast* ast)
{
    if (ast->arg0) {
        free_ast(ast->arg0);
    }
    if (ast->arg1) {
        free_ast(ast->arg1);
    }
    free(ast);
}

int
ast_eval(calc_ast* ast)
{
    return 5;
}

char*
ast_string(calc_ast* ast)
{
    if (ast->op == 0) {
        char* tmp = malloc(16); 
        sprintf(tmp, "%s", ast->value);
        int j = 0;
        while(j<ast->args->size){
            printf("%s ", svec_get(ast->args, j));
            ++i;
        }
        return tmp;
    }
    else {
        char* aa = ast_string(ast->arg0);
        char* bb = ast_string(ast->arg1);
        char* cc = malloc(128);
        sprintf(cc, "(%s %c %s)", aa, ast->op, bb);
        free(aa);
        free(bb);
        return cc;
    }
}

void
print_ast(calc_ast* ast)
{
    char* text = ast_string(ast);
    printf("%s\n", text);
    free(text);
}