#include <stdio.h>

void putnbr(long n)
{

}

int main()
{
#if 1
    long arr[] = {11, 12, 13, 14};
    long arr1 = arr[1];
    int i = 2;
    putnbr(arr[i]);
#else
    int i = 5;
    printf("%d\n", i);
#endif
}
