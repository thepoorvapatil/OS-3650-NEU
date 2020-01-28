
#include <stdio.h>
#include <stdlib.h>

long fib(long x){
    if(x == 0) 
        return 0;
    if(x == 1)  
        return 1;
    return fib(x - 1) + fib(x-2);
}

int
main(int argc, char* argv[])
{
    if (argc != 2 || atol(argv[1]) < 0) {
        printf("Usage:\n  %s N, where N > 0\n", argv[0]);
        return 1;
    }

    printf("fib(%ld) = %ld", atol(argv[1]), fib(atol(argv[1])));
    return 0;
}