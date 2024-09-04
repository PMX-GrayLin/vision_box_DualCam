#pragma once


#include "BaseDataStructureDef.h"
#include "cvip.h"
#include "./ThirdPartyLibrary/PatternMatchTool.h"


typedef struct TagAlignmentResults_ImageCalibration
{
	double dbPixelCount_X;
	double dbPixelCount_Y;

	seCircle circle_01;
	seCircle circle_02;
	seCircle circle_03;

} seImageCalibration_Results, * LPImageCalibration_Results;


typedef struct TagAlignmentResults_DetectCircle
{
	std::vector<int> vec_CntArryOut;
	std::vector<int> vec_1DArrayOut;
	
	seBoundingBox seBoundBox_Circle;

} seDetectCirle_Results, * LPDetectCirle_Results;


typedef struct TagAlignmentResults_CropImage
{
	emBoxShape		emShapeType;
	seBoundingBox	seOffsetCrop_Rect;
	seCircle		seOffsetCrop_Circle;
	seAnnulus		seOffsetCrop_Annulus;

} seCropImage_Results, * LPCropImage_Results;


class CMethod_Alignment : public CCVIPItem
{
public:
	CMethod_Alignment();
	~CMethod_Alignment();

public:
	//int Initiation();
	//int Release();

	//Setp_02:
	// Alingment mthod_Resolution Calculate( Image Calibration )
	int Align_Calibration_SetParameter(LPImageInfo pCalibPatterntIn, seRect roiRect_01, seRect roiRect_02);
	int Align_Calibration_SetParameter(const cv::Mat m_CalibPattern_grayImg, seRect roiRect_01, seRect roiRect_02);
	int Align_Calibration_TestIt();
	int Align_Calibration_GetResult(LPImageCalibration_Results myResultSet);


	// Alignment mthod_Pattern Matach
	int Align_Pattern_SetParameter(seRect roiSearch, const LPImageInfo pTemplatIn);
	int Align_Pattern_TestIt(const LPImageInfo pIn);
	int Align_Pattern_TestIt(const cv::Mat ref_src_gray);
	int Align_Pattern_GetResult(LPBoundingBox pFMarkBox, double* dbScore); //return info of bounding box and score.


	// Alignment mthod_Find Circle
	int Align_FindProfile_SetParameter(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iSeletLineNo, int iKSize);
	int Align_FindProfile_TestIt(const LPImageInfo pIn, int iSelLineNo);
	int Align_FindProfile_TestIt(const cv::Mat srcImg_gray, int iSelLineNo);
	int Align_FindProfile_GetResult(seDetectCirle_Results& myResultSet); //return info of bounding box and 1d array data.


	// Alignment mthod_Detect Circle
	int Align_DetectCircle_SetParameter(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize);
	int Align_DetectCircle_TestIt(const LPImageInfo pIn);
	int Align_DetectCircle_TestIt(const cv::Mat srcImg_gray);
	int Align_DetectCircle_GetResult(seDetectCirle_Results& myResultSet); //return info of bounding box and 1d array data.


	// InspecBoxSetup_Annulus
	int InspectBox_SetParameter(seAnnulus roiAnnuls);
	int InspectBox_SetParameter(seBoundingBox roiRect);
	int InspectBox_SetParameter(seCircle roiCircle);
	int InspectBox_TestIt(const LPImageInfo pIn);
	int InspectBox_TestIt(const cv::Mat matImg_gray);
	int InspectBox_GetResult(LPBoundingBox pInspBox); //retur ROI Image

	//multiple
	//int InspectBox_Annulus_SetParameter(int iCnt, LPAnnulus roiAnnuls);
	//int InspectBox_Rectangle_SetParameter(int iCnt, LPRect roiSearch);
	//int InspectBox_Circle_SetParameter(int icnt, LPCircle roiSearch);

	// calculate the dependency coordinates 
	int CoordBind_Calcu_SetParameter(seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn);
	int CoordBind_Calcu_TestIt();
	int CoordBind_Calcu_GetResult(LPCoordBindBox pCoordBoxInfo);

	// crop image from source image by the dependency coordinates 
	int CoordBind_CropROI_SetParameter(seCoordBindBox seCoordBindBox, seAnnulus roiAnnulus);
	int CoordBind_CropROI_SetParameter(seCoordBindBox seCoordBindBox, seBoundingBox roiRect);
	int CoordBind_CropROI_SetParameter(seCoordBindBox seCoordBindBox, seCircle roiCircle);
	int CoordBind_CropROI_TestIt(const LPImageInfo pIn);
	int CoordBind_CropROI_TestIt(const cv::Mat matImg_Dest);
	int CoordBind_CropROI_GetResult(LPImageInfo pOut, seCropImage_Results& myResultSet);
	int CoordBind_CropROI_GetResult(cv::Mat& matImg_Crop, seCropImage_Results& myResultSet);

