#pragma once

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <string>

#include "BaseDataStructureDef.h"
#include "Measure_GlueWidth_InterfaceClass.h"
#include "IPLAlgoDataStructureDef.h"

#include "Method_Alignment.h"
#include "Method_ImgProcessing.h"
#include "Method_Measure.h"
#include "Method_JudgmentMech.h"
#include <opencv2/opencv.hpp>
// #include "../../aisCtl/MLDL/Method_ELIC.h"


#if defined(_MSC_VER)

	#include "spdlog\spdlog.h"
	#include "spdlog\sinks\daily_file_sink.h"
	#include "spdlog\sinks\basic_file_sink.h"
	#include "spdlog\fmt\bin_to_hex.h"
	#include "spdlog\async.h"
	#include "spdlog\spdlog.h"
	#include "spdlog\logger.h"

#endif	//#if defined(_MSC_VER)



//Concrete class
class PTWDLL_C_GlueWidth : public PTWDLL_I_GlueWidth
{
public:
	PTWDLL_C_GlueWidth();
	~PTWDLL_C_GlueWidth();


public:
	//Algorithm list for VisionBox 
	//int vbs_Align_CropTemplate(const char* strSrcImg, seRect roiSearch, seExpandable seExpandable) override;
	int vbs_Align_CropTemplate(void* pSrcImg, seRect roiSearch, seExpandable* seExpandable) override;
	
	//int vbs_Align_ImageCalibration(const char* strSrcImg, seRect roiRect_01, seRect roiRect_02, const char* strSavePath, double* pPixelCount_X, double* pPixelCount_Y) override;
	int vbs_Align_ImageCalibration(void* pSrcImg, seRect roiRect_01, seRect roiRect_02, const char* strSavePath, double* pPixelCount_X, double* pPixelCount_Y) override;

	//int vbs_Align_PatternMatch(const char* strSrcImg, const char* strTemplateImg, seRect roiSearch, const char* strSavePath, LPBoundingBox pFMarkBoxOut, double* pdbScoreOut) override;
	int vbs_Align_PatternMatch(void* pSrcImg, const char* strTemplateImg, seRect roiSearch, const char* strSavePath, LPBoundingBox pFMarkBoxOut, double* pdbScoreOut) override;

	//int vbs_Align_FindProfile(const char* strSrcImg, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int iSelLineNo, const char* strSavePath, int* pCntArryOut, int* p1DArrayOut) override;
	int vbs_Align_FindProfile(void* pSrcImg, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int iSelLineNo, const char* strSavePath, int* pCntArryOut, int* p1DArrayOut) override;

	//int vbs_Align_DetectCircle(const char* strSrcImg,
	int vbs_Align_DetectCircle(void* pSrcImg,
		seCircle roiSearch,
		seCircle roiMask,
		bool bDirection,
		bool bPolarity,
		int iMinEdgeStrength,
		const char* strSavePath,
		LPBoundingBox pFMarkBox_Circle,
		int* pCntArryOut,
		sePoint* pPoinitArrayOut) override;
	


	//int vbs_InspectBox_Annulus(const char* strSrcImg, seAnnulus roiAnnulus, const char* strSavePath, LPBoundingBox pBoundBoxOut) override;
	int vbs_InspectBox_Annulus(void* pSrcImg, seAnnulus roiAnnulus, const char* strSavePath, LPBoundingBox pBoundBoxOut) override;
	//int vbs_InspectBox_Rect(const char* strSrcImg, seBoundingBox roiRect, const char* strSavePath, LPBoundingBox pBoundBoxOut)  override;
	int vbs_InspectBox_Rect(void* pSrcImg, seBoundingBox roiRect, const char* strSavePath, LPBoundingBox pBoundBoxOut)  override;
	//int vbs_InspectBox_Circle(const char* strSrcImg, seCircle roiCircle, const char* strSavePath, LPBoundingBox pBoundBoxOut)  override;
	int vbs_InspectBox_Circle(void* pSrcImg, seCircle roiCircle, const char* strSavePath, LPBoundingBox pBoundBoxOut)  override;


