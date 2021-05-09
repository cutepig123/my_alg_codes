#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <math.h>

using namespace std;

//#define PI 3.1415926
//#define DBG 

namespace Baidu09_1
{

	struct Phone{
		char d[10];
	};

	void DelN(char *s)
	{
		int nPos=strlen(s)-1;
		for(; (s[nPos]=='\n' || s[nPos]==' ' || s[nPos]=='\t') && nPos>=0; nPos--);
		s[nPos+1]=0;
	}

	vector <Phone> vPhone, vFriend;
	unsigned int n,m;
	int gailv[10][10];

	void Test(unsigned int i,vector <Phone> &vPhone, vector <Phone> &vFriend)
	{
		unsigned int sum=0;
		char *from = vFriend[i].d;
		for (unsigned int j=0; j<n;j++)
		{
			if(j == i)continue;

			unsigned int glv=1;
			char *to = vPhone[j].d;
			for (int k=0; k<6;k++)
			{
				glv *= gailv[from[k] - '0'][to[k] - '0'];
				if(0==glv)
					break;
			}

			if(glv)
				sum += glv;
		}
		printf("%d\n", sum);
	}

	int main()
	{
		vector <Phone> vPhone;
		unsigned int i,j;
		scanf("%d\n", &n);
		//printf("n=%d\n",n);
		vPhone.reserve(n);

		for (i=0; i<n; i++)
		{
			Phone ph;
			fgets(ph.d, 9, stdin);
			DelN(ph.d);

			vPhone.push_back(ph);
		}
		//printf("vPhone size %d\n", vPhone.size());

		for (i=0; i<10; i++)
		{
			for (j=0; j<10;j++)
				scanf("%d", &gailv[i][j]);
		}

		scanf("%d\n", &m);
		//printf("m=%d\n",m);
		vFriend.reserve(m);
		for (i=0; i<m; i++)
		{
			Phone ph;
			fgets(ph.d, 9, stdin);
			DelN(ph.d);

			vFriend.push_back(ph);
		}
		//printf("vFriend size %d\n", vFriend.size());

		for (i=0; i<m; i++)
			Test(i,vPhone,vFriend);

		return 0;
	}
};
