#pragma once


#include "BaseDataStructureDef.h"
#include "cvip.h"


typedef struct TagMeasureResults_GlueWidth
{
	std::vector<double> Length_Inner;
	std::vector<double> Length_Outer;
	std::vector<sePoint> Position_Inner;
	std::vector<sePoint> Position_Outer;
	double GlueArea;
	double GlueRatio;

} seGlueWidth_Results, * LPGlueWidth_Results;



class CMethod_Measure : public CCVIPItem
{
	typedef std::pair< cv::Point, cv::Point > pairPos;

public:
	CMethod_Measure();
	~CMethod_Measure();

public:
	//Setp_02:
	// Alignment 

	//Step_03
	// Image Processing

	//Step_4
	// Measure Glue Width
	int MeasGlueWidth_SetParameter(seAnnulus roiAnnuls, int stepSize /*degrees*/);
	int MeasGlueWidth_SetParameter(seBoundingBox roiRect, int stepSize /*degrees*/);
	int MeasGlueWidth_TestIt(const LPImageInfo pIn);
	int MeasGlueWidth_TestIt(const cv::Mat mSrc_Img);
	int MeasGlueWidth_GetResult(seGlueWidth_Results& myResultSet);

	int MeasGlueWidth_GetRetImg(const char* strSavePath);

	int MeasGlueWidth_GetInfo4Dump(std::vector<std::string>& vecInof_forDump);
	int MeasGlueWidth_SetInfo4Dump( std::string strInfo4Dump);


private:
	emShapeTypes m_emBoxShape;
	seAnnulus m_roiAnnuls;
	seBoundingBox m_roiRect;


	int m_StepSize;
	int m_Direction;	//// 0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
	cv::Mat matSrcImg;
	cv::Mat matRetImg;

	std::vector<pairPos> m_vecPos_BoxShape_CenterToOuter;

	std::vector<pairPos> m_vecPos_Annulus_Inner;
	std::vector<pairPos> m_vecPos_Annulus_Outer;

	std::vector<sePoint> m_vecPos_GlueWidth_Inner;
	std::vector<sePoint> m_vecPos_GlueWidth_Outer;

	std::vector<double> m_vecVal_GlueWidth_Inner;
	std::vector<double> m_vecVal_GlueWidth_Outer;


	std::vector< std::string> m_Measure_vecInof_forDump;

	double m_dbArea_GlueWidth;
	double m_dbRatio_GlueWidth;

private:
	int Algo_GlueWidth_RangeSetting(seAnnulus roiAnnuls, double dbStepSize, std::vector<pairPos>& vec_Res_PairPos);
	int Algo_GlueWidth_RangeSetting(seBoundingBox roiRect, double dbStepSize, std::vector<pairPos>& vec_List_PairPos);


	int Algo_GlueWidth_MeasureLength(const cv::Mat& mat_objImg, std::vector<pairPos> vec_List_PairPos, std::vector<int>& vec_Res_GlueWidth);
	int Algo_GlueWidth_MeasureLength_GetPosistion(const cv::Mat& mat_objImg, std::vector<pairPos> vec_List_PairPos, std::vector< pairPos>& vec_Pos_GlueWidth_Inner, std::vector< pairPos>& vec_Pos_GlueWidth_Outer);
	int Algo_GlueWidth_MeasureLength_Calculate(std::vector< pairPos> vec_Pos_GlueWidth, std::vector<double>& vec_ResValue_GlueWidth, std::vector<sePoint>& vec_ResPos_GlueWidth);
	int Algo_GlueWidth_MeasureArea(const cv::Mat& mat_objImg, double& dbArea_GlueWidth);


private:
	void MeasGlueWidth_Clear()
	{
		m_emBoxShape = emBoxShape::SHAPE_NODEF;

		m_roiAnnuls = seAnnulus();
		m_roiRect = seBoundingBox();
		m_StepSize = 0;

		if (matSrcImg.empty()) { matSrcImg.release(); }
		if (matRetImg.empty()) { matRetImg.release(); }

		if (!m_vecPos_BoxShape_CenterToOuter.empty()) { m_vecPos_BoxShape_CenterToOuter.clear(); }

		if (!m_vecPos_Annulus_Inner.empty()) { m_vecPos_Annulus_Inner.clear(); }
		if (!m_vecPos_Annulus_Outer.empty()) { m_vecPos_Annulus_Outer.clear(); }

		if (!m_vecPos_GlueWidth_Inner.empty()) { m_vecPos_GlueWidth_Inner.clear(); }
		if (!m_vecPos_GlueWidth_Outer.empty()) { m_vecPos_GlueWidth_Outer.clear(); }
		
		if (!m_vecVal_GlueWidth_Inner.empty()) { m_vecVal_GlueWidth_Inner.clear(); }
		if (!m_vecVal_GlueWidth_Outer.empty()) { m_vecVal_GlueWidth_Outer.clear(); }

		if (!m_Measure_vecInof_forDump.empty()) { m_Measure_vecInof_forDump.clear(); }

		m_dbArea_GlueWidth = 0.0;
		m_dbRatio_GlueWidth = 0.0;
	};



};

