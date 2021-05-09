#include "stdafx.h"
#include "cv.h"
#include "highgui.h"

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
/*
2008年7月6日
1.Fix a 严重的problem of vote algorithm of srch rotated image

2. add new function :
getImgEdgeAndDirection_New
GHT_CalcSinTbl_int
GHT_CalcRTable_int
GHT_SinFast_int
GHT_CosFast_int
GHT_SearchXYRot_int
TestGHT_int
printRTable_int

add new struct:
stR_Elem_int stRTableNode_int stRTableType_int

问题是，速度的提高很小，比如一次测试为19s->15s，edgept为765，rotRange为[-90:2:90]
发现：代码有问题，找的不对！


下一步计划：
1.用int存放rtable，用int做srch，预存正余弦表在mem中,对于精度可才采用保存时<<,srch时再>>的方法
将图像指针定位改为指针++
用map存放RTable得了，生的老是计算
2.lrn时选一部分edge pt（<=150,>=20pts）存到rtable中（如何选择？reduction,边缘强度？）
3.rtable中存放rotation?(是不是占内存太大了,item: rotation->edge_angle->[Ri,sitai]or[xi yi] since we need not compute sin/cos now)
4.reduction when srch (这么得将低分辨率的位置差值到高分辨率上，哪么如何判断低分辨率的位置是正确的呢？)
5.如何判分？最起码在srch lrn image时应该为100分左右吧！
6.用canny检测边缘似乎不是很好，可以改用sobel，但是用sobel的话，必须得考虑地edge pt采样否则数据量太大了

*/

using namespace std;

#define PI 3.1415926

#define _SHOW_IMG(p)  cvNamedWindow(#p);cvShowImage(#p,p);
#define INT_PRECISION 8

struct stR_Elem
{
	double r;
	double alpha;
};

struct stR_Elem_int
{
	int r;
	int alpha;//0~180
};
typedef vector<stR_Elem> stRTableNode; 
typedef vector<stRTableNode> stRTableType;

typedef vector<stR_Elem_int> stRTableNode_int; 
typedef vector<stRTableNode_int> stRTableType_int;


//gray byte image
void printIplImage(IplImage const *pImg)
{
	assert(pImg);

	int nW=pImg->width;
	int nH=pImg->height;

	for (int y=0;y<nH;y++)
	{
		for (int x=0;x<nW;x++)
		{
			int nGray;
			nGray = CV_IMAGE_ELEM(pImg,unsigned char,y,x);
			//assert(0==GHT_GetIplPixel(pImg,y,x,&nGray));
			printf("%3d ",nGray);
		}
		printf("\n");
	}
}
template <typename T>
void printArray(T const *pArray,int nH,int nW,int nStep)
{
	assert(pArray);

	for (int y=0;y<nH;y++)
	{
		for (int x=0;x<nW;x++)
		{
			int nGray = pArray[y*nStep + x];

			printf("%3d ",nGray);
		}
		printf("\n");
	}
}
void copyArrayToImage(char const *pArray,
					  IplImage *pImg)
{
	assert(pImg);

	int nW=pImg->width;
	int nH=pImg->height;
	for(int y=0;y<nH;y++)
	{
		for (int x=0;x<nW;x++)
		{
			CV_IMAGE_ELEM(pImg,char,y,x)=pArray[y*nW +x] ;
		}
	}

}

long printRTable(const stRTableType &	stRTable)
{
	unsigned long nTotalNum = 0;

	for (int i=0;i<stRTable.size();i++)
	{
		nTotalNum += stRTable[i].size();

		/*printf("%3d:",i);
		for (int j=0;j<stRTable[i].size();j++)
		{
			const stR_Elem &stElem = stRTable[i][j];
			printf("(%3.3f,%3.3f)",stElem.r,stElem.alpha);
		}
		printf("\n");*/
	}

	printf("NumEdgePts of RTable:%d\n",nTotalNum);

	return nTotalNum;

}

long printRTable_int(const stRTableType_int &	stRTable_int)
{
	unsigned long nTotalNum = 0;

	for (int i=0;i<stRTable_int.size();i++)
	{
		nTotalNum += stRTable_int[i].size();

		/*printf("%3d:",i);
		for (int j=0;j<stRTable[i].size();j++)
		{
		const stR_Elem &stElem = stRTable[i][j];
		printf("(%3.3f,%3.3f)",stElem.r,stElem.alpha);
		}
		printf("\n");*/
	}

	printf("NumEdgePts of RTable_int:%d\n",nTotalNum);

	return nTotalNum;

}
#define PRINTIMAGE(x) printf("Image " #x "\n");printIplImage(x);

//step 1,sin(0 to 89)<<8
//return 
//0: Ok
//piSinTbl size must>=90
int GHT_CalcSinTbl_int(int *piSinTbl)
{
	assert(piSinTbl);
	for (int i=0;i<=90;i++)
	{
		piSinTbl[i] =(int)(sin(i * PI/180) * (1<<INT_PRECISION));
	}
	return 0;
}


inline int GHT_SinFast_int(int			sita,
				   int	const	*piSinTbl)
{
	sita = 	(sita)%360;
	if(sita<0)
		sita +=360;
	if (sita<90)
		return piSinTbl[sita];
	if (sita<180)
		return piSinTbl[180-sita];

	if (sita<270)
		return - piSinTbl[sita - 180];
	if (sita<360)
		return - piSinTbl[360-sita];

	assert(0);
	return -1;
}

//0<=sita<=90
inline int GHT_CosFast_in90(int			sita,
					   int	const	*piSinTbl)
{
	assert(sita>=0 && sita<=90);
	return piSinTbl[90-sita];
}
inline int GHT_CosFast_int(int			sita,
					   int	const	*piSinTbl)
{
	sita = 	(sita)%360;
	if(sita<0)
		sita +=360;
	if (sita<90)
		return GHT_CosFast_in90(sita,piSinTbl);
	if (sita<180)
		return -GHT_CosFast_in90(180-sita,piSinTbl);

	if (sita<270)
		return - GHT_CosFast_in90(sita - 180,piSinTbl);
	if (sita<360)
		return GHT_CosFast_in90(360-sita,piSinTbl);

	assert(0);
	return -1;
}

