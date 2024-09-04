#include "Measure_GlueWidth_DLL.h"  //Interface class for LabView
#include "Measure_GlueWidth_InterfaceClass.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <string>
#include <stdarg.h>
#include "cvip.h"


using namespace std;


void* CreateObject_Labview() 
{

#if defined(_MSC_VER)	// /* Visual C++ */

	void* phandle = static_cast<PTWDLL_I_GlueWidth*>(CreateObject_Class());
	
	char szInfo[200] = {'\0'};

	format(szInfo, strlen(szInfo), "CreateObject_Labview__Handle ==> 0x%llx", phandle);
	static_cast<PTWDLL_I_GlueWidth*>(phandle)->EnableLogger(0, 2, "d:\\myptwdll_log.txt"); //erase and disable
	static_cast<PTWDLL_I_GlueWidth*>(phandle)->EnableLogger(1, 2, "d:\\myptwdll_log.txt"); // enable	
	static_cast<PTWDLL_I_GlueWidth*>(phandle)->LogMsg(szInfo);

	return phandle;

#elif defined (__GNUC__)		//__GNUC__

	return CreateObject_Class();

#endif

}

void DestoryObject_Labview(void** pObj)
{
	if (pObj != nullptr && *pObj != nullptr) {

		delete static_cast<PTWDLL_I_GlueWidth*>(*pObj);
		*pObj = nullptr;
	}
}

int printHelloworld(void* pObj, unsigned char* pBuf)
{
	if (pBuf == nullptr) 
		return ER_ABORT;

	char szInfo[200] = { "" };
	//sprintf_s(szInfo, sizeof(szInfo), "printHelloworld()__Handle ==> 0x%llx", pObj);
	format(szInfo, strlen(szInfo), "printHelloworld()__Handle ==> 0x%llx", pObj);
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(szInfo);


	static_cast<PTWDLL_I_GlueWidth*>(pObj)->printHelloWorld(pBuf);

	std::string strTmp((char*)pBuf);
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strTmp.c_str());

	return 0;
}

void EnableLogger(void* pObj, unsigned char enable, unsigned short level, const char* filepath)
{
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->EnableLogger(enable, level, filepath);
}

void LogMsg(void* pObj, const char* msg, unsigned short level)
{
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(msg, level);
}

///////////////////////////////////////////////////////////////////////////////////////////
// Algorithm
///////////////////////////////////////////////////////////////////////////////////////////

