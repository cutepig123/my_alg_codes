#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <math.h>

using namespace std;

//#define PI 3.1415926
//#define DBG(x, format) printf(#x "=" format "\n", (x))
#define DBG(x, format) 

namespace Baidu09_2
{

	void DelN(char *s)
	{
		int nPos=strlen(s)-1;
		for(; (s[nPos]=='\n' || s[nPos]==' ' || s[nPos]=='\t') && nPos>=0; nPos--);
		s[nPos+1]=0;
	}

	struct   Point   {                             //   二维点或矢量  
		double   x,   y;   
		double arCos;
	};

	struct Line{
		//double from[2];
		//double to[2];
		Point from;
		Point to;
	};

	//get   the   intersection   point   of   line   L1,L2  
	//returns:    
	//0:   parallel  
	//1:   unique   intersection   via   inter  
	//2:   same   line    
	int   GetIntersect(Point  & L11,   Point  & L12,   Point   &L21,   Point  & L22,   Point&   inter)  
	{  
		//L1:   a1x+b1y=c1  
		double   a1=L12.y-L11.y;  
		double   b1=L11.x-L12.x;  
		double   c1=L11.x*L12.y-L12.x*L11.y;  
		//L2:   a2x+b2y=c2  
		double   a2=L22.y-L21.y;  
		double   b2=L21.x-L22.x;  
		double   c2=L21.x*L22.y-L22.x*L21.y;  

		double   detab=a1*b2-a2*b1;  
		if(detab==0)  
		{  
			double   r;  
			if(a2!=0)   r=a1/a2;  
			else   r=b1/b2;  

			if(c1==0&&c2==0)   return   2;  
			if(r==c1/c2)   return   2;  
			else   return   0;  
		}  

		inter.x=(c1*b2-c2*b1)/detab;  
		inter.y=(a1*c2-a2*c1)/detab;  
		return   1;  
	}
#define MAXN 10000
	int stack[MAXN];  // 栈
	class TuBao
	{
	public:
		int n;     // 顶点的个数

		Point *points;     // 顶点数组
		TuBao(Point *points_, int n_): points(points_), n(n_)
		{}

		

		int Multi(Point px, Point py, Point pz)

		{

			double k;

			k = (py.x-px.x)*(pz.y-py.y) - (pz.x-py.x)*(py.y-px.y);  // 计算两个向量的向量积,

			// 判断3个点所成的角是不是一个"凹"角.

			if(k < 0) return 1;

			return 0;

		}



		/*角度计算函数*/

		double Angle(int i)

		{

			double j, k, m, n;

			j = fabs(points[i].x - points[0].x);

			k = fabs(points[i].y - points[0].y);

			m = sqrt(j*j+k*k);     //得到顶点i 到第一顶点的线段长度.

			n = acos(j/m);      //得到该线段与x轴的角度

			//强悍

			return n;

		}



		void Swap(int px, int py)

		{

			Point k;

			k = points[px];

			points[px] = points[py]; //注意

			points[py] = k;

		}

		int Loc(int top, int bot)

		{

			//void Swap(int, int);

			double x = points[top].arCos;

			int j, k;

			j = top+1;

			k = bot;

			while(1) {

				while(j < bot && points[j].arCos < x) j++;

				while(k > top && points[k].arCos > x) k--;



				if(j >= k) break;



				Swap(j, k);

			}

			Swap(top, k);

			return k;

		}

		void QSort(int top, int bot)

		{

			//快排

			//int Loc(int, int);

			int pos;



			if(top < bot) {

				pos = Loc(top, bot);

				QSort(top, pos-1);

				QSort(pos+1, bot);

			}

		}

		double Distance(Point &pt1, Point &pt2)
		{
			double dx = pt1.x - pt2.x;
			double dy = pt1.y - pt2.y;

			return fabs(dx*dx + dy*dy);
		}

		//点到直线距离
		double Dist(Point &pt, Point &pt1, Point &pt2)
		{
			double dy=pt1.y - pt2.y;
			double dx=pt1.x - pt2.x;
			double a = dy;
			double b = -dx;
			double c = -dy * pt1.x + dx * pt1.y;
			double dist = fabs(a*pt.x + b*pt.y + c) / sqrt(dx*dx + dy*dy);
			return dist;
		}

