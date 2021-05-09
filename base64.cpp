#include <stdio.h>

void printBin(char c, int n)
{
	for(int i=0;i<n;i++)
		printf("%d", (c>>i)&1);
//	printf(" ");
}

void encode(char const *s, long sn, char *d, long *dn)
{
	long nBitsPos = 0;
	
	for(; nBitsPos<(sn<<3); nBitsPos+=6)
	{
		long n2 = nBitsPos+6;
		long c1 = (nBitsPos>>3);
		long c2 = (n2>>3);
		int p1 = nBitsPos&0x7;
		int p2 = n2&0x7;
		char c;
		if(c1==c2)
			c = (s[c1] & ((1<<p2)-1)) >> p1;
		else
			c = ( s[c1] >> p1 ) | (s[c2] & ((1<<p2)-1))<<(8-p1);
		printBin(c, 6);
	}
}

void main()
{
	char s[5] = {1,2,3,4,5};
	printf("s:\n");
	for(int i=0; i<5; i++)
		printBin(s[i], 8);
	printf("d:\n");
	encode(s,5,0,0);
}