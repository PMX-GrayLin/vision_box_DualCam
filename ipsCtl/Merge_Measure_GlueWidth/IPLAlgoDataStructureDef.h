#pragma once


#ifndef _IPS_ALGO_STRUCTURE_DEF_H_
#define	_IPS_ALGO_STRUCTURE_DEF_H_


#include "BaseDataStructureDef.h"

/////////////////////////////////////////////////////////////
// Method_Alignment 
/////////////////////////////////////////////////////////////


/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Auto running mode 
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
///  
typedef struct tagMode_AutoRunning {

	tagMode_AutoRunning() 
		: bFlg_AutoRunning(false)
		, bFlg_Enable_TriggerMode(false)
	{}

	bool bFlg_AutoRunning;
	bool bFlg_Enable_TriggerMode;

} seMode_AutoRunning, * LPMode_AutoRunning;


typedef struct tagResult_AutoRunning {

	tagResult_AutoRunning() 
		: retState(0)
		, bRet_AutoRunning(false)
		, bRet_Enable_TriggerMode(false)	
	{}

	int retState;
	//Results
	bool bRet_AutoRunning;
	bool bRet_Enable_TriggerMode;

} seMode_AutoRunning_Ret, * LPMode_AutoRunning_Ret;



/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// trigger mode selection
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
///  
typedef struct tagMode_TriggerModeType {

	tagMode_TriggerModeType()
		: bFlg_TriggerMode_Activate(false)
	{}

	bool bFlg_TriggerMode_Activate;

} seMode_TriggerModeType, * LPMode_TriggerModeType;


typedef struct tagResult_TriggerModeType {

	tagResult_TriggerModeType() 
		: retState(0)
		, bRet_TriggerMode_Activate(false)
	{}

	int retState;
	//Results
	bool bRet_TriggerMode_Activate;

} seMode_TriggerModeType_Ret, * LPMode_TriggerModeType_Ret;




/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Image Calibration
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
///  
typedef struct tagMethod_ImageCalibration {

	tagMethod_ImageCalibration()
		: dbE2E_Distance_mm(0.0)
		, strInputImgPath("")
		, strResultImgPath("")
	{}

	double dbE2E_Distance_mm;
	std::string strInputImgPath;
	std::string strResultImgPath;

} seMth_ImageCalibration, * LPMth_ImageCalibration;

typedef struct tagResult_ImageCalibration {

	tagResult_ImageCalibration()
		: retState(0)
		, dbPixelCount_X(0.0)
		, dbPixelCount_Y(0.0)
		, dbmm_per_pixel(0.0)
	{}

	int retState;

	//Results
	double dbPixelCount_X;
	double dbPixelCount_Y;

	double dbmm_per_pixel;

	seCircle circle_01;
	seCircle circle_02;
	seCircle circle_03;


} seMth_ImageCalibration_Ret, * LPMth_ImageCalibration_Ret;




/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Crop Template Image
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 
typedef struct tagCropROI_GoldenTemplate {

	tagCropROI_GoldenTemplate()
		: strInputImgPath("")
		, strSaveImgPath("")
		, strResultImgPath("")
	{}

	//LPImageInfo pIn;
	seRect roiRect;
	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;

} seCropROI_GTemplate, * LPCropROI_GTemplate;

typedef struct tagCropROI_GoldenTemplate_Result {

	tagCropROI_GoldenTemplate_Result()
		: retState(0)
		, TBD(-99999)
	{}
	int retState;
	//TBD
	int TBD;

} seCropROI_GTemplate_Ret, * LpCropROI_GTemplate_Ret;




/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Pattern Match
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
///  
typedef struct tagMethod_PatternMatch {

	tagMethod_PatternMatch()
		: strInputImgPath("")
		, strTemplateImgPath("")
		, strResultImgPath("")
	{}
	
	seRect roiSearch;
	std::string strInputImgPath;
	std::string strTemplateImgPath;
	std::string strResultImgPath;		

} seMth_PatternMatch, * LPMth_PatternMatch;

typedef struct tagResult_PatternMatch {

	int retState;

	//Results
	seBoundingBox seFMarkBoxOut;
	double dbScoreOut;

} seMth_PatternMatch_Ret, * LPMth_PatternMatch_Ret;




/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Find Profile
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 
typedef struct tagMethod_FindProfile {

	//LPImageInfo pIn;
	seCircle roiSearch;
	seCircle roiMask;
	bool bDirection;
	bool bPolarity;
	int stepSize;
	int iKSize;
	int iSelLineNo;

	//Results
	//int* CntArryOut;
	//int* p1DArrayOut;

	std::string strInputImgPath;
	//std::string strSaveImgPath;
	std::string strResultImgPath;

} seMth_FindProfile, * LPFindProfile;

