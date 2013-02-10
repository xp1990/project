#include <stdio.h>
#include <stdlib.h>

#define DIM 2048
#define LIFE 3
#define NAME "2048.dat"

int main()
{
    FILE *fp;
    int i,j;

    fp = fopen(NAME, "wr");
    
    if(fp == NULL)
    {
        printf("File open/creation failed.");
        exit(0);
    }

    for(i = 1; i <= DIM; i++)
    {
        for(j = 1; j <= DIM; j++)
        {
            if(rand() % LIFE == 1)
                fprintf(fp, "%d", 1);
            else
                fprintf(fp, "%d", 0);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}
