#include <opencv2/opencv.hpp>
#include "Method_JudgmentMech.h"


using namespace std;
using namespace cv;

CMethod_JudgmentMech::CMethod_JudgmentMech()
{}

CMethod_JudgmentMech::~CMethod_JudgmentMech()
{}


int CMethod_JudgmentMech::Judg_IsOverRange(cv::Mat imgM, cv::Mat imgS)
{
	int res = 0;

	if (imgM.empty() || imgS.empty()) {
		return -1;
	}

	cv::Size dbM = imgM.size();
	cv::Size dbS = imgS.size();

	if ((dbM.width < dbS.width) || (dbM.height < dbS.height)) {
		return -1;
	}

	return res;
}


int CMethod_JudgmentMech::Judg_IsOverRange(cv::Rect rectM, cv::Rect rectS)
{
	int res = 0;

	if (rectM.empty() || rectS.empty()) {
		return -1;
	}

	cv::Size dbM = rectM.size();
	cv::Size dbS = rectS.size();

	if ((dbM.width < dbS.width) || (dbM.height < dbS.height)) {
		return -1;
	}

	return res;
}


int CMethod_JudgmentMech::Judg_IsOverRange(seRect rectM, seRect rectS)
{
	int res = 0;

	cv::Rect tmpRectM, tmpRectS;

	tmpRectM.x		= rectM.left;
	tmpRectM.y		= rectM.top;
	tmpRectM.width	= rectM.width;
	tmpRectM.height = rectM.height;

	tmpRectS.x		= rectS.left;
	tmpRectS.y		= rectS.top;
	tmpRectS.width	= rectS.width;
	tmpRectS.height	= rectS.height;

	res = Judg_IsOverRange(tmpRectM, tmpRectS);

	return res;
}



int CMethod_JudgmentMech::Cvt_Circle2Rect(seCircle circleM, seRect& rectM)
{
	int res = 0;

	seRect rectTmp;

	rectTmp.left	= circleM.cX - circleM.dbRadius;
	rectTmp.top		= circleM.cY - circleM.dbRadius;
	rectTmp.right	= circleM.cX + circleM.dbRadius;
	rectTmp.bottom	= circleM.cY + circleM.dbRadius;
	rectTmp.width	= abs(rectTmp.right - rectTmp.left);
	rectTmp.height	= abs(rectTmp.bottom - rectTmp.top);

	rectM = rectTmp;

	return res;
}


int CMethod_JudgmentMech::Cvt_Annulus2Rect(seAnnulus annulusM, seRect& rectM)
{
	int res = 0;

	seRect rectTmp;

	rectTmp.left	= annulusM.cX - annulusM.dbRadius_Outer;
	rectTmp.top		= annulusM.cY - annulusM.dbRadius_Outer;
	rectTmp.right	= annulusM.cX + annulusM.dbRadius_Outer;
	rectTmp.bottom	= annulusM.cY + annulusM.dbRadius_Outer;
	rectTmp.width	= abs(rectTmp.right - rectTmp.left);
	rectTmp.height	= abs(rectTmp.bottom - rectTmp.top);

	rectM = rectTmp;


	return res;
}