void TestFastSinCos()
{
	int astSinTbl[100];
	GHT_CalcSinTbl_int(astSinTbl);

	for (int i=-360;i<360;i+=3)
	{
		int diff = GHT_SinFast_int(i,astSinTbl) - sin(i*PI/180.0) *(1<<INT_PRECISION);
		int diff_c = GHT_CosFast_int(i,astSinTbl) - cos(i*PI/180.0) *(1<<INT_PRECISION);
		if(abs(diff)>2 || abs(diff_c)>2)
		{
			assert(0);
		}
	}
}
/************************************************************************/
/* return:
0	OK
-1	input error
-2	check image size equal error
-3	GHT_GetIplPixel error
-4	pDirImg elem invalid(not between 0~180)
*/
//加入ROI支持
/************************************************************************/

int GHT_CalcRTable(const IplImage		*pEdgeImg,	//0 for backgrond, non 0 for foreground
				   const IplImage		*pDirImg,	//单位是度
						int				nStepOfRTable,	//step of RTable,can be 1~180
						bool			bDebugFlag,
				   stRTableType			&stRTable
				   )
{
	int nStatus=0;

	if(!pDirImg || !CV_IS_IMAGE(pDirImg) ||
		!pEdgeImg || !CV_IS_IMAGE(pEdgeImg) ||
		 nStepOfRTable<1 || nStepOfRTable>180)
	{
		assert(0);
		return -1;
	}

	CvRect stRoi = cvGetImageROI( pEdgeImg ); 
	printf("GHT_CalcRTable stRoi: x %d y %d w %d h %d\n",stRoi.x, stRoi.y,stRoi.width,stRoi.height);

	//check size equal
	if (pEdgeImg->height != pDirImg->height || pEdgeImg->width != pDirImg->width)
	{
		assert(0);
		return -2;
	}
	
	int xc=stRoi.x + stRoi.width/2;
	int yc=stRoi.y + stRoi.height/2;
	printf("calc RTable: xc %d yc %d\n",xc,yc);

	int nSizeOfRTable = 180/nStepOfRTable+1;
	stRTable.clear();	
	stRTable.resize(nSizeOfRTable);

	int tDegVal,tEdgeVal;
	if (bDebugFlag)
	{
		printf("build rtable from y %d->%d x %d->%d\n",stRoi.y,stRoi.y + stRoi.height,
			stRoi.x,stRoi.x + stRoi.width);
	}
	for (int y=stRoi.y; y<stRoi.y + stRoi.height; y++)
	{
		for (int x=stRoi.x;x<stRoi.x + stRoi.width; x++)
		{
			//check if is eage pt, if not,continue
			tEdgeVal = CV_IMAGE_ELEM(pEdgeImg,unsigned char,y,x);

			if (tEdgeVal==0) //back gnd
				continue;

			//is edge pt,get direction
			tDegVal = CV_IMAGE_ELEM(pDirImg,unsigned char,y,x);

			if (tDegVal<0 || tDegVal>180)
			{
				nStatus = -4;
				assert(0);
				//goto EXIT_FUN;
			}
	
			//get idx
			int nIdx = tDegVal/nStepOfRTable;
			assert(nIdx>=0 && nIdx<nSizeOfRTable);

			//calc r, alpha
			stR_Elem stRElem;
			stRElem.r = sqrt((double)(x - xc)*(x - xc) + (y-yc)*(y-yc));
			stRElem.alpha = atan2((double)(y-yc), (x - xc));
			
			if(stRElem.alpha<0)
				stRElem.alpha+=PI;
			
			if (bDebugFlag)
			{
				printf("insert rtable[%d] r %g alpha %g\n",nIdx,stRElem.r,stRElem.alpha);
			}
			stRTable[nIdx].push_back(stRElem);
		}
	}

EXIT_FUN:
	return nStatus;
}

int GHT_CalcRTable_int(
					   const IplImage		*pEdgeImg,	//0 for backgrond, non 0 for foreground
					   const IplImage		*pDirImg,	//单位是度
					   const int			*piSinTbl,	//useless now
					   int					nStepOfRTable,	//step of RTable,can be 1~180
					   bool					bDebugFlag,
					   stRTableType_int		&stRTable_int
					   )
{
	int nStatus=0;

	if(!pDirImg || !CV_IS_IMAGE(pDirImg) ||
		!pEdgeImg || !CV_IS_IMAGE(pEdgeImg) ||
		nStepOfRTable<1 || nStepOfRTable>180)
	{
		assert(0);
		return -1;
	}

	CvRect stRoi = cvGetImageROI( pEdgeImg ); 
	printf("GHT_CalcRTable stRoi: x %d y %d w %d h %d\n",stRoi.x, stRoi.y,stRoi.width,stRoi.height);

	//check size equal
	if (pEdgeImg->height != pDirImg->height || pEdgeImg->width != pDirImg->width)
	{
		assert(0);
		return -2;
	}

	int xc=stRoi.x + stRoi.width/2;
	int yc=stRoi.y + stRoi.height/2;
	printf("calc RTable: xc %d yc %d\n",xc,yc);

	int nSizeOfRTable = 180/nStepOfRTable+1;
	stRTable_int.clear();	
	stRTable_int.resize(nSizeOfRTable);

	int tDegVal,tEdgeVal;
	if (bDebugFlag)
	{
		printf("build rtable from y %d->%d x %d->%d\n",stRoi.y,stRoi.y + stRoi.height,
			stRoi.x,stRoi.x + stRoi.width);
	}
	for (int y=stRoi.y; y<stRoi.y + stRoi.height; y++)
	{
		for (int x=stRoi.x;x<stRoi.x + stRoi.width; x++)
		{
			//check if is eage pt, if not,continue
			tEdgeVal = CV_IMAGE_ELEM(pEdgeImg,unsigned char,y,x);

			if (tEdgeVal==0) //back gnd
				continue;

			//is edge pt,get direction
			tDegVal = CV_IMAGE_ELEM(pDirImg,unsigned char,y,x);

			if (tDegVal<0 || tDegVal>180)
			{
				nStatus = -4;
				assert(0);
				//goto EXIT_FUN;
			}

			//get idx
			int nIdx = tDegVal/nStepOfRTable;
			assert(nIdx>=0 && nIdx<nSizeOfRTable);

			//calc r, alpha
			stR_Elem_int stRElem;
			stRElem.r = sqrt((double)(x - xc)*(x - xc) + (y-yc)*(y-yc));
			stRElem.alpha = atan2((double)(y-yc), (x - xc))* 180/PI;

			if(stRElem.alpha<0)
				stRElem.alpha+=180;
			
			if (bDebugFlag)
			{
				printf("insert rtable[%d] r %d alpha %d\n",nIdx,stRElem.r,stRElem.alpha);
			}
			stRTable_int[nIdx].push_back(stRElem);
		}
	}

EXIT_FUN:
	return nStatus;
}

