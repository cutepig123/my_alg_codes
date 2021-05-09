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

namespace Baidu09_2
{

	void DelN(char *s)
	{
		int nPos=strlen(s)-1;
		for(; (s[nPos]=='\n' || s[nPos]==' ' || s[nPos]=='\t') && nPos>=0; nPos--);
		s[nPos+1]=0;
	}

	struct NInt{
		int data[200];	//-1,other person; 0,empty; >=1, xiaoMing's
	};

	struct Pos{
		int n;
		int k;
	};

	struct MovPath{
		Pos from;
		Pos to;
	};

	vector <MovPath> vPath;

	int g_N,g_K;

	void PrintPath()
	{
		size_t nPath = vPath.size();

		printf("%d\n", nPath);
		for (size_t i=0; i<nPath; i++)
		{
			printf("(%d,%d)->(%d,%d)\n", vPath[i].from.n,vPath[i].from.k,
				vPath[i].to.n, vPath[i].to.k);
		}
	}

	void PrintSeat(vector <NInt> &vSeat)
	{
		for(size_t n=0; n<g_N;n++)
		{
			for(size_t k=0; k<g_K; k++)
			{
				if( vSeat[n].data[k] == -1)
					printf("*");
				else if( vSeat[n].data[k] == 0)
					printf(".");
				else
					printf("%d",vSeat[n].data[k]);
			}

			printf("\n");
		}
	}

	size_t Process(vector <NInt> &vSeat,vector<NInt> &vScore)
	{
		size_t sum=0;

		for(size_t i=0; i<g_N; i++)
		{
			for(size_t j=0;j<g_K;j++ )
			{
				if(vSeat[i].data[j]>0)
					sum += (vScore[i].data[j]);
			}
		}

		
		return sum;
	}

	bool Process(vector <NInt> &vSeat, vector <Pos> &vMingCar, int visited[])
	{
		int curCar=-1;

		for(int k=0; k<vMingCar.size(); k++)
		{
			if(visited[k])continue;
			
			//printf("visiting car %d\n", k);
			visited[k] = 1;
			curCar = k;
	
			Pos &curPos = vMingCar[curCar];
			int curCarId = vSeat[curPos.n].data[curPos.k];
			int N=g_N;
			int K=g_K;
			
			//srch an empty pos
			for (int i=0; i<N; i++)
			{
				if(i==curPos.n) continue;

				for(int j=0; j<K;j++)	
				{
					if(vSeat[i].data[j]!=0)continue;
					
					//Go
					vSeat[curPos.n].data[curPos.k] = 0;
					vSeat[i].data[j] = curCarId;
					MovPath mvPath;
					mvPath.from = curPos;
					mvPath.to.n = i;
					mvPath.to.k = j;
					vPath.push_back(mvPath);

					//printf("DBG: from %d %d-> %d %d\n", curPos.n, curPos.k, i, j);
					vMingCar[curCar] = mvPath.to;
					//PrintSeat(vSeat);

					//End?
					if(visited+vMingCar.size()== find(visited,visited+vMingCar.size(),0))
					{
						PrintPath();
						//return true;
					}
					else
						if(Process(vSeat, vMingCar, visited))
							return true;

					//Back
					vSeat[i].data[j] = 0;
					vSeat[curPos.n].data[curPos.k] = curCarId;
					vMingCar[curCar] = vPath.back().from;
					vPath.pop_back();
				}
			}

			visited[k] = 0;

		}

		
		return false;
	}

	int main()
	{
		char line[200];
		int op;
		int N,K;
		
		scanf("%d", &op);
		int numMing = 0;
		
		vector <NInt> vSeat;
		vector <Pos> vMingCar;
		
		Pos pos;

		scanf("%d%d\n", &N, &K);
		g_N = N;
		g_K = K;

		//read data
		vSeat.reserve(N);
		vMingCar.reserve(N);
		
		for(int i=0; i<N;i++)
		{
			fgets(line, 199, stdin);
			DelN(line);

			NInt nInt;
			for(int j=0; j<K;j++)
			{
				switch(line[j])
				{
				case '#':
						pos.n = i;
						pos.k = j;
						vMingCar.push_back(pos);

						nInt.data[j]=numMing+1;
						numMing++;
						break;
					case '*':
						nInt.data[j]=-1;
						break;
					case '.':
						nInt.data[j]=0;
						break;
					default:
						assert(0);
				}
			}

			vSeat.push_back(nInt);
		}

		
		//process
		if(op==1)
		{
			vector<int> visited;
			visited.resize(vMingCar.size());
			memset(&visited[0],0, sizeof(int)*vMingCar.size());
			if(!Process(vSeat, vMingCar, &visited[0]))
			printf("-1\n");
		}
		else if(op==2)
		{
			vector<NInt> vScore;
			vScore.reserve(N);

			NInt nInt;
			for(int i=0; i<N;i++)
			{
				for(int j=0; j<K;j++)
					scanf("%d",&nInt.data[j]);
				vScore.push_back(nInt);
			}

			size_t sum=Process(vSeat,vScore);
			printf("%d\n", sum);
		}//if op
		
		return 0;
	}
};
