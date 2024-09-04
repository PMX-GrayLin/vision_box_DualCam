#pragma once

#include "GigECamDataStructureDef.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include "Handler_CamCtrl_Strategy.h"
#include "../ThirdPartyLibrary/OverwriteRingBuffer.h"
#include "../ThirdPartyLibrary/RestfulClient.h"

/* Hik Vision header*/
#include "MvCameraControl.h"

using namespace std;
using namespace cv;



class CMethod_HikGigECamCtrl : public CHandler_CamCtrl
{

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
	static unsigned char* g_pDataForRGB;


public:
	CMethod_HikGigECamCtrl();
	~CMethod_HikGigECamCtrl();


public:
	int GigeCam_Init() override;
	int GigECam_SetConfig(const LpGigECamConfig pParamIn)  override;
	int GigECam_GetConfig(LpGigECamConfig pParamOut)  override;
	int GigECam_AcquireImages(string strFilePath)  override;
	int GigECam_AcquireImages(cv::Mat& matImg)  override;
	int GigECam_Release() override;


	int GigECam_Strm_Prepare() override;
	int GigECam_Strm_Start()  override;
	int GigECam_Strm_AcquireImages(string strFilePath)  override;
	int GigECam_Strm_AcquireImages(cv::Mat& matImg)  override;
	int GigECam_Strm_Stop() override;
	int GigECam_Strm_Close() override;

	int GigECam_DebugPrint() override;



private:
	int PrintDeviceInfo(void* pCam);
	bool PrintDeviceInfo_HikVision(MV_CC_DEVICE_INFO* pstMVDevInfo);

	int Configure_Default(void* pCam, const LpGigECamConfig pParamIn);
	int Configure_ImageFormat(void* pCam, const LpGigECamConfig pParamIn);
	int Configure_Binning(void* pCam, const LpGigECamConfig pParamIn);
	int Configure_Decimation(void* pCam, const LpGigECamConfig pParamIn);
	int Configure_Exposure(void* pCam, const LpGigECamConfig pParamIn);
	int Configure_PersistentIP(void* pCam, const LpGigECamConfig pParamIn);
	int Configure_ResetConfigureCustomImageSettings(void* pCam);
	int Configure_ResetExposure(void* pCam);
	int Configure_Get(void* pCam, LpGigECamConfig pParamOut, bool bDumpInf = 0);
	int Configure_FrameRate(void* pCam, const LpGigECamConfig pParamIn);
	int Configure_TriggerMode(void* pCam, const LpGigECamConfig pParamIn);


	int AcquireImages(void* pCam, string strFilePath);
	int AcquireImages(void* pCam, cv::Mat& matImg);
	int AcquireStreaming(void* pCam, cv::Mat& matImg);
	static void __stdcall AcquireCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);


	int AcquireStreaming_Prepare();
	int AcquireStreaming_StartorStop(bool bflgEnableThd);
	int AcquireStreaming_Capture(cv::Mat& matImg);
	int AcquireStreaming_Close();



	int Thread_Acquire(void* pCam, LpGigECamConfig pParamOut);
	int Thread_TriggerMode(void* pCam, LpGigECamConfig pParamOut);



	int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);
	int Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData, cv::Mat& mat_img);

private:
	string m_strFilePath;

	seGigECamConfig m_global_cfg_ParamInfo;

	double m_fLimit_ExposureTime_Min;
	double m_fLimit_ExposureTime_Max;

	void* m_pCam;
	unsigned long m_iFrameRate_Cnt;

	MV_CC_DEVICE_INFO_LIST m_stDevList;

	int m_RetryCnt;
	bool bflgIsBinningMode;

	CurlHandler restful_client;


};
