#pragma once

#ifndef _PATTERN_MATCH_TOOL_H_
#define _PATTERN_MATCH_TOOL_H_

#define VISION_TOLERANCE 0.0000001
#define D2R (CV_PI / 180.0)
#define R2D (180.0 / CV_PI)
#define MATCH_CANDIDATE_NUM 5

#define MAX_SCALE_TIMES 10
#define MIN_SCALE_TIMES 0
#define SCALE_RATIO 1.25

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/types_c.h>

using namespace cv;
using namespace std;



struct s_TemplData
{
	vector<Mat> vecPyramid;
	vector<Scalar> vecTemplMean;
	vector<double> vecTemplNorm;
	vector<double> vecInvArea;
	vector<bool> vecResultEqual1;
	bool bIsPatternLearned;
	int iBorderColor;

	void clear()
	{
		vector<Mat>().swap(vecPyramid);
		vector<double>().swap(vecTemplNorm);
		vector<double>().swap(vecInvArea);
		vector<Scalar>().swap(vecTemplMean);
		vector<bool>().swap(vecResultEqual1);
	}

	void resize(int iSize)
	{
		vecTemplMean.resize(iSize);
		vecTemplNorm.resize(iSize, 0);
		vecInvArea.resize(iSize, 1);
		vecResultEqual1.resize(iSize, false);
	}

	s_TemplData()
	{
		bIsPatternLearned = false;
	}
};

struct s_MatchParameter
{
	Point2d pt;
	double dMatchScore;
	double dMatchAngle;

	Rect rectRoi;
	double dAngleStart;
	double dAngleEnd;
	RotatedRect rectR;
	Rect rectBounding;
	bool bDelete;

	double vecResult[3][3];    //for subpixel
	int iMaxScoreIndex;        //for subpixel
	bool bPosOnBorder;
	Point2d ptSubPixel;
	double dNewAngle;

	s_MatchParameter(Point2f ptMinMax, double dScore, double dAngle)//,Mat matRotatedSrc = Mat ())
	{
		pt = ptMinMax;
		dMatchScore = dScore;
		dMatchAngle = dAngle;

		bDelete = false;
		dNewAngle = 0.0;

		bPosOnBorder = false;
	}

	s_MatchParameter()
	{
		double dMatchScore = 0;
		double dMatchAngle = 0;
	}

	~s_MatchParameter()
	{

	}
};

struct s_SingleTargetMatch
{
	Point2d ptLT, ptRT, ptRB, ptLB, ptCenter;
	double dMatchedAngle;
	double dMatchScore;

	s_SingleTargetMatch() {

		ptLT = ptRT = ptRB = ptLB = ptCenter = Point2d(0,0);
		dMatchedAngle = dMatchScore = 0.0;
	}
};

struct s_BlockMax
{
	struct Block
	{
		Rect rect;
		double dMax;
		Point ptMaxLoc;
		Block()
		{}
		Block(Rect rect_, double dMax_, Point ptMaxLoc_)
		{
			rect = rect_;
			dMax = dMax_;
			ptMaxLoc = ptMaxLoc_;
		}
	};
	s_BlockMax()
	{}
	vector<Block> vecBlock;
	Mat matSrc;
	s_BlockMax(Mat matSrc_, Size sizeTemplate)
	{
		matSrc = matSrc_;

		int iBlockW = sizeTemplate.width * 2;
		int iBlockH = sizeTemplate.height * 2;

		int iCol = matSrc.cols / iBlockW;
		bool bHResidue = matSrc.cols % iBlockW != 0;

		int iRow = matSrc.rows / iBlockH;
		bool bVResidue = matSrc.rows % iBlockH != 0;

		if (iCol == 0 || iRow == 0)
		{
			vecBlock.clear();
			return;
		}

		vecBlock.resize(iCol * iRow);
		int iCount = 0;
		for (int y = 0; y < iRow; y++)
		{
			for (int x = 0; x < iCol; x++)
			{
				Rect rectBlock(x * iBlockW, y * iBlockH, iBlockW, iBlockH);
				vecBlock[iCount].rect = rectBlock;
				minMaxLoc(matSrc(rectBlock), 0, &vecBlock[iCount].dMax, 0, &vecBlock[iCount].ptMaxLoc);
				vecBlock[iCount].ptMaxLoc += rectBlock.tl();
				iCount++;
			}
		}
		if (bHResidue && bVResidue)
		{
			Rect rectRight(iCol * iBlockW, 0, matSrc.cols - iCol * iBlockW, matSrc.rows);
			Block blockRight;
			blockRight.rect = rectRight;
			minMaxLoc(matSrc(rectRight), 0, &blockRight.dMax, 0, &blockRight.ptMaxLoc);
			blockRight.ptMaxLoc += rectRight.tl();
			vecBlock.push_back(blockRight);

			Rect rectBottom(0, iRow * iBlockH, iCol * iBlockW, matSrc.rows - iRow * iBlockH);
			Block blockBottom;
			blockBottom.rect = rectBottom;
			minMaxLoc(matSrc(rectBottom), 0, &blockBottom.dMax, 0, &blockBottom.ptMaxLoc);
			blockBottom.ptMaxLoc += rectBottom.tl();
			vecBlock.push_back(blockBottom);
		}
		else if (bHResidue)
		{
			Rect rectRight(iCol * iBlockW, 0, matSrc.cols - iCol * iBlockW, matSrc.rows);
			Block blockRight;
			blockRight.rect = rectRight;
			minMaxLoc(matSrc(rectRight), 0, &blockRight.dMax, 0, &blockRight.ptMaxLoc);
			blockRight.ptMaxLoc += rectRight.tl();
			vecBlock.push_back(blockRight);
		}
		else
		{
			Rect rectBottom(0, iRow * iBlockH, matSrc.cols, matSrc.rows - iRow * iBlockH);
			Block blockBottom;
			blockBottom.rect = rectBottom;
			minMaxLoc(matSrc(rectBottom), 0, &blockBottom.dMax, 0, &blockBottom.ptMaxLoc);
			blockBottom.ptMaxLoc += rectBottom.tl();
			vecBlock.push_back(blockBottom);
		}
	}
	void UpdateMax(Rect rectIgnore)
	{
		if (vecBlock.size() == 0)
			return;

		int iSize = vecBlock.size();
		for (int i = 0; i < iSize; i++)
		{
			Rect rectIntersec = rectIgnore & vecBlock[i].rect;

			if (rectIntersec.width == 0 && rectIntersec.height == 0)
				continue;
			minMaxLoc(matSrc(vecBlock[i].rect), 0, &vecBlock[i].dMax, 0, &vecBlock[i].ptMaxLoc);
			vecBlock[i].ptMaxLoc += vecBlock[i].rect.tl();
		}
	}
	void GetMaxValueLoc(double& dMax, Point& ptMaxLoc)
	{
		int iSize = vecBlock.size();
		if (iSize == 0)
		{
			minMaxLoc(matSrc, 0, &dMax, 0, &ptMaxLoc);
			return;
		}

		int iIndex = 0;
		dMax = vecBlock[0].dMax;
		for (int i = 1; i < iSize; i++)
		{
			if (vecBlock[i].dMax >= dMax)
			{
				iIndex = i;
				dMax = vecBlock[i].dMax;
			}
		}
		ptMaxLoc = vecBlock[iIndex].ptMaxLoc;
	}
};




