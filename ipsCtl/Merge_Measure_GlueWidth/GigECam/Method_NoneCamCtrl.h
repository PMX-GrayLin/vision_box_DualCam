#pragma once

#include "GigECamDataStructureDef.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include "Handler_CamCtrl_Strategy.h"
#include "../ThirdPartyLibrary/OverwriteRingBuffer.h"


using namespace std;
using namespace cv;



class CMethod_NoneCamCtrl : public CHandler_CamCtrl
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
	CMethod_NoneCamCtrl();
	~CMethod_NoneCamCtrl();


public:
	int GigeCam_Init() override;
	int GigECam_SetConfig(const LpGigECamConfig pParamIn) override;
	int GigECam_GetConfig(LpGigECamConfig pParamOut) override;
	int GigECam_AcquireImages(string strFilePath) override;
	int GigECam_AcquireImages(cv::Mat& matImg) override;
	int GigECam_Release() override;


	int GigECam_Strm_Prepare() override;
	int GigECam_Strm_Start() override;
	int GigECam_Strm_AcquireImages(string strFilePath) override;
	int GigECam_Strm_AcquireImages(cv::Mat& matImg) override;
	int GigECam_Strm_Stop() override;
	int GigECam_Strm_Close() override;

	int GigECam_DebugPrint() override;


private:
	int PrintDeviceInfo(int* pCam);

	int Configure_Default(int* pCam, const LpGigECamConfig pParamIn);
	int Configure_ImageFormat(int* pCam, const LpGigECamConfig pParamIn);
	int Configure_Binning(int* pCam, const LpGigECamConfig pParamIn);
	int Configure_Decimation(int* pCam, const LpGigECamConfig pParamIn);
	int Configure_Exposure(int* pCam, const LpGigECamConfig pParamIn);
	int Configure_PersistentIP(int* pCam, const LpGigECamConfig pParamIn);
	int Configure_ResetConfigureCustomImageSettings(int* pCam);
	int Configure_ResetExposure(int* pCam);
	int Configure_Get(int* pCam, LpGigECamConfig pParamOut, bool bDumpInf = 0);
	int Configure_FrameRate(int* pCam, const LpGigECamConfig pParamIn);
	int Configure_TriggerMode(int* pCam, const LpGigECamConfig pParamIn);


	int AcquireImages(int* pCam, string strFilePath);
	int AcquireImages(int* pCam, cv::Mat& matImg);
	int AcquireStreaming(int* pCam, cv::Mat& matImg);


	int AcquireStreaming_Prepare();
	int AcquireStreaming_StartorStop(bool bflgEnableThd);
	int AcquireStreaming_Capture(cv::Mat& matImg);
	int AcquireStreaming_Close();



	int Thread_Acquire(int* pCam, LpGigECamConfig pParamOut);
	int Thread_TriggerMode(int* pCam, LpGigECamConfig pParamOut);



	int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);
	int Convert2Mat(void* pstImageInfo, unsigned char* pData, cv::Mat& mat_img);

private:
	string m_strFilePath;

	seGigECamConfig m_global_cfg_ParamInfo;

	double m_fLimit_ExposureTime_Min;
	double m_fLimit_ExposureTime_Max;

	int* m_pCam;
	unsigned long m_iFrameRate_Cnt;

	std::vector<int> m_stDevList;

	int m_RetryCnt;

};
