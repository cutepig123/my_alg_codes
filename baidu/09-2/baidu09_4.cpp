#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <assert.h>
#include <math.h>

using namespace std;

//#define PI 3.1415926
//#define DBG 

namespace Baidu09_4
{
//#define DBG(x, format) printf(#x "=" format "\n", (x))
#define DBG(x, format) 

	struct POI 
	{
		unsigned int id;
		char Type[20];
		int grade;
		double x;
		double y;
	};

	void Select(vector <POI> &vPOI,char Type[20], int Min, int Max, vector <unsigned int> &vP)
	{
		unsigned int N = vPOI.size();

		for (unsigned int i=0; i<N; i++)
		{
			POI &poi = vPOI[i];
			if(poi.grade >= Min && poi.grade <= Max &&
				strcmp(poi.Type, Type)==0)
				vP.push_back(i);

		}
	}

	//struct   Point   {                            
	//	double   x,   y;   
	//	//double arCos;
	//};

	template <typename Point>
	double Distance(Point &pt1, Point &pt2)
	{
		double dx = pt1.x - pt2.x;
		double dy = pt1.y - pt2.y;

		return fabs(dx*dx + dy*dy);
	}

	void MinDist(vector <POI> &vPOI,
		vector <unsigned int> &vP1,
		vector <unsigned int> &vP2,
		vector <unsigned int> &vP3)
	{
		unsigned int p1=-1,p2=-1,p3=-1;
		double minDist;
		int start=1;

		size_t n1 = vP1.size();
		size_t n2 = vP2.size();
		size_t n3 = vP3.size();
		size_t i,j,k;

		for (i=0; i<n1; i++)
		{
			POI & pt1 = vPOI[vP1[i]];
			for (j=0; j<n2; j++)
			{
				POI & pt2 = vPOI[vP2[j]];
				for (k=0; k<n3; k++)
				{
					POI & pt3 = vPOI[vP3[k]];
					double dist = 
						Distance(pt1, pt2) + Distance(pt3, pt2) + Distance(pt1, pt3);
					if(start)
					{
						p1 = i;
						p2 = j;
						p3 = k;
						minDist = dist;
						start = 0;
					}
					else
					{
						if(minDist > dist)
						{
							p1 = i;
							p2 = j;
							p3 = k;
							minDist = dist;
						}
					}
				}
			}
		}

		printf("%d %d %d\n", vPOI[vP1[p1]].id, vPOI[vP2[p2]].id, vPOI[vP3[p3]].id);
	}

	int main()
	{
		unsigned int k,i,n;
		scanf("%d\n", &k);

		vector <POI> vPOI;
		vPOI.reserve(k);
		for (i=0; i<k; i++)
		{
			POI poi;
			
			scanf("%d %s %d %lf %lf\n", &poi.id, poi.Type,
				&poi.grade, &poi.x, &poi.y);
			DBG(poi.Type,"%s");
			vPOI.push_back(poi);
		}
		DBG(vPOI.size(),"%d");

		scanf("%d\n", &n);
		DBG(n,"%d");

		for (i=0; i<n; i++)
		{
			char Type[3][20];
			int Min, Max;
			scanf("%s %s %s %d %d\n", Type[0], Type[1], Type[2],
				&Min, &Max);

			vector <unsigned int> vP1,vP2,vP3;

			Select(vPOI, Type[0], Min, Max, vP1);
			DBG(vP1.size(), "%d");
			Select(vPOI, Type[1], Min, Max, vP2);
			DBG(vP2.size(), "%d");
			Select(vPOI, Type[2], Min, Max, vP3);
			DBG(vP3.size(), "%d");

			MinDist(vPOI, vP1, vP2, vP3);
		}
		return 0;
	}
};
