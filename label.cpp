#include "stdafx.h"
/************************************************************************/
/* 
blob analysis
对于与某些图像，会出现莫名其妙的问题
*/
/************************************************************************/
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
#include "image.h"

using namespace std;

namespace LabelImage
{
	int getLabelRoot(int *plblTbl,int lblId)
	{
		assert(lblId>0);
		//int cuId=lblId;
		int cnt=0;
		while(plblTbl[lblId]!=lblId)
		{
			//cuId = lblId;
			lblId = plblTbl[lblId];
			assert(lblId>0);
			if(cnt++>200)
				assert(0);
		}

		return lblId;
	}

	void drawLbl(IplImage *src,int	*lblBuf,int lblNum)
	{
		CvPoint *pt = new CvPoint[lblNum * 2];
		memset(pt,0,lblNum * 2 * sizeof(CvPoint));

		int x,y;
		for (y=0;y<src->height;y++)
		{
			for (x=0;x<src->width;x++)
			{
				char c =lblBuf[y* src->width + x];
				assert(c>=0 && c<lblNum);
				if(c==0)
					continue;

				CvPoint *pt1 = &pt[2*c];
				CvPoint *pt2 = pt1+1;

				if (pt1->x==0 || pt1->x>x)
					pt1->x=x;
				if (pt1->y==0 || pt1->y>y)
					pt1->y=y;

				if (pt2->x==0 || pt2->x<x)
					pt2->x=x;
				if (pt2->y==0 || pt2->y<y)
					pt2->y=y;
			}
		}
		CvScalar ColorArray[] = {CV_RGB(255,0,0),CV_RGB(255,255,0),CV_RGB(0,255,0),CV_RGB(0,0,255)};
		int nNumColor = sizeof(ColorArray)/sizeof(ColorArray[0]);
		for (int i=1;i<lblNum;i++)
		{
			CvPoint *pt1 = &pt[2*i];
			CvPoint *pt2 = pt1+1;
			//cvLine(src,*pt1,*pt2,CV_RGB(255,0,0),2);
			cvRectangle(src,*pt1,*pt2,ColorArray[i % nNumColor],1);
			printf("lblId %d, pos %d %d %d %d\n",i,pt1->x,pt1->y,pt2->x,pt2->y);
		}

		delete []pt;
	}
	/************************************************************************/
	/* 
	input: binary image, 0 is bkgnd, non 0 is foreground
	*/
	/************************************************************************/
	int   label(char * srcImage,int lineStep,int width,int height,
		int *dstlabelImage)
	{
		assert(srcImage && lineStep>0 && width>1 &&height>1 );

		int i,x,y,labelNum = 1;
		int *plblTbl = new int[width * height];
		memset(plblTbl,0,width*height*sizeof(int));
		memset(dstlabelImage,0,lineStep*height*sizeof(int));

		//1st line
		char *pS = srcImage;
		int  *pDLbl = dstlabelImage;

		if(pS[0]!=0)
		{
			plblTbl[labelNum]=labelNum;
			*pDLbl = labelNum;
			labelNum ++;
		}
		pS++; pDLbl++;

		for(x=1;x<width;x++,pS++,pDLbl++)
		{
			if (*pS)
			{
				if(pDLbl[-1])
					*pDLbl = pDLbl[-1];
				else //new label
				{
					*pDLbl = labelNum;
					plblTbl[labelNum]=labelNum;
					labelNum++;
				}
			}
		}

		//other line
		for (y=1;y<height;y++)
		{
			pS = srcImage + lineStep * y;
			pDLbl = dstlabelImage + width * y;
			//x=0
			if(*pS)
			{
				if(pDLbl[-width] )//top
				{
					assert(pDLbl[-width]>0);
					
					int topRoot = getLabelRoot(plblTbl,pDLbl[-width]);
					if(pDLbl[-width+1] )//top right
					{
						int trRoot = getLabelRoot(plblTbl,pDLbl[-width+1]);
						assert(topRoot == trRoot);
					}

					*pDLbl = pDLbl[-width];
				}
				else if(pDLbl[-width+1] )//top right
				{
					*pDLbl = pDLbl[-width+1];
				}
				else //new label
				{
					*pDLbl = labelNum;
					plblTbl[labelNum]=labelNum;
					labelNum++;
				}
			}
			pS++;
			pDLbl++;
			for (x=1;x<width-1;x++,pS++,pDLbl++)
			{
				if(!*pS)
					continue;
				
				if(pDLbl[-1] )//left
				{
					assert(pDLbl[-1]>0);
					
					int leftRoot=getLabelRoot(plblTbl,pDLbl[-1]);
					if(pDLbl[-width])//top
					{
						int toproot = getLabelRoot(plblTbl,pDLbl[-width]);
						assert(toproot == leftRoot);
					}
					if(pDLbl[-width-1])//top left
					{
						int tlRoot=getLabelRoot(plblTbl,pDLbl[-width-1]);
						assert( tlRoot==leftRoot );
					}
					
					if(pDLbl[-width+1] && pDLbl[-width+1]!=pDLbl[-1])//top right
					{
						int lbl_tr = pDLbl[-width+1];
						int lbl_l = pDLbl[-1];
						assert(lbl_tr>0);
						if (lbl_tr > lbl_l)
						{
							plblTbl[lbl_tr] = getLabelRoot(plblTbl, lbl_l);
							*pDLbl = plblTbl[lbl_tr];
						}
						else
						{
							plblTbl[lbl_l] = getLabelRoot(plblTbl, lbl_tr);
							*pDLbl = plblTbl[lbl_l];
						}
					}
					else
						*pDLbl = pDLbl[-1];
				}
				else if(pDLbl[-width-1] )//top left
				{
					assert(pDLbl[-width-1]>0);
					
					int tlRoot = getLabelRoot(plblTbl,pDLbl[-width-1]);
					if(pDLbl[-width])//top
					{
						int topRoot = getLabelRoot(plblTbl,pDLbl[-width]);
						assert(topRoot == tlRoot);
					}
					
					if(pDLbl[-width+1] && pDLbl[-width+1]!=pDLbl[-width-1])//top right
					{
						int lbl_tr = pDLbl[-width+1];
						int lbl_l = pDLbl[-width-1];
						assert(lbl_tr>0);
						if (lbl_tr > lbl_l)
						{
							plblTbl[lbl_tr] = getLabelRoot(plblTbl, lbl_l);
							*pDLbl = plblTbl[lbl_tr];
						}
						else
						{
							plblTbl[lbl_l] = getLabelRoot(plblTbl, lbl_tr);
							*pDLbl = plblTbl[lbl_l];
						}
					}
					else
						*pDLbl = pDLbl[-width-1];
				}
				else if(pDLbl[-width] )//top
				{
					int topRoot=getLabelRoot(plblTbl,pDLbl[-width]);
					if(pDLbl[-width+1])//top right
					{
						int trRoot =  getLabelRoot(plblTbl,pDLbl[-width+1]);
						assert( topRoot==trRoot);
					}

					*pDLbl = pDLbl[-width];
				}
				else if(pDLbl[-width+1] )//top right
				{
					*pDLbl = pDLbl[-width+1];
				}
				else //new label
				{
					*pDLbl = labelNum;
					plblTbl[labelNum]=labelNum;
					labelNum++;
				}
			}
		}

		printf("******oldlblNum %d\n",labelNum-1);
		//处理label
		for (i=1;i<labelNum;i++)
		{
			plblTbl[i] = getLabelRoot(plblTbl,i);
			assert(plblTbl[i]>0);
			//printf("lblId %d parent %d\n",i,plblTbl[i]);
		}
		
		
		int NewlblNum = 1;
		for (i=1;i<labelNum;i++)
		{
			if(i != plblTbl[i] )
			{
				assert(plblTbl[i]>0);
				plblTbl[i] = plblTbl[plblTbl[i]];
				assert(plblTbl[i]>0);
			}
			else
			{
				plblTbl[i]=NewlblNum;
				NewlblNum++;
			}
			printf("lblId %d parent %d\n",i,plblTbl[i]);
		}
		printf("*****NewlblNum %d\n",NewlblNum-1);
		
		for (y=0;y<height-1;y++)
		{
			pDLbl = dstlabelImage + width * y;
			for (x=0;x<width-1;x++,pDLbl++)
			{
				if(*pDLbl)
				{
					assert(*pDLbl>0);
					//*pD = getLabelRoot(plblTbl,*pD);
					*pDLbl = plblTbl[*pDLbl];
					assert(*pDLbl>0);
				}
			}
		}

		
		delete []plblTbl;

		return NewlblNum;
	}

