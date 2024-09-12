#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "../cvip.h"
#include "Method_NoneCamCtrl.h"



using namespace std;

static OverwriteRingBuffer<cv::Mat> g_owRingbuffer(6);

int CMethod_NoneCamCtrl::g_exPrev_W = 0;
int CMethod_NoneCamCtrl::g_exPrev_H = 0;
unsigned char* CMethod_NoneCamCtrl::g_pDataForRGB = nullptr;

//////////////////////////////////////////////////////////////////////////////
///  Hik vision Gige camera controller function
/////////////////////////////////////////////////////////////////////////////

CMethod_NoneCamCtrl::CMethod_NoneCamCtrl()
	: m_pCam(nullptr)
	, m_stDevList({ 0 })
	, m_global_cfg_ParamInfo(seGigECamConfig())
	, m_fLimit_ExposureTime_Min(15.0)			//define by the MVS of HikVision
	, m_fLimit_ExposureTime_Max(9.9995e+06)		//define by the MVS of HikVision
	, m_iFrameRate_Cnt(0)
	, m_RetryCnt(10)
{}


CMethod_NoneCamCtrl::~CMethod_NoneCamCtrl()
{
	GigECam_Release();
}


int CMethod_NoneCamCtrl::GigeCam_Init()
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	m_pCam = new int;  // allocate memory for an integer
	*m_pCam = 1;       // initialize the integer

	return nRet;
}


int CMethod_NoneCamCtrl::GigECam_SetConfig(const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::GigECam_GetConfig(LpGigECamConfig pParamOut)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}



int CMethod_NoneCamCtrl::GigECam_AcquireImages(string strFilePath)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}



int CMethod_NoneCamCtrl::GigECam_AcquireImages(cv::Mat& matImg)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}



int CMethod_NoneCamCtrl::GigECam_Release()
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	if (m_pCam) {
		delete m_pCam;
		m_pCam = nullptr;
	}

	return nRet;
}


int CMethod_NoneCamCtrl::GigECam_Strm_Prepare()
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::GigECam_Strm_Start()
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::GigECam_Strm_AcquireImages(string strFilePath)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::GigECam_Strm_AcquireImages(cv::Mat& matImg)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::GigECam_Strm_Stop()
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::GigECam_Strm_Close()
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::PrintDeviceInfo(int* pCam)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::Configure_Default(int* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}



int CMethod_NoneCamCtrl::Configure_ImageFormat(int* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::Configure_Binning(int* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}



int CMethod_NoneCamCtrl::Configure_Decimation(int* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::Configure_Exposure(int* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::Configure_FrameRate(int* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::Configure_TriggerMode(int* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}



int CMethod_NoneCamCtrl::Configure_PersistentIP(int* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::Configure_ResetConfigureCustomImageSettings(int* pCam)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::Configure_ResetExposure(int* pCam)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::Configure_Get(int* pCam, LpGigECamConfig pParamOut, bool bDumpInf)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::AcquireImages(int* pCam, string strFilePath)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}



int CMethod_NoneCamCtrl::AcquireImages(int* pCam, cv::Mat& matImg)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}



int CMethod_NoneCamCtrl::AcquireStreaming(int* pCam, cv::Mat& matImg)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::AcquireStreaming_Prepare()
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return 0;
}


int CMethod_NoneCamCtrl::AcquireStreaming_StartorStop(bool bflgEnableThd)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return 0;
}


int CMethod_NoneCamCtrl::AcquireStreaming_Capture(cv::Mat& matImg)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}


int CMethod_NoneCamCtrl::AcquireStreaming_Close()
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return 0;
}


int CMethod_NoneCamCtrl::Thread_Acquire(int* pCam, LpGigECamConfig pParamOut)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return 0;
}


int CMethod_NoneCamCtrl::Thread_TriggerMode(int* pCam, LpGigECamConfig pParamOut)
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return 0;
}

int CMethod_NoneCamCtrl::GigECam_DebugPrint()
{
	int nRet = 0;

	CAMW("This class does not have a camera handle for use [%x]\n", nRet);

	return 0;
}