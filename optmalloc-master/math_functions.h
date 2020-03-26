int
ilog2(long x)
{
    int counter = 0;
    long num = 1;
    while (num < x)
    {
        num <<= 1;
        counter++;
    }
    return counter;
}

int
ilog2floor(long x)
{
    if (x == 0) {
        return 0;
    }
    
    int counter = -1;
    long num = 1;
    while (num <= x)
    {
        num <<= 1;
        counter++;
    }
    return counter;
}


int ipow2 (long x)
{
    long num = 1;
    while (x > 0) {
    num <<= 1;
    x--;
    }
    return num;
}
