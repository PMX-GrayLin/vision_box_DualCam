#pragma once
#include <string>
#include <cstdlib>

using namespace std;


enum {
	CAM_LOG_LEVEL_NONE = 0,	//No debug information is output.
	CAM_LOG_LEVEL_ERROR,	//Logs all fatal errors.
	CAM_LOG_LEVEL_WARNING,	//Logs all warnings.
	CAM_LOG_LEVEL_MAJOR,	//Logs all "major" messages.
	CAM_LOG_LEVEL_INFO,		//Logs all informational messages.
	CAM_LOG_LEVEL_DEBUG,	//Logs all debug messages.
	CAM_LOG_LEVEL_LOG,		//Logs all log messages.     (Reserved)
	CAM_LOG_LEVEL_TRACE,	//Logs all trace messages.   (Reserved)
	CAM_LOG_LEVEL_VERBOSE,	//Logs all level messages.
};



extern int camDebugLevel;

inline void Cam_Log_Level() {
	char* szLogLevel = getenv("CAM_LOG_LEVEL");
	if (szLogLevel) {
		camDebugLevel = atoi(szLogLevel);
		printf("# camDebugLevel = %d\n", camDebugLevel);
	}
	else {
		camDebugLevel = CAM_LOG_LEVEL_NONE;
		printf("# camDebugLevel = %d\n", camDebugLevel);
	}
}


// ??
/* define the tty color */
#define camNONE "\033[0m"
#define camWHITE "\033[37m"
#define camGREEN "\033[32m"
#define camBLUE "\033[34m"
#define camYELLOW "\033[33m"
#define camRED "\033[31m"


// white
#define CAMV(format, b...) if ((camDebugLevel) >= (CAM_LOG_LEVEL_VERBOSE))  printf("__cam_# %s(): ln:%d :" camWHITE format camNONE, __FUNCTION__, __LINE__, ##b)
	//if ((camDebugLevel) >= (CAM_LOG_LEVEL_VERBOSE))  printf("__cam_# %s: %s(): ln:%d :" camWHITE format camNONE, __FILE__, __FUNCTION__, __LINE__, ##b)

// white
#define CAMD(format, b...) if ((camDebugLevel) >= (CAM_LOG_LEVEL_DEBUG))  printf("__cam_# %s(): ln:%d :" camWHITE format camNONE, __FUNCTION__, __LINE__, ##b)
	//if ((camDebugLevel) >= (CAM_LOG_LEVEL_DEBUG))  printf("__cam_# %s: %s(): ln:%d :" camWHITE format camNONE, __FILE__, __FUNCTION__, __LINE__, ##b)

// green
#define CAMI(format, b...) if ((camDebugLevel) >= (CAM_LOG_LEVEL_INFO))  printf("__cam_# %s(): ln:%d :" camGREEN format camNONE, __FUNCTION__, __LINE__, ##b)
	//if ((camDebugLevel) >= (CAM_LOG_LEVEL_INFO))  printf("__cam_# %s: %s(): ln:%d :" camGREEN format camNONE, __FILE__, __FUNCTION__, __LINE__, ##b)

// blue
#define CAMM(format, b...) if ((camDebugLevel) >= (CAM_LOG_LEVEL_MAJOR))  printf("__cam_# %s(): ln:%d :" camBLUE format camNONE, __FUNCTION__, __LINE__, ##b)
	//if ((camDebugLevel) >= (CAM_LOG_LEVEL_INFO))  printf("__cam_# %s: %s(): ln:%d :" camGREEN format camNONE, __FILE__, __FUNCTION__, __LINE__, ##b)

// yellow
#define CAMW(format, b...) if ((camDebugLevel) >= (CAM_LOG_LEVEL_WARNING))  printf("__cma_# %s: %s(): ln:%d :" camYELLOW format camNONE, __FILE__, __FUNCTION__, __LINE__, ##b)
	//if ( (camDebugLevel) >= (CAM_LOG_LEVEL_WARNING) )  printf("__cam_# %s(): ln:%d :" camYELLOW format camNONE, __FUNCTION__, __LINE__, ##b) 