// Alignment mthod_Pattern Matach
int Align_PatternMatch(void* pObj, LPImageInfo pIn, LPImageInfo pTemplat, seRect roiSearch, LPBoundingBox pFMarkBoxOut, double* dbScoreOut)
{
	int res = 0;

	//static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== Align_PatternMatch(...) ======== ");
	printD("======== Align_PatternMatch(...) ======== >>> \n");

    printD("Align_Pattern_SetParameter() \n");
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->Align_Pattern_SetParameter(roiSearch, pTemplat);
	if (res < ER_OK) {
		//static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
	    printD(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}

    printD("Align_Pattern_TestIt() \n");	
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->Align_Pattern_TestIt(pIn);
	if (res < ER_OK) {
		//static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		printD(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}

    printD("Align_Pattern_GetResult() \n");	
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->Align_Pattern_GetResult(pFMarkBoxOut, dbScoreOut);
	if (res < ER_OK) {
		//static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		printD(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}


	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
/*	
#if( _Enb_DebugMode)


	//result image Output. 
	cv::Mat matSrc, matTmp, matTemplat;
	CCVIPItem::Uint8ToCvMat(pIn, matSrc);
	CCVIPItem::Uint8ToCvMat_ColorScalar(pIn, matTmp);
	CCVIPItem::Uint8ToCvMat_ColorScalar(pTemplat, matTemplat);


	seBoundingBox tmpBB = *pFMarkBoxOut;
	cv::Point posStart(tmpBB.rectBox.left, tmpBB.rectBox.top);
	cv::Point posEnd(tmpBB.rectBox.right, tmpBB.rectBox.bottom);
	cv::rectangle(matTmp, posStart, posEnd, cv::Scalar(0, 0, 255), 2);

	string strFullPath[3];
	CCVIPItem::SaveTestImage("IPL_Alignment", "SrcImg_PatternMatch", matSrc, strFullPath[0]);
	CCVIPItem::SaveTestImage("IPL_Alignment", "Results_PatternMatch", matTmp, strFullPath[1]);
	CCVIPItem::SaveTestImage("IPL_Alignment", "SrcImg_Template", matTemplat, strFullPath[2]);

	//dump information to Log().
	std::string strLog;
	strLog += "\r\n";
	strLog += "Parameter:\r\n";
	strLog += "pIn->iColorSpace = " + std::to_string(pIn->iColorSpace) + "\r\n";
	strLog += "pIn->iChannels = " + std::to_string(pIn->iChannels) + "\r\n";
	strLog += "pTemplat->iColorSpace = " + std::to_string(pTemplat->iColorSpace) + "\r\n";
	strLog += "pTemplat->iChannels = " + std::to_string(pTemplat->iChannels) + "\r\n";

	strLog += "Results:\r\n";
	strLog += "pFMarkBoxOut->cX = " + std::to_string(pFMarkBoxOut->cX) + "\r\n";
	strLog += "pFMarkBoxOut->cY = " + std::to_string(pFMarkBoxOut->cY) + "\r\n";
	strLog += "pFMarkBoxOut->dbAngle = " + std::to_string(pFMarkBoxOut->dbAngle) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.left = " + std::to_string(pFMarkBoxOut->rectBox.left) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.top = " + std::to_string(pFMarkBoxOut->rectBox.top) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.right = " + std::to_string(pFMarkBoxOut->rectBox.right) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.bottom = " + std::to_string(pFMarkBoxOut->rectBox.bottom) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.width = " + std::to_string(pFMarkBoxOut->rectBox.width) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.height = " + std::to_string(pFMarkBoxOut->rectBox.height) + "\r\n";
	strLog += "dbScoreOut = " + std::to_string(*dbScoreOut) + "\r\n";
	strLog += "Input_ImagePath = " + strFullPath[0] + "\r\n";
	strLog += "ResImagePath = " + strFullPath[1] + "\r\n";
	strLog += "Input_templateImg = " + strFullPath[2] + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif //#if( _Enb_DebugMode)
*/

	printD("======== Align_PatternMatch(...) ======== <<< \n");


	return res;
}

int Align_FindProfile(void* pObj, LPImageInfo pIn, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iKSize, int iSelLineNo, int* CntArryOut, int* p1DArrayOut)
{
	int res = 0;

	//static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== Align_FindProfile(...) ======== ");
	printD("======== Align_FindProfile(...) ======== >>> \n");

	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->Align_FindProfile_SetPattern(roiSearch, roiMask, bDirection, bPolarity, stepSize, 0, iKSize);
	if (res < ER_OK) {
		//static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		printD(" SetParameter(...)  ===> ER_ABORT !!!\n");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->Align_FindProfile_TestIt(pIn, iSelLineNo);
	if (res < ER_OK) {
		//static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		printD(" TestIt(...)  ===> ER_ABORT !!!\n");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->Align_FindProfile_GetResult(CntArryOut, p1DArrayOut);
	if (res < ER_OK) {
		//static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		printD(" GetResult(...)  ===> ER_ABORT !!!\n");
		return ER_ABORT;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	//result image Output. 
	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "CntArryOut = " + std::to_string(*CntArryOut) + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif //#if( _Enb_DebugMode)

	printD("======== Align_FindProfile(...) ======== <<< \n");

	return res;
}

int Align_DetectCircle(void* pObj, LPImageInfo pIn, seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize, LPBoundingBox pFMarkBoxOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== Align_DetectCircle(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->Align_DetectCircle_SetPattern(roiSearch, roiMask, bDirection, bPolarity, stepSize, iMinEdgeStrength, iKSize);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->Align_DetectCircle_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->Align_DetectCircle_GetResult(pFMarkBoxOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}


	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat_ColorScalar(pIn, matTmp);


	seBoundingBox tmpBB = *pFMarkBoxOut;
	cv::Point posStart(tmpBB.rectBox.left, tmpBB.rectBox.top);
	cv::Point posEnd(tmpBB.rectBox.right, tmpBB.rectBox.bottom);
	cv::rectangle(matTmp, posStart, posEnd, cv::Scalar(0, 0, 255), 2);

	string strFullPath;
	CCVIPItem::SaveTestImage("IPL_Alignment", "Results_DetectCircle", matTmp, strFullPath);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Parameter:\r\n";
	strLog += "pIn->iColorSpace = " + std::to_string(pIn->iColorSpace) + "\r\n";
	strLog += "pIn->iChannels = " + std::to_string(pIn->iChannels) + "\r\n";

	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "pFMarkBoxOut->cX = " + std::to_string(pFMarkBoxOut->cX) + "\r\n";
	strLog += "pFMarkBoxOut->cY = " + std::to_string(pFMarkBoxOut->cY) + "\r\n";
	strLog += "pFMarkBoxOut->dbAngle = " + std::to_string(pFMarkBoxOut->dbAngle) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.left = " + std::to_string(pFMarkBoxOut->rectBox.left) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.top = " + std::to_string(pFMarkBoxOut->rectBox.top) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.right = " + std::to_string(pFMarkBoxOut->rectBox.right) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.bottom = " + std::to_string(pFMarkBoxOut->rectBox.bottom) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.width = " + std::to_string(pFMarkBoxOut->rectBox.width) + "\r\n";
	strLog += "pFMarkBoxOut->rectBox.height = " + std::to_string(pFMarkBoxOut->rectBox.height) + "\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());


#endif	//#if( _Enb_DebugMode)


	return res;
}


// InspecBoxSetup_Annulus
int InspectBox_Annulus(void* pObj, LPImageInfo pIn, seAnnulus roiAnnulus, LPImageInfo pOut, LPBoundingBox pBoundBoxOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== InspectBox_Annulus(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->InspectBox_Annulus_SetParameter(roiAnnulus);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->InspectBox_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->InspectBox_GetResult(pBoundBoxOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}


	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)
	
	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat_ColorScalar(pIn, matTmp);


	seBoundingBox tmpBB = *pBoundBoxOut;
	cv::Point posStart(tmpBB.rectBox.left, tmpBB.rectBox.top);
	cv::Point posEnd(tmpBB.rectBox.right, tmpBB.rectBox.bottom);
	cv::rectangle(matTmp, posStart, posEnd, cv::Scalar(0, 0, 255), 2);

	string strFullPath;
	CCVIPItem::SaveTestImage("IPL_Alignment", "Results_InspectBox_Annulus", matTmp, strFullPath);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "pBoundBoxOut->cX = " + std::to_string(pBoundBoxOut->cX) + "\r\n";
	strLog += "pBoundBoxOut->cY = " + std::to_string(pBoundBoxOut->cY) + "\r\n";
	strLog += "pBoundBoxOut->dbAngle = " + std::to_string(pBoundBoxOut->dbAngle) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.left = " + std::to_string(pBoundBoxOut->rectBox.left) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.top = " + std::to_string(pBoundBoxOut->rectBox.top) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.right = " + std::to_string(pBoundBoxOut->rectBox.right) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.bottom = " + std::to_string(pBoundBoxOut->rectBox.bottom) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.width = " + std::to_string(pBoundBoxOut->rectBox.width) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.height = " + std::to_string(pBoundBoxOut->rectBox.height) + "\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());


#endif	//#if( _Enb_DebugMode)

	return res;
}

int InspectBox_Rect(void* pObj, LPImageInfo pIn, seBoundingBox roiRect, LPImageInfo pOut, LPBoundingBox pBoundBoxOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== InspectBox_Rect(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->InspectBox_Rectangle_SetParameter(roiRect);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->InspectBox_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->InspectBox_GetResult(pBoundBoxOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}


	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	
	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat_ColorScalar(pIn, matTmp);


	seBoundingBox tmpBB = *pBoundBoxOut;
	cv::Point posStart(tmpBB.rectBox.left, tmpBB.rectBox.top);
	cv::Point posEnd(tmpBB.rectBox.right, tmpBB.rectBox.bottom);
	cv::rectangle(matTmp, posStart, posEnd, cv::Scalar(0, 0, 255), 2);

	string strFullPath;
	CCVIPItem::SaveTestImage("IPL_Alignment", "Results_InspectBox_Rect", matTmp, strFullPath);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "pBoundBoxOut->cX = " + std::to_string(pBoundBoxOut->cX) + "\r\n";
	strLog += "pBoundBoxOut->cY = " + std::to_string(pBoundBoxOut->cY) + "\r\n";
	strLog += "pBoundBoxOut->dbAngle = " + std::to_string(pBoundBoxOut->dbAngle) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.left = " + std::to_string(pBoundBoxOut->rectBox.left) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.top = " + std::to_string(pBoundBoxOut->rectBox.top) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.right = " + std::to_string(pBoundBoxOut->rectBox.right) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.bottom = " + std::to_string(pBoundBoxOut->rectBox.bottom) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.width = " + std::to_string(pBoundBoxOut->rectBox.width) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.height = " + std::to_string(pBoundBoxOut->rectBox.height) + "\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif //#if( _Enb_DebugMode)

	return res;
}

int InspectBox_Circle(void* pObj, LPImageInfo pIn, seCircle roiCircle, LPImageInfo pOut, LPBoundingBox pBoundBoxOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== InspectBox_Circle(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->InspectBox_Circle_SetParameter(roiCircle);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->InspectBox_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->InspectBox_GetResult(pBoundBoxOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}


	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)
	
	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat_ColorScalar(pIn, matTmp);


	seBoundingBox tmpBB = *pBoundBoxOut;
	cv::Point posStart(tmpBB.rectBox.left, tmpBB.rectBox.top);
	cv::Point posEnd(tmpBB.rectBox.right, tmpBB.rectBox.bottom);
	cv::rectangle(matTmp, posStart, posEnd, cv::Scalar(0, 0, 255), 2);

	string strFullPath;
	CCVIPItem::SaveTestImage("IPL_Alignment", "Results_InspectBox_Circle", matTmp, strFullPath);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "pBoundBoxOut->cX = " + std::to_string(pBoundBoxOut->cX) + "\r\n";
	strLog += "pBoundBoxOut->cY = " + std::to_string(pBoundBoxOut->cY) + "\r\n";
	strLog += "pBoundBoxOut->dbAngle = " + std::to_string(pBoundBoxOut->dbAngle) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.left = " + std::to_string(pBoundBoxOut->rectBox.left) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.top = " + std::to_string(pBoundBoxOut->rectBox.top) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.right = " + std::to_string(pBoundBoxOut->rectBox.right) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.bottom = " + std::to_string(pBoundBoxOut->rectBox.bottom) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.width = " + std::to_string(pBoundBoxOut->rectBox.width) + "\r\n";
	strLog += "pBoundBoxOut->rectBox.height = " + std::to_string(pBoundBoxOut->rectBox.height) + "\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif	//#if( _Enb_DebugMode)


	return res;
}


// calculate the dependency coordinates 
int CoordBind_CoordCalculate(void* pObj, seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn, LPCoordBindBox pCoorBindBoxOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== CoordBind_CoordCalculate(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_Calcu_SetParameter(seFMarkBox, seInspBox, seCoorBindBoxIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_Calcu_TestIt();
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_Calcu_GetResult(pCoorBindBoxOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)


	//result image Output. 
	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";

	strLog += "pCoorBindBoxOut->iDelta_W = " + std::to_string(pCoorBindBoxOut->CalibCoord.iDelta_W) + "\r\n";
	strLog += "pCoorBindBoxOut->iDelta_H = " + std::to_string(pCoorBindBoxOut->CalibCoord.iDelta_H) + "\r\n";
	strLog += "pCoorBindBoxOut->dbAngle = " + std::to_string(pCoorBindBoxOut->CalibCoord.dbAngle) + "\r\n";

	strLog += "FMark Info.\r\n";
	strLog += "seCoorBindBoxIn.iDelta_W = " + std::to_string(seCoorBindBoxIn.CalibCoord.iDelta_W) + "\r\n";
	strLog += "seCoorBindBoxIn.iDelta_H = " + std::to_string(seCoorBindBoxIn.CalibCoord.iDelta_H) + "\r\n";
	strLog += "seCoorBindBoxIn.dbAngle = " + std::to_string(seCoorBindBoxIn.CalibCoord.dbAngle) + "\r\n";

	strLog += "FMark Info.\r\n";
	strLog += "pCoorBindBoxOut->FMark.cX = " + std::to_string(pCoorBindBoxOut->FMark.cX) + "\r\n";
	strLog += "pCoorBindBoxOut->FMark.cY = " + std::to_string(pCoorBindBoxOut->FMark.cY) + "\r\n";
	strLog += "pCoorBindBoxOut->FMark.dbAngle = " + std::to_string(pCoorBindBoxOut->FMark.dbAngle) + "\r\n";
	strLog += "pCoorBindBoxOut->FMark.rectBox.left = " + std::to_string(pCoorBindBoxOut->FMark.rectBox.left) + "\r\n";
	strLog += "pCoorBindBoxOut->FMark.rectBox.top = " + std::to_string(pCoorBindBoxOut->FMark.rectBox.top) + "\r\n";
	strLog += "pCoorBindBoxOut->FMark.rectBox.right = " + std::to_string(pCoorBindBoxOut->FMark.rectBox.right) + "\r\n";
	strLog += "pCoorBindBoxOut->FMark.rectBox.bottom = " + std::to_string(pCoorBindBoxOut->FMark.rectBox.bottom) + "\r\n";
	strLog += "pCoorBindBoxOut->FMark.rectBox.width = " + std::to_string(pCoorBindBoxOut->FMark.rectBox.width) + "\r\n";
	strLog += "pCoorBindBoxOut->FMark.rectBox.height = " + std::to_string(pCoorBindBoxOut->FMark.rectBox.height) + "\r\n";

	strLog += "InsptBox Info.\r\n";
	strLog += "pCoorBindBoxOut->InsptBox.cX = " + std::to_string(pCoorBindBoxOut->InsptBox.cX) + "\r\n";
	strLog += "pCoorBindBoxOut->InsptBox.cY = " + std::to_string(pCoorBindBoxOut->InsptBox.cY) + "\r\n";
	strLog += "pCoorBindBoxOut->InsptBox.dbAngle = " + std::to_string(pCoorBindBoxOut->InsptBox.dbAngle) + "\r\n";
	strLog += "pCoorBindBoxOut->InsptBox.rectBox.left = " + std::to_string(pCoorBindBoxOut->InsptBox.rectBox.left) + "\r\n";
	strLog += "pCoorBindBoxOut->InsptBox.rectBox.top = " + std::to_string(pCoorBindBoxOut->InsptBox.rectBox.top) + "\r\n";
	strLog += "pCoorBindBoxOut->InsptBox.rectBox.right = " + std::to_string(pCoorBindBoxOut->InsptBox.rectBox.right) + "\r\n";
	strLog += "pCoorBindBoxOut->InsptBox.rectBox.bottom = " + std::to_string(pCoorBindBoxOut->InsptBox.rectBox.bottom) + "\r\n";
	strLog += "pCoorBindBoxOut->InsptBox.rectBox.width = " + std::to_string(pCoorBindBoxOut->InsptBox.rectBox.width) + "\r\n";
	strLog += "pCoorBindBoxOut->InsptBox.rectBox.height = " + std::to_string(pCoorBindBoxOut->InsptBox.rectBox.height) + "\r\n";

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif	//#if( _Enb_DebugMode)


	return 0;
}


//Cropping image by ROI from Source Image.
int CoordBind_CropROI_Annulus(void* pObj, LPImageInfo pIn, seCoordBindBox seCoordBox, seAnnulus roiAnnulus, LPImageInfo pOut, LPAnnulus pRoiOffset_Annulus)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== CoordBind_CropROI_Annulus(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_CropROI_Annulus_SetParameter(seCoordBox, roiAnnulus);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_CropROI_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_CropROI_Annulus_GetResult(pOut, pRoiOffset_Annulus);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}


	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	
	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat(pOut, matTmp);

	string strFullPath;
	CCVIPItem::SaveTestImage("IPL_Alignment", "Results_CropROI_Annulus", matTmp, strFullPath);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif	//#if( _Enb_DebugMode)

	return 0;
}

int CoordBind_CropROI_Rect(void* pObj, LPImageInfo pIn, seCoordBindBox seCoordBox, seBoundingBox roiRect, LPImageInfo pOut, LPBoundingBox pRoiOffset_Rect)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== CoordBind_CropROI_Rect(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_CropROI_Rectangle_SetParameter(seCoordBox, roiRect);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_CropROI_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_CropROI_Rectangle_GetResult(pOut, pRoiOffset_Rect);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	
	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat(pOut, matTmp);

	string strFullPath;
	CCVIPItem::SaveTestImage("IPL_Alignment", "Results_CropROI_Rect", matTmp, strFullPath);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif	//#if( _Enb_DebugMode)


	return 0;
}

int CoordBind_CropROI_Circle(void* pObj, LPImageInfo pIn, seCoordBindBox seCoordBox, seCircle roiCircle, LPImageInfo pOut, LPCircle pRoiOffset_Circle)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== CoordBind_CropROI_Circle(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_CropROI_Circle_SetParameter(seCoordBox, roiCircle);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_CropROI_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->CoordBind_CropROI_Circle_GetResult(pOut, pRoiOffset_Circle);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	
	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat(pOut, matTmp);

	string strFullPath;
	CCVIPItem::SaveTestImage("IPL_Alignment", "Results_CropROI_Circle", matTmp, strFullPath);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif //#if( _Enb_DebugMode)


	return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Step_03
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Image Processing_(Threshold)
int ImgProc_Threshold(void* pObj, LPImageInfo pIn, double* pThresh, double* pMaxVal, emThresholdTypes emTypes, LPImageInfo pOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== ImgProc_Threshold(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpThreshold_SetParameter(pIn->iChannels, pThresh, pMaxVal, emTypes);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpThreshold_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpThreshold_GetResult(pOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	
	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat(pOut, matTmp);


	std::string strFullPath;
	CCVIPItem::SaveTestImage("IPL_ImageProcessing", "Results_ImgProc_Threshold", matTmp, strFullPath);


	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif	//#if( _Enb_DebugMode)

	return 0;
}

// Image Processing_( Histogram )
int ImgProc_Histogram(void* pObj, LPImageInfo pIn, double* p1DArray)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== ImgProc_Histogram(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_SetParameter(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_TestIt();
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_GetResult(p1DArray);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)


	//result image Output. 
	int iChannels = pIn->iChannels;
	int histHeight = 256;	//�nø�s����Ϫ��̤j����
	int histSize = 256;		//����ϨC�@����bin�Ӽ�
	int scale = 2;			//����v�H���e�j�p

	char szColorName[3] = { 'R', 'G', 'B' };
	std::vector< std::string> vec_strFullPath(iChannels);

	cv::Mat histImg = cv::Mat::zeros(cv::Size(histSize * scale, histSize), CV_8UC1);//�Ω���ܪ����		

	for (int c = 0; c < iChannels; c++) {

		double* p = &p1DArray[c * histSize];
		
		for (size_t i = 0; i < histSize; i++)//�i�檽��Ϫ�ø�s
		{
			int intensity = p[i];
			
			for (size_t j = 0; j < scale; j++) //ø�s���u �o�̥ΨCscale�����u�N��@��bin
			{
				cv::line(histImg, cv::Point(i * scale + j, histHeight - intensity), cv::Point(i * scale + j, histHeight - 1), 255);
			}

		}

		//std::string strPath;
		std::string strImgName("Result_ImgProc_Histogram_");
		strImgName += szColorName[c];
		CCVIPItem::SaveTestImage("IPL_ImageProcessing", strImgName, histImg, vec_strFullPath.at(c));
		//vec_strFullPath.at(c) = strPath;

//#ifdef _RexTY_DEBUG
//
//		cv::imshow("histImg", histImg);
//		cv::waitKey(0);
//		cv::destroyAllWindows();
//#endif

	}

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	for (int i = 0; i < iChannels; i++) {

		strLog += "Histogram_ImagePath = " + vec_strFullPath.at(i) + "\r\n";
	}
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());


#endif	//#if( _Enb_DebugMode)

	return 0;
}

int ImgProc_Histogram_Annulus(void* pObj, LPImageInfo pIn, seAnnulus roiAnnulus, double* p1DArray)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== ImgProc_Histogram_Annulus(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_Annulus_SetParameter(pIn, roiAnnulus);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_TestIt();
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_GetResult(p1DArray);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)


	//result image Output. 
	int iChannels = pIn->iChannels;
	int histHeight = 256;	//�nø�s����Ϫ��̤j����
	int histSize = 256;		//����ϨC�@����bin�Ӽ�
	int scale = 2;			//����v�H���e�j�p

	char szColorName[3] = { 'R', 'G', 'B' };
	std::vector< std::string> vec_strFullPath(iChannels);

	cv::Mat histImg = cv::Mat::zeros(cv::Size(histSize * scale, histSize), CV_8UC1);//�Ω���ܪ����		

	for (int c = 0; c < iChannels; c++) {

		double* p = &p1DArray[c * histSize];

		for (size_t i = 0; i < histSize; i++)//�i�檽��Ϫ�ø�s
		{
			int intensity = p[i];

			for (size_t j = 0; j < scale; j++) //ø�s���u �o�̥ΨCscale�����u�N��@��bin
			{
				cv::line(histImg, cv::Point(i * scale + j, histHeight - intensity), cv::Point(i * scale + j, histHeight - 1), 255);
			}

		}

		//std::string strPath;
		std::string strImgName("Result_ImgProc_Histogram_");
		strImgName += szColorName[c];
		CCVIPItem::SaveTestImage("IPL_ImageProcessing", strImgName, histImg, vec_strFullPath.at(c));
		//vec_strFullPath.at(c) = strPath;

//#ifdef _RexTY_DEBUG
//
//		cv::imshow("histImg", histImg);
//		cv::waitKey(0);
//		cv::destroyAllWindows();
//#endif

	}

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	for (int i = 0; i < iChannels; i++) {

		strLog += "Histogram_ImagePath = " + vec_strFullPath.at(i) + "\r\n";
	}
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());


#endif	//#if( _Enb_DebugMode)

	return 0;
}

int ImgProc_Histogram_Rect(void* pObj, LPImageInfo pIn, seBoundingBox roiRect, double* p1DArray)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== ImgProc_Histogram_Rect(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_Rectangle_SetParameter(pIn, roiRect);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_TestIt();
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_GetResult(p1DArray);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)


	//result image Output. 
	int iChannels = pIn->iChannels;
	int histHeight = 256;	//�nø�s����Ϫ��̤j����
	int histSize = 256;		//����ϨC�@����bin�Ӽ�
	int scale = 2;			//����v�H���e�j�p

	char szColorName[3] = { 'R', 'G', 'B' };
	std::vector< std::string> vec_strFullPath(iChannels);

	cv::Mat histImg = cv::Mat::zeros(cv::Size(histSize * scale, histSize), CV_8UC1);//�Ω���ܪ����		

	for (int c = 0; c < iChannels; c++) {

		double* p = &p1DArray[c * histSize];

		for (size_t i = 0; i < histSize; i++)//�i�檽��Ϫ�ø�s
		{
			int intensity = p[i];

			for (size_t j = 0; j < scale; j++) //ø�s���u �o�̥ΨCscale�����u�N��@��bin
			{
				cv::line(histImg, cv::Point(i * scale + j, histHeight - intensity), cv::Point(i * scale + j, histHeight - 1), 255);
			}

		}

		//std::string strPath;
		std::string strImgName("Result_ImgProc_Histogram_");
		strImgName += szColorName[c];
		CCVIPItem::SaveTestImage("IPL_ImageProcessing", strImgName, histImg, vec_strFullPath.at(c));
		//vec_strFullPath.at(c) = strPath;

//#ifdef _RexTY_DEBUG
//
//		cv::imshow("histImg", histImg);
//		cv::waitKey(0);
//		cv::destroyAllWindows();
//#endif

	}

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	for (int i = 0; i < iChannels; i++) {

		strLog += "Histogram_ImagePath = " + vec_strFullPath.at(i) + "\r\n";
	}
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());


#endif	//#if( _Enb_DebugMode)

	return 0;
}

int ImgProc_Histogram_Circle(void* pObj, LPImageInfo pIn, seCircle roiCircle, double* p1DArray)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== ImgProc_Histogram_Circle(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_Circle_SetParameter(pIn, roiCircle);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_TestIt();
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpHistogram_GetResult(p1DArray);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)


	//result image Output. 
	int iChannels = pIn->iChannels;
	int histHeight = 256;	//�nø�s����Ϫ��̤j����
	int histSize = 256;		//����ϨC�@����bin�Ӽ�
	int scale = 2;			//����v�H���e�j�p

	char szColorName[3] = { 'R', 'G', 'B' };
	std::vector< std::string> vec_strFullPath(iChannels);

	cv::Mat histImg = cv::Mat::zeros(cv::Size(histSize * scale, histSize), CV_8UC1);//�Ω���ܪ����		

	for (int c = 0; c < iChannels; c++) {

		double* p = &p1DArray[c * histSize];

		for (size_t i = 0; i < histSize; i++)//�i�檽��Ϫ�ø�s
		{
			int intensity = p[i];

			for (size_t j = 0; j < scale; j++) //ø�s���u �o�̥ΨCscale�����u�N��@��bin
			{
				cv::line(histImg, cv::Point(i * scale + j, histHeight - intensity), cv::Point(i * scale + j, histHeight - 1), 255);
			}

		}

		//std::string strPath;
		std::string strImgName("Result_ImgProc_Histogram_");
		strImgName += szColorName[c];
		CCVIPItem::SaveTestImage("IPL_ImageProcessing", strImgName, histImg, vec_strFullPath.at(c));
		//vec_strFullPath.at(c) = strPath;

//#ifdef _RexTY_DEBUG
//
//		cv::imshow("histImg", histImg);
//		cv::waitKey(0);
//		cv::destroyAllWindows();
//#endif

	}

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	for (int i = 0; i < iChannels; i++) {

		strLog += "Histogram_ImagePath = " + vec_strFullPath.at(i) + "\r\n";
	}
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());


#endif	//#if( _Enb_DebugMode)

	return 0;
}


// Image Processing_( Morphology )
int ImgProc_Morphology(void* pObj, LPImageInfo pIn, emMorphShapes emShapes, int iKSize, emMorphOperation emOperation, LPImageInfo pOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== ImgProc_Morphology(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpMorphology_SetParameter(emShapes, iKSize);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpMorphology_TestIt(pIn, emOperation);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpMorphology_GetResult(pOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	
	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat(pOut, matTmp);

	string strFullPath;
	CCVIPItem::SaveTestImage("IPL_ImageProcessing", "Results_ImgProc_Morphology", matTmp, strFullPath);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif	//#if( _Enb_DebugMode)


	return res;
}

// Image Processing_( Morpholoy )( Fill Holes )
int ImgProc_NoiseRemoval(void* pObj, LPImageInfo pIn, double dbLimit_min, double dbLimit_max, LPImageInfo pOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== ImgProc_NoiseRemoval(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpNoiseRemoval_SetParameter(dbLimit_min, dbLimit_max);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpNoiseRemoval_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->IpNoiseRemoval_GetResult(pOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	
	//result image Output. 
	cv::Mat matTmp;
	CCVIPItem::Uint8ToCvMat(pOut, matTmp);

	string strFullPath;
	CCVIPItem::SaveTestImage("IPL_ImageProcessing", "Results_ImgProc_NoiseRemoval", matTmp, strFullPath);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "ResImagePath = " + strFullPath + "\r\n";
	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif	//#if( _Enb_DebugMode)

	return res;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Step_4
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// Measure 
int Measure_GlueWidth_Annulus(void* pObj, LPImageInfo pIn, seAnnulus roiAnnuls, int stepSize, double* pLength_InnerOut, double* pLength_OuterOut, double* pGlueAreaOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== Measure_GlueWidth(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->MeasGlueWidth_Annulus_SetParameter(roiAnnuls, stepSize);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->MeasGlueWidth_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->MeasGlueWidth_GetResult(pLength_InnerOut, pLength_OuterOut, pGlueAreaOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}


	std::vector< std::string> vec_strdumpInfo;
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->MeasGlueWidth_GetInfo4Dump(vec_strdumpInfo);
	if (res < ER_OK) return ER_ABORT;

	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)

	//result image Output. 
	int iCount = 0;
	if ((0 == roiAnnuls.dbEndAngle) && (0 == roiAnnuls.dbStartAngle)) {
		iCount = 360.0 / stepSize;
	}
	else {
		iCount = (roiAnnuls.dbEndAngle - roiAnnuls.dbStartAngle) / stepSize;
	}

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "PixelCount of Object = " + std::to_string(*pGlueAreaOut) + "\r\n";

	if (iCount > 0) {

		strLog += "GlueWidth length_Center to Inner = ";
		for (int i = 0; i < iCount; i++) {
			int iVal = pLength_InnerOut[i];
			strLog += std::to_string(iVal) + ", ";
		}
		strLog += "\r\n";

		strLog += "GlueWidth length_Center to Outer = ";
		for (int i = 0; i < iCount; i++) {
			int iVal = pLength_OuterOut[i];
			strLog += std::to_string(iVal) + ", ";
		}
		strLog += "\r\n";

	}

	for (int i = 0; i < vec_strdumpInfo.size(); i++) {

		strLog += vec_strdumpInfo.at(i) + "\r\n";
	}

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());


#endif	//#if( _Enb_DebugMode)


	return res;
}

int Measure_GlueWidth_Rect(void* pObj, LPImageInfo pIn, seBoundingBox roiRect, int stepSize, double* pLength_InnerOut, double* pLength_OuterOut, double* pGlueAreaOut)
{
	int res = 0;

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg("======== Measure_GlueWidth(...) ======== ");


	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->MeasGlueWidth_Rectangle_SetParameter(roiRect, stepSize);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" SetParameter(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->MeasGlueWidth_TestIt(pIn);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" TestIt(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->MeasGlueWidth_GetResult(pLength_InnerOut, pLength_OuterOut, pGlueAreaOut);
	if (res < ER_OK) {
		static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(" GetResult(...)  ===> ER_ABORT !!!");
		return ER_ABORT;
	}


	std::vector< std::string> vec_strdumpInfo;
	res = static_cast<PTWDLL_I_GlueWidth*>(pObj)->MeasGlueWidth_GetInfo4Dump(vec_strdumpInfo);
	if (res < ER_OK) return ER_ABORT;

	///////////////////////////////////////////////////////////////////////////////////////////
	// Information storage and Log dump.
	//////////////////////////////////////////////////////////////////////////////////////////
#if( _Enb_DebugMode)
	
	//result image Output. 
	int iCount = round(roiRect.rectBox.height / stepSize);

	std::string strLog;
	strLog += "\r\n";
	strLog += "Results:\r\n";
	strLog += "PixelCount of Object = " + std::to_string(*pGlueAreaOut) + "\r\n";

	if (iCount > 0) {

		strLog += "GlueWidth length_Center to Inner = ";
		for (int i = 0; i < iCount; i++) {
			int iVal = pLength_InnerOut[i];
			strLog += std::to_string(iVal) + ", ";
		}
		strLog += "\r\n";

		strLog += "GlueWidth length_Center to Outer = ";
		for (int i = 0; i < iCount; i++) {
			int iVal = pLength_OuterOut[i];
			strLog += std::to_string(iVal) + ", ";
		}
		strLog += "\r\n";

	}

	for (int i = 0; i < vec_strdumpInfo.size(); i++) {

		strLog += vec_strdumpInfo.at(i) + "\r\n";
	}

	static_cast<PTWDLL_I_GlueWidth*>(pObj)->LogMsg(strLog.c_str());

#endif	//#if( _Enb_DebugMode)


	return res;
}





