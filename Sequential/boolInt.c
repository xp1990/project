#include <stdbool.h>

int f(int i) 
{
    if ( i ) return 99;   //if(int)
    else  return -99;
}
int g(bool b)
{
    if ( b ) return 99;   //if(bool)
    else  return -99;
}
int main(){}