class C_MatchPattern
{
public:
	C_MatchPattern();
	~C_MatchPattern();

	bool		m_bDebugMode;
	int 		m_iMaxPos;
	double 		m_dMaxOverlap;
	double 		m_dScore;
	double 		m_dToleranceAngle;
	int 		m_iMinReduceArea;

	void		setSrcFile(std::string filename);
	void		setDstFile(std::string filename);

	void		setSrcFile(cv::Mat matSrc);
	void		setDstFile(cv::Mat matDst);

	bool    	LearnPattern();
	bool    	Match(bool bUseSIMD = false);
	bool		getResult(s_SingleTargetMatch& retTargetMatch);
	bool		getResult(vector<s_SingleTargetMatch>& vecRetTargetMatch);

private:
	std::string	m_srcFile;
	std::string m_dstFile;
	bool		m_bUseSubPixel;
	cv::Mat 	m_matSrc;
	cv::Mat 	m_matDst;
	s_TemplData m_TemplData;
	vector<s_SingleTargetMatch> m_vecSingleTargetData;

	bool 		m_bToleranceRange;
	double 		m_dTolerance1;
	double 		m_dTolerance2;
	double 		m_dTolerance3;
	double 		m_dTolerance4;

	bool		LoadSrcFile(const char* filename);
	bool		LoadDstFile(const char* filename);

	int     	GetTopLayer(Mat* matTempl, int iMinDstLength);
	bool    	SubPixEsimation(vector<s_MatchParameter>* vec, double* dNewX, double* dNewY, double* dNewAngle, double dAngleStep, int iMaxScoreIndex);
	void    	OutputRoi(s_SingleTargetMatch sstm);
	void    	MatchTemplate(cv::Mat& matSrc, s_TemplData* pTemplData, cv::Mat& matResult, int iLayer, bool bUseSIMD);
	void    	GetRotatedROI(Mat& matSrc, Size size, Point2f ptLT, double dAngle, Mat& matROI);
	void    	CCOEFF_Denominator(cv::Mat& matSrc, s_TemplData* pTemplData, cv::Mat& matResult, int iLayer);
	Size    	GetBestRotationSize(Size sizeSrc, Size sizeDst, double dRAngle);
	Point2f 	ptRotatePt2f(Point2f ptInput, Point2f ptOrg, double dAngle);
	void    	FilterWithScore(vector<s_MatchParameter>* vec, double dScore);
	void    	FilterWithRotatedRect(vector<s_MatchParameter>* vec, int iMethod, double dMaxOverLap);
	Point   	GetNextMaxLoc(Mat& matResult, Point ptMaxLoc, Size sizeTemplate, double& dMaxValue, double dMaxOverlap);
	Point   	GetNextMaxLoc(Mat& matResult, Point ptMaxLoc, Size sizeTemplate, double& dMaxValue, double dMaxOverlap, s_BlockMax& blockMax);
	void    	SortPtWithCenter(vector<Point2f>& vecSort);

	void 		DrawROI();
	void 		DrawDashLine(Mat& matDraw, Point ptStart, Point ptEnd, Scalar color1 = Scalar(0, 0, 255), Scalar color2 = Scalar::all(255));
	void 		DrawMarkCross(Mat& matDraw, int iX, int iY, int iLength, Scalar color, int iThickness);


};





#endif  // _PATTERN_MATCH_TOOL_H_