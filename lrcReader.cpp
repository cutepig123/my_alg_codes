#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <vector>
#include <string>
#include <map>

using namespace std;
//读取并分析 .lrc歌词列表文件

namespace LrcRdr
{
	inline bool isBlank(char c)
	{
		return c==' ' || c=='\n' || c=='\t';
	}

	//return 0~9 OK
	//-1 ERROR
	inline int getNumIfIsNum(char c)
	{
		if(c>='0' && c<='9')
			return c-'0';
		else
			return -1;
	}
	
	void delBlank(char *& psText)
	{
		while (isBlank(*psText))
			psText++;
	}
	//return true / false
	//decode main function
	bool decTimeAndText(char * psLine, //in
					char ** ppsText,//out,不需要在外部分配内存，因为这里返回一个指向sLine的指针
					vector<long> &stTime	//out,由于一行可能有多个时间，所以放到vectoe了
					)
	{
		if(!psLine || ! ppsText)
			return false;
		
		char *p1 = psLine; //1st char
		char *p2 = psLine + strlen(psLine)-1; //last char

		if (p2<p1)
			return false;
	
		//del blank
		while (isBlank(*p1))
			p1++;
		while (isBlank(*p2))
			p2--;
		if (p2<p1)
			return false;

		p2[1] = 0;

		//get time时间应该站7byte or 10byts..
		stTime.clear();

		while(1)
		{
			if(strlen(p1)<7)
				break;

			if(p1[0]!='['  || p1[3]!=':')	//no time
				break;

			if(p1[6]==']')
			{
				int t1=getNumIfIsNum(p1[1]);
				int t2=getNumIfIsNum(p1[2]);
				int t3=getNumIfIsNum(p1[4]);
				int t4=getNumIfIsNum(p1[5]);
				if(t1<0 || t2<0 || t3<0 || t4<0)
					break;

				long lTm = (t1*10 + t2)* 60 + t3*10 + t4;

				stTime.push_back(lTm);

				p1 += 7;

			}
			else if(strlen(p1)>10 && p1[9])
			{
				int t1=getNumIfIsNum(p1[1]);
				int t2=getNumIfIsNum(p1[2]);
				int t3=getNumIfIsNum(p1[4]);
				int t4=getNumIfIsNum(p1[5]);
				if(t1<0 || t2<0 || t3<0 || t4<0)
					break;

				long lTm = (t1*10 + t2)* 60 + t3*10 + t4;

				stTime.push_back(lTm);

				p1 += 10;
			}
			else
				break;

			delBlank(p1);
		}
		if (stTime.size()>0 && strlen(p1)>0)
		{
			*ppsText = p1;
			return true;
		}
		else
			return false;
	}
	void test(const char * sLrcName)
	{
		char sLine[1000];
		char *psText;
		vector<long> stTime;

		vector <string> stSongCont;	//歌的正文
		map <long, int> stTimeToIdx;//时间和歌正文idx的对应关系
		int nIdx = 0;

		FILE * fp=fopen(sLrcName,"r");
		assert(fp);

		printf("一行行读入，并分析后的结果:\n");
		while (fgets(sLine,999,fp))
		{
			bool rStatus = decTimeAndText(sLine, &psText, stTime);

			if (rStatus)
			{
				printf("Time ");
				for (int i=0;i<stTime.size();i++)
				{
					long lTm = stTime[i];
					printf("%d:%d ",lTm/60,lTm%60);
				}

				printf("Text:%s#\n",psText);

				assert(strlen(psText)<1000);

				//添加时间和字符串
				stSongCont.push_back(psText);

				for (int i=0;i<stTime.size();i++)
				{
					long lTm = stTime[i];
					stTimeToIdx[lTm] = nIdx;
				}
				
				nIdx++;
			}
		}
		fclose(fp);

		printf("整理后的列表:\n");

		for (map<long, int>::iterator iter = stTimeToIdx.begin();
			iter != stTimeToIdx.end();	++iter)
		{
			long lTm = iter->first;
			int nIdx = iter->second;

			assert(nIdx >=0 && nIdx < stSongCont.size());

			string stext = stSongCont[nIdx];

			printf("[%.2d:%.2d] %s\n",lTm/60, lTm%60, stext.c_str());
		}
	}
}