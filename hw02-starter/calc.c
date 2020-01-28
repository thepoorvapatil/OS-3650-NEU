#include<stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    if(argc != 4 || (strncmp(argv[2], "+", 8) && strncmp(argv[2], "-", 8) && strncmp(argv[2], "*", 8) && strncmp(argv[2], "/", 8)) ){
        puts("Usage:\n./ccalc N op N");
        return 1;
    }
    //argv[1], argv[2], argv[3]
    int xx = atoi(argv[1]);
    int yy = atoi(argv[3]);
    char * caclform = "%d %s %d = %d\n";

    if(!strncmp(argv[2], "+", 8))
        printf(caclform, xx, argv[2], yy, (xx + yy));

    if(!strncmp(argv[2], "-", 8))
        printf(caclform, xx, argv[2], yy, (xx - yy));
    
    if(!strncmp(argv[2], "*", 8))
        printf(caclform, xx, argv[2], yy, (xx * yy));
    
    if(!strncmp(argv[2], "/", 8))
        printf(caclform, xx, argv[2], yy, (xx / yy));

    return 0;
}