/************************************************************************/
/* return:
0	OK
-1	input error
-2	check image size equal error
-3	GHT_GetIplPixel error
-4	pDirImg elem invalid(not between 0~360)
*/
/************************************************************************/
int GHT_SearchXY(const IplImage				*pEdgeImg,	//0 for backgrond, non 0 for foreground
				 const IplImage				*pDirImg,	//单位是度
				 int						nStepOfRTable,	//step of RTable,can be 1~180
				 const stRTableType			&stRTable,
				 bool						bDebugFlag,
				 int						*px,
				 int						*py,
				 int						*pnMaxVote)
{
	int nStatus=0;

	if(!pDirImg || !CV_IS_IMAGE(pDirImg) ||
		!pEdgeImg || !CV_IS_IMAGE(pEdgeImg) ||
		 nStepOfRTable<1 || nStepOfRTable>180 ||
		!px || !py || !pnMaxVote)
	{
		assert(0);
		return -1;
	}

	//check size equal
	int nH = pEdgeImg->height;
	int nW = pEdgeImg->width;
	if (nH != pDirImg->height || nW != pDirImg->width)
	{
		assert(0);
		return -2;
	}
	
	//malloc memory for vote tble P[]
	int *P=new int[nH * nW];
	memset(P,0,sizeof(int)*nH * nW);
#define _P(y,x) P[y*nW + x]

	int nSizeOfRTable = 180/nStepOfRTable+1;
	assert(nSizeOfRTable == stRTable.size());

	int tDegVal,tEdgeVal;
	for (int y=0;y<nH;y++)
	{
		for (int x=0;x<nW;x++)
		{
			//check if is edge pt, if not,continue
			tEdgeVal = CV_IMAGE_ELEM(pEdgeImg,unsigned char,y,x);
			

			if (tEdgeVal==0) //back gnd
				continue;
			
			//is edge pt,get direction
			tDegVal = CV_IMAGE_ELEM(pDirImg,unsigned char,y,x);
			//should between 0~180
			if (tDegVal<0 || tDegVal>180)
			{
				nStatus = -4;
				assert(0);
				//goto EXIT_FUN;
			}

			int nIdx = tDegVal/nStepOfRTable;
			assert(nIdx>=0 && nIdx<nSizeOfRTable);
			
			int nVoteNum = stRTable[nIdx].size();
			for (int i=0;i<nVoteNum;i++)
			{
				const stR_Elem &stRElem = stRTable[nIdx][i];
				int xc = x - stRElem.r * cos(stRElem.alpha);
				int yc = y - stRElem.r * sin(stRElem.alpha);
				if(xc>=0 && xc<nW && yc>=0 && yc<nH)
					_P(yc,xc)++;

				xc = x + stRElem.r * cos(stRElem.alpha);
				yc = y + stRElem.r * sin(stRElem.alpha);
				if(xc>=0 && xc<nW && yc>=0 && yc<nH)
					_P(yc,xc)++;
			}
		}
	}
	
	if(bDebugFlag)
	{
		printf("vote table:\n");
		printArray(P,nH,nW,nW);
	}

	//find max item in vote table
	int maxVote =0;
	int max_x=0,max_y=0;

	for (int y=1;y<nH-1;y++)
	{
		for (int x=1;x<nW-1;x++)
		{
			int nCurVote = _P(y,x);
			//printf("nCurVote %d %d %d(max %d)\n",y,x,nCurVote,maxVote);
			if (/*nCurVote>_P(y,x-1) && nCurVote>_P(y-1,x) &&
				nCurVote>_P(y,x+1) && nCurVote>_P(y+1,x) &&*/
				nCurVote>maxVote)
			{
				maxVote = nCurVote;
				max_x = x;
				max_y = y;
			}
		}
	}
	*pnMaxVote = maxVote;
	*px=max_x;
	*py=max_y;

#undef _P

	delete[] P;
	return nStatus;
}


