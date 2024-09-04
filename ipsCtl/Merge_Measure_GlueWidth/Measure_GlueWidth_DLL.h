#pragma once


#include "BaseDataStructureDef.h"


#if defined(_MSC_VER)	// /* Visual C++ */

	#if defined( PTW_IMAGEPROCESSINGLIBRARY_EXPORTS )
		#define LABVIEW_DLLCALL __declspec(dllexport)
	#else
		#define LABVIEW_DLLCALL __declspec(dllimport)
	#endif


	#ifdef __cplusplus 
		extern "C" {
	#endif


		LABVIEW_DLLCALL int printHelloworld(void* pObj, unsigned char* pBuf);

		//int Initiation();
		LABVIEW_DLLCALL void* CreateObject_Labview();
		//int Release();
		LABVIEW_DLLCALL void DestoryObject_Labview(void** pObj);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Step_02
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Alignment mthod_Pattern Matach
		LABVIEW_DLLCALL int Align_PatternMatch(void* pObj, LPImageInfo pIn, LPImageInfo pTemplat, seRect roiSearch, LPBoundingBox pFMarkBoxOut, double* dbScoreOut);
		LABVIEW_DLLCALL int Align_FindProfile(void* pObj, LPImageInfo pIn, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iKSize, int iSelLineNo, int* CntArryOut, int* p1DArrayOut);
		LABVIEW_DLLCALL int Align_DetectCircle(void* pObj, LPImageInfo pIn, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize, LPBoundingBox pFMarkBoxOut);

		// InspecBoxSetup_Annulus
		LABVIEW_DLLCALL int InspectBox_Annulus(void* pObj, LPImageInfo pIn, seAnnulus roiAnnulus, LPImageInfo pOut, LPBoundingBox pBoundBoxOut);
		LABVIEW_DLLCALL int InspectBox_Rect(void* pObj, LPImageInfo pIn, seBoundingBox roiRect, LPImageInfo pOut, LPBoundingBox pBoundBoxOut);
		LABVIEW_DLLCALL int InspectBox_Circle(void* pObj, LPImageInfo pIn, seCircle roiCircle, LPImageInfo pOut, LPBoundingBox pBoundBoxOut);

		// calculate the dependency coordinates 
		LABVIEW_DLLCALL int CoordBind_CoordCalculate(void* pObj, seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn, LPCoordBindBox pCoorBindBoxOut);

		//Cropping image by ROI from Source Image.
		LABVIEW_DLLCALL int CoordBind_CropROI_Annulus(void* pObj, LPImageInfo pIn, seCoordBindBox seCoordBox, seAnnulus roiAnnulus, LPImageInfo pOut, LPAnnulus pRoiOffset_Annulus);
		LABVIEW_DLLCALL int CoordBind_CropROI_Rect(void* pObj, LPImageInfo pIn, seCoordBindBox seCoordBox, seBoundingBox roiRect, LPImageInfo pOut, LPBoundingBox pRoiOffset_Rect);
		LABVIEW_DLLCALL int CoordBind_CropROI_Circle(void* pObj, LPImageInfo pIn, seCoordBindBox seCoordBox, seCircle roiCircle, LPImageInfo pOut, LPCircle pRoiOffset_Circle);



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Step_03
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Image Processing_(Threshold)
		LABVIEW_DLLCALL int ImgProc_Threshold(void* pObj, LPImageInfo pIn, double* pThresh, double* pMaxVal, emThresholdTypes emTypes, LPImageInfo pOut);

		// Image Processing_( Histogram )
		LABVIEW_DLLCALL int ImgProc_Histogram(void* pObj, LPImageInfo pIn, double* p1DArray);
		LABVIEW_DLLCALL int ImgProc_Histogram_Annulus(void* pObj, LPImageInfo pIn, seAnnulus roiAnnulus, double* p1DArray);
		LABVIEW_DLLCALL int ImgProc_Histogram_Rect(void* pObj, LPImageInfo pIn, seBoundingBox roiRect, double* p1DArray);
		LABVIEW_DLLCALL int ImgProc_Histogram_Circle(void* pObj, LPImageInfo pIn, seCircle roiCircle, double* p1DArray);

		// Image Processing_( Morphology )
		LABVIEW_DLLCALL int ImgProc_Morphology(void* pObj, LPImageInfo pIn, emMorphShapes emShapes, int iKSize, emMorphOperation emOperation, LPImageInfo pOut);

		// Image Processing_( Morpholoy )( Fill Holes )
		LABVIEW_DLLCALL int ImgProc_NoiseRemoval(void* pObj, LPImageInfo pIn, double dbLimit_min, double dbLimit_max, LPImageInfo pOut);


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Step_4
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
		// Measure 
		LABVIEW_DLLCALL int Measure_GlueWidth_Annulus(void* pObj, LPImageInfo pIn, seAnnulus roiAnnuls, int stepSize, double* pLength_InnerOut, double* pLength_OuterOut, double* pGlueAreaOut);
		LABVIEW_DLLCALL int Measure_GlueWidth_Rect(void* pObj, LPImageInfo pIn, seBoundingBox roiRect, int stepSize, double* pLength_InnerOut, double* pLength_OuterOut, double* pGlueAreaOut);


	#ifdef __cplusplus
		}
	#endif