	//Step_03
	// Image Processing
	//Step_4
	// Measure Glue Width

private:
	int Algo_PatternMatch(cv::Mat& ref_gray, cv::Mat& tpl_gray, sePoint posShift, seBoundingBox& seBBox);
	int Algo_EdgeDetection_Sobel_X(std::vector<std::vector<int>> vec2d_In, std::vector<std::vector<int>>& vec2d_Out);
	int Algo_EdgeDetection_Sobel_X(int iKernelSize, std::vector<std::vector<int>> vec2d_In, std::vector<std::vector<int>>& vec2d_Out);
	int Algo_EdgeDetection_Sobel_X_New(int iKernelSize, std::vector<std::vector<int>> vec2d_In, std::vector<std::vector<float>>& vec2d_Out);


	int Algo_CalcuAngle(cv::Mat& img_gray, double& dbAngle);
	int Algo_CalcuPixelCnt_TwoPoints(cv::Mat& img_Source, seRect rect_01, seRect rect_02, double& iPixelCount_X, double& iPixelCount_Y);
	int Algo_CalcuPixelCnt_FullImage(cv::Mat& img_Source, double& iPixelCount_X, double& iPixelCount_Y);


private:
		// Aligment method_Image Calibration
		seRect m_rect_01;
		seRect m_rect_02;
		cv::Mat m_CalibPattern_grayImg;
		double m_PixelCount_X;
		double m_PixelCount_Y;
		seCircle m_Circle_01;
		seCircle m_Circle_02;
		seCircle m_Circle_03;


		// Alignment mthod_Pattern Matach
		seRect m_rectSearch; 
		seRect m_rectTemplate; 
		double m_dbScore;
		cv::Mat m_matched_grayImg;
		cv::Mat m_template_grayImg;
		seBoundingBox m_FMarkBox;
		C_MatchPattern m_tpl_matchpattern;	//Third Party Library Tools.
		s_SingleTargetMatch m_retTargetMatch;


		// Alignment mthod_Find Profile / Detect Circle
		double m_dbResizeScale;
		seCircle m_circSearch;
		seCircle m_circMask;
		bool m_bDirection;		//0: Outside to Inside:�ѥ~�V���j�M; 1: Inside to Outside:�Ѥ��V�~�j�M
		bool m_bPolarity;		//0: Rising Edges_�ѷt��G����; 1: Falling Edges_�ѻP�G��t����
		int m_iMinEdgeStrength;	//�̤p����t�j��	����Ų�ȡA�ƭȷU�j��t�¥չ��U����A�i�L�o���n����t
		int m_iSelectLineNo;	//�q Degrees �u�������X��@�u���s��
		int m_stepSize;
		int m_iKSize;
		std::vector<std::vector<int>> m_vec2d_val_intensity;
		std::vector<std::vector<cv::Point>> m_vec2d_val_position;
		std::vector<pairPos> m_vec_List_PairPos;
		seCircle m_Outline_Circle;
		seBoundingBox m_BoundBox_Circle;
		std::vector<sePoint> m_vecPos_Circle;


		// Alignment mthod_Find Profile / Detect Straight Line
		double m_line_dbResizeScale;
		seBoundingBox m_line_Search;
		int m_line_iDirection;		//0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
		bool m_line_bPolarity;		//0: Rising Edges_由暗到亮的邊; 1: Falling Edges_由與亮到暗的邊
		int m_line_iMinEdgeStrength;	//最小的邊緣強度	的門鑑值，數值愈大邊緣黑白對比愈明顯，可過濾不要的邊緣
		int m_line_iStepSize;
		int m_line_iKSize;
		std::vector<sePoint> m_Ret_vecPos_Line;
		sePoint m_Ret_LinePos[2];

		// InspecBoxSetup_Annulus, Circle, Rectangle
		//std::vector<seAnnulus>		m_vecAnnulus;
		//std::vector<seBoundingBox>	m_vecRect;
		//std::vector<seCircle>		m_vecCircle;
		seAnnulus		m_Annulus;
		seBoundingBox	m_Rect;
		seCircle		m_Circle;
		cv::Mat			m_Src_img;
		seBoundingBox	m_BoundBox;
		emBoxShape		m_emBoxShape;


		// calculate the dependency coordinates 
		bool bflg_DeployMode;
		seCoordBindBox	m_CoordInfo_In;
		seCoordBindBox	m_CoordInfo_Out;


