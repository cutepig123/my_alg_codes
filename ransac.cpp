//#include "cv.h"
//#include "highgui.h"
#include "stdafx.h"
#include <Windows.h>
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
#include <float.h>

#define DBG_PRINT printf
/*
做过直线拟合测试！
*/
namespace RANSAC
{
//input:
//	data - a set of observations
//		model - a model that can be fitted to data 
//		n - the minimum number of data required to fit the model
//		k - the maximum number of iterations allowed in the algorithm
//		t - a threshold value for determining when a datum fits a model
//		d - the number of close data values required to assert that a model fits well to data
//output:
//	best_model - model parameters which best fit the data (or nil if no good model is found)
//		best_consensus_set - data point from which this model has been estimated
//		best_error - the error of this model relative to the data 

	typedef int (*fit_model)(const void * pdata,size_t data_num,void * pModel, double *pError);
	typedef double (*distance_to_model)(const void * pdata,size_t data_num,const void * pModel);

#define OK						0
#define ERR_THRES_TOO_SMALL		-1
#define ERR_PTS_TOO_SMALL		-2

template <typename ModelType>
	int Ransac(const void * pdata,size_t elem_size,size_t data_num,	
		size_t min_num_req_to_fit,size_t max_iter,double thres,size_t min_num_req_to_assert,
		int (*fit_model)(const void * pdata,size_t data_num,void * pModel, double *pError),
		double (*distance_to_model)(const void * pdata,size_t data_num,const void * pModel),	//input
		ModelType * pModel, double *pError,
		void * pbest_consensus_set,size_t *pdata_num					//output
		)
	{
		assert(min_num_req_to_fit<=data_num);

		size_t i,iter_num;
		int wStatus = OK;
		*pError = -1;
		int finded = 0;
		void *pTmpData = malloc(elem_size*data_num);
#define COPY_DATA(i,j)  memcpy(pTmpData+j*elem_size, pdata+i*elem_size,elem_size)

		srand(GetTickCount());

		for(iter_num=0;iter_num<max_iter;iter_num++)
		{
			//产生随即的输入数据
			for (i=0;i<min_num_req_to_fit;i++)
			{
				size_t from = rand()%data_num;
				memcpy((char*)pTmpData+i*elem_size, (char*)pdata+from*elem_size,elem_size);
			}

			//计算模型参数
			double error;
			ModelType currModel;

			wStatus = fit_model(pTmpData,min_num_req_to_fit,&currModel,&error);
			if(wStatus!=OK)
			{
				assert(0);
				goto exit_fn;
			}
			
			/*if(error>thres)
				continue;*/
			
			if(*pError>=0 && error>*pError)
				continue;

			//统计是否有足够多的点在误差内
			size_t num_of_consensus_set = 0;
			for(i=0;i<data_num;i++)
			{
				if(distance_to_model((char*)pdata+i*elem_size,1,&currModel)<thres)
				{
					//DBG_PRINT("%d inlier\n",i);
					num_of_consensus_set++;
				}
			}
			if(num_of_consensus_set>=min_num_req_to_assert)
			{
				finded =1;
				*pError = error;
				*pModel = currModel;
			}
		}

		if(!finded)
			wStatus = ERR_THRES_TOO_SMALL;
exit_fn:
		
		free(pTmpData);
		return wStatus;
	}

	struct line_model
	{
		double a,b;	//y=ax+b
	};
	struct Pt
	{
		double x,y;
	};
	double distance_fn(
		const void * pdata,size_t data_num,const void * pModel	//input
		)
	{
		line_model *pline_model = (line_model*)pModel;
		Pt *pt = (Pt*)pdata;
		double a=pline_model->a;
		double b=pline_model->b;

		double error = -1;
		for (size_t i=0;i<data_num;i++)
		{
			double x=pt[i].x;
			double y=pt[i].y;
			double dy = fabs(a*x+b-y);
			if(error<0 || error<dy)
				error=dy;
		}
		//error = sqrt(error)/data_num;

		return error;
	}
	int fit_line_fn(const void * pdata,size_t data_num,	//input
		void * pModel, double *pError								//output
		)
	{
		line_model *pline_model = (line_model*)pModel;
		Pt *pt = (Pt*)pdata;

		double sig_x2=0,sig_y=0,sig_x=0,sig_xy=0;
		for (size_t i=0;i<data_num;i++)
		{
			double x=pt[i].x;
			double y=pt[i].y;
			sig_x += x;
			sig_x2 += x*x;
			sig_xy += x*y;
			sig_y +=y;
		}
		double a = (sig_xy*data_num - sig_x * sig_y)/(sig_x2 *data_num - sig_x*sig_x);
		double b = (sig_y - a * sig_x)/data_num;
		pline_model->a = a;
		pline_model->b = b;

		double error = distance_fn(pdata, data_num,pModel);
		*pError = error;

		return OK;
	}

	void test()
	{
		//产生数据
		double a=10.1,b=20.2;
		size_t n=1000;
		Pt *pt_array=new Pt[n];
		
		srand(GetTickCount());
		for (size_t i=0;i<n;i++)
		{
			pt_array[i].x=rand();
			pt_array[i].y=pt_array[i].x * a + b + (rand()%100-50.0)/1000.0;
			if(rand()%4==0)
				pt_array[i].y =  rand();
		}

		//调用ransac算法计算直线参数
		size_t min_num_req_to_fit = 5;
		size_t max_iter = 50;
		double thres = 0.1;
		size_t min_num_req_to_assert = n*5/7;
	
		line_model Model;
		double Error;
			
		int status =
		Ransac(pt_array,sizeof(Pt),n,	
			min_num_req_to_fit,max_iter,thres,min_num_req_to_assert,
			fit_line_fn,distance_fn,	//input
			&Model, &Error,
			0,0		);

		printf("直线参数 a %g b %g 产生的点的数目 %d\n",a,b,n) ;
		printf("ransac算法拟合结果a %g b %g status %d Error %g\n",Model.a,Model.b,status,Error);

		delete []pt_array;
	}
}