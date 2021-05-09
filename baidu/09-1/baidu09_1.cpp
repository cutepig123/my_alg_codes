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
	inline int mod(int a,int b,int *c)
	{
		int t=a%b;
		*c=a/b;
		if(t<0)
		{
			t+=b;
			(*c)--;
		}
		return t;
	}

	class CLargeUInt
	{
	public:
		vector <int> m_data;//[0~n]从低位到高位

		CLargeUInt(const char*s=0)
		{
			m_data.clear();

			if(s)
			{
				int n=strlen(s);
				m_data.reserve(n);
				for (int i=n-1;i>=0;i--)
				{
					int t=s[i]-'0';
					assert(t>=0 && t<=9);
					m_data.push_back(t);
				}
			}
		}

		CLargeUInt(unsigned int t)
		{
			m_data.clear();
			while (t>0)
			{
				m_data.push_back(t%10);
				t/=10;
			}
		}

		CLargeUInt add(CLargeUInt & r)
		{
			CLargeUInt lt;
			int n1=m_data.size();
			int n2=r.m_data.size();
			int n_min=min(n1,n2);
			int n_max=max(n1,n2);
			lt.m_data.reserve(n_max);
			for (int i=0;i<n_min;i++)
			{
				lt.m_data.push_back(m_data[i]+r.m_data[i]);
			}

			if(n1>n2)
				for (int i=n_min;i<n_max;i++)
				{
					lt.m_data.push_back(m_data[i]);
				}
			else if(n1<n2)
				for (int i=n_min;i<n_max;i++)
				{
					lt.m_data.push_back(r.m_data[i]);
				}

				//lt.Print();
				lt.simplify();
				return lt;
		}

		//必须是大数减去小数
		CLargeUInt sub(CLargeUInt & r)
		{
			assert(compare(r)>=0);
			CLargeUInt lt;
			int n1=m_data.size();
			int n2=r.m_data.size();
			assert(n1>=n2);

			lt.m_data.reserve(n1);

			for (int i=0;i<n2;i++)
			{
				lt.m_data.push_back(m_data[i]-r.m_data[i]);
			}

			for (int i=n2;i<n1;i++)
			{
				lt.m_data.push_back(m_data[i]);
			}
			lt.Print();
			lt.simplify();
			return lt;
		}
		//比较大小
		int compare( CLargeUInt & r) 
		{
			simplify();
			r.simplify();
			int n1=m_data.size();
			int n2=r.m_data.size();
			if (n1>n2)
			{
				return 1;
			}
			else if (n1<n2)
			{
				return -1;
			}
			else
			{
				for (int i=n1-1;i>=0;i--)
				{
					int t1=m_data[i];
					int t2=r.m_data[i];
					if(t1>t2)
						return 1;
					else if(t1<t2)
						return -1;
				}
			}

			return 0;
		}
		
		void Print(char *s=0)
		{
			for (int i=m_data.size()-1;i>=0;i--)
				//for (int i=0;i<m_data.size();i++)
			{
				if(s)
					sprintf(s, "%d",m_data[i]);
				else
					printf("%d",m_data[i]);
			}
			//printf("\n");
		}

		unsigned long toULong()
		{
			unsigned long t=0;
			for (int i=m_data.size()-1;i>=0;i--)
			{
				t=t *10 + m_data[i];
			}

			return t;
		}
	protected:
		//计算商和mod

		void simplify()
		{
			int n=m_data.size();
			int i;
			for (i=0;i<n-1;i++)
			{
				//m_data[i+1]+=m_data[i]/10; //高位
				//m_data[i]=m_data[i]%10; //低位
				int c;
				m_data[i]=mod(m_data[i],10,&c);
				m_data[i+1]+=c;
			}
			int t=m_data[i];
			if(t>=10)
			{
				// 				m_data[i]=t%10;
				// 				t=t/10;
				m_data[i]=mod(t,10,&t);

				while (t>0)
				{
					// 					m_data.push_back(t%10);
					// 					t=t/10;
					m_data.push_back(mod(t,10,&t));
				}
			}
			//删除后面多余的0
			while (!m_data.empty() && m_data[m_data.size()-1]==0)
			{
				m_data.pop_back();
			}
		}
	private:
	};

	struct Trans{
		int acInc[5];
		int acDec[5];
	};

	struct Trans trans0 = {
		{8,-1}, {-1}
	};

	struct Trans trans1 = {
		{7,-1}, {-1}
	};

	struct Trans trans2 = {
		{-1}, {-1}
	};

	struct Trans trans3 = {
		{9,-1}, {-1}
	};

	struct Trans trans4 = {
		{-1}, {-1}
	};

	struct Trans trans5 = {
		{6,9,-1}, {-1}
	};

	struct Trans trans6 = {
		{8,-1}, {5,-1}
	};

	struct Trans trans7 = {
		{-1}, {1,-1}
	};

	struct Trans trans8 = {
		{-1}, {0,6,9,-1}
	};

	struct Trans trans9 = {
		{8,-1}, {3,5,-1}
	};

	struct Trans astTrans[10]={trans0,
	trans1,trans2,trans3,trans4,trans5,trans6,
	trans7,trans8,trans9};

	struct Res{
		CLargeUInt A;
		CLargeUInt B;
		CLargeUInt C;
		char op;
	};

	struct Res2{
		char s[200];
	};
	vector <Res> vRes;
	

	/*int cmp ( const void *a , const void *b )
	{
		Res* pA = ( Res* )a;
		Res* pB = ( Res* )b;

		int nA = pA->A.compare( pB->A);
		if(nA!=0)return nA;

		int nB = pA->B.compare( pB->B);
		return nB;
	} */

	int cmp2 ( const void *a , const void *b )
	{
		Res2* pA = ( Res2* )a;
		Res2* pB = ( Res2* )b;

		return strcmp(pA->s, pB->s);
	} 

	/*void SortAndPrint()
	{
		

		qsort(&vRes[0],vRes.size(),sizeof(Res),cmp); 

		
		for(size_t i=0 ; i<vRes.size(); i++)
		{
			vRes[i].A.Print();
			printf("%c", vRes[i].op);
			vRes[i].B.Print();
			printf("=");
			vRes[i].C.Print();
			printf("\n");
		}
	}*/

	void SortAndPrint2()
	{
		vector <Res2> vRes2;
		vRes2.reserve(vRes.size());

		char s[200];
		char sAll[200]={0};
		Res2 res2;

		for(size_t i=0 ; i<vRes.size(); i++)
		{
			sAll[0] = 0;
			vRes[i].A.Print(s);
			strcat(sAll, s);

			sprintf(s, "%c", vRes[i].op);
			strcat(sAll, s);

			vRes[i].B.Print(s);
			strcat(sAll, s);

			sprintf(s, "=");
			strcat(sAll, s);

			vRes[i].C.Print(s);
			strcat(sAll, s);

			sprintf(s,"\n");
			strcat(sAll, s);

			strcpy(res2.s, sAll);
			vRes2.push_back(res2);
		}

		
		qsort(&vRes2[0],vRes2.size(),sizeof(Res2),cmp2); 


		for(size_t i=0 ; i<vRes2.size(); i++)
		{
			printf("%s",vRes2[i].s);
		}
	}

	int TestEqu(char *s)
	{	
		int num=0;
		char a[100];
		char b[100];
		char c[100];
		char op=0;
		char *p = s;
		char *p2 = s;
		while (*p!='+' && *p!='-') p++;
		strncpy(a, s, p-s);
		a[p-s]=0;
		op = *p;

		p++;
		p2 = p;
		while (*p2!='=' ) p2++;
		strncpy(b, p, p2-p);
		b[p2-p]=0;

		p2++;
		strcpy(c, p2);

		/*printf("a %s\n",a);
		printf("op %c\n",op);
		printf("b %s\n",b);
		printf("c %s\n",c);*/
		if(a[0]=='0') return 0;
		if(b[0]=='0') return 0;
		if(c[0]=='0') return 0;

		CLargeUInt unA(a);
		CLargeUInt unB(b);
		CLargeUInt unC(c);
		CLargeUInt unAB;

		int ok=0;
		switch(op)
		{
		case '+':
			unAB = unA.add(unB);
			ok = 1;
			break;
		case '-':
			if(unA.compare(unB)<0)
				ok=0;
			else
				unAB = unA.sub(unB);
			break;
		default:
			ok=0;
		}
		
		if(ok && unAB.compare(unC)==0)
		{
			Res res;
			res.A = unA;
			res.B = unB;
			res.C = unC;
			res.op = op;

			vRes.push_back(res);

			/*unA.Print();
			printf("%c", op);
			unB.Print();
			printf("=");
			unC.Print();
			printf("\n");*/
			num++;
		}

		return num;
	}

	void Test(char *s)
	{
		int num=0;
		int sLen=strlen(s);
		for(int i=0; i<sLen; i++)
		{
			if(s[i]<'0' || s[i]>'9')continue;

			//a+1
			char aBk = s[i];
			for(int ia = 0; astTrans[aBk-'0'].acInc[ia]>=0; ia++)
			{
				s[i] = astTrans[aBk-'0'].acInc[ia]+'0';

				//b-1
				for(int j=i+1; j<sLen; j++)
				{
					if(j==i)continue;
					if(s[j]<'0' || s[j]>'9')continue;

					char bBk = s[j];
					for(int ib = 0; astTrans[bBk-'0'].acDec[ib]>=0; ib++)
					{
						s[j] = astTrans[bBk-'0'].acDec[ib]+'0';

						//test
						num += TestEqu(s);

						s[j] = bBk;
					}
					
				}

				s[i] = aBk;
			}

			//a-1
			aBk = s[i];
			for(int ia = 0; astTrans[aBk-'0'].acDec[ia]>=0; ia++)
			{
				s[i] = astTrans[aBk-'0'].acDec[ia]+'0';

				//b+1
				for(int j=i+1; j<sLen; j++)
				{
					if(j==i)continue;
					if(s[j]<'0' || s[j]>'9')continue;

					char bBk = s[j];
					for(int ib = 0; astTrans[bBk-'0'].acInc[ib]>=0; ib++)
					{
						s[j] = astTrans[bBk-'0'].acInc[ib]+'0';

						//test
						num += TestEqu(s);

						s[j] = bBk;
					}

				}

				s[i] = aBk;
			}
		}

		if(num==0)
			printf("-1\n");
		else
			SortAndPrint2();
	}

	int main()
	{
		char s[200];
		
		fgets(s,199, stdin);

		int nPos=strlen(s)-1;
		for(; (s[nPos]<'0' || s[nPos]>'9') && nPos>=0; nPos--);
		s[nPos+1]=0;
		
		Test(s);
		return 0;
	}
};
