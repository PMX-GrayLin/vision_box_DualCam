#pragma once


#if defined(_MSC_VER)	// /* Visual C++ */


	#if defined( PTW_IMAGEPROCESSINGLIBRARY_EXPORTS )
	#define DLLCALL __declspec(dllexport)
	#else
	#define DLLCALL __declspec(dllimport)
	#endif

#endif // #if defined(_MSC_VER)	// /* Visual C++ */



#include "BaseDataStructureDef.h"
#include "IPLAlgoDataStructureDef.h"
// #include "AILAlgoDataStructureDef.h"
#include <vector>
#include <string>



typedef struct tagImgSrcTab
{
	bool bSelInputSrc;		//0: Path of SrcImg, 1:pointer of LPImageInfo
	bool bDumpRetImg;	//0: no dump image, 1: dump result image.

	const char* strSrcImg;
	const seImageInfo* ptrImgInfo;
	const seImageInfo* ptrImgInfo_Color;

	tagImgSrcTab() {

		bSelInputSrc = 0;
		bDumpRetImg = 0;

		strSrcImg = nullptr;
		ptrImgInfo = nullptr;

		ptrImgInfo_Color = nullptr;
	}

} seImgSrcTab, * LPImgSrcTab;


//Interface class
//class DLLCALL PTWDLL_I_GlueWidth
class PTWDLL_I_GlueWidth
{
public:
	virtual ~PTWDLL_I_GlueWidth() { ; };


public:
	//Algorithm list for VisionBox 
	//virtual int vbs_Align_CropTemplate(const char* strSrcImg, seRect roiSearch, seExpandable seExpandable) = 0;
	virtual int vbs_Align_CropTemplate(void* pSrcImg, seRect roiSearch, seExpandable* seExpandable) = 0;

	//virtual int vbs_Align_ImageCalibration(const char* strSrcImg, seRect roiRect_01, seRect roiRect_02, const char* strSavePath, double* pPixelCount_X, double* pPixelCount_Y) = 0;
	virtual int vbs_Align_ImageCalibration(void* pSrcImg, seRect roiRect_01, seRect roiRect_02, const char* strSavePath, double* pPixelCount_X, double* pPixelCount_Y) = 0;
	
	//virtual int vbs_Align_PatternMatch(const char* strSrcImg, const char* strTemplateImg, seRect roiSearch, const char* strSavePath, LPBoundingBox pFMarkBoxOut, double* pdbScoreOut) = 0;
	virtual int vbs_Align_PatternMatch(void* pSrcImg, const char* strTemplateImg, seRect roiSearch, const char* strSavePath, LPBoundingBox pFMarkBoxOut, double* pdbScoreOut) = 0;


	//virtual int vbs_Align_FindProfile(const char* strSrcImg, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int iSelLineNo, const char* strSavePath, int* pCntArryOut, int* p1DArrayOut) = 0;
	virtual int vbs_Align_FindProfile(void* pSrcImg, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int iSelLineNo, const char* strSavePath, int* pCntArryOut, int* p1DArrayOut) = 0;
	//virtual int vbs_Align_DetectCircle(const char* strSrcImg,
	virtual int vbs_Align_DetectCircle(void* pSrcImg,
		seCircle roiSearch,
		seCircle roiMask,
		bool bDirection,
		bool bPolarity,
		int iMinEdgeStrength,
		const char* strSavePath,
		LPBoundingBox pFMarkBox_Circle,
		int* pCntArryOut,
		sePoint* pPoinitArrayOut) = 0;