typedef struct tagMethod_FindProfile_Result {

	int retState;

	//Results
	//int* CntArryOut;
	int iDataCnt;
	int i1DArrayOut[4096];

} seMth_FindProfile_Ret, * LPFindProfile_Ret;



/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Detect Circle
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 

typedef struct tagMethod_DetectCircle {

	//LPImageInfo pIn;
	seCircle roiSearch;
	seCircle roiMask;
	bool bDirection;
	bool bPolarity;
	int stepSize;
	int iMinEdgeStrength;
	int iKSize;

	//Reultes
	//LPBoundingBox pFMarkBoxOut

	std::string strInputImgPath;
	//std::string strSaveImgPath;
	std::string strResultImgPath;

} seMth_DetectCircle, * LPMth_DetectCircle;

typedef struct tagMethod_DetectCircle_Result {

	int retState;

	//Reultes
	seBoundingBox seFMarkBoxOut;

	int iDataCnt;
	sePoint sePointArrayOut[380];


} seMth_DetectCircle_Ret, * LPMth_DetectCircle_Ret;



/////////////////////////////////////////////////////////////
// Method_InspecBox
/////////////////////////////////////////////////////////////

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// InspectionBox setting_Annulus
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 

typedef struct tagInspectBox_Annulus {

	//LPImageInfo pIn;
	seAnnulus roiAnnulus;

	//Resulte
	//LPImageInfo pOut;

	std::string strInputImgPath;
	//std::string strSaveImgPath;
	std::string strResultImgPath;


} seIBox_Annulus, * LPIBox_Annulus;

typedef struct tagInspectBox_Annulus_Result {

	int retState;

	seBoundingBox seBoundBoxOut;

} seIBox_Annulus_Ret, * LPIBox_Annulus_Ret;




/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// InspectionBox setting_Rectangle
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 

typedef struct tagInspectBox_Rect {

	//LPImageInfo pIn;
	seBoundingBox roiRect;

	//Results
	//LPImageInfo pOut;
	//LPBoundingBox pBoundBoxOut;

	std::string strInputImgPath;
	//std::string strSaveImgPath;
	std::string strResultImgPath;

} seIBox_Rect, * LPIBox_Rect;

typedef struct tagInspectBox_Rect_Result {

	int retState;

	//Results
	seBoundingBox seBoundBoxOut;

} seIBox_Rect_Ret, * LPIBox_Rect_Ret;




/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// InspectionBox setting_Circle
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 

typedef struct tagInspectBox_Circle {

	//LPImageInfo pIn;
	seCircle roiCircle;

	//Results
	//LPImageInfo pOut;
	//LPBoundingBox pBoundBoxOut;

	std::string strInputImgPath;
	//std::string strSaveImgPath;
	std::string strResultImgPath;

} seIBox_Circle, * LPIBox_Circle;

typedef struct tagInspectBox_Circle_Result {

	int retState;

	//Results
	seBoundingBox seBoundBoxOut;

} seIBox_Circle_Ret, * LPIBox_Circle_Ret;




/////////////////////////////////////////////////////////////
// Calculate coordinate
/////////////////////////////////////////////////////////////

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Calculate Coordination
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 
typedef struct tagCalculate_Coordinate {

	seBoundingBox seFMarkBox;
	seBoundingBox seInspBox;
	seCoordBindBox seCoorBindBoxIn;

	std::string strInputImgPath;
	//std::string strSaveImgPath;
	//std::string strResultImgPath;


} seCalcCoord, * LPCalcCoord;


typedef struct tagCalculate_Coordinate_Result {

	int retState;

	//Results
	seCoordBindBox seCoorBindBoxOut;

} seCalcCoord_Ret, * LPCalcCoord_Ret;



/////////////////////////////////////////////////////////////
// Crop Image by Shape define
/////////////////////////////////////////////////////////////

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Crop Image of Annulus
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 
typedef struct tagCropROI_Annulus {

	//LPImageInfo pIn;
	seCoordBindBox seCoordBox;
	seAnnulus roiAnnulus;

	//Results
	//LPImageInfo pOut;
	//LPAnnulus pRoiOffset_Annulus;

	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;

} seCropROI_Annulus, * LPCropROI_Annulus;

typedef struct tagCropROI_Annulus_Result {

	int retState;

	//Results
	seAnnulus seRoiOffset_Annulus;

} seCropROI_Annulus_Ret, * LPCropROI_Annulus_Ret;



/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Crop Image of Rectangle
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 

typedef struct tagCropROI_Rect {

	//LPImageInfo pIn;
	seCoordBindBox seCoordBox;
	seBoundingBox roiRect;

	//Results
	//LPImageInfo pOut;
	//LPBoundingBox pRoiOffset_Rect;

	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;

} seCropROI_Rect, * LPCropROI_Rect;