#elif defined (__GNUC__)

	#ifdef __cplusplus 
		extern "C" {
	#endif


		int printHelloworld(void* pObj, unsigned char* pBuf);

		//int Initiation();
		void* CreateObject_Labview();
		//int Release();
		void DestoryObject_Labview(void** pObj);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Step_02
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Alignment mthod_Pattern Matach
		int Align_PatternMatch(void* pObj, LPImageInfo pIn, LPImageInfo pTemplat, seRect roiSearch, LPBoundingBox pFMarkBoxOut, double* dbScoreOut);
		int Align_FindProfile(void* pObj, LPImageInfo pIn, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iKSize, int iSelLineNo, int* CntArryOut, int* p1DArrayOut);
		int Align_DetectCircle(void* pObj, LPImageInfo pIn, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize, LPBoundingBox pFMarkBoxOut);

		// InspecBoxSetup_Annulus
		int InspectBox_Annulus(void* pObj, LPImageInfo pIn, seAnnulus roiAnnulus, LPImageInfo pOut, LPBoundingBox pBoundBoxOut);
		int InspectBox_Rect(void* pObj, LPImageInfo pIn, seBoundingBox roiRect, LPImageInfo pOut, LPBoundingBox pBoundBoxOut);
		int InspectBox_Circle(void* pObj, LPImageInfo pIn, seCircle roiCircle, LPImageInfo pOut, LPBoundingBox pBoundBoxOut);

		// calculate the dependency coordinates 
		int CoordBind_CoordCalculate(void* pObj, seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn, LPCoordBindBox pCoorBindBoxOut);

		//Cropping image by ROI from Source Image.
		int CoordBind_CropROI_Annulus(void* pObj, LPImageInfo pIn, seCoordBindBox seCoordBox, seAnnulus roiAnnulus, LPImageInfo pOut, LPAnnulus pRoiOffset_Annulus);
		int CoordBind_CropROI_Rect(void* pObj, LPImageInfo pIn, seCoordBindBox seCoordBox, seBoundingBox roiRect, LPImageInfo pOut, LPBoundingBox pRoiOffset_Rect);
		int CoordBind_CropROI_Circle(void* pObj, LPImageInfo pIn, seCoordBindBox seCoordBox, seCircle roiCircle, LPImageInfo pOut, LPCircle pRoiOffset_Circle);



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Step_03
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Image Processing_(Threshold)
		int ImgProc_Threshold(void* pObj, LPImageInfo pIn, double* pThresh, double* pMaxVal, emThresholdTypes emTypes, LPImageInfo pOut);

		// Image Processing_( Histogram )
		int ImgProc_Histogram(void* pObj, LPImageInfo pIn, double* p1DArray);
		int ImgProc_Histogram_Annulus(void* pObj, LPImageInfo pIn, seAnnulus roiAnnulus, double* p1DArray);
		int ImgProc_Histogram_Rect(void* pObj, LPImageInfo pIn, seBoundingBox roiRect, double* p1DArray);
		int ImgProc_Histogram_Circle(void* pObj, LPImageInfo pIn, seCircle roiCircle, double* p1DArray);

		// Image Processing_( Morphology )
		int ImgProc_Morphology(void* pObj, LPImageInfo pIn, emMorphShapes emShapes, int iKSize, emMorphOperation emOperation, LPImageInfo pOut);

		// Image Processing_( Morpholoy )( Fill Holes )
		int ImgProc_NoiseRemoval(void* pObj, LPImageInfo pIn, double dbLimit_min, double dbLimit_max, LPImageInfo pOut);


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Step_4
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
		// Measure 
		int Measure_GlueWidth_Annulus(void* pObj, LPImageInfo pIn, seAnnulus roiAnnuls, int stepSize, double* pLength_InnerOut, double* pLength_OuterOut, double* pGlueAreaOut);
		int Measure_GlueWidth_Rect(void* pObj, LPImageInfo pIn, seBoundingBox roiRect, int stepSize, double* pLength_InnerOut, double* pLength_OuterOut, double* pGlueAreaOut);
		
	#ifdef __cplusplus
		}
	#endif


#endif //#if defined(_MSC_VER)	// /* Visual C++ */

