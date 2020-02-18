#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "parse.h"

int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

int
find_first_index(svec* toks, const char* tt)
{
    int ii = 0;
    for (svec* it = toks; it; it = it->tail) {
        if (streq(it->head, tt)) {
            return ii;
        }
        ii++;
    }

    return -1;
}

int
contains(svec* toks, const char* tt)
{
    return find_first_index(toks, tt) != -1;
}

svec*
slice(svec* toks, int first, int last)
{
    svec* vector = make_svec();

    for (int a = first; a < last; ++a) {
        char* token = svec_get(toks, a);
        svec_push_back(vector, token);
    }
    return vector;
}

calc_ast*
parse(svec* toks)
{
    if (length(toks) == 1) {
        int vv = atoi(toks->head);
        return make_ast_value(vv);
    }

    const char* ops[] = {"+", "-", "*", "/"};

    for (int ii = 0; ii < 4; ++ii) {
        const char* op = ops[ii];

        if (contains(toks, op)) {
            int jj = find_first_index(toks, op);
            svec* xs = slice(toks, 0, jj);
            svec* ys = slice(toks, jj + 1, length(toks));
            calc_ast* ast = make_ast_op(op[0], parse(xs), parse(ys));
            free(xs);
            free(ys);
            return ast;
        }
    }
}