typedef struct tagCropROI_Rect_Result {

	int retState;

	//Results
	seBoundingBox seRoiOffset_Rect;

} seCropROI_Rect_Ret, * LPCropROI_Rect_Ret;



/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Crop Image of Circle
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 

typedef struct tagCropROI_Circle {

	//LPImageInfo pIn;
	seCoordBindBox seCoordBox;
	seCircle roiCircle;

	//Results
	//LPImageInfo pOut;
	//LPCircle pRoiOffset_Circle;

	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;

} seCropROI_Circle, * LPCropROI_Circle;

typedef struct tagCropROI_Circle_Result {

	int retState;

	//Results
	//LPImageInfo pOut;
	seCircle seRoiOffset_Circle;

} seCropROI_Circle_Ret, * LPCropROI_Circle_Ret;




/////////////////////////////////////////////////////////////
// ImgProc_Threshold
/////////////////////////////////////////////////////////////

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Threshols algorithm
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 
/// 
typedef struct tagImgProc_Threshold {

	//LPImageInfo pIn;
	double dbThresh;
	double dbMaxVal;
	emThresholdTypes emTypes;

	//Results
	//LPImageInfo pOut;


	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;


} seIP_Threshold, * LPIP_Threshold;

typedef struct tagImgProc_Threshold_Result {

	int retState;

	//TBD
	int TBD = -9999;

} seIP_Threshold_Ret, * LPIP_Threshold_Ret;





/////////////////////////////////////////////////////////////
// ImgProc_Histogram
/////////////////////////////////////////////////////////////

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Histogram of Annulus
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 
typedef struct tagHistogram_Annulus {

	//LPImageInfo pIn;
	seAnnulus roiAnnulus;

	//Results
	//double* p1DArray


	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;


} seHisg_Annulus, * LPHisg_Annulus;

typedef struct tagHistogram_Annulus_Result {

	int retState;

	//Results
	int iDataCnt;
	double db1DArray[256 * 3];

} seHisg_Annulus_Ret, * LPHisg_Annulus_ret;



/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Crop Image of rectangle
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 

typedef struct tagHistogram_Rect {

	//LPImageInfo pIn;
	seBoundingBox roiRect;

	//Results
	//double* p1DArray


	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;


} seHisg_Rect, * LPHisg_Rect;

typedef struct tagHistogram_Rect_Result {

	int retState;

	//Results
	int iDataCnt;
	double db1DArray[256 * 3];

} seHisg_Rect_Ret, * LPHisg_Rect_Ret;



/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Crop Image of Circle
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 
/// 
typedef struct tagHistogram_Circle {

	//LPImageInfo pIn;
	seCircle roiCircle;

	//Results
	//double* p1DArray


	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;


} seHisg_Circle, * LPHisg_Circle;

typedef struct tagHistogram_Circle_Result {

	int retState;

	//Results
	int iDataCnt;
	double db1DArray[256 * 3];

} seHisg_Circle_Ret, * LPHisg_Circle_Ret;




/////////////////////////////////////////////////////////////
// ImgProc_Morphology
/////////////////////////////////////////////////////////////

typedef struct tagMorphology {

	//LPImageInfo pIn;

	emMorphShapes emShapes;
	int iKSize;	
	emMorphOperation emOperation;


	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;


} seMorphology, * LPMorphology;

typedef struct tagMorphology_Result {

	int retState;

	//TBD
	int TBD = -9999;

} seMorphology_Ret, * LPMorphology_Ret;



/////////////////////////////////////////////////////////////
// ImgProc_NoiseRemoval
/////////////////////////////////////////////////////////////

typedef struct tagNoiseRemoval {

	//LPImageInfo pIn;
	double dbLimit_min;
	double dbLimit_max;


	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;


} seNoiseRemoval, * LPNoiseRemoval;

typedef struct tagNoiseRemoval_Result {

	int retState;

	//TBD
	int TBD = -9999;

} seNoiseRemoval_Ret, * LPNoiseRemoval_Ret;



/////////////////////////////////////////////////////////////
// ImgProc_DataAugmentation
/////////////////////////////////////////////////////////////

typedef struct tagDataAugmentation {

	seDataAugmentationInfo seDA_Param;

	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;

} seDataAugmentation, * LPDataAugmentation;

typedef struct tagDataAugmentation_Result {

	int retState;
	//TBD
	int TBD = -9999;

} seDataAugmentation_Ret, * LPDataAugmentation_Ret;




/////////////////////////////////////////////////////////////
// Measure_GlueWidth
/////////////////////////////////////////////////////////////

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Glue Width Measure of Annulus
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 
typedef struct tagMeasGlueWidth_Annulus {

	//LPImageInfo pIn;
	seAnnulus roiAnnuls;
	int stepSize;
	double dbmm_per_pixel;

	//Results
	//double* pLength_InnerOut;
	//double* pLength_OuterOut;
	//double* pGlueAreaOut;


	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;


} seMeasGW_Annulus, * LPMeasGW_Annulus;