		//矩形周长
		double JuXing(Point *pts, unsigned int n,
			unsigned int i)	//pt i,&, i+1
		{
			Point pt1 = pts[i];
			Point pt2 = pts[(1+i)%n];
			double dy=pt1.y - pt2.y;
			double dx=pt1.x - pt2.x;
			double dist1 = sqrt(dx*dx + dy*dy);
			double angle = atan2(pt1.y - pt2.y, pt1.x - pt2.x);

			double dist = 0;
			for (unsigned int j=0; j<n; j++)
			{
				//if(j==i || j==i+1)continue;

				dist = max(Dist(pts[j],pt1,pt2), dist);
			}

			return dist*2 + dist1*2;
		}

		void Print(Point *pts, unsigned int n)
		{
			for (unsigned int i=0; i<n; i++)
			{
				printf("%lf %lf,", pts[i].x, pts[i].y);
			}

			printf("\n");
		}
		void Make(int Type)

		{

			//int Multi(Point, Point, Point);   //计算两个向量的积

			//double Angle(int);      //计算其余顶点与第一顶点的角度,为排序做准备

			//void QSort(int, int);     //对顶点进行快速排序

			//void Swap(int, int);     

			int i, j, t;

			double min = 32767.0;

			for(i = 0; i < n; ++i){     //找第一个顶点,做为算法的起始顶点

				if(points[i].y < min) {

					j = i;

					min = points[i].y;

				}

			}

			Swap(0, j);

			for(i = 1; i < n; ++i){     //计算除第一顶点外的其余顶点到第一点的线段与x轴的夹角

				points[i].arCos = Angle(i);

			}

			QSort(1, n-1);       //根据所得到的角度进行快速排序.

			for(i = 0; i <= 2; ++i) stack[i] = i;   //将前3个顶点压栈

			t = 2;

			while(i < n) {

				/*如果新的点,与最近入栈中的2点构成了一个"凹"角, 则将栈顶元素出栈. 直到把栈检查完*/

				while(Multi(points[stack[t-1]], points[stack[t]], points[i]) && t >= 1) 

					t--;    

				t++;        // 将新点压栈

				stack[t] = i;

				i++;

			}

			/*打印结果*/


			double dist=0;
			t++;
			if(Type == 1)
			{
				for(i = 0; i <t; ++i)
				dist += Distance(points[stack[i]], points[stack[(i+1)%t]] );
			//	printf("<%.2lf, %.2lf>\n",points[stack[i]].x, points[stack[i]].y);
				printf("%.2lf\n", dist);
			}
			else
			{
				vector <Point> vPtVect;
				vPtVect.reserve(t);
				for(i = 0; i <t; ++i)
					vPtVect.push_back(points[stack[i]]);
				
				//double dist = 0;
				DBG(vPtVect.size(), "%d");
				//Print(&vPtVect[0],t);
				for(i = 0; i <t; ++i)
				{
					//_asm int 3;
					double curDist = JuXing(&vPtVect[0],t,i);
					dist = max(dist, curDist);
					DBG(curDist, "%lf");
				}

				printf("%.2lf\n", dist);
			}
		}

	};

	

	int main()
	{
		unsigned int Type,N,i,j;
		vector <Line> vLine;

		scanf("%d\n", &Type);
		scanf("%d\n", &N);
		DBG(Type, "%d");
		DBG(N, "%d");

		vLine.reserve(N);
		for (i=0; i<N; i++)
		{
			Line line;
			scanf("%lf%lf%lf%lf\n",&line.from.x,&line.from.y,
				&line.to.x,&line.to.y);
			vLine.push_back(line);
		}
		DBG(vLine.size(), "%d");

		//Point ptLT, ptRB;
		//int start=1;
		vector <Point> vIter;
		vIter.reserve(N*(N-1)/2);

		for (i=0; i<N; i++)
		{
			for (j=i+1; j<N; j++)
			{
				Point iter;
				Line &l1 = vLine[i];
				Line &l2 = vLine[j];
				if(1==GetIntersect(l1.from,l1.to, l2.from,l2.to,iter ))
				{
					vIter.push_back(iter);
					/*if(start)
					{
						start = 0;
						ptLT = ptRB = iter;
					}
					else
					{
						ptLT.x = min(ptLT.x, iter.x);
						ptLT.y = min(ptLT.y, iter.y);

						ptRB.x = max(ptRB.x, iter.x);
						ptRB.y = max(ptRB.y, iter.y);
					}*/
				}
			}
		}
		DBG(vIter.size(), "%d");

		TuBao tb(&vIter[0], vIter.size());
		DBG(1,"%d");
		tb.Make(Type);
		DBG(2,"%d");
		return 0;
	}
};