/************************************************************************/
/* 

*/
/************************************************************************/
//int getImgEdgeAndDirection(IplImage		const	*pSrcImg,	//uChar
//							IplImage			*pEdgeImg,	//val:0 & non0,uChar
//							IplImage			*pDirImg //val:0 to 180, uchar
//						   )
//{
//	IplImage *pBinImg = 0;
//
//	CV_FUNCNAME( "getImgEdgeAndDirection" );
//	 __BEGIN__;
//	
//	 if(pSrcImg->depth != IPL_DEPTH_8U ||
//		pEdgeImg->depth != IPL_DEPTH_8U ||
//		pDirImg->depth != IPL_DEPTH_8U )
//		CV_ERROR(CV_StsError,"depth != IPL_DEPTH_8U");
//
//	if (pSrcImg->nChannels!=1||
//		pDirImg->nChannels!=1||
//		pEdgeImg->nChannels!=1)
//		CV_ERROR(CV_StsError,"nChannels!=1");
//	
//	if (pSrcImg->width != pDirImg->width ||
//		pSrcImg->height != pDirImg->height ||
//		pSrcImg->width != pEdgeImg->width ||
//		pSrcImg->height != pEdgeImg->height )
//		CV_ERROR(CV_StsError,"img size not equal");
//
//
//	//cvSobel(pSrcImg,pEdgeImg,1,1,3);
//	pBinImg = cvCloneImage(pSrcImg);
//	int block_size = cvRound(min(pSrcImg->width,pSrcImg->height)*0.2)|1;
//	//cvAdaptiveThreshold( pSrcImg, pBinImg, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, block_size, 0 );
//	cvThreshold(pSrcImg,pBinImg,120,255,CV_THRESH_BINARY);
//	cvCanny(pBinImg,pEdgeImg,0,50,5);
//	//cvSobel(pSrcImg,pEdgeImg,1,1,3);
//	
//	int nH = pSrcImg->height;
//	int nW = pSrcImg->width;
//
//	cvZero(pDirImg);
//
//#define _P(img,y,x) CV_IMAGE_ELEM(img,unsigned char,y,x)
//
//	for (int y=1;y<nH - 1;y++)
//	{
//		for (int x=1; x<nW-1; x++)
//		{
//			if(_P(pEdgeImg,y,x)!=0)
//			{
//				//compute direction
//				int dx = _P(pSrcImg,y,x+1) - _P(pSrcImg,y,x-1);
//				int dy = _P(pSrcImg,y+1,x) - _P(pSrcImg,y-1,x);
//				double dSita = atan2((double)dy,dx);
//				if(dSita<0)
//					dSita += PI;
//
//				_P(pDirImg,y,x) = (unsigned char)(dSita*180/PI);
//			}
//		}
//	}
//
//#undef _P
//
//	 __END__;
//	
//	 cvReleaseImage(&pBinImg);
//
//	 return 0;
//}


int getImgEdgeAndDirection_New(IplImage		const	*pSrcImg,	//uChar
						   IplImage			*pEdgeImg,	//val:0 & non0,uChar
						   IplImage			*pDirImg //val:0 to 180, uchar
						   )
{
	IplImage *pBinImg = 0;

	CV_FUNCNAME( "getImgEdgeAndDirection" );
	__BEGIN__;

	if(pSrcImg->depth != IPL_DEPTH_8U ||
		pEdgeImg->depth != IPL_DEPTH_8U ||
		pDirImg->depth != IPL_DEPTH_8U )
		CV_ERROR(CV_StsError,"depth != IPL_DEPTH_8U");

	if (pSrcImg->nChannels!=1||
		pDirImg->nChannels!=1||
		pEdgeImg->nChannels!=1)
		CV_ERROR(CV_StsError,"nChannels!=1");

	if (pSrcImg->width != pDirImg->width ||
		pSrcImg->height != pDirImg->height ||
		pSrcImg->width != pEdgeImg->width ||
		pSrcImg->height != pEdgeImg->height )
		CV_ERROR(CV_StsError,"img size not equal");


	//cvSobel(pSrcImg,pEdgeImg,1,1,3);
	pBinImg = cvCloneImage(pSrcImg);
	int block_size = cvRound(min(pSrcImg->width,pSrcImg->height)*0.2)|1;
	//cvAdaptiveThreshold( pSrcImg, pBinImg, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, block_size, 0 );
	cvThreshold(pSrcImg,pBinImg,120,255,CV_THRESH_BINARY);
	cvCanny(pBinImg,pEdgeImg,0,50,5);
	//cvSobel(pSrcImg,pEdgeImg,1,1,3);

	int nH = pSrcImg->height;
	int nW = pSrcImg->width;

	cvZero(pDirImg);

#define _P(img,y,x) CV_IMAGE_ELEM(img,unsigned char,y,x)

	const unsigned char * pS ;
	const unsigned char * pS_NextLine ;
	const unsigned char * pS_PrevLine ;
	const unsigned char * pE ;
	unsigned char * pD ;

	for (int y=1;y<nH - 1;y++)
	{
		pS = &_P(pSrcImg,y,1);
		pD = &_P(pDirImg,y,1);
		pS_NextLine = &_P(pSrcImg,y+1,1);
		pS_PrevLine = &_P(pSrcImg,y-1,1);
		pE = &_P(pEdgeImg,y,1);
		for (int x=1; x<nW-1; x++,pS++,pD++,pS_NextLine++,pS_PrevLine++,pE++)
		{
			if(pE[0] !=0)
			{
				//compute direction
				int dx = pS[1] - pS[-1];
				int dy = pS_NextLine[0] - pS_PrevLine[0];
				double dSita = atan2((double)dy,dx);
				if(dSita<0)
					dSita += PI;

				*pD = (unsigned char)(dSita*180/PI);
			}
		}
	}

#undef _P

	__END__;

	cvReleaseImage(&pBinImg);

	return 0;
}

