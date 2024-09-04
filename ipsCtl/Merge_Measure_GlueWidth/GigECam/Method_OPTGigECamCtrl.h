#pragma once

#include "GigECamDataStructureDef.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include "../ThirdPartyLibrary/OverwriteRingBuffer.h"

/* Optmv header*/
#include "OPTApi.h"

using namespace std;
using namespace cv;



class CMethod_OPTGigECamCtrl {

	std::mutex u_mutex;
	std::condition_variable cond_var;
	std::mutex mtx_triggrt;
	std::condition_variable cv_trigger;
	std::mutex mtx_triggrt_end;
	std::condition_variable cv_trigger_end;
	bool notified_IsDone = false;
	bool notified_IsRun = false;
	bool notified_IsTrigger = false;
	std::thread::id tid;
	bool bIsCreated = false;
	unsigned long iCnt_0 = 0;
	unsigned long iCnt_1 = 0;


	static int g_exPrev_W;
	static int g_exPrev_H;
	static OPT_EPixelType g_exConvertFormat;
	static unsigned char* g_pDataForRGB;


public:
	CMethod_OPTGigECamCtrl();
	~CMethod_OPTGigECamCtrl();


public:
	int GigeCam_Init();
	int GigECam_SetConfig(const LpGigECamConfig pParamIn);
	int GigECam_GetConfig(LpGigECamConfig pParamOut);
	int GigECam_AcquireImages(string strFilePath);
	int GigECam_AcquireImages(cv::Mat& matImg);
	int GigECam_Release();


	int GigECam_Strm_Prepare();
	int GigECam_Strm_Start();
	int GigECam_Strm_AcquireImages(string strFilePath);
	int GigECam_Strm_AcquireImages(cv::Mat& matImg);
	int GigECam_Strm_Stop();
	int GigECam_Strm_Close();



	static int GigECam_DebugPrint();

private:
	static int PrintDeviceInfo(OPT_DeviceList deviceInfoList);
	static void AcquireCallBackEx(OPT_Frame* pFrame, void* pUser);

	int Configure_Default(OPT_HANDLE pCam, const LpGigECamConfig pParamIn);
	int Configure_ImageFormat(OPT_HANDLE pCam, const LpGigECamConfig pParamIn);
	int Configure_Binning(OPT_HANDLE pCam, const LpGigECamConfig pParamIn);
	int Configure_Decimation(OPT_HANDLE pCam, const LpGigECamConfig pParamIn);
	int Configure_Exposure(OPT_HANDLE pCam, const LpGigECamConfig pParamIn);
	int Configure_PersistentIP(OPT_HANDLE pCam, const LpGigECamConfig pParamIn);
	int Configure_ResetConfigureCustomImageSettings(OPT_HANDLE pCam);
	int Configure_ResetExposure(OPT_HANDLE pCam);
	int Configure_Get(OPT_HANDLE pCam, LpGigECamConfig pParamOut, bool bDumpInf = 0);
	int Configure_FrameRate(OPT_HANDLE pCam, const LpGigECamConfig pParamIn);
	int Configure_TriggerMode(OPT_HANDLE pCam, const LpGigECamConfig pParamIn);


	int AcquireImages(OPT_HANDLE pCam, string strFilePath);
	int AcquireImages(OPT_HANDLE pCam, cv::Mat& matImg);
	int AcquireStreaming(OPT_HANDLE pCam, cv::Mat& matImg);
	

	int AcquireStreaming_Prepare();
	int AcquireStreaming_StartorStop(bool bflgEnableThd);
	int AcquireStreaming_Capture(cv::Mat& matImg);
	int AcquireStreaming_Close();

	int Thread_Acquire(OPT_HANDLE pCam, LpGigECamConfig pParamOut);

	int imageConvert(OPT_HANDLE devHandle, OPT_Frame frame, cv::Mat& matImg);

private:
	string m_strFilePath;

	seGigECamConfig m_global_cfg_ParamInfo;

	double m_fLimit_ExposureTime_Min;
	double m_fLimit_ExposureTime_Max;

	OPT_HANDLE m_pCam;

	OPT_DeviceList m_stDevList;

	unsigned long m_iFrameRate_Cnt;

	int m_RetryCnt;

};
