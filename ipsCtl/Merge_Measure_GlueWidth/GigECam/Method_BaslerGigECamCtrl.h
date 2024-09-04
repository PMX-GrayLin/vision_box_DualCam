#pragma once

#include "GigECamDataStructureDef.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opencv2/opencv.hpp>

// Include files to use the pylon API.
#include <pylon/PylonIncludes.h>

// Include file to use pylon universal instant camera parameters.
#include <pylon/BaslerUniversalInstantCamera.h>

#include "../ThirdPartyLibrary/OverwriteRingBuffer.h"

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using pylon universal instant camera parameters.
using namespace Basler_UniversalCameraParams;

// Namespace for using cout.
using namespace std;
using namespace cv;



class CMethod_BaslerGigECamCtrl {

	std::mutex u_mutex;
	std::condition_variable cond_var;
	bool notified_IsDone = false;
	bool notified_IsRun = false;
	std::thread::id tid;
	bool bIsCreated = false;
	int iCnt_0 = 0;
	int iCnt_1 = 0;

public:
	CMethod_BaslerGigECamCtrl();
	~CMethod_BaslerGigECamCtrl();


public:
	int GigeCam_Init();
	int GigECam_SetConfig(const LpGigECamConfig pParamIn);
	int GigECam_GetConfig(LpGigECamConfig pParamOut);
	int GigECam_AcquireImages(string strFilePath);
	int GigECam_AcquireImages(cv::Mat& matImg);
	int GigECam_Release();


	int GigECam_Strm_Prepare();
	int GigECam_Strm_Start();
	int GigECam_Strm_Stop();
	int GigECam_Strm_Close();



	static int GigECam_DebugPrint();

private:
	int PrintDeviceInfo(CBaslerUniversalInstantCamera* pCam);

	int Configure_ImageFormat(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn);
	int Configure_Binning(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn);
	int Configure_Decimation(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn);
	int Configure_Exposure(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn);
	int Configure_PersistentIP(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn);
	int Configure_ResetConfigureCustomImageSettings(CBaslerUniversalInstantCamera* pCam);
	int Configure_ResetExposure(CBaslerUniversalInstantCamera* pCam);
	int Configure_Get(CBaslerUniversalInstantCamera* pCam, LpGigECamConfig pParamOut, bool bDumpInf = 0);
	int Configure_FrameRate(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn);


	int AcquireImages(CBaslerUniversalInstantCamera* pCam, string strFilePath);
	int AcquireImages(CBaslerUniversalInstantCamera* pCam, cv::Mat& matImg);
	int AcquireStreaming(CBaslerUniversalInstantCamera* pCam, cv::Mat& matImg);


	int AcquireStreaming_Prepare();
	int AcquireStreaming_StartorStop(bool bflgEnableThd);
	int AcquireStreaming_Close();



	int Thread_Acquire(CBaslerUniversalInstantCamera* pCam, LpGigECamConfig pParamOut);



	//int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);
	//int Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData, cv::Mat& mat_img);

private:
	string m_strFilePath;

	seGigECamConfig m_global_cfg_ParamInfo;

	double m_fLimit_ExposureTime_Min;
	double m_fLimit_ExposureTime_Max;

	CBaslerUniversalInstantCamera* m_pCam;


	unsigned long m_iFrameRate_Cnt;
	int m_RetryCnt;

};
