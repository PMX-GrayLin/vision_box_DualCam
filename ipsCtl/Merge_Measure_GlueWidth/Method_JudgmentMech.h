#pragma once


#include "BaseDataStructureDef.h"
#include "cvip.h"



class CMethod_JudgmentMech : public CCVIPItem
{

public:
	CMethod_JudgmentMech();
	~CMethod_JudgmentMech();


public:
	int Judg_IsOverRange(cv::Mat imgM, cv::Mat imgS);
	int Judg_IsOverRange(cv::Rect rectM, cv::Rect rectS);
	int Judg_IsOverRange(seRect rectM, seRect rectS);

	int Cvt_Circle2Rect(seCircle circleM, seRect& rectM);
	int Cvt_Annulus2Rect(seAnnulus annulusM, seRect& rectM);



};
