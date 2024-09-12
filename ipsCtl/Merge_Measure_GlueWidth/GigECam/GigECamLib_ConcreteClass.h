#pragma once
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <chrono>

#include "GigECamDataStructureDef.h"
#include "GigECamLib_InterfaceClass.h"

#include "Handler_CamCtrl_Strategy.h"

// # Gige cmaera controller
//#include "Method_GigECamCtrl.h"
//#include "Method_ArvGigECamCtrl.h"
//#include "Method_HikGigECamCtrl.h"
//#include "Method_BaslerGigECamCtrl.h"
//#include "Method_OPTGigECamCtrl.h"
//#include "Method_NoneCamCtrl.h"
#include "Method_V4L2CamCtrl.h"

using namespace std;

// GigeCamera Privider List
enum class emCamProviderList {

	CAM_PROVIDER_NONE = 0,	
	CAM_PROVIDER_HikVision,	
	CAM_PROVIDER_Basler,	
	CAM_PROVIDER_PointGrey,	
	CAM_PROVIDER_OPT,
	CAM_PROVIDER_UVC
	
};

inline int Cam_Provider_Selection() {

	char* szWord = getenv("CAM_PROVIDER_SELECTION");

	int nIndex = static_cast<int>(emCamProviderList::CAM_PROVIDER_NONE);

	if (szWord) {

		nIndex = atoi(szWord);
		printf("# CamProviderSelection Index = %d\n", nIndex);
	}
	else {

		nIndex = static_cast<int>(emCamProviderList::CAM_PROVIDER_NONE);
		printf("# CamProviderSelection Index = %d\n", nIndex);
	}
	return nIndex;
}


class GigECam_C_Library : public GigECam_I_Library
{

public: 
	GigECam_C_Library();
	~GigECam_C_Library();


public:
	int Init() override;
	int SetConfig(const LpGigECamConfig pParamIn) override;
	int GetConfig(LpGigECamConfig pParamIn) override;
	int AcquireImages(std::string strFilePath) override;
	int AcquireImages(seImageInfo* pImgInfo, std::string strFilePath, bool bStaticImgMode = 0) override;
	int Release() override;


	int Streaming_Prepare() override;
	int Streaming_Start() override;
	int Streaming_AcquireImages(std::string strFilePath) override;
	int Streaming_AcquireImages(seImageInfo* pImgInfo, std::string strFilePath) override;
	int Streaming_Stop() override;
	int Streaming_Colse() override;

	int DebugPrint() override;


	CHandler_CamCtrl* phdl_cam;

private:
	//CMethod_ArvGigECamCtrl cam_ctrl;	// # Aarvis
	//CMethod_GigECamCtrl cam_ctrl;		// # Point Grey
	//CMethod_HikGigECamCtrl cam_ctrl;	// # Hik vision
	//CMethod_BaslerGigECamCtrl cam_ctrl;	// # Basler
	//CMethod_OPTGigECamCtrl cam_ctrl;	// # OPT vision

	//CMethod_HikGigECamCtrl cam_ctrl_hik;	// # Hik vision
	//CMethod_NoneCamCtrl cam_ctrl_none;	// # None camera using
	CMethod_V4L2CamCtrl cam_ctrl_uvc;	// # UVC camera using

private:
	seGigECamConfig m_seConfig;


};

