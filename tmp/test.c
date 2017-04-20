#include <stdio.h>

int main(void){

	int a[]={1,2,3,4,5,6,7}, *p=a, *q=&a[5];
	
	printf("%d %d %d %d\n", *q--, *--q, --*q, q-p);

	int c1 = 'b', c2= 'e';
	printf("%c\n", c2-'a'+"A");
}