	//virtual int vbs_InspectBox_Annulus(const char* strSrcImg, seAnnulus roiAnnulus, const char* strSavePath, LPBoundingBox pBoundBoxOut) = 0;
	virtual int vbs_InspectBox_Annulus(void* pSrcImg, seAnnulus roiAnnulus, const char* strSavePath, LPBoundingBox pBoundBoxOut) = 0;
	//virtual int vbs_InspectBox_Rect(const char* strSrcImg, seBoundingBox roiRect, const char* strSavePath, LPBoundingBox pBoundBoxOut) = 0;
	virtual int vbs_InspectBox_Rect(void* pSrcImg, seBoundingBox roiRect, const char* strSavePath, LPBoundingBox pBoundBoxOut) = 0;
	//virtual int vbs_InspectBox_Circle(const char* strSrcImg, seCircle roiCircle, const char* strSavePath, LPBoundingBox pBoundBoxOut) = 0;
	virtual int vbs_InspectBox_Circle(void* pSrcImg, seCircle roiCircle, const char* strSavePath, LPBoundingBox pBoundBoxOut) = 0;


	//virtual int vbs_InspectBox_CoordCalculate(const char* strSrcImg, seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn, LPCoordBindBox pCoorBindBoxOut) = 0;
	virtual int vbs_InspectBox_CoordCalculate(void* pSrcImg, seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn, LPCoordBindBox pCoorBindBoxOut) = 0;


	//virtual int vbs_InspectBox_CropImg_Annulus(const char* strSrcImg, seCoordBindBox seCoordBox, seAnnulus roiAnnulus, seExpandable seExpandable, LPAnnulus pRoiOffset_Annulus) = 0;
	virtual int vbs_InspectBox_CropImg_Annulus(void* pSrcImg, seCoordBindBox seCoordBox, seAnnulus roiAnnulus, seExpandable* seExpandable, LPAnnulus pRoiOffset_Annulus) = 0;
	//virtual int vbs_InspectBox_CropImg_Rect(const char* strSrcImg, seCoordBindBox seCoordBox, seBoundingBox roiRect, seExpandable seExpandable, LPBoundingBox pRoiOffset_Rect) = 0;
	virtual int vbs_InspectBox_CropImg_Rect(void* pSrcImg, seCoordBindBox seCoordBox, seBoundingBox roiRect, seExpandable* seExpandable, LPBoundingBox pRoiOffset_Rect) = 0;
	//virtual int vbs_InspectBox_CropImg_Circle(const char* strSrcImg, seCoordBindBox seCoordBox, seCircle roiCircle, seExpandable seExpandable, LPCircle pRoiOffset_Circle) = 0;
	virtual int vbs_InspectBox_CropImg_Circle(void* pSrcImg, seCoordBindBox seCoordBox, seCircle roiCircle, seExpandable* seExpandable, LPCircle pRoiOffset_Circle) = 0;


	//virtual int vbs_Histogram_Annulus(const char* strSrcImg, seAnnulus roiAnnulus, seExpandable seExpandable, int* pDataCntOut, double* p1DArrayOut) = 0;
	virtual int vbs_Histogram_Annulus(void* pSrcImg, seAnnulus roiAnnulus, seExpandable* seExpandable, int* pDataCntOut, double* p1DArrayOut) = 0;
	//virtual int vbs_Histogram_Rect(const char* strSrcImg, seBoundingBox roiRect, seExpandable seExpandable, int* pDataCntOut, double* p1DArrayOut) = 0;
	virtual int vbs_Histogram_Rect(void* pSrcImg, seBoundingBox roiRect, seExpandable* seExpandable, int* pDataCntOut, double* p1DArrayOut) = 0;
	//virtual int vbs_Histogram_Circle(const char* strSrcImg, seCircle roiCircle, seExpandable seExpandable, int* pDataCntOut, double* p1DArrayOut) = 0;
	virtual int vbs_Histogram_Circle(void* pSrcImg, seCircle roiCircle, seExpandable* seExpandable, int* pDataCntOut, double* p1DArrayOut) = 0;


	//virtual int vbs_Threshold(const char* strSrcImg, double* pThresh, double* pMaxVal, emThresholdTypes emTypes, seExpandable seExpandable) = 0;
	virtual int vbs_Threshold(void* pSrcImg, double* pThresh, double* pMaxVal, emThresholdTypes emTypes, seExpandable* seExpandable) = 0;