	//黑白变白黑
	void Invert(IplImage * image)
	{
		
		for (int y=0;y<image->height;y++)
		{
			char *p = &CV_IMAGE_ELEM(image,char,y,0);

			for (int x=0;x<image->width;x++,p++)
			{
				*p = (*p)?0:255;
			}
		}
	}
	void Test(char * srcImgName,bool inv)
	{
		IplImage * pSrcImg = cvLoadImage(srcImgName,0);
		cvAdaptiveThreshold(pSrcImg,pSrcImg,255);
		if(inv)
			Invert(pSrcImg);

		cvNamedWindow("SRC");
		cvShowImage("SRC",pSrcImg);
		
		//IplImage *pDstImg = cvCloneImage(pSrcImg);
		//cvZero(pDstImg);
		int *labelBuf = new int[pSrcImg->width * pSrcImg->height];
		memset(labelBuf,0,pSrcImg->width * pSrcImg->height * sizeof(int));
		
		int n =label(pSrcImg->imageData,pSrcImg->widthStep,pSrcImg->width,pSrcImg->height,
			labelBuf);
		
		IplImage * pSrcImgColor = cvLoadImage(srcImgName,1);
		drawLbl(pSrcImgColor,labelBuf,n);
		cvNamedWindow("SRC2",0);
		cvShowImage("SRC2",pSrcImgColor);

		cvWaitKey(0);
		cvReleaseImage(&pSrcImg);
		cvReleaseImage(&pSrcImgColor);
		delete []labelBuf;
	}
}