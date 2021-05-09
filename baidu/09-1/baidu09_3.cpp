#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <assert.h>
#include <math.h>

using namespace std;

//#define PI 3.1415926
//#define DBG 

namespace Baidu09_3
{
	struct Row{
		int data[100];
	};
	vector <Row> vGraph;

	struct Path{
		int r;
		int c;
		int h;
	};
	vector <Path>  vPath;
	vector <Path>  vPathMax;
	
	void Test(vector <Row> &vGraph, int R, int C, int r, int c)
	{
		int dir[4][2]={
			{0,1},
			{0,-1},
			{1,0},
			{-1,0},
		};

		bool canGo =false;
		for (int i=0; i<4; i++)
		{
			int r1=r+dir[i][0];
			int c1=c+dir[i][1];

			if(r1>=0 && r1<R && c1>=0 && c1<C && vGraph[r1].data[c1]<vGraph[r].data[c])
			{
				canGo = true;
				Path path = {r1, c1, vGraph[r1].data[c1]};
				vPath.push_back(path);

				Test(vGraph,R,C,r1,c1);

				vPath.pop_back();
			}
		}

		if(!canGo)
		{
			if(vPathMax.size()<vPath.size())
				vPathMax = vPath;
			/*printf("\n----%d\n",vPath.size());
			for(size_t i=0; i<vPath.size(); i++)
				printf("%d ", vPath[i].h);*/
		}
	}

	int main()
	{
		int R,C;
		scanf("%d%d", &R, &C);
		vGraph.reserve(R);
		
		for(int i=0; i<R; i++)
		{
			Row row;

			for(int j=0; j<C; j++)
				scanf("%d", &row.data[j]);

			vGraph.push_back(row);
		}

		for(int i=0; i<R; i++)
		{
			for(int j=0; j<C; j++)
			{
				vPath.clear();

				Path path = {i, j, vGraph[i].data[j]};
				vPath.push_back(path);
				Test(vGraph,R,C,i,j);
			}
		}
		
		printf("%d\n",vPathMax.size());
		/*for(size_t i=0; i<vPathMax.size(); i++)
			printf("%d ", vPathMax[i].h);*/
		return 0;
	}
};