/************************************************************************/
/* return:
0	OK
-1	input error
-2	check image size equal error
-3	GHT_GetIplPixel error
-4	pDirImg elem invalid(not between 0~360)
*/
/************************************************************************/
int GHT_SearchXYRot(const IplImage			*pEdgeImg,	//0 for backgrond, non 0 for foreground
					const IplImage			*pDirImg,	//单位是度
					int						nStepOfRTable,	//step of RTable,can be 1~180
					const stRTableType		&stRTable,
					int						nSitaBegin,//search angle,-180~180
					int						nSitaEnd,//must > nSitaBegin
					int						nSitaStep ,//1~40
					int						*px,
					int						*py,
					int						*pSita,
					int						*pnMaxVote)
{
	int nStatus=0;

	if(!pDirImg || !CV_IS_IMAGE(pDirImg) ||
		!pEdgeImg || !CV_IS_IMAGE(pEdgeImg) ||
		nStepOfRTable<1 || nStepOfRTable>180 ||
		!px || !py || !pnMaxVote)
	{
		assert(0);
		return -1;
	}

	//check size equal
	int nH = pEdgeImg->height;
	int nW = pEdgeImg->width;
	if (nH != pDirImg->height || nW != pDirImg->width)
	{
		assert(0);
		return -2;
	}

	//malloc memory for vote tble P[]
	
	int nSitaSize = (nSitaEnd - nSitaBegin)/nSitaStep +1;
	int *P=new int[nH * nW * nSitaSize];
	memset(P,0,sizeof(int)*nH * nW * nSitaSize);
#define _P(y,x,sita) P[y*(nW* nSitaSize) + x * nSitaSize + sita]
#define _SITA_TO_IDX(sita)  (((sita)-nSitaBegin)/nSitaStep)
#define _IDX_TO_SITA(idx)  ((idx)*nSitaStep + nSitaBegin)
#define _VALID_SITA_IDX(sita_idx) ((sita_idx)>=0 && (sita_idx)<nSitaSize)

	int nSizeOfRTable = 180/nStepOfRTable+1;
	assert(nSizeOfRTable == stRTable.size());

	int tDegVal,tEdgeVal;
	for(int sita=nSitaBegin;sita<=nSitaEnd;sita+=nSitaStep)
	{
		for (int y=0;y<nH;y++)
		{
			for (int x=0;x<nW;x++)
			{
				//check if is edge pt, if not,continue
				tEdgeVal = CV_IMAGE_ELEM(pEdgeImg,unsigned char,y,x);

				if (tEdgeVal==0) //back gnd
					continue;

				//is edge pt,get direction
				tDegVal = CV_IMAGE_ELEM(pDirImg,unsigned char,y,x);
				if (tDegVal<0 || tDegVal>180)
				{
					nStatus = -4;
					assert(0);
					goto EXIT_FUN;
				}

				tDegVal = (tDegVal - sita + 360) % 180;
				if (tDegVal<0 || tDegVal>180)//算法错误
				{
					nStatus = -5;
					assert(0);
					goto EXIT_FUN;
				}

				int nIdx = tDegVal/nStepOfRTable;
				assert(nIdx>=0 && nIdx<nSizeOfRTable);

				int nVoteNum = stRTable[nIdx].size();
				for (int i=0;i<nVoteNum;i++)
				{
					const stR_Elem &stRElem = stRTable[nIdx][i];
					int xc = x - stRElem.r * cos(stRElem.alpha + sita*PI/180);
					int yc = y - stRElem.r * sin(stRElem.alpha + sita*PI/180);
					if(xc>=0 && xc<nW && yc>=0 && yc<nH)
						_P(yc,xc,_SITA_TO_IDX(sita))++;

					xc = x + stRElem.r * cos(stRElem.alpha + sita*PI/180);
					yc = y + stRElem.r * sin(stRElem.alpha + sita*PI/180);
					if(xc>=0 && xc<nW && yc>=0 && yc<nH)
						_P(yc,xc,_SITA_TO_IDX(sita))++;
				}
			}
		}
	}

	//printf("vote table:\n");
	//printArray(P,nH,nW,nW);

	//find max item in vote table
	int maxVote =0;
	int max_x=0,max_y=0,max_sita=0;

	for(int sitaIdx=0;sitaIdx<nSitaSize;sitaIdx++)
	{
		for (int y=1;y<nH-1;y++)
		{
			for (int x=1;x<nW-1;x++)
			{
				int nCurVote = _P(y,x,sitaIdx);
				//printf("nCurVote %d %d %d(max %d)\n",y,x,nCurVote,maxVote);
				if (/*nCurVote>_P(y,x-1,sitaIdx) && nCurVote>_P(y-1,x,sitaIdx) &&
					nCurVote>_P(y,x+1,sitaIdx) && nCurVote>_P(y+1,x,sitaIdx) &&*/
					nCurVote>maxVote)
				{
					maxVote = nCurVote;
					max_x = x;
					max_y = y;
					max_sita = sitaIdx;
				}
			}
		}
	}
	*pnMaxVote = maxVote;
	*px=max_x;
	*py=max_y;
	*pSita=_IDX_TO_SITA(max_sita);
#undef _P
EXIT_FUN:

	delete[] P;
	return nStatus;
}
int GHT_SearchXYRot_int(
						const IplImage			*pEdgeImg,	//0 for backgrond, non 0 for foreground
						const IplImage			*pDirImg,	//单位是度
						int						nStepOfRTable,	//step of RTable,can be 1~180
						const stRTableType_int	&stRTable_int,
						const int				*piSinTbl,
						int						nSitaBegin,//search angle,-180~180
						int						nSitaEnd,//must > nSitaBegin
						int						nSitaStep ,//1~40
						int						*px,
						int						*py,
						int						*pSita,
						int						*pnMaxVote)
{
	int nStatus=0;

	if(!pDirImg || !CV_IS_IMAGE(pDirImg) ||
		!pEdgeImg || !CV_IS_IMAGE(pEdgeImg) ||
		nStepOfRTable<1 || nStepOfRTable>180 ||
		!px || !py || !pnMaxVote)
	{
		assert(0);
		return -1;
	}

	//check size equal
	int nH = pEdgeImg->height;
	int nW = pEdgeImg->width;
	if (nH != pDirImg->height || nW != pDirImg->width)
	{
		assert(0);
		return -2;
	}

	//malloc memory for vote tble P[]

	int nSitaSize = (nSitaEnd - nSitaBegin)/nSitaStep +1;
	int *P=new int[nH * nW * nSitaSize];
	memset(P,0,sizeof(int)*nH * nW * nSitaSize);
#define _P(y,x,sita) P[y*(nW* nSitaSize) + x * nSitaSize + sita]
#define _SITA_TO_IDX(sita)  (((sita)-nSitaBegin)/nSitaStep)
#define _IDX_TO_SITA(idx)  ((idx)*nSitaStep + nSitaBegin)
#define _VALID_SITA_IDX(sita_idx) ((sita_idx)>=0 && (sita_idx)<nSitaSize)

	int nSizeOfRTable = 180/nStepOfRTable+1;
	assert(nSizeOfRTable == stRTable_int.size());

	int tDegVal,tEdgeVal;
	
	for (int y=0;y<nH;y++)
	{
		unsigned char *pE = &CV_IMAGE_ELEM(pEdgeImg,unsigned char,y,0);
		unsigned char *pDeg = &CV_IMAGE_ELEM(pDirImg,unsigned char,y,0);

		for (int x=0;x<nW;x++)
		{
#if 0
			tEdgeVal = *pE++;
#else
			tEdgeVal = CV_IMAGE_ELEM(pEdgeImg,unsigned char,y,x);
#endif

			if (tEdgeVal==0) //back gnd
				continue;
		
			//check if is edge pt, if not,continue
			//is edge pt,get direction
#if 0
			tDegVal = *pDeg++;
#else
			tDegVal = CV_IMAGE_ELEM(pDirImg,unsigned char,y,x);
#endif
			if (tDegVal<0 || tDegVal>180)
			{
				nStatus = -4;
				assert(0);
				goto EXIT_FUN;
			}

			for(int sita=nSitaBegin;sita<=nSitaEnd;sita+=nSitaStep)
			{
				tDegVal = (tDegVal - sita + 360) % 180;
				if (tDegVal<0 || tDegVal>180)//算法错误
				{
					nStatus = -5;
					assert(0);
					goto EXIT_FUN;
				}

				int nIdx = tDegVal/nStepOfRTable;
				assert(nIdx>=0 && nIdx<nSizeOfRTable);

				int nVoteNum = stRTable_int[nIdx].size();
				for (int i=0;i<nVoteNum;i++)
				{
					const stR_Elem_int &stRElem_int = stRTable_int[nIdx][i];
					int sita_int = (stRElem_int.alpha) + sita;
					int dx_int = (stRElem_int.r * GHT_CosFast_int(sita_int,piSinTbl)) >>INT_PRECISION;
					int dy_int = (stRElem_int.r * GHT_SinFast_int(sita_int,piSinTbl)) >>INT_PRECISION;
					
					int xc = x - dx_int;
					int yc = y - dy_int;
					if(xc>=0 && xc<nW && yc>=0 && yc<nH)
						_P(yc,xc,_SITA_TO_IDX(sita))++;

					xc = x + dx_int;
					yc = y + dy_int;
					if(xc>=0 && xc<nW && yc>=0 && yc<nH)
						_P(yc,xc,_SITA_TO_IDX(sita))++;
				}
			}
		}
	}

	//printf("vote table:\n");
	//printArray(P,nH,nW,nW);

	printf("find max item in vote table\n");
	DWORD dwTime = GetTickCount();
	int maxVote =0;
	int max_x=0,max_y=0,max_sita=0;

	for(int sitaIdx=0;sitaIdx<nSitaSize;sitaIdx++)
	{
		for (int y=1;y<nH-1;y++)
		{
			for (int x=1;x<nW-1;x++)
			{
				int nCurVote = _P(y,x,sitaIdx);
				//printf("nCurVote %d %d %d(max %d)\n",y,x,nCurVote,maxVote);
				if (/*nCurVote>_P(y,x-1,sitaIdx) && nCurVote>_P(y-1,x,sitaIdx) &&
					nCurVote>_P(y,x+1,sitaIdx) && nCurVote>_P(y+1,x,sitaIdx) &&*/
					nCurVote>maxVote)
				{
					maxVote = nCurVote;
					max_x = x;
					max_y = y;
					max_sita = sitaIdx;
				}
			}
		}
	}
	*pnMaxVote = maxVote;
	*px=max_x;
	*py=max_y;
	*pSita=_IDX_TO_SITA(max_sita);
	printf("dwTime %d\n",GetTickCount() - dwTime);
#undef _P
EXIT_FUN:

	delete[] P;
	return nStatus;
}
class CRoiSelector
{
	IplImage	const	*m_pImg;
	char				m_sWinText[100];

public :
	CvPoint ptLeftTop,ptRightBottom;
	