		// crop image from source image by the dependency coordinates
		seCoordBindBox	m_crop_CoordInfo;
		seAnnulus		m_crop_Annulus;
		seBoundingBox	m_crop_Rect;
		seCircle		m_crop_Circle;
		emBoxShape		m_emCrop_BoxShape;
		cv::Mat			m_Crop_img;
		cv::Mat			m_Crop_Mask;

		seBoundingBox	m_offsetCrop_Rect;
		seCircle		m_offsetCrop_Circle;
		seAnnulus		m_offsetCrop_Annulus;



private:
	void Align_Image_Calibration()
	{
		m_rect_01 = m_rect_02 = seRect();
		m_PixelCount_X = m_PixelCount_Y = 0.0;
		m_Circle_01 = m_Circle_02 = m_Circle_03 = seCircle();
		if (!m_CalibPattern_grayImg.empty()) m_CalibPattern_grayImg.release();

	}
	void Align_Pattern_Clear()
	{
		m_rectSearch = m_rectTemplate = seRect();
		m_dbScore = 0.0;
		if(!m_matched_grayImg.empty()) m_matched_grayImg.release();
		if(!m_template_grayImg.empty()) m_template_grayImg.release();
		m_FMarkBox = seBoundingBox();
	};
	void Align_FindProfile_Clear()
	{
		m_dbResizeScale = 1.0;
		m_circSearch = m_circMask = seCircle();
		m_bDirection = false;
		m_bPolarity = false;
		m_iMinEdgeStrength = 0;
		m_iSelectLineNo = 0;
		m_stepSize = 0;
		m_iKSize = 0;
		std::for_each(m_vec2d_val_intensity.begin(), m_vec2d_val_intensity.end(), [](std::vector<int>& v)
			{
				std::fill(v.begin(), v.end(), 0);
			});
		std::for_each(m_vec2d_val_position.begin(), m_vec2d_val_position.end(), [](std::vector<cv::Point>& v)
			{
				std::fill(v.begin(), v.end(), cv::Point(0,0));
			});
		if (!m_vec_List_PairPos.empty()) { m_vec_List_PairPos.clear(); }

		m_Outline_Circle = seCircle();
		m_BoundBox_Circle = seBoundingBox();
	};
	void Align_DetectCircle_Clear()
	{
		m_dbResizeScale = 1.0;
		m_circSearch = m_circMask = seCircle();
		m_bDirection = false;
		m_bPolarity = false;
		m_iMinEdgeStrength = 0;
		m_iSelectLineNo = 0;
		m_stepSize = 0;
		m_iKSize = 0;
		std::for_each(m_vec2d_val_intensity.begin(), m_vec2d_val_intensity.end(), [](std::vector<int>& v)
			{
				std::fill(v.begin(), v.end(), 0);
			});
		m_Outline_Circle = seCircle();
		m_BoundBox_Circle = seBoundingBox();
		if (!m_vecPos_Circle.empty()) { m_vecPos_Circle.clear(); }

	};
	void Align_StraightLine_Clear()
	{
		m_line_dbResizeScale = 1.0;
		m_line_Search = seBoundingBox();
		m_line_iDirection = 0;
		m_line_bPolarity = false;
		m_line_iMinEdgeStrength = 0;
		m_line_iStepSize = 0;
		m_line_iKSize = 0;
		if (!m_Ret_vecPos_Line.empty()) m_Ret_vecPos_Line.clear();
		m_Ret_LinePos[0] = sePoint();
		m_Ret_LinePos[1] = sePoint();

	};
	void InspectBox_Clear()
	{
		m_Annulus = seAnnulus();
		m_Rect = seBoundingBox();
		m_Circle = seCircle();
		if (!m_Src_img.empty()) m_Src_img.release();
		m_BoundBox = seBoundingBox();
		m_emBoxShape = emBoxShape::SHAPE_NODEF;
	};
	void CoordBind_Clear()
	{
		bflg_DeployMode = 0;
		m_CoordInfo_In = seCoordBindBox();
		m_CoordInfo_Out = seCoordBindBox();
	}
	void CoordBind_CropROI_Clear()
	{
		m_crop_CoordInfo = seCoordBindBox();
		m_crop_Annulus = seAnnulus();
		m_crop_Rect = seBoundingBox();
		m_crop_Circle = seCircle();
		m_offsetCrop_Annulus= seAnnulus();
		m_offsetCrop_Rect	= seBoundingBox();
		m_offsetCrop_Circle	= seCircle();
		m_emCrop_BoxShape = emBoxShape::SHAPE_NODEF;
		if (!m_Crop_img.empty()) m_Crop_img.release();
		if (!m_Crop_Mask.empty()) m_Crop_Mask.release();
	}
};