	//virtual int vbs_Morphology(const char* strSrcImg, emMorphShapes emShapes, int iKSize, emMorphOperation emOperation, seExpandable seExpandable) = 0;
	virtual int vbs_Morphology(void* pSrcImg, emMorphShapes emShapes, int iKSize, emMorphOperation emOperation, seExpandable* seExpandable) = 0;


	//virtual int vbs_NoiseRemoval(const char* strSrcImg, double dbLimit_min, double dbLimit_max, seExpandable seExpandable) = 0;
	virtual int vbs_NoiseRemoval(void* pSrcImg, double dbLimit_min, double dbLimit_max, seExpandable* seExpandable) = 0;


	virtual int vbs_DataAugmentation(void* pSrcImg, seDataAugmentationInfo seDA_Param, seExpandable* seExpandable) = 0;


	virtual int vbs_GlueWidth_Measure_Annulus(void* pSrcImg, seAnnulus roiAnnuls, int stepSize, seExpandable* seExpandable, int* pDataCount, double* pLength_InnerOut, double* pLength_OuterOut, sePoint* pPosition_InnerOut, sePoint* pPosition_OuterOut, double* pGlueAreaOut) = 0;
	virtual int vbs_GlueWidth_Measure_Rect(void* pSrcImg, seBoundingBox roiRect, int stepSize, seExpandable* seExpandable, int* pDataCntOut, double* pLength_InnerOut, double* pLength_OuterOut, sePoint* pPosition_InnerOut, sePoint* pPosition_OuterOut, double* pGlueAreaOut) = 0;

	// AI_ELIC
	// virtual int vsb_AiELIC_Initialize(seAI_FUNC_Init seParam) = 0;
	// virtual int vbs_AiELIC_GlueWidth_Measure_Color(void* pSrcImg, seExpandable* seExpandable, seAiELIC_MeasGlueWidth_Color seAiELIC_Param, char* szPredClassm, double* dbTime) = 0;



public:
	//int Initiation();
	//int Release();

	virtual int printHelloWorld(unsigned char* pBuf) = 0;

	virtual void EnableLogger(unsigned char enable, unsigned short level, const char* filepath) = 0;
	virtual void LogMsg(const char* msg, unsigned short level=2) = 0 ;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Setp_02:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Alignment mthod_Pattern Matach
	virtual int Align_Pattern_SetParameter(seRect roiSearch, LPImageInfo pTemplatIn) = 0;
	virtual int Align_Pattern_TestIt(LPImageInfo pIn) = 0;
	virtual int Align_Pattern_GetResult(LPBoundingBox pFMarkBox, double* dbScore) = 0; //return info of bounding box and score.


	//Alignment method_Find Profile
	virtual int Align_FindProfile_SetPattern(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize) = 0;
	virtual int Align_FindProfile_TestIt(LPImageInfo pIn, int iSelLineNo) = 0;
	virtual int Align_FindProfile_GetResult(int* CntArryOut, int* p1DArrayOut) = 0; //return info of bounding box and 1d array data.

	// Alignment mthod_Find Circle
	virtual int Align_DetectCircle_SetPattern(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize) = 0;
	virtual int Align_DetectCircle_TestIt(LPImageInfo pIn) = 0;
	virtual int Align_DetectCircle_GetResult(LPBoundingBox pFMarkBox) = 0; //return info of bounding box and 1d array data.


	// InspecBoxSetup_Annulus
	virtual int InspectBox_Annulus_SetParameter(seAnnulus roiAnnuls) = 0;
	virtual int InspectBox_Rectangle_SetParameter(seBoundingBox roiRect) = 0;
	virtual int InspectBox_Circle_SetParameter(seCircle roiCircle) = 0;
	virtual int InspectBox_TestIt(LPImageInfo pIn) = 0;
	//virtual int InspectBox_GetResult(LPBoundingBox pInspBox, LPImageInfo pOut) = 0; //retur ROI Image
	virtual int InspectBox_GetResult(LPBoundingBox pInspBox) = 0; //retur ROI Image