// red
#define CAME(format, b...) if ((camDebugLevel) >= (CAM_LOG_LEVEL_ERROR))  printf("__cma_# %s: %s(): ln:%d :" camRED format camNONE, __FILE__, __FUNCTION__, __LINE__, ##b)
	//if ( (camDebugLevel) >= (CAM_LOG_LEVEL_ERROR) )  printf("__cam_# %s(): ln:%d :" camRED format camNONE, __FUNCTION__, __LINE__, ##b)






enum class emTriggerType
{
	SOFTWARE,
	HARDWARE
};


enum class emExposureAuto
{
	Timed_Auto,
	Timed,
	//TriggerWidth
};


typedef struct tagGigECamConfig {

	bool	bIsConnected;	//0: Not yet, 1: Yes.
	bool	bIsStreaming;	//0: No, 1: Yes.
	bool	bPixelFormat;	//0:PixelFormat_Mono8, 1:PixelFormat_BayerGR8
	bool	bIsEnbAcquisitionFrameRate; //0: No, 1: the camera's maximum frame rate is limited by the value.
	bool	bIsEnbReadImageMode; // 0: Normail Mode, 1: Using static image from Path for testing.
	bool	bIsEnbTriggerMode;	//0: No, 1: Yes.

	unsigned int	iCamId;

	unsigned int	iOffset_X;		//Resl_Offset_X
	unsigned int	iOffset_Y;		//Real_Offset_Y
	unsigned int	iWidth;			//Real_Width
	unsigned int	iHeight;		//Real_Height

	unsigned int	iSensor_Width;		//Max_Width
	unsigned int	iSensor_Height;		//Max_Height

	unsigned int	iBinning_Scale;		//�����P����V�U����

	//bool	bTriggerMode;	//0:Software, 1:Hardware

	double	dbExposureTime; //unit: us.
	bool	bExposureAuto;  //0:Auto, 1:Timed_(Off);

	double dbAcquisitionFrameRate;	//camera's maximum frame rate is limited.

	unsigned int	iTriggerSrc;	// 0: Line0, 1:Line2, 2:Software
	unsigned int	iTriggerActivation;	//0:RisingEdge, 1:FallingEdge, 2:LevelHigh, 3:LevelLow

	string strInputImgPath;
	string strSaveImgPath;
	string strResultImgPath;

	string strPersistentIP;


	tagGigECamConfig() {

		bIsConnected = false;
		bIsStreaming = false;
		bPixelFormat = false;
		bIsEnbAcquisitionFrameRate = false;
		bIsEnbReadImageMode = false;
		bIsEnbTriggerMode = false;
		iCamId = 0;
		iOffset_X = 0;
		iOffset_Y = 0;
		iWidth = 3072;// 2592;
		iHeight = 2048;	// 1944;
		iSensor_Width = 0;
		iSensor_Height = 0;
		iBinning_Scale = 1;
		dbExposureTime = 21151.44;
		bExposureAuto = (bool)(emExposureAuto::Timed_Auto);
		dbAcquisitionFrameRate = 40.0;

		iTriggerSrc = 0;
		iTriggerActivation = 1;

		strInputImgPath = "/tmp";
		strSaveImgPath = "/tmp";
		strResultImgPath = "/tmp";
		strPersistentIP = "169.254.200.50";
	}

} seGigECamConfig, *LpGigECamConfig, seGigECamCapture, *LpseGigECamCapture;


typedef struct tagGigECamConfig_Result {	

	int retState;

	//TBD
	int TBD = -9999;
	seGigECamConfig seCamConfig;

} seGigECamConfig_Ret, * LpGigECamConfig_Ret, seGigECamCapture_Ret, * LpseGigECamCapture_Ret;
