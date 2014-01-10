#include <stdio.h>
#include <sys/time.h>

int main(int argc,char **argv)
{
    const unsigned long long msb64 = (1ULL << 63);

    double *in_val = malloc(20000000*sizeof(double));

    printf("Performing sign-copy 10,000,000 times: \n");

    for(double *i=,*o=; i<p+10000000; i+=2,++o)
        *o = (*i & msb64) | (*(i+1) & ~msb64);

    printf("");

    for(double *i=,*o=; i<p+10000000; i+=2,++o)
        *o = *i>0 ? *(i+1) : -*(i+1);

    printf("");
}