	//int vbs_InspectBox_CoordCalculate(const char* strSrcImg, seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn, LPCoordBindBox pCoorBindBoxOut) override;
	int vbs_InspectBox_CoordCalculate(void* pSrcImg, seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn, LPCoordBindBox pCoorBindBoxOut) override;


	//int vbs_InspectBox_CropImg_Annulus(const char* strSrcImg, seCoordBindBox seCoordBox, seAnnulus roiAnnulus, seExpandable seExpandable, LPAnnulus pRoiOffset_Annulus) override;
	int vbs_InspectBox_CropImg_Annulus(void* pSrcImg, seCoordBindBox seCoordBox, seAnnulus roiAnnulus, seExpandable* seExpandable, LPAnnulus pRoiOffset_Annulus) override;
	//int vbs_InspectBox_CropImg_Rect(const char* strSrcImg, seCoordBindBox seCoordBox, seBoundingBox roiRect, seExpandable seExpandable, LPBoundingBox pRoiOffset_Rect) override;
	int vbs_InspectBox_CropImg_Rect(void* pSrcImg, seCoordBindBox seCoordBox, seBoundingBox roiRect, seExpandable* seExpandable, LPBoundingBox pRoiOffset_Rect) override;
	//int vbs_InspectBox_CropImg_Circle(const char* strSrcImg, seCoordBindBox seCoordBox, seCircle roiCircle, seExpandable seExpandable, LPCircle pRoiOffset_Circle) override;
	int vbs_InspectBox_CropImg_Circle(void* pSrcImg, seCoordBindBox seCoordBox, seCircle roiCircle, seExpandable* seExpandable, LPCircle pRoiOffset_Circle) override;


	//int vbs_Histogram_Annulus(const char* strSrcImg, seAnnulus roiAnnulus, seExpandable seExpandable, int* pDataCntOut, double* p1DArrayOut)  override;
	int vbs_Histogram_Annulus(void* pSrcImg, seAnnulus roiAnnulus, seExpandable* seExpandable, int* pDataCntOut, double* p1DArrayOut)  override;
	//int vbs_Histogram_Rect(const char* strSrcImg, seBoundingBox roiRect, seExpandable seExpandable, int* pDataCntOut, double* p1DArrayOut) override;
	int vbs_Histogram_Rect(void* pSrcImg, seBoundingBox roiRect, seExpandable* seExpandable, int* pDataCntOut, double* p1DArrayOut) override;
	//int vbs_Histogram_Circle(const char* strSrcImg, seCircle roiCircle, seExpandable seExpandable, int* pDataCntOut, double* p1DArrayOut) override;
	int vbs_Histogram_Circle(void* pSrcImg, seCircle roiCircle, seExpandable* seExpandable, int* pDataCntOut, double* p1DArrayOut) override;


	//int vbs_Threshold(const char* strSrcImg, double* pThresh, double* pMaxVal, emThresholdTypes emTypes, seExpandable seExpandable)  override;
	int vbs_Threshold(void* pSrcImg, double* pThresh, double* pMaxVal, emThresholdTypes emTypes, seExpandable* seExpandable)  override;



	//int vbs_Morphology(const char* strSrcImg, emMorphShapes emShapes, int iKSize, emMorphOperation emOperation, seExpandable seExpandable)  override;
	int vbs_Morphology(void* pSrcImg, emMorphShapes emShapes, int iKSize, emMorphOperation emOperation, seExpandable* seExpandable)  override;



	//int vbs_NoiseRemoval(const char* strSrcImg, double dbLimit_min, double dbLimit_max, seExpandable seExpandable)  override;
	int vbs_NoiseRemoval(void* pSrcImg, double dbLimit_min, double dbLimit_max, seExpandable* seExpandable)  override;

	int vbs_DataAugmentation(void* pSrcImg, seDataAugmentationInfo seDA_Param, seExpandable* seExpandable) override;



	int vbs_GlueWidth_Measure_Annulus(void* pSrcImg, seAnnulus roiAnnuls, int stepSize, seExpandable* seExpandable, int* pDataCount, double* pLength_InnerOut, double* pLength_OuterOut, sePoint* pPosition_InnerOut, sePoint* pPosition_OuterOut, double* pGlueAreaOut) override;
	int vbs_GlueWidth_Measure_Rect(void* pSrcImg, seBoundingBox roiRect, int stepSize, seExpandable* seExpandable, int* pDataCntOut, double* pLength_InnerOut, double* pLength_OuterOut, sePoint* pPosition_InnerOut, sePoint* pPosition_OuterOut, double* pGlueAreaOut) override;


