#pragma once

#include "GigECamDataStructureDef.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <opencv2/opencv.hpp>

/* Point Grey SDK */
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "../ThirdPartyLibrary/OverwriteRingBuffer.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

using namespace std;
using namespace cv;



class CMethod_GigECamCtrl { 

	std::mutex u_mutex;
	std::condition_variable cond_var;
	bool notified_IsDone = false;
	bool notified_IsRun = false;
	std::thread::id tid;
	bool bIsCreated = false;
	int iCnt_0 = 0;
	int iCnt_1 = 0;

public:
	CMethod_GigECamCtrl();
	~CMethod_GigECamCtrl();


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
	int PrintDeviceInfo(CameraPtr pCam);

	int Configure_ImageFormat(CameraPtr pCam, const LpGigECamConfig pParamIn);
	int Configure_Decimation(CameraPtr pCam, const LpGigECamConfig pParamIn);
	int Configure_Exposure(CameraPtr pCam, const LpGigECamConfig pParamIn);
		int Configure_PersistentIP(CameraPtr pCam, const LpGigECamConfig pParamIn);
	void Configure_ResetCameraUserSetToDefault(CameraPtr pCam);
	int Configure_Get(CameraPtr pCam, LpGigECamConfig pParamOut, bool bDumpInf = 0);
	int Configure_FrameRate(CameraPtr pCam, const LpGigECamConfig pParamIn);
	bool Configure_EnableManualFramerate(CameraPtr pCam);


	int AcquireImages(CameraPtr pCam, string strFilePath);
	int AcquireImages(CameraPtr pCam, cv::Mat& matImg);
	int AcquireStreaming(CameraPtr pCam, cv::Mat& matImg);

	int AcquireStreaming_Prepare();
	int AcquireStreaming_StartorStop(bool bflgEnableThd);
	int AcquireStreaming_Close();



	int Thread_Acquire(CameraPtr pCam, LpGigECamConfig pParamOut);



	//int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);
	//int Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData, cv::Mat& mat_img);

private:
	string m_strFilePath;

	seGigECamConfig m_global_cfg_ParamInfo;


	SystemPtr m_system;
	CameraPtr m_pCam;
	unsigned long m_iFrameRate_Cnt;

	CameraList m_camList;

	int m_RetryCnt;
	unsigned long long m_Timeout;


};