	CRoiSelector(IplImage const	*pImg)
	{
		assert(pImg);

		m_pImg = pImg;
		ptLeftTop.x =0;
		ptLeftTop.y =0;

		ptRightBottom.x =pImg->width;
		ptRightBottom.y =pImg->height;

		sprintf(m_sWinText,"CRoiSelector");
	}
	
	void drawRoi(CRoiSelector *pThis)
	{
		//显示鼠标位置
		HWND hWnd = (HWND)cvGetWindowHandle(pThis->m_sWinText);
		if(!hWnd)
			return;
		
		HDC hDC = GetDC(hWnd);
		if(!hDC)
			return;

		if(ptRightBottom.y > ptLeftTop.y && ptRightBottom.x > ptLeftTop.x)
		{
			HPEN pen = CreatePen(PS_SOLID,2,RGB(255,255,255));
			SelectObject(hDC,pen);

			POINT pt;
			MoveToEx(hDC, ptLeftTop.x, ptLeftTop.y, &pt);
			LineTo(hDC, ptLeftTop.x, ptRightBottom.y);
			LineTo(hDC, ptRightBottom.x, ptRightBottom.y);
			LineTo(hDC, ptRightBottom.x, ptLeftTop.y);
			LineTo(hDC, ptLeftTop.x, ptLeftTop.y);
		}

	}
	static void MouseCallbackFunc (int event, int x, int y, int flags, void* param)
	{
		if(!param)
			return;
		
		CRoiSelector *pThis = (CRoiSelector *)param;
		assert(pThis);

		//显示鼠标位置
		HWND hWnd = (HWND)cvGetWindowHandle(pThis->m_sWinText);
		if(!hWnd)
			return;
	
		char s[100];
		sprintf(s,"[%d %d]   ",x,y);
		TextOutA(GetDC(hWnd),0,0,s,strlen(s));

		
		//设置ROI坐标
		if(CV_EVENT_LBUTTONDOWN == event)
		{
			pThis->ptLeftTop.x =x;
			pThis->ptLeftTop.y =y;
			pThis->drawRoi(pThis);
		}
		else if(CV_EVENT_RBUTTONDOWN == event)
		{
			pThis->ptRightBottom.x =x;
			pThis->ptRightBottom.y =y;
			pThis->drawRoi(pThis);
		}
	}