typedef struct tagMeasGlueWidth_Annulus_Result {

	int retState;

	//Results
	int iDataCnt;
	double dbLength_InnerOut[380];
	double dbLength_OuterOut[380];
	sePoint pos_InnerOut[380];
	sePoint pos_OuterOut[380];
	double dbGlueAreaOut;

} seMeasGW_Annulus_Ret, * LPMeasGW_Annulus_Ret;



/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// Glue Width Measure of Rectangle
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  
/// 
typedef struct tagMeasGlueWidth_Rect {

	//LPImageInfo pIn;
	seBoundingBox roiRect;
	int stepSize;
	double dbmm_per_pixel;

	//Results
	//double* pLength_InnerOut;
	//double* pLength_OuterOut;
	//double* pGlueAreaOut;


	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;


} seMeasGW_Rect, * LPMeasGW_Rect;

typedef struct tagMeasGlueWidth_Rect_Result {

	int retState;

	//Results
	int iDataCnt;
	double dbLength_InnerOut[1024];
	double dbLength_OuterOut[1024];
	sePoint pos_InnerOut[1024];
	sePoint pos_OuterOut[1024];
	double dbGlueAreaOut;
	
} seMeasGW_Rect_Ret, * LPMeasGW_Rect_Ret;



/////////////////////////////////////////////////////////////
// AiELIC_Glue Width Measure of Color detection
/////////////////////////////////////////////////////////////

typedef struct tagAiELIC_MeasGlueWidth_Color {

	tagAiELIC_MeasGlueWidth_Color() {

		strMode_Project = "";
		strMode_Name="";
		strMode_Attr="";

		strInputImgPath="";
		strSaveImgPath="";
		strResultImgPath="";

	}

	//seAiELIC_MaseGW_Color_Param seAiELIC_MGS_Color_Param;

	std::string strMode_Project;	//ex: PROJECT
	std::string strMode_Name;		//ex: utils
	std::string strMode_Attr;		//ex: Engine

	std::string strInputImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;

} seAiELIC_MeasGlueWidth_Color, * LPAiELIC_MeasGlueWidth_Color;

typedef struct tagAiELIC_MeasGlueWidth_Color_Result {

	int retState;

	double dbAccuracy;
	double dbTime;
	char szPredict_Class[100];

} seAiELIC_MeasGlueWidth_Color_Ret, * LPAiELIC_MeasGlueWidth_Color_Ret;





/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// 
/// TBD
/// /// /// /// /// /// /// /// /// /// /// /// /// /// ///  

typedef struct tag_IPL_AllParamTable {

	seMode_AutoRunning			set_Mode_AutoRunning;
	seMode_AutoRunning_Ret		ret_Mode_AutoRunning;
	seMth_PatternMatch_Ret		ret_FMark_PatternMatch;
	seMth_DetectCircle_Ret		ret_FMark_DetectCircle;
	seIBox_Annulus				set_IBox_Annulus;
	seIBox_Annulus_Ret			ret_BBox_Annulus;
	seIBox_Rect					set_IBox_Rect;
	seIBox_Rect_Ret				ret_BBox_Rect;	
	seIBox_Circle				set_IBox_Circle;
	seIBox_Circle_Ret			ret_BBox_Circle;	
	seCalcCoord_Ret				ret_CoBBox_CalcCoord;
	seCropROI_Annulus_Ret		ret_RoiOffset_CropImg_Annulu;
	seCropROI_Rect_Ret			ret_RoiOffset_CropImg_Rect;
	seCropROI_Circle_Ret		ret_RoiOffset_CropImg_Circle;
	seIP_Threshold				set_InRange_Threshold;


} seIPL_AllParamTable, * LPIPL_AllParamTable;



typedef struct tag_MesGlueWidth_AllParamTable {

	seMode_AutoRunning			ret_Mode_AutoRuning;
	seMth_PatternMatch_Ret		ret_FMark_PatternMatch;
	seIBox_Annulus				set_IBox_Annulus;
	seIBox_Annulus_Ret			ret_BBox_Annulus;
	seIBox_Rect					set_IBox_Rect;
	seIBox_Rect_Ret				ret_BBox_Rect;
	seCalcCoord_Ret				ret_CoBBox_CalcCoord;
	seCropROI_Annulus_Ret		ret_CropImg_Annulu;
	seCropROI_Rect_Ret			ret_CropImg_Rect;
	seIP_Threshold				set_InRange_Threshold;

} seAllParamTable_MeasGW_Annulus, * LPAllParamTable_MeasGW_Annulus;



#endif	//_IPS_ALGO_STRUCTURE_DEF_H_