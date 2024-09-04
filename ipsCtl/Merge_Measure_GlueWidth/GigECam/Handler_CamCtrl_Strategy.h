#pragma once

#include "GigECamDataStructureDef.h"
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


class CHandler_CamCtrl {

public:
	virtual ~CHandler_CamCtrl() { ; };


public:
	virtual int GigeCam_Init() = 0;
	virtual int GigECam_SetConfig(const LpGigECamConfig pParamIn) = 0;
	virtual int GigECam_GetConfig(LpGigECamConfig pParamOut) = 0;
	virtual int GigECam_AcquireImages(string strFilePath) = 0;
	virtual int GigECam_AcquireImages(cv::Mat& matImg) = 0;
	virtual int GigECam_Release() = 0;


	virtual int GigECam_Strm_Prepare() = 0;
	virtual int GigECam_Strm_Start() = 0;
	virtual int GigECam_Strm_AcquireImages(string strFilePath) = 0;
	virtual int GigECam_Strm_AcquireImages(cv::Mat& matImg) = 0;
	virtual int GigECam_Strm_Stop() = 0;
	virtual int GigECam_Strm_Close() = 0;

	virtual int GigECam_DebugPrint() = 0;

};
