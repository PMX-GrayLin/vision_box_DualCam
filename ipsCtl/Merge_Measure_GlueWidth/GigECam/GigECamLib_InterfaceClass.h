#pragma once

#include <vector>
#include <string>


#include "GigECamDataStructureDef.h"
#include "../BaseDataStructureDef.h"


using namespace std;

class GigECam_I_Library
{
public:
	virtual ~GigECam_I_Library() {};

public:
	virtual int Init() = 0;
	virtual int SetConfig(const LpGigECamConfig pParamIn) = 0;
	virtual int GetConfig(LpGigECamConfig pParamIn) = 0;
	virtual int AcquireImages(std::string strFilePath) = 0;
	virtual int AcquireImages(seImageInfo* pImgInfo, std::string strFilePath, bool bStaticImgMode) = 0;
	virtual int Release() = 0;


	virtual int Streaming_Prepare() = 0;
	virtual int Streaming_Start() = 0;
	virtual int Streaming_AcquireImages(std::string strFilePath) = 0;
	virtual int Streaming_AcquireImages(seImageInfo* pImgInfo, std::string strFilePath) = 0;
	virtual int Streaming_Stop() = 0;
	virtual int Streaming_Colse() = 0;

	virtual int DebugPrint() = 0;

};


GigECam_I_Library* CreateObject_GigECam();