	CvRect getImageROI()
	{
		cvNamedWindow(m_sWinText);
		cvShowImage(m_sWinText, m_pImg);
		cvSetMouseCallback(m_sWinText,MouseCallbackFunc,this);
		cvWaitKey(0);
		if(ptRightBottom.y > ptLeftTop.y &&
			ptRightBottom.x > ptLeftTop.x)
			return cvRect(ptLeftTop.x, ptLeftTop.y,
				ptRightBottom.x - ptLeftTop.x,ptRightBottom.y - ptLeftTop.y);
		else
			return cvRect(0,0,m_pImg->width,m_pImg->height);
	}
};

IplImage * loadImageAndResize(const char *sImgNameSearch,bool bModeGray=true)
{
	IplImage *pSrcImg = cvLoadImage(sImgNameSearch,bModeGray?0:1);
	if(!pSrcImg)
		return 0;

	IplImage *pImg = 0;
	if (pSrcImg->width > 180)
	{
		double scale = pSrcImg->width / 180.0;

		pImg = cvCreateImage(cvSize(pSrcImg->width /scale, pSrcImg->height/scale),8,pSrcImg->nChannels);

		cvResize(pSrcImg,pImg);

		cvReleaseImage(&pSrcImg);
	}
	else
		pImg = pSrcImg;

	return pImg;
}