	// calculate the dependency coordinates 
	virtual int CoordBind_Calcu_SetParameter(seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn) = 0;
	virtual int CoordBind_Calcu_TestIt() = 0;
	virtual int CoordBind_Calcu_GetResult(LPCoordBindBox pCoordBoxInfo) = 0;


	virtual int CoordBind_CropROI_Annulus_SetParameter(seCoordBindBox seCoordBox, seAnnulus roiAnnuls) = 0;
	virtual int CoordBind_CropROI_Rectangle_SetParameter(seCoordBindBox seCoordBox, seBoundingBox roiRect) = 0;
	virtual int CoordBind_CropROI_Circle_SetParameter(seCoordBindBox seCoordBox, seCircle roiCircle) = 0;
	virtual int CoordBind_CropROI_TestIt(LPImageInfo pIn) = 0;
	virtual int CoordBind_CropROI_Annulus_GetResult(LPImageInfo pOut, LPAnnulus pRoiOffset_Annuls) = 0;
	virtual int CoordBind_CropROI_Rectangle_GetResult(LPImageInfo pOut, LPBoundingBox pRoiOffset_Rect) = 0;
	virtual int CoordBind_CropROI_Circle_GetResult(LPImageInfo pOut, LPCircle pRoiOffset_Circle) = 0;



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Step_03
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Image Processing_(Threshold)
	virtual int IpThreshold_SetParameter(int iChannels, double* pThresh, double* pMaxVal, emThresholdTypes emTypes) = 0;
	virtual int IpThreshold_TestIt(LPImageInfo pIn) = 0;
	virtual int IpThreshold_GetResult(LPImageInfo pOut) = 0;


	// Image Processing_( Histogram )
	virtual int IpHistogram_SetParameter(LPImageInfo pIn ) = 0;
	virtual int IpHistogram_Annulus_SetParameter(LPImageInfo pIn, seAnnulus roiAnnuls) = 0;
	virtual int IpHistogram_Rectangle_SetParameter(LPImageInfo pIn, seBoundingBox roiRect) = 0;
	virtual int IpHistogram_Circle_SetParameter(LPImageInfo pIn, seCircle roiCircle) = 0;
	virtual int IpHistogram_TestIt() = 0;
	virtual int IpHistogram_GetResult(double* p1DArray) = 0;


	// Image Processing_( Morphology )
	virtual int IpMorphology_SetParameter(emMorphShapes emShapes, int iKSize) = 0;
	virtual int IpMorphology_TestIt(LPImageInfo pIn, emMorphOperation emOperation) = 0;
	virtual int IpMorphology_GetResult(LPImageInfo pOut) = 0;


	// Image Processing_( Morpholoy )( Fill Holes )
	virtual int IpNoiseRemoval_SetParameter(double dbLimit_min, double dbLimit_max) = 0;
	virtual int IpNoiseRemoval_TestIt(LPImageInfo pIn) = 0;
	virtual int IpNoiseRemoval_GetResult(LPImageInfo pOut) = 0;



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Step_4
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
	// Measure 
	virtual int MeasGlueWidth_Annulus_SetParameter(seAnnulus roiAnnuls, int stepSize) = 0;
	virtual int MeasGlueWidth_Rectangle_SetParameter(seBoundingBox roiRect, int stepSize) = 0;
	virtual int MeasGlueWidth_TestIt(LPImageInfo pIn) = 0;
	virtual int MeasGlueWidth_GetResult(double* pLength_Inner, double* pLength_Outer, double* pGlueArea) = 0;
	virtual int MeasGlueWidth_GetInfo4Dump(std::vector<std::string>& vec_strInfo) = 0;






};



#if defined(_MSC_VER)	// /* Visual C++ */

extern "C" /*Important for avoiding Name decoration*/
{

	DLLCALL PTWDLL_I_GlueWidth* _cdecl CreateObject_Class();

};

#elif defined (__GNUC__)


	PTWDLL_I_GlueWidth* CreateObject_Class();

#endif	//#if defined(_MSC_VER)	// /* Visual C++ */
