
#include <stdio.h>
#include <ctype.h>
#include <string.h>
// #include <svec.h>


char*
read_line(const char* text, int ii)
{
    int nn = 0;
    while (isalpha(text[ii + nn])) {
        ++nn;
    }

    char* num = malloc(nn + 1);
    memcpy(num, text + ii, nn);
    num[nn] = 0;
    return num;
}


char*
tokenize(const char* text)
{
    list* xs = 0;

    int nn = strlen(text);
    int ii = 0;
    while (ii < nn) {
        if (isspace(text[ii])) {
            ++ii;
            continue;
        }

        if (isdigit(text[ii])) {
            char* num = read_number(text, ii);
            xs = cons(num, xs);
            ii += strlen(num);
            free(num);
            continue;
        }

        // Else, operator.
        char op[4] = "x";
        op[0] = text[ii];
        xs = cons(op, xs);
        ++ii;
    }

    return rev_free(xs);
}



int main(){
   char line;

   while (1) {
      printf("tokens$ ");
      fflush(stdout);
      line = read_line();
      if (line != EOF) {
         exit(0);
      }
      tokens = tokenize(line);
      for token in reverse(tokens):
         puts(token)
      }
}
   