void Rectangle(IplImage * pImg,
			   CvPoint &pt1,CvPoint &pt2,CvPoint &pt3,CvPoint &pt4,
			   CvScalar color )
{
	int thick = 2;

	cvLine(pImg,pt1,pt2,color,thick);
	cvLine(pImg,pt3,pt2,color,thick);
	cvLine(pImg,pt3,pt4,color,thick);
	cvLine(pImg,pt1,pt4,color,thick);

}
void TestGHT(const char * sImgNameLearn,const char * sImgNameSearch)
{
	stRTableType	stRTable;
	CvSize			szRoi;
	long			totalEdgeNum;

	int nStepOfRTable = 10;
	//--------learn------------
	{
		IplImage *pImg = loadImageAndResize(sImgNameLearn);
		if(!pImg)
			return;
		
		IplImage *pEdgeImg = cvCloneImage(pImg);
		IplImage *pDirImg = cvCloneImage(pImg);
		int block_size = cvRound(min(pImg->width,pImg->height)*0.2)|1;

		//_SHOW_IMG(pImg);


		getImgEdgeAndDirection_New(pImg,pEdgeImg,pDirImg);
		//_SHOW_IMG(pEdgeImg);
		_SHOW_IMG(pDirImg);

		{
			//get Roi
			CRoiSelector stRoiSel(pEdgeImg);
			CvRect rect= stRoiSel.getImageROI();
			printf("you select x %d y %d w %d h %d\n",rect.x, rect.y, rect.width, rect.height);
			
			szRoi.width = rect.width;
			szRoi.height = rect.height;

			cvSetImageROI(pEdgeImg,rect);

			//learn
			printf("***********GHT_CalcRTable..\n");
			if(0!=GHT_CalcRTable(pEdgeImg,pDirImg,nStepOfRTable,false,stRTable))
				assert(0);
			
			totalEdgeNum = printRTable(stRTable);

			//--------self search------------
			int x,y,nMaxVote;
			printf("***********GHT_SearchXY..\n");
			if(0!=GHT_SearchXY(pEdgeImg,pDirImg,nStepOfRTable,stRTable,false,&x,&y,&nMaxVote))
				assert(0);
			printf("searchXY result, x %d y %d nmaxVote %d,分数 %d\n",x,y,nMaxVote,
				nMaxVote * 100/totalEdgeNum);

		}

		//cvWaitKey(0);

		cvReleaseImage(&pImg);
		cvReleaseImage(&pEdgeImg);
		cvReleaseImage(&pDirImg);
	}
	
	//--------search------------
	{
		IplImage *pImg = loadImageAndResize(sImgNameSearch);
		if(!pImg)
			return;
		

		IplImage *pEdgeImg = cvCloneImage(pImg);
		IplImage *pDirImg = cvCloneImage(pImg);
		int block_size = cvRound(min(pImg->width,pImg->height)*0.2)|1;

		_SHOW_IMG(pImg);

		getImgEdgeAndDirection_New(pImg,pEdgeImg,pDirImg);
		_SHOW_IMG(pEdgeImg);
		//_SHOW_IMG(pDirImg);

		{
			//searchXY
			int x,y,nMaxVote;
			printf("***********GHT_SearchXY..\n");
			if(0!=GHT_SearchXY(pEdgeImg,pDirImg,nStepOfRTable,stRTable,false,&x,&y,&nMaxVote))
				assert(0);
			printf("searchXY result, x %d y %d nmaxVote %d 分数 %d\n",x,y,nMaxVote,
				nMaxVote * 100/totalEdgeNum);
			
			//searchXYRot
			int nSita=0;
			DWORD  dwTime = GetTickCount();
			if(0!=GHT_SearchXYRot(pEdgeImg,pDirImg,nStepOfRTable,stRTable,-90,90,2,
				&x,&y,&nSita,&nMaxVote))
				assert(0);
			dwTime = GetTickCount() - dwTime;
			printf("searchXYRot result, x %d y %d sita %d nmaxVote %d 分数 %d,Time %dms\n",x,y,nSita,nMaxVote,
				nMaxVote * 100/totalEdgeNum,dwTime);

			//cvCircle(pImg,cvPoint(x,y),5,CV_RGB(255,255,255),2);
			double recR = sqrt((double)szRoi.width * szRoi.width + szRoi.height * szRoi.height)/2;
			double recSita = atan2((double)szRoi.height,szRoi.width);
			
			CvPoint pt1={x + recR * cos( nSita*PI/180 + recSita),
						y + recR * sin( nSita*PI/180 + recSita)};
			CvPoint pt2={x + recR * cos(nSita*PI/180 + PI -recSita),
				y + recR * sin(nSita*PI/180 + PI -recSita)};
			CvPoint pt4={x + recR * cos(nSita*PI/180 - recSita),
				y + recR * sin( nSita*PI/180 -recSita)};
			CvPoint pt3={x + recR * cos( nSita*PI/180 - PI +recSita),
				y + recR * sin( nSita*PI/180 - PI +recSita)};

			cvReleaseImage(&pImg);
			pImg = loadImageAndResize(sImgNameSearch,false);

			Rectangle(pImg,pt1,pt2,pt3,pt4,CV_RGB(255,0,0));
// 			cvRectangle(pImg,cvPoint(x-szRoi.width/2,y - szRoi.height/2),
// 				cvPoint(x + szRoi.width/2,y + szRoi.height/2),CV_RGB(0,0,0),2);
			_SHOW_IMG(pImg);
		}
		
		cvWaitKey(0);

		cvReleaseImage(&pImg);
		cvReleaseImage(&pEdgeImg);
		cvReleaseImage(&pDirImg);
	}
}
	void TestGHT_int(const char * sImgNameLearn,const char * sImgNameSearch)
	{
		stRTableType_int	stRTable_int;
		CvSize				szRoi;
		long				totalEdgeNum;
		int					astSinTbl[100];
		int					nStepOfRTable = 10;

		GHT_CalcSinTbl_int(astSinTbl);

		//--------learn------------
		{
			IplImage *pImg = loadImageAndResize(sImgNameLearn);
			if(!pImg)
				return;

			IplImage *pEdgeImg = cvCloneImage(pImg);
			IplImage *pDirImg = cvCloneImage(pImg);
			int block_size = cvRound(min(pImg->width,pImg->height)*0.2)|1;

			//_SHOW_IMG(pImg);


			getImgEdgeAndDirection_New(pImg,pEdgeImg,pDirImg);
			//_SHOW_IMG(pEdgeImg);
			_SHOW_IMG(pDirImg);

			{
				//get Roi
				CRoiSelector stRoiSel(pEdgeImg);
				CvRect rect= stRoiSel.getImageROI();
				printf("you select x %d y %d w %d h %d\n",rect.x, rect.y, rect.width, rect.height);

				szRoi.width = rect.width;
				szRoi.height = rect.height;

				cvSetImageROI(pEdgeImg,rect);

				//learn
				printf("***********GHT_CalcRTable..\n");
				if(0!=GHT_CalcRTable_int(pEdgeImg,pDirImg,astSinTbl,nStepOfRTable,false,stRTable_int))
					assert(0);

				totalEdgeNum = printRTable_int(stRTable_int);

			}

			//cvWaitKey(0);

			cvReleaseImage(&pImg);
			cvReleaseImage(&pEdgeImg);
			cvReleaseImage(&pDirImg);
		}

		//--------search------------
		{
			IplImage *pImg = loadImageAndResize(sImgNameSearch);
			if(!pImg)
				return;


			IplImage *pEdgeImg = cvCloneImage(pImg);
			IplImage *pDirImg = cvCloneImage(pImg);
			int block_size = cvRound(min(pImg->width,pImg->height)*0.2)|1;

			_SHOW_IMG(pImg);

			getImgEdgeAndDirection_New(pImg,pEdgeImg,pDirImg);
			_SHOW_IMG(pEdgeImg);
			//_SHOW_IMG(pDirImg);

			{
				//searchXYRot
				int nSita=0,x=0,y=0,nMaxVote=0;
				DWORD  dwTime = GetTickCount();
				if(0!=GHT_SearchXYRot_int(pEdgeImg,pDirImg,nStepOfRTable,stRTable_int,
					astSinTbl,-90,90,2,
					&x,&y,&nSita,&nMaxVote))
					assert(0);

				dwTime = GetTickCount() - dwTime;
				printf("searchXYRot_int result, x %d y %d sita %d nmaxVote %d 分数 %d,Time %dms\n",x,y,nSita,nMaxVote,
					nMaxVote * 100/totalEdgeNum,dwTime);

				//cvCircle(pImg,cvPoint(x,y),5,CV_RGB(255,255,255),2);
				double recR = sqrt((double)szRoi.width * szRoi.width + szRoi.height * szRoi.height)/2;
				double recSita = atan2((double)szRoi.height,szRoi.width);

				CvPoint pt1={x + recR * cos( nSita*PI/180 + recSita),
					y + recR * sin( nSita*PI/180 + recSita)};
				CvPoint pt2={x + recR * cos(nSita*PI/180 + PI -recSita),
					y + recR * sin(nSita*PI/180 + PI -recSita)};
				CvPoint pt4={x + recR * cos(nSita*PI/180 - recSita),
					y + recR * sin( nSita*PI/180 -recSita)};
				CvPoint pt3={x + recR * cos( nSita*PI/180 - PI +recSita),
					y + recR * sin( nSita*PI/180 - PI +recSita)};

				cvReleaseImage(&pImg);
				pImg = loadImageAndResize(sImgNameSearch,false);

				Rectangle(pImg,pt1,pt2,pt3,pt4,CV_RGB(255,0,0));
				// 			cvRectangle(pImg,cvPoint(x-szRoi.width/2,y - szRoi.height/2),
				// 				cvPoint(x + szRoi.width/2,y + szRoi.height/2),CV_RGB(0,0,0),2);
				_SHOW_IMG(pImg);
			}

			cvWaitKey(0);

			cvReleaseImage(&pImg);
			cvReleaseImage(&pEdgeImg);
			cvReleaseImage(&pDirImg);
		}
}