	// int vsb_AiELIC_Initialize(seAI_FUNC_Init seParam) override;
	// int vbs_AiELIC_GlueWidth_Measure_Color(void* pSrcImg, seExpandable* seExpandable, seAiELIC_MeasGlueWidth_Color seAiELIC_Param, char* szPredClassm, double* dbTime) override;






public:
	int printHelloWorld(unsigned char* pBuf) override;

	void EnableLogger(unsigned char enable, unsigned short level, const char* filepath) override;
	void LogMsg(const char* msg, unsigned short level=2) override;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Setp_02:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Alignment mthod_Pattern Matach
	int Align_Pattern_SetParameter(seRect roiSearch, LPImageInfo pTemplatIn) override;
	int Align_Pattern_TestIt(LPImageInfo pIn) override;
	int Align_Pattern_GetResult(LPBoundingBox pFMarkBox, double* dbScore) override; //return info of bounding box and score.


	//Alignment method_Find Profile
	int Align_FindProfile_SetPattern(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize) override;
	int Align_FindProfile_TestIt(LPImageInfo pIn, int iSelLineNo) override;
	int Align_FindProfile_GetResult(int* CntArryOut, int* p1DArrayOut) override; //return info of bounding box and 1d array data.


	// Alignment mthod_Find Circle
	int Align_DetectCircle_SetPattern(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize) override;
	int Align_DetectCircle_TestIt(LPImageInfo pIn) override;
	int Align_DetectCircle_GetResult(LPBoundingBox pFMarkBox) override; //return info of bounding box and 1d array data.


	// InspecBoxSetup_Annulus
	int InspectBox_Annulus_SetParameter(seAnnulus roiAnnuls) override;
	int InspectBox_Rectangle_SetParameter(seBoundingBox roiRect) override;
	int InspectBox_Circle_SetParameter(seCircle roiCircle) override;
	int InspectBox_TestIt(LPImageInfo pIn) override;
	int InspectBox_GetResult(LPBoundingBox pInspBox) override; //retur ROI Image


	// calculate the dependency coordinates 
	int CoordBind_Calcu_SetParameter(seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn) override;
	int CoordBind_Calcu_TestIt() override;
	int CoordBind_Calcu_GetResult(LPCoordBindBox pCoordBoxInfo) override;


	int CoordBind_CropROI_Annulus_SetParameter(seCoordBindBox seCoordBox, seAnnulus roiAnnuls) override;
	int CoordBind_CropROI_Rectangle_SetParameter(seCoordBindBox seCoordBox, seBoundingBox roiRect) override;
	int CoordBind_CropROI_Circle_SetParameter(seCoordBindBox seCoordBox, seCircle roiCircle) override;
	int CoordBind_CropROI_TestIt(LPImageInfo pIn) override;
	int CoordBind_CropROI_Annulus_GetResult(LPImageInfo pOut, LPAnnulus pRoiOffset_Annuls) override;
	int CoordBind_CropROI_Rectangle_GetResult(LPImageInfo pOut, LPBoundingBox pRoiOffset_Rect) override;
	int CoordBind_CropROI_Circle_GetResult(LPImageInfo pOut, LPCircle pRoiOffset_Circle) override;



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Step_03
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Image Processing_(Threshold)
	int IpThreshold_SetParameter(int iChannels, double* pThresh, double* pMaxVal, emThresholdTypes emTypes) override;
	int IpThreshold_TestIt(LPImageInfo pIn) override;
	int IpThreshold_GetResult(LPImageInfo pOut) override;


	// Image Processing_( Histogram )
	int IpHistogram_SetParameter(LPImageInfo pIn) override;
	int IpHistogram_Annulus_SetParameter(LPImageInfo pIn, seAnnulus roiAnnuls) override;
	int IpHistogram_Rectangle_SetParameter(LPImageInfo pIn, seBoundingBox roiRect) override;
	int IpHistogram_Circle_SetParameter(LPImageInfo pIn, seCircle roiCircle) override;
	int IpHistogram_TestIt() override;
	int IpHistogram_GetResult(double* p1DArray) override;


	// Image Processing_( Morphology )
	int IpMorphology_SetParameter(emMorphShapes emShapes, int iKSize) override;
	int IpMorphology_TestIt(LPImageInfo pIn, emMorphOperation emOperation) override;
	int IpMorphology_GetResult(LPImageInfo pOut) override;


	// Image Processing_( Morpholoy )( Fill Holes )
	int IpNoiseRemoval_SetParameter(double dbLimit_min, double dbLimit_max) override;
	int IpNoiseRemoval_TestIt(LPImageInfo pIn) override;
	int IpNoiseRemoval_GetResult(LPImageInfo pOut) override;



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Step_4
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
	// Measure 
	int MeasGlueWidth_Annulus_SetParameter(seAnnulus roiAnnuls, int stepSize) override;
	int MeasGlueWidth_Rectangle_SetParameter(seBoundingBox roiRect, int stepSize) override;
	int MeasGlueWidth_TestIt(LPImageInfo pIn) override;
	int MeasGlueWidth_GetResult(double* pLength_Inner, double* pLength_Outer, double* pGlueArea) override;
	int MeasGlueWidth_GetInfo4Dump(std::vector<std::string>& vec_strInfo) override;

private:
	CMethod_Alignment		cvip_Align;
	CMethod_ImgProcessing	cvip_IP;
	CMethod_Measure			cvip_Measure;
	CMethod_JudgmentMech	cvip_Judgment;
	// CMethod_ELIC			cvai_elic;


	bool m_bLogger;
	std::string strpath_LOG;

#if defined(_MSC_VER)

	std::shared_ptr<spdlog::logger> m_hLogger;

#endif	// #if defined(_MSC_VER)

	 
private:
	void EnableLogger(unsigned char bOn, unsigned short level, std::string filePath);
	void LogMsg(std::string msg, unsigned short level = 2);	//2 : SPDLOG_LEVEL_INFO
	bool IsFileExists(const std::string& path);


private:
	cv::Mat m_source_colorImg;
	cv::Mat m_source_grayImg;

	//Alignment_Pattern Match
	//In
	//string m_strPath_SrcImg;
	cv::Mat m_template_grayImg;
	cv::Rect m_Align_rect_search;
	cv::Rect m_Align_rect_template;
	//Out
	double m_dbScoreOut;
	seBoundingBox m_Border_FMark;	// <---BoundingBox of FiducialMark(PatternMatch)

	//Alignment_Find Profile/ Detect Circle
	seCircle m_FindProfile_search;
	seCircle m_FindProfile_mask;

	//InspectBox_( Rectangle; Circle; Annulus ) 
	seBoundingBox m_InspectBox_Rect;
	seBoundingBox m_Border_Rect;	// <---BoundingBox of InspectBox(Rectangle)

	seCircle m_InspectBox_Circle;
	seBoundingBox m_Border_Circle;	// <---BoundingBox of InspectBox(Circle)

	seAnnulus m_InspectBox_Annulus;
	seBoundingBox m_Border_Annulus;	// <---BoundingBox of InspectBox(Annulus)

	//Coordination caluculate
	seCoordBindBox m_CoorCalcu_AnyShape;

	seCoordBindBox m_CoorCalcu_Rectangle;

	seCoordBindBox m_CoorCalcu_Circle;

	seCoordBindBox m_CoorCalcu_Annulus;

	//Crop image from source image 
	cv::Mat m_roiTarget_Rectangle;		// <--- target ROI image (Rectangle)
	seBoundingBox m_seOffsetBox_Rectangle;

	cv::Mat m_roiTarget_Circle;			// <--- target ROI image (Circle)
	seCircle m_seOffsetBox_Circle;

	cv::Mat m_roiTarget_Annulus;			// <--- target ROI image (Annulus)
	seAnnulus m_seOffsetBox_Annulus;

	//Image Proessing (Threshold)
	cv::Mat mat_Threshold_Img;

	cv::Mat mat_Morphology_Img;

	cv::Mat mat_NoiseRemoveal_Img;

	//Measure GlueWidth
	seAnnulus m_Measure_IBox_Annulus;



};

