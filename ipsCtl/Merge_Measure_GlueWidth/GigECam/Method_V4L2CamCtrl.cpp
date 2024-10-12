#include <iostream>
#include <stdio.h>
#include <stdlib.h>
//#include <filesystem>
#include <cstdio>
#include <chrono>
#include <thread>
#include <atomic>
#include <sstream>
#include <string>
#include <functional>
#include <curl/curl.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <assert.h>
#include <memory.h>
#include <linux/videodev2.h>

#include "../cvip.h"
#include "Method_V4L2CamCtrl.h"
#include "common.hpp"
#include "global.hpp"

/////////////////////////////////////////////////////////////////////////////
/// V4L2 >>> V4L2 >>> V4L2 >>> V4L2 >>> V4L2 >>> V4L2 >>> V4L2 >>>
/////////////////////////////////////////////////////////////////////////////
//  {"cmd":"CAMERA_STREAM_SET_PREPARE","noWaitResponse":true,"args":{"status":"TBD","CameraId":"1","msgId":"3625_1718766757173","cmd":"CAMERA_STREAM_SET_PREPARE"}}
//  {"cmd":"CAMERA_STREAM_SET_START","noWaitResponse":true,"args":{"status":"TBD","CameraId":"1","msgId":"4769_1718766757187","cmd":"CAMERA_STREAM_SET_START"}}

//#include <json_helper.h>

//#define LOGTAG "VIDEOTEST"
#define VIV_CTRL_NAME "viv_ext_ctrl"	

#define DWE_OFF (char *)"{<id>:<pipeline.s.dwe.onoff>;<enable>: false}"

#define VIV_CUSTOM_CID_BASE (V4L2_CID_USER_BASE | 0xf000)
#define V4L2_CID_VIV_EXTCTRL (VIV_CUSTOM_CID_BASE + 1)

#ifndef MIN
#define MIN(a, b)   ( ((a)<=(b)) ? (a) : (b) )
#endif /* MIN */

#ifndef MAX
#define MAX(a, b)   ( ((a)>=(b)) ? (a) : (b) )
#endif /* MAX */

#define S_EXT_FLAG 555

#define CLEAR(x) memset(&(x), 0, sizeof(x))

//#define LOGTAG "VVEXT:"


/////////////////////////////////////////////////////////////////////////////
/// V4L2 <<< V4L2 <<< V4L2 <<< V4L2 <<< V4L2 <<< V4L2 <<< V4L2 <<< 
/////////////////////////////////////////////////////////////////////////////

using namespace std;

int gRingBufSzine = 2;

static OverwriteRingBuffer<cv::Mat> g_owRingbuffer(gRingBufSzine);
static TimestampedRingBuffer<cv::Mat> g_timerRingbuffer(gRingBufSzine);
int CMethod_V4L2CamCtrl::prev_W(0);
int CMethod_V4L2CamCtrl::prev_H(0);
// unsigned char* CMethod_V4L2CamCtrl::g_pDataForRGB = nullptr;

//////////////////////////////////////////////////////////////////////////////
///  V4L2 camera controller function
/////////////////////////////////////////////////////////////////////////////

CMethod_V4L2CamCtrl::CMethod_V4L2CamCtrl()
	: m_pCam(-1)
	, m_Streamid(0)
	, m_global_cfg_ParamInfo(seGigECamConfig())
	, m_fLimit_ExposureTime_Min(0.000034)
	, m_fLimit_ExposureTime_Max(0.041820)
	, m_fLimit_Gain_Min(1.0)
	, m_fLimit_Gain_Max(6.0)
	, m_iFrameRate_Cnt(0)
	, m_RetryCnt(10)
	, m_DeviceId(2)
	, m_strDeviceName("")
	, m_SensorMode(0)
	, m_szFormat("YUYV")
	, m_isRawData(0)
	, m_Crop_enable(0)
	, m_Scale_enable(0)
	, m_DisplayType(-1)
	, m_BUFFER_COUNT(gRingBufSzine)
	, m_Prev_width(0)
	, m_Prev_height(0)
	, m_Prev_pixelFormat(0)
	, m_rectCrop_Frame(cv::Rect(0, 0, 0, 0))
	, m_rectCrop_ROI(cv::Rect(0, 0, 0, 0))
	, m_IsCropped(false)
	, restful_client("http://127.0.0.1:8000/streaming")
	, cap_uvc(5, cv::CAP_V4L)
{
    // Check whether the device is turned on successfully
    if (!cap_uvc.isOpened()) {
        std::cerr << "Error: Unable to open UVC device." << std::endl;
    }

    // Set video screen size
    cap_uvc.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap_uvc.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
}


CMethod_V4L2CamCtrl::~CMethod_V4L2CamCtrl()
{
	GigECam_Release();
	
    // Release resources
    cap_uvc.release();
}


int CMethod_V4L2CamCtrl::GigeCam_Init()
{
	std::unique_lock<std::mutex> lock(u_mutex);

	int nRet = 0;
	int cam = 0;

	IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

	if (Streaming_Inquiry()) {

		IPSLOG(1, "Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(5000); /* delay 5  ms */
	}

	if (m_DeviceId < 0) {

		for (int i = 0; i < 20; i++) {

			if ((m_pCam = OpenDevice(i)) >= 0) {
				break;
			}
		}
	}
	else {
		m_pCam = OpenDevice(m_DeviceId);
	}

	if (m_pCam < 0) {
        // IPSLOG(1, "Error!! open camdev nRet : %d\n", m_pCam);
		// IPSLOG(0, "failed to open camera device (%d)!\n", m_DeviceId);
		xlog("fail open camdev, nRet:%d, m_DeviceId:%d \n\r", __func__, __LINE__, m_pCam, m_DeviceId);
		return -1;
	}

	nRet = ioctl(m_pCam, VIDIOC_QUERYCAP, &m_Caps);
	if (nRet < 0) {
		IPSLOG(0, "Error!! failed to get device m_Caps for %s (%d = %s)\n", m_strDeviceName.c_str(), errno, strerror(errno));
		return -1;
	}

	IPSLOG(1, "@_@!! VIDIOC_QUERYCAP ; v4l2_capability === === >>>\n");
	IPSLOG(1, "  >>Query the functions supported by the device\n");
	nRet = ioctl(m_pCam, VIDIOC_QUERYCAP, &m_Caps);
	if (nRet < 0) {

		// IPSLOG(0, "failed to get device m_Caps for %s (%d = %s)\n", m_strDeviceName, errno, strerror(errno));
		v4l2_close_device();
		return -1;
	}

	IPSLOG(1, "Open Device: %s (fd=%d)\n", m_strDeviceName, m_pCam);
	IPSLOG(1, "  Driver: %s\n", m_Caps.driver);

	if (strcmp((const char*)m_Caps.driver, "mtk-mdp3")) {
		v4l2_close_device();
		IPSLOG(0, "m_Caps.driver=[%s] fail.\n", m_Caps.driver);
		//return -1;
	}
	IPSLOG(1, "found viv video dev %s\n", m_strDeviceName);

	//@ VIV 
	// get viv ctrl id by it's name "viv_ext_ctrl"
	IPSLOG(1, "@_@!! VIDIOC_QUERYCTRL ; v4l2_queryctrl === === >>>\n");
	IPSLOG(1, "  >>Query the detailed information of the specified control\n");

	struct v4l2_queryctrl queryctrl;
	CLEAR(queryctrl);

	queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

	while (0 == ioctl(m_pCam, VIDIOC_QUERYCTRL, &queryctrl)) {

		if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
			continue;

		IPSLOG(1, "%s Control %s\n", __func__, queryctrl.name);

		if (strcmp((char*)queryctrl.name, VIV_CTRL_NAME) == 0) {

			g_ctrl_id = queryctrl.id;
			IPSLOG(1, "%s, find viv ctrl id 0x%x\n", __func__, g_ctrl_id);
			break;
		}

		queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}


	// Report device properties
	IPSLOG(1, "# Open Device: %s (fd=%d)\n", m_strDeviceName.c_str(), m_pCam);
	IPSLOG(1, "\tDriver: %s\n", m_Caps.driver);
	IPSLOG(1, "\tCard: %s\n", m_Caps.card);
	IPSLOG(1, "\tVersion: %u.%u.%u\n", (m_Caps.version >> 16) & 0xFF, (m_Caps.version >> 8) & 0xFF, (m_Caps.version) & 0xFF);
	IPSLOG(1, "\tAll Caps: %08X\n", m_Caps.capabilities);
	IPSLOG(1, "\tDev Caps: %08X\n", m_Caps.device_caps);

	IPSLOG(1, "open camdev ret : %d\n", m_pCam);
	// Judgement the capabilities information.
	if (!(m_Caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
		!(m_Caps.capabilities & V4L2_CAP_STREAMING)) {

		IPSLOG(0, "Error!! Streaming capture not supported by %s.\n", m_strDeviceName.c_str());
		return -1;
	}

	// Judgement the value of "m_szFormat" define
	if (string2V4L2MediaFormat.find(m_szFormat) == string2V4L2MediaFormat.end()) {

		IPSLOG(0, "Error!! unsupported format: %s\n",	strlen(m_szFormat) > 0 ? m_szFormat : "(null)");
		// by joe return -1;
	}


	if (Configure_Get(m_pCam, &m_global_cfg_ParamInfo, 1)) {

		IPSLOG(1, "Configure_Get( m_pCam, \" m_global_cfg_ParamInfo \", 1 )\n");
	}


	return nRet;
}



int CMethod_V4L2CamCtrl::GigECam_SetConfig(const LpGigECamConfig pParamIn)
{

	int nRet = 0;

	if (m_pCam <= 0) {
		IPSLOG(0, " > Erroe!!! m_pCam [%d] <= 0\n", m_pCam);
		//return -1;
	}

	std::unique_lock<std::mutex> lock(u_mutex);

	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;

	{

		//IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

		if (!Streaming_Inquiry()) {

			//if (!nRet) nRet = Configure_Decimation(m_pCam, pParamIn);			

			// # 01 >>
			start = std::chrono::high_resolution_clock::now();



			//Update camera config to global parameter.
			if (!nRet) nRet = Configure_Get(m_pCam, &m_global_cfg_ParamInfo);




			// # 01 <<
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			strtmp = std::to_string(duration.count());
			//CAMM(" > Configure_Get()_CycleTime : %s (ms)\n",  strtmp.c_str());


			// # 02 >>
			start = std::chrono::high_resolution_clock::now();

			
			
			// Bonning value for Streaming 
			if (!nRet) nRet = Configure_Binning(m_pCam, pParamIn);


			// # 02 <<
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			strtmp = std::to_string(duration.count());
			//CAMM(" > Configure_Binning()_CycleTime : %s (ms)\n",  strtmp.c_str());


			// # 03 >>
			start = std::chrono::high_resolution_clock::now();


			// Image format information
			if (!nRet) nRet = Configure_ImageFormat(m_pCam, pParamIn);


			// # 03 <<
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			strtmp = std::to_string(duration.count());
			//CAMM(" > Configure_ImageFormat()_CycleTime : %s (ms)\n",  strtmp.c_str());


			//// # 04 >>
			//start = std::chrono::high_resolution_clock::now();


			// # 05 >>
			start = std::chrono::high_resolution_clock::now();

			//Enable Cropping and setting crop roi.
			//if(!nRet) nRet = Configure_Cropping(m_pCam, pParamIn);

			// # 05 <<
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			strtmp = std::to_string(duration.count());
			//CAMM(" > Configure_Frame_Cropping()_CycleTime : %s (ms)\n",  strtmp.c_str());

		}
		else {

			IPSLOG(0, "This is Streaming Mode, con not setting any config.\n");
			IPSLOG(0, "This is Streaming Mode, con not setting any config.\n");
		}

		// # 04 <<
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		strtmp = std::to_string(duration.count());
		//CAMM(" > Configure_AutoWhiteBalance()_CycleTime : %s (ms)\n",  strtmp.c_str());



		// # 05 >>
		start = std::chrono::high_resolution_clock::now();

		// # 05 <<
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		strtmp = std::to_string(duration.count());

		//// Configure Inet Addr <==== no function
		//nRet = Configure_PersistentIP(m_pCam, pParamIn);
		//if (nRet < 0)
		//{
		//    return nRet;
		//}

		if (nRet < 0)
		{
			//IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

			if (Streaming_Inquiry()) {

				IPSLOG(1, "Inof ~~, Streaming mode is already running. Stop now.\n");

				//AcquireStreaming_StartorStop(false);
			}
AcquireStreaming_Prepare();
			return nRet;
		}
	}


	return nRet;
}


int CMethod_V4L2CamCtrl::GigECam_GetConfig(LpGigECamConfig pParamOut)
{
	int nRet = 0;

	if (m_pCam <= 0) {
		IPSLOG(0, " > Erroe!!! m_pCam <= 0\n");
		// by joe return -1;
	}

	//IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

	if (Streaming_Inquiry()) {

		IPSLOG(1, "Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(50000); /* delay 50  ms */
	}


#ifdef ALGO_Enable_OpenCV_getBuildInformation_DEBUG

	IPSLOG(1, "\n");
	IPSLOG(1, " > ---. ---- > ---. ---- > ---. ---- > ---. ---- > ---. ---- \n");
	IPSLOG(1, " >>> >>> OpenCV Regist List. Start >>> >>> \n");
	IPSLOG(1, " > ---. ---- > ---. ---- > ---. ---- > ---. ---- > ---. ---- \n");

	//std::cout << cv::getBuildInformation() << std::endl;

	std::string buildInfo = cv::getBuildInformation();
	std::cout << buildInfo << std::endl;

	std::cout << "--- # --- # --- # --- # --- # --- # --- # --- " << std::endl;
	if (buildInfo.find("carotene") != std::string::npos) {
		std::cout << " # carotene is enabled." << std::endl;
}
	else {
		std::cout << " # carotene is Not enabled." << std::endl;
	}
	std::cout << "--- # --- # --- # --- # --- # --- # --- # --- " << std::endl;
	std::cout << "-----------------------------------------------------------------" << std::endl;

	IPSLOG(1, " > ---. ---- > ---. ---- > ---. ---- > ---. ---- > ---. ---- \n");
	IPSLOG(1, " <<< <<<  OpenCV Regist List. End  <<< <<< \n");
	IPSLOG(1, " > ---. ---- > ---. ---- > ---. ---- > ---. ---- > ---. ---- \n");
	IPSLOG(1, "\n");

#endif

	// Configure information
	nRet = Configure_Get(m_pCam, pParamOut);

	if (nRet < 0)
	{
		return nRet;
	}

	return nRet;
}



int CMethod_V4L2CamCtrl::GigECam_AcquireImages(string strFilePath)
{
	int nRet = 0;

	if (m_pCam <= 0) {
		IPSLOG(0, " > Erroe!!! m_pCam <= 0\n");
		return -1;
	}

	//IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

	if (Streaming_Inquiry()) {

		IPSLOG(1, "Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(50000); /* delay 50  ms */
	}

	nRet = AcquireImages(m_pCam, strFilePath);

	return nRet;
}



int CMethod_V4L2CamCtrl::GigECam_AcquireImages(cv::Mat& matImg)
{
	int nRet = 0;

	if (m_pCam <= 0) {
		IPSLOG(0, " > Erroe!!! m_pCam <= 0\n");
		return -1;
	}

	//IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

	if (Streaming_Inquiry()) {

		IPSLOG(1, "Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(50000); /* delay 50  ms */
	}

	nRet = AcquireImages(m_pCam, matImg);

	return nRet;
}

int CMethod_V4L2CamCtrl::GigECam_Release()
{
	int nRet = 0;

	if (m_pCam) {

		IPSLOG(1, "GigECam_Release() === === >>>\n");

		prev_W = 0;
		prev_H = 0;

		//IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

		if (Streaming_Inquiry()) {

			IPSLOG(1, "Inof ~~, Streaming mode is already running. Stop now.\n");

			AcquireStreaming_StartorStop(false);
			usleep(5000); /* delay 5  ms */
		}

		IPSLOG(1, "\tRelease before, m_pCam Handle Address = %02d\n", m_pCam);

		//销毁句柄，释放资源
		v4l2_stop_capturing(m_pCam);

		v4l2_uninit_device();

		v4l2_close_device();

		sleep(1);

		IPSLOG(1, "\tRelease after, m_pCam Handle Address = %02d\n", m_pCam);

		IPSLOG(1, "MV_CC_DestroyHandle(m_pCam) is done!!!!\n");

		IPSLOG(1, "GigECam_Release() <<< === ===\n");

	}

	return nRet;
}


int CMethod_V4L2CamCtrl::GigECam_Strm_Prepare()
{
	int nRet = 0;

	AcquireStreaming_Prepare();

	return nRet;
}


/*
int CMethod_V4L2CamCtrl::GigECam_Strm_Inquiry(seStreamingInfo* pStrmInfo)
{
	int nRet = 0;

	AcquireStreaming_Inquiry(pStrmInfo);

	if (pStrmInfo->IsEnbStreaming) {
		sleep(6); //RexTYW_20230926_for front-end request 
	}

	return nRet;
}
*/

int CMethod_V4L2CamCtrl::GigECam_Strm_Start()
{
	int nRet = 0;
	AcquireStreaming_StartorStop(true);
	usleep(10000); /* delay 1  ms */

	return nRet;
}



int CMethod_V4L2CamCtrl::GigECam_Strm_AcquireImages(string strFilePath)
{
	int nRet = 0;

	IPSLOG(0, "This class does not have a camera handle for use [%x]\n", nRet);

	return nRet;
}

int CMethod_V4L2CamCtrl::GigECam_Strm_AcquireImages(cv::Mat& matImg)
{
	int nRet = 0;

	nRet = AcquireStreaming_Capture(matImg);

	return nRet;
}


int CMethod_V4L2CamCtrl::GigECam_Strm_Stop()
{
	int nRet = 0;

	AcquireStreaming_StartorStop(false);

	usleep(50000); /* delay 50  ms */

	return nRet;
}


int CMethod_V4L2CamCtrl::GigECam_Strm_Close()
{
	int nRet = 0;

	AcquireStreaming_Close();

	return nRet;
}


int CMethod_V4L2CamCtrl::OpenDevice(int id)
{
	int fd = v4l2_open_device(id);

	if (fd < 0) {
		IPSLOG(0, "can't open video file\n");
		return -1;
	}
	
	return fd;
}


void CMethod_V4L2CamCtrl::UpdateCtrlIDList(const int pCam) {

	struct v4l2_query_ext_ctrl qctrl;

	int camera = pCam;

	int id = 0;
	do {
		CLEAR(qctrl);
		qctrl.id = id |
			V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;

		if (ioctl(camera, VIDIOC_QUERY_EXT_CTRL, &qctrl) < 0) {\

			CAMD("VIDIOC_QUERY_EXT_CTRL: %s\n", strerror(errno));
			break;
		}
		else if (qctrl.type == V4L2_CTRL_TYPE_STRING) {

			CAMD("V4L2 ext ctrl id: 0x%x\n", qctrl.id);
			CAMD("V4L2 ext ctrl name: %s\n", qctrl.name);
			CAMD("V4L2 ext ctrl elem size: %d\n", qctrl.elem_size);
			memcpy(&m_VivExtQctrl, &qctrl, sizeof(qctrl));
		}
		else {
			if (ctrlIUVCDList.find(qctrl.name) != ctrlIUVCDList.end()) {

				ctrlIUVCDList[qctrl.name] = qctrl.id;
				CAMD("V4L2 ext ctrl id: 0x%x\n", qctrl.id);
				CAMD("V4L2 ext ctrl name: %s\n", qctrl.name);
				CAMD("V4L2 ext ctrl elem size: %d\n", qctrl.elem_size);
			}
		}

		id = qctrl.id;

	} while (true);

}


int CMethod_V4L2CamCtrl::SetFeature(const char* value)
{
	int ret = 0;
	struct v4l2_ext_controls ctrls;
	struct v4l2_ext_control ctrl;

	if (value == nullptr)
		return -1;

	if ((m_pCam <= 0) || (g_ctrl_id <= 0))
		return -1;

	CLEAR(ctrl);
	ctrl.id = g_ctrl_id;
	ctrl.size = strlen(value) + 1;
	ctrl.string = strdup(value);

	CLEAR(ctrls);
	ctrls.which = V4L2_CTRL_ID2WHICH(ctrl.id);
	ctrls.count = 1;
	ctrls.controls = &ctrl;

	IPSLOG(1, "setFeature, fd %d, id 0x%x, str %s", m_pCam, g_ctrl_id, value);

	ret = ioctl(m_pCam, VIDIOC_S_EXT_CTRLS, &ctrls);
	IPSLOG(1, "setFeature, ret %d", ret);
	if (ret < 0)
		IPSLOG(0, "%s VIDIOC_S_EXT_CTRLS failed, value %s, errno %d, %s",
			__func__, value, errno, strerror(errno));

	free(ctrl.string);

	return ret;
}


int CMethod_V4L2CamCtrl::PrintDeviceInfo(const int pCam)
{
	int nRet = 0;

	/* Connect to the first available camera */
	int camera = pCam;

	if (camera) {

		seGigECamConfig pParamOut;

		Configure_Get(camera, &pParamOut, 1);

	}
	else {
		/* En error happened, display the correspdonding message */
		IPSLOG(0, "error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}



int CMethod_V4L2CamCtrl::Configure_ImageFormat(const int pCam, const LpGigECamConfig pParamIn)
{
	IPSLOG(1, "Configure_ImageFormat ==> \n");

	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;


	int nRet = 0;

	/* Connect to the first available camera */
	int camera = pCam;

	int width = 0, height = 0;

	int iLimit_Max_Width = m_global_cfg_ParamInfo.iSensor_Width;
	int iLimit_Max_Height = m_global_cfg_ParamInfo.iSensor_Height;

	if (iLimit_Max_Width == 0 || iLimit_Max_Height == 0) {

		return -1;
	}

	// Judgment the prev_W/prev_H, If they are the same as the current width and height, do not change the configuration. 
	if ((prev_W == width) && (prev_H == height)) {
	
		return 0;
	}


	//Record last time Width/Height vlaue 
	if (0 == prev_W) { prev_W = width; }
	if (0 == prev_H) { prev_H = height; }

	IPSLOG(1, " ###  iLimit_Max_Width = %d\n", iLimit_Max_Width);
	IPSLOG(1, " ###  iLimit_Max_Height = %d\n", iLimit_Max_Height);
	IPSLOG(1, " ###  width = %d\n", width);
	IPSLOG(1, " ###  height = %d\n", height);


	struct v4l2_ext_controls ectrls;
	struct v4l2_ext_control ectrl;

	//# crop_enable flag
	IPSLOG(1, "crop_enable flag_crop_enable(%d)\n", m_Crop_enable);
	// ...TBD
	//if (crop_enable)
	if (0)
	{
		int crop_left = 420;
		int crop_top = 0;
		int crop_width = 640;
		int crop_height = 480;

		struct v4l2_selection sel;
		CLEAR(sel);
		sel.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		sel.target = V4L2_SEL_TGT_CROP_BOUNDS;
		nRet = ioctl(camera, VIDIOC_G_SELECTION, &sel);
		if (nRet)
		{
			IPSLOG(0, "Video Test: Get Crop Bounds Failed\n");
			//return -1;
		}
		IPSLOG(1, "Video Test: Get Crop Bounds from (%d, %d), size: %dx%d\n", sel.r.left, sel.r.top, sel.r.width, sel.r.height);

		sel.target = V4L2_SEL_TGT_CROP_DEFAULT;
		nRet = ioctl(camera, VIDIOC_G_SELECTION, &sel);
		if (nRet)
		{
			IPSLOG(0, "Video Test: Get Default Crop Size Failed\n");
			//return -1;
		}
		IPSLOG(1, "Video Test: Get Default Crop from (%d, %d), size: %dx%d\n", sel.r.left, sel.r.top, sel.r.width, sel.r.height);

		sel.target = V4L2_SEL_TGT_CROP;
		if (crop_left >= 0 && crop_top >= 0 && crop_width > 0 && crop_height > 0 &&
			(unsigned int)(crop_left + crop_width) <= sel.r.width &&
			(unsigned int)(crop_top + crop_height) <= sel.r.height) {
			sel.r.left = crop_left;
			sel.r.top = crop_top;
			sel.r.height = crop_height;
			sel.r.width = crop_width;
			IPSLOG(1, "Video Test: Set Crop from (%d, %d), Size %dx%d\n", crop_left, crop_top, crop_width, crop_height);
		}
		else if (crop_width == 0 && crop_height == 0) {
			sel.r.left = 0;
			sel.r.top = 0;
			sel.r.height = height;
			sel.r.width = width;
			IPSLOG(1, "Video Test: Set Crop from (0, 0), Size %dx%d\n", width, height);
		}
		else {
			IPSLOG(0, "Not Support crop from (%d, %d), size: {crop_width:%d crop_height:%d} \n", crop_left, crop_top, crop_width, crop_height);
			//return -1;
		}

		IPSLOG(1, "==== VIDIOC_S_SELECTION, crop %dx%d", sel.r.width, sel.r.height);
		nRet = ioctl(camera, VIDIOC_S_SELECTION, &sel);
		if (nRet)
		{
			IPSLOG(0, "Video Test: Set Set Crop Size Failed %d\n", nRet);
			//return -1;
		}

	}

	//# scale_enable flag
	IPSLOG(1, "scale_enable flag_scale_enable(%d)\n", m_Scale_enable);
	// ...TBD
	//if (scale_enable)
	if (0)
	{
		struct v4l2_selection sel;
		CLEAR(sel);
		sel.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		sel.target = V4L2_SEL_TGT_COMPOSE_DEFAULT;

		nRet = ioctl(camera, VIDIOC_G_SELECTION, &sel);
		if (nRet)
		{
			IPSLOG(0, "Video Test: Get Default Scale Size Failed\n");
			//return -1;
		}

		IPSLOG(1, "Video Test: Get Default Scale Size %dX%d\n", sel.r.width, sel.r.height);
		IPSLOG(1, "Video Test: Set Scale Size %dX%d\n", width, height);

		sel.target = V4L2_SEL_TGT_COMPOSE;
		sel.r.top = 0;
		sel.r.left = 0;
		sel.r.height = height;
		sel.r.width = width;
		nRet = ioctl(camera, VIDIOC_S_SELECTION, &sel);
		if (nRet)
		{
			IPSLOG(0, "Video Test:Set Scale Size Failed %d\n", nRet);
			//return -1;
		}
	}
	
	//# disable dwe if capture raw format
	// ...TBD

	//# choose RawData type.
	/* by joe 
	if ((string2V4L2MediaFormat[m_szFormat] == MEDIA_PIX_FMT_RAW8) ||
		(string2V4L2MediaFormat[m_szFormat] == MEDIA_PIX_FMT_RAW10) ||
		(string2V4L2MediaFormat[m_szFormat] == MEDIA_PIX_FMT_RAW12)) {

		SetFeature(DWE_OFF);
		m_isRawData = 1;
	}*/

	IPSLOG(1, "\n");
	IPSLOG(1, "Image Foramt:\"%s\"; isRawData=(%d)\n", m_szFormat, m_isRawData);

	//// # 01 >>
	start = std::chrono::high_resolution_clock::now();


	IPSLOG(1, "@_@!! VIDIOC_S_FMT ; v4l2_format === === >>>\n");
	IPSLOG(1, "  >> Set data format\n");
	unsigned int pixelFormat = m_isRawData ? V4L2_PIX_FMT_YUYV : mediaUVCFormat2V4l2Format[string2V4L2MediaFormat[m_szFormat]];


	IPSLOG(1, "@_@!! VIDIOC_G_FMT ; v4l2_format === === >>>\n");
	IPSLOG(1, " >> Get data format\n");
	v4l2_format format_get;
	format_get.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	// 調用 VIDIOC_G_FMT 以獲取當前格式
	if (ioctl(camera, VIDIOC_G_FMT, &format_get) < 0) {

		IPSLOG(0, "VIDIOC_G_FMT: %s\n", strerror(errno));
		return -1;
	}

	// 輸出當前格式的信息
	IPSLOG(1, "Current format: \n");
	IPSLOG(1, "  Width: %d\n", format_get.fmt.pix.width);
	IPSLOG(1, "  Height: %d\n", format_get.fmt.pix.height);
	IPSLOG(1, "  Pixel format: 0x%11x\n", format_get.fmt.pix.pixelformat);
	IPSLOG(1, "  Bytes per line: %d\n", format_get.fmt.pix.bytesperline);
	IPSLOG(1, "  Image size: %d\n", format_get.fmt.pix.sizeimage); 



	// # 01 <<
	end = std::chrono::high_resolution_clock::now();
	duration = end - start;
	strtmp = std::to_string(duration.count());
	//CAMM(" > # 01 >>()_CycleTime : %s (ms)\n",  strtmp.c_str());



	//// # 02 >>
	start = std::chrono::high_resolution_clock::now();


	//if (m_Prev_width != width ||
	//	m_Prev_height != height ||
	//	m_Prev_pixelFormat != pixelFormat) 
	{
		IPSLOG(1, ">_>!!  munmap() and m_vecData.clear()\n");
		if (!m_vecData.empty()) {

			v4l2_uninit_device();
			m_vecData.clear();
		}

		IPSLOG(1, "@_@!! VIDIOC_STREAMOFF  === === >>>\n");
		IPSLOG(1, "  >> Close stream I/O operations\n");
		int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		nRet = ioctl(pCam, VIDIOC_STREAMOFF, &type);
		if (nRet < 0) {
			IPSLOG(1, "VIDIOC_STREAMOFF: %s", strerror(errno));
		}

	}

	// # 02 <<
	end = std::chrono::high_resolution_clock::now();
	duration = end - start;
	strtmp = std::to_string(duration.count());
	//CAMM(" > # 02 >>()_CycleTime : %s (ms)\n",  strtmp.c_str());



	//// # 03 >>
	start = std::chrono::high_resolution_clock::now();


	v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.pixelformat = pixelFormat;
	format.fmt.pix.width = width;
	format.fmt.pix.height = height;
	//format.fmt.pix.field = V4L2_FIELD_INTERLACED;

	//class of yuyv2rgb_cvtr init().
	yuyv2rgb_cvtr.Init(width, height);

	// 輸出當前格式的信息
	IPSLOG(1, "Want to setting of format: \n");
	IPSLOG(1, "  Width : %d\n", width);
	IPSLOG(1, "  Height : %d\n", height);
	IPSLOG(1, "  Pixel format: 0x11x\n", pixelFormat);
	//IPSLOG(1, "  Bytes per line: %d", format_get.fmt.pix.bytesperline );
	//IPSLOG(1, "  Image size: %d", format_get.fmt.pix.sizeimage );


	// # 03 <<
	end = std::chrono::high_resolution_clock::now();
	duration = end - start;
	strtmp = std::to_string(duration.count());
	//CAMM(" > # 03 >>()_CycleTime : %s (ms)\n",  strtmp.c_str());



	//// # 04 >>
	start = std::chrono::high_resolution_clock::now();


	if (ioctl(camera, VIDIOC_S_FMT, &format) < 0) {
		IPSLOG(0, "VIDIOC_S_FMT: %s\n", strerror(errno));
		return -1;
	}

	//Record image information
	m_Prev_width = width;
	m_Prev_height = height;
	m_Prev_pixelFormat = pixelFormat;

	IPSLOG(1, "updateCtrlIDList()\n");
	UpdateCtrlIDList(camera);


	// # 04 <<
	end = std::chrono::high_resolution_clock::now();
	duration = end - start;
	strtmp = std::to_string(duration.count());
	//CAMM(" > # 04 >>()_CycleTime : %s (ms)\n",  strtmp.c_str());



	//// # 05 >>
	start = std::chrono::high_resolution_clock::now();


	IPSLOG(1, "\n");
	/////////////////////////////////////////////////
	// Start >>> 
	/////////////////////////////////////////////////
	IPSLOG(1, "@_@!! VIDIOC_S_EXT_CTRLS ; v4l2_ext_control ; --> v4l2_ext_controls ; === === >>>\n");
	IPSLOG(1, "  >> Set the value of multiple controls for ISP\n");

	IPSLOG(1, "v4l2_ext_controls ==> Sample for querying sensor caps, m_VivExtQctrl.id (%d)\n", m_VivExtQctrl.id);

	/* Sample for querying sensor caps */
	while (m_VivExtQctrl.id > 0) {

		//struct v4l2_ext_controls ectrls;
		//struct v4l2_ext_control ectrl;
		CLEAR(ectrls);
		CLEAR(ectrl);

		ectrl.string = new char[m_VivExtQctrl.elem_size];
		if (!ectrl.string)
			break;

		ectrl.id = m_VivExtQctrl.id;
		ectrl.size = m_VivExtQctrl.elem_size;
		ectrls.controls = &ectrl;
		ectrls.count = 1;

		/* JSON format string request */
		IPSLOG(1, "  {<id>:<sensor.query>}\n");
		strncpy(ectrl.string, "{<id>:<sensor.query>}", m_VivExtQctrl.elem_size - 1);

		/* Set the control: 'sensor.query' */
		if (ioctl(camera, VIDIOC_S_EXT_CTRLS, &ectrls) < 0) {

			IPSLOG(0, "VIDIOC_S_EXT_CTRLS: %s\n", strerror(errno));
			delete[] ectrl.string;
			break;
		}

		/* Get the result */
		if (ioctl(camera, VIDIOC_G_EXT_CTRLS, &ectrls) < 0) {

			IPSLOG(0, "VIDIOC_G_EXT_CTRLS: %s\n", strerror(errno));
		}
		CAMD("Sensor's caps (JSON format string):%s", ectrl.string);
		delete[] ectrl.string;

		break;
	}


	// # 05 <<
	end = std::chrono::high_resolution_clock::now();
	duration = end - start;
	strtmp = std::to_string(duration.count());
	//CAMM(" > # 05 >>()_CycleTime : %s (ms)\n",  strtmp.c_str());



	//// # 06 >>
	start = std::chrono::high_resolution_clock::now();


	IPSLOG(1, "\n");
	IPSLOG(1, "v4l2_ext_controls ==> Sample for sensor lib preloading & warm-up_vivExtQctrl.id(%d)\n", m_VivExtQctrl.id);
	while (m_VivExtQctrl.id > 0) {

		//# v4l2_ext_controls_?�於設置?�查詢擴展控?��?例�?：亮度、�?比度等�?        
		//struct v4l2_ext_controls ectrls;
		//struct v4l2_ext_control ectrl;
		CLEAR(ectrls);
		CLEAR(ectrl);

		ectrl.string = new char[m_VivExtQctrl.elem_size];
		if (!ectrl.string)
			break;
		ectrl.id = m_VivExtQctrl.id;
		ectrl.size = m_VivExtQctrl.elem_size;
		ectrls.controls = &ectrl;
		ectrls.count = 1;

		/* JSON format string request */
		IPSLOG(1, "{<id>:<sensor.lib.preload>}\n");
		strncpy(ectrl.string, "{<id>:<sensor.lib.preload>}", m_VivExtQctrl.elem_size - 1);

		/* Set the control 'sensor lib preload' */
		if (ioctl(camera, VIDIOC_S_EXT_CTRLS, &ectrls) < 0) {
			IPSLOG(0, "VIDIOC_S_EXT_CTRLS: %s\n", strerror(errno));
			delete[] ectrl.string;
			break;
		}

		/* Get the result */
		if (ioctl(camera, VIDIOC_G_EXT_CTRLS, &ectrls) < 0) {
			IPSLOG(0, "VIDIOC_S_EXT_CTRLS: %s\n", strerror(errno));
			delete[] ectrl.string;
			break;
		}

		/* JSON format string request */
		IPSLOG(1, "  {<id>:<pipeline.warm.up>}\n");
		strncpy(ectrl.string, "{<id>:<pipeline.warm.up>}", m_VivExtQctrl.elem_size - 1);

		/* Set the control: 'warm up' */
		if (ioctl(camera, VIDIOC_S_EXT_CTRLS, &ectrls) < 0) {
			IPSLOG(0, "VIDIOC_S_EXT_CTRLS: %s\n", strerror(errno));
			delete[] ectrl.string;
			break;
		}

		/* Get the result */
		if (ioctl(camera, VIDIOC_G_EXT_CTRLS, &ectrls) < 0) {
			IPSLOG(0, "VIDIOC_G_EXT_CTRLS: %s\n", strerror(errno));
		}

		delete[] ectrl.string;

		break;
	}




	// # 06 <<
	end = std::chrono::high_resolution_clock::now();
	duration = end - start;
	strtmp = std::to_string(duration.count());
	//CAMM(" > # 06 >>()_CycleTime : %s (ms)\n",  strtmp.c_str());


	////////////////////////////////////////////////////////////////
	// ## init_device() === >>
	////////////////////////////////////////////////////////////////
	//// # 08 >>
	start = std::chrono::high_resolution_clock::now();


	v4l2_init_device(camera);


	// # 09 <<
	end = std::chrono::high_resolution_clock::now();
	duration = end - start;
	strtmp = std::to_string(duration.count());
	//CAMM(" > # 08 >>()_CycleTime : %s (ms)\n",  strtmp.c_str());
	////////////////////////////////////////////////////////////////
	// ## init_mmap() << ===
	////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////
	// ## start_capturing() === >>
	////////////////////////////////////////////////////////////////
	//// # 09 >>
	start = std::chrono::high_resolution_clock::now();

	IPSLOG(1, "\n");
	IPSLOG(1, "@_@!! VIDIOC_STREAMON  === === >>>\n");
	IPSLOG(1, "  >> Start stream I/O operation, capture or output device\n");


	v4l2_start_capturing(camera);


	// # 09 <<
	end = std::chrono::high_resolution_clock::now();
	duration = end - start;
	strtmp = std::to_string(duration.count());
	//CAMM(" > # 09 >>()_CycleTime : %s (ms)\n",  strtmp.c_str());
	////////////////////////////////////////////////////////////////
	// ## start_capturing() << ===
	////////////////////////////////////////////////////////////////


	return nRet;
}


int CMethod_V4L2CamCtrl::Configure_Binning(const int pCam, const LpGigECamConfig pParamIn)
{
	IPSLOG(1, "Configure_Binning ==> \n");

	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	int camera = pCam;

	//Update statte of "iBinning_Scale" to globa structure.
	m_global_cfg_ParamInfo.iBinning_Scale = pParamIn->iBinning_Scale;


	IPSLOG(1, " ###  pParamIn->iBinning_Scale = %d\n", pParamIn->iBinning_Scale);
	IPSLOG(1, " ###  m_global_cfg_ParamInfo.iBinning_Scale = %d\n", m_global_cfg_ParamInfo.iBinning_Scale);

	return result;
}



int CMethod_V4L2CamCtrl::Configure_Decimation(const int pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	int camera = pCam;

	if (camera) {

		// TBD

	}
	else {
		/* En error happened, display the correspdonding message */
		IPSLOG(0, "error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return result;
}


int CMethod_V4L2CamCtrl::Configure_PersistentIP(const int pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	/* Connect to the first available camera */
	int camera = pCam;

	if (camera) {

		// TBD

	}
	else {
		/* En error happened, display the correspdonding message */
		IPSLOG(0, "error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}



int CMethod_V4L2CamCtrl::Configure_ResetConfigureCustomImageSettings(const int pCam)
{
	int result = 0;
	// TBD
	return result;
}


int CMethod_V4L2CamCtrl::Configure_ResetExposure(const int pCam)
{
	int result = 0;
	//TBD
	return result;
}


int CMethod_V4L2CamCtrl::Configure_Get(const int pCam, LpGigECamConfig pParamOut, bool bDumpInf)
{
	if (nullptr == pParamOut)
		return -1;

	int nRet = 0, i =0;
	int nID = 0;
	int width = 0, height = 0;
#if 0
	/* Connect to the first available camera */
	int camera = pCam;

	LpGigECamConfig pParam = pParamOut;

	if (camera) {

		//#viv_caps_supports_相機裝置影像格式資訊
		IPSLOG(0, "\n");
		IPSLOG(1, "$_$?? VIV_VIDIOC_GET_CAPS_SUPPORTS ; viv_caps_supports  ### ### >>>\n");
		IPSLOG(1, " >> viv_caps_supports_ NXP define : VIV_xxxx information\n");
		do {

			CLEAR(m_Caps_supports);
			nRet = ioctl(camera, VIV_VIDIOC_GET_CAPS_SUPPORTS, &m_Caps_supports);
			i++;

		} while (nRet && i < 2);

		if(nRet) {

			IPSLOG(0, "Video Test:Get Caps Supports Failed[%d]\n", nRet);
			return -1;
		}		
		else {

			IPSLOG(1, "# Video Test:caps supports:{\n");
			IPSLOG(1, "\tcount = %02d\n", m_Caps_supports.count);
			nID = ( m_Caps_supports.count > 0 ) ? (m_Caps_supports.count-1) : 0;

			for (unsigned int i = 0; i < m_Caps_supports.count; i++) {
				IPSLOG(1, "\t{\n");
				IPSLOG(1, "\tindex            = %d\n", m_Caps_supports.mode[i].index);
				IPSLOG(1, "\tbounds_width     = %d\n", m_Caps_supports.mode[i].bounds_width);
				IPSLOG(1, "\tbounds_height    = %d\n", m_Caps_supports.mode[i].bounds_height);
				IPSLOG(1, "\ttop              = %d\n", m_Caps_supports.mode[i].top);
				IPSLOG(1, "\tleft             = %d\n", m_Caps_supports.mode[i].left);
				IPSLOG(1, "\twidth            = %d\n", m_Caps_supports.mode[i].width);
				IPSLOG(1, "\theight           = %d\n", m_Caps_supports.mode[i].height);
				IPSLOG(1, "\thdr_mode         = %d\n", m_Caps_supports.mode[i].hdr_mode);
				IPSLOG(1, "\tstitching_mode   = %d\n", m_Caps_supports.mode[i].stitching_mode);
				IPSLOG(1, "\tbit_width        = %d\n", m_Caps_supports.mode[i].bit_width);
				IPSLOG(1, "\tbayer_pattern    = %d\n", m_Caps_supports.mode[i].bayer_pattern);
				IPSLOG(1, "\tfps              = %d\n", m_Caps_supports.mode[i].fps);
				IPSLOG(1, "\t}\n");
			}
			IPSLOG(1, "}\n");
		}


		//# viv_caps_mode_s 
		IPSLOG(1, "$_$?? VIV_VIDIOC_S_CAPS_MODE ; viv_caps_mode_s  ### ### >>>\n");
		IPSLOG(1, " >> viv_caps_supports_ NXP define : VIV_xxxx information\n");
		IPSLOG(1, "\n");
		IPSLOG(1, "viv_caps_mode_s <--sensorMode(%d) \n", m_SensorMode);
		if (m_SensorMode >= 0) {

			struct viv_caps_mode_s caps_mode;
			CLEAR(caps_mode);

			caps_mode.mode = m_SensorMode;

			if (ioctl(camera, VIV_VIDIOC_S_CAPS_MODE, &caps_mode) == 0) {

				IPSLOG(1, "Video Test: Set mode[%d]\n", caps_mode.mode);
			}
			else {

				IPSLOG(0, "Video Test: Set mode[%d] Failed\n", caps_mode.mode);
				return -1;
			}
		}
		IPSLOG(1, "\n");


/*
		//# Exposure Time limit
		// fetch current value
		IPSLOG(1, "$_$?? IF_EC_G_CFG ; Json::Value jRequest, jResponse  ### ### >>>\n");
		IPSLOG(1, " >> Json::Value jRequest, jResponse_ NXP define : ExposureTime\n");
		IPSLOG(1, "\n");
		Json::Value jRequest, jResponse;
		int ret = viv_private_ioctl(camera, IF_EC_G_CFG, jRequest, jResponse);
		if (S_EXT_FLAG != ret) {

			m_fLimit_ExposureTime_Min = jResponse[EC_INTEGRATION_MIN_PARAMS].asDouble();
			m_fLimit_ExposureTime_Max = jResponse[EC_INTEGRATION_MAX_PARAMS].asDouble();
			double currentInt = jResponse[EC_TIME_PARAMS].asDouble();

			IPSLOG(1, "Exposure time value( %f ~ %f )%f:\n", m_fLimit_ExposureTime_Min, m_fLimit_ExposureTime_Max, currentInt);

			m_fLimit_Gain_Min = jResponse[EC_GAIN_MIN_PARAMS].asDouble();
			m_fLimit_Gain_Max = jResponse[EC_GAIN_MAX_PARAMS].asDouble();
			double currentGain = jResponse[EC_GAIN_PARAMS].asDouble();

			IPSLOG(1, "Gain value( %f ~ %f )%f:\n", m_fLimit_Gain_Min, m_fLimit_Gain_Max, currentGain);

		}
		else {

			IPSLOG(0, "EC exposure time get failed, return\n");
			IPSLOG(0, "EC gain get failed, return\n");
			//return;
		}

//*/


		//# v4l2_fmtdesc_設備支持的像素格式描述
		IPSLOG(1, "@_@!! VIDIOC_ENUM_FMT ; v4l2_fmtdesc === === >>>\n");
		IPSLOG(1, "  >> Enumerate all data formats supported by the device\n");

		v4l2_fmtdesc formatDescriptions;
		formatDescriptions.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		for (int i = 0; true; i++) {

			formatDescriptions.index = i;

			if (ioctl(camera, VIDIOC_ENUM_FMT, &formatDescriptions) == 0) {

				IPSLOG(1, "  %2d: %s 0x%08X 0x%X\n", 
					i,
					formatDescriptions.description,
					formatDescriptions.pixelformat,
					formatDescriptions.flags
				);

				switch (formatDescriptions.pixelformat)
				{
				case V4L2_PIX_FMT_SBGGR8:
				case V4L2_PIX_FMT_SGBRG8:
				case V4L2_PIX_FMT_SGRBG8:
				case V4L2_PIX_FMT_SRGGB8:
					mediaUVCFormat2V4l2Format[MEDIA_PIX_FMT_RAW8] = formatDescriptions.pixelformat;
					break;
				case V4L2_PIX_FMT_SBGGR10:
				case V4L2_PIX_FMT_SGBRG10:
				case V4L2_PIX_FMT_SGRBG10:
				case V4L2_PIX_FMT_SRGGB10:
					mediaUVCFormat2V4l2Format[MEDIA_PIX_FMT_RAW10] = formatDescriptions.pixelformat;
					break;
				case V4L2_PIX_FMT_SBGGR12:
				case V4L2_PIX_FMT_SGBRG12:
				case V4L2_PIX_FMT_SGRBG12:
				case V4L2_PIX_FMT_SRGGB12:
					mediaUVCFormat2V4l2Format[MEDIA_PIX_FMT_RAW12] = formatDescriptions.pixelformat;
					break;
				case V4L2_PIX_FMT_NV16:
					mediaUVCFormat2V4l2Format[MEDIA_PIX_FMT_YUV422SP] = formatDescriptions.pixelformat;
					break;
				case V4L2_PIX_FMT_YUYV:
					mediaUVCFormat2V4l2Format[MEDIA_PIX_FMT_YUV422I] = formatDescriptions.pixelformat;
					break;
				case V4L2_PIX_FMT_NV12:
					mediaUVCFormat2V4l2Format[MEDIA_PIX_FMT_YUV420SP] = formatDescriptions.pixelformat;
					break;
				}

			}
			else {
				break;
			}
		}

		IPSLOG(1, "Final Selection__mediaUVCFormat2V4l2Format[]_List:\n");
		for (const auto& pair : mediaUVCFormat2V4l2Format) {

			IPSLOG(1, "   case_%2d: 0x%08X\n", pair.first, pair.second);
		}
		IPSLOG(1, "\n");

		IPSLOG(1, "@_@!! VIDIOC_G_FMT ; v4l2_format === === >>>\n");
		IPSLOG(1, " >> Get data format\n");
		v4l2_format format_get;
		format_get.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		// 調用 VIDIOC_G_FMT 以獲取當前格式
		if (ioctl(camera, VIDIOC_G_FMT, &format_get) < 0) {

			IPSLOG(0, "VIDIOC_G_FMT: %s\n", strerror(errno));
			return -1;
		}
		// 輸出當前格式的信息
		IPSLOG(1, "Current format: \n");
		IPSLOG(1, "  Width: %d\n", format_get.fmt.pix.width);
		IPSLOG(1, "  Height: %d\n", format_get.fmt.pix.height);
		IPSLOG(1, "  Pixel format: 0x%llx\n", format_get.fmt.pix.pixelformat);
		IPSLOG(1, "  Bytes per line: %d\n", format_get.fmt.pix.bytesperline);
		IPSLOG(1, "  Image size: %d\n", format_get.fmt.pix.sizeimage);
		

		// RexTYW
		pParam->bIsConnected = true;

		pParam->iSensor_Width	= m_Caps_supports.mode[nID].bounds_width;
		pParam->iSensor_Height	= m_Caps_supports.mode[nID].bounds_height;


		pParam->iBinning_Scale = m_global_cfg_ParamInfo.iBinning_Scale;

		pParam->bPixelFormat = 1;	// (cfg_PixelFormat.nCurValue == 0x01080001) ? 0 : 1;  //0:PixelFormat_Mono8, 1:PixelFormat_BayerGR8

		//pParam->iOffset_X = m_Caps_supports.mode[nID].left;
		//pParam->iOffset_Y = m_Caps_supports.mode[nID].top;

		//pParam->iWidth = m_Caps_supports.mode[nID].width;
		//pParam->iHeight = m_Caps_supports.mode[nID].height;

		//pParam->bIsEnbCrop = m_global_cfg_ParamInfo.bIsEnbCrop;
		//pParam->iOffset_X = m_global_cfg_ParamInfo.iOffset_X;
		//pParam->iOffset_Y = m_global_cfg_ParamInfo.iOffset_Y;
		//pParam->iWidth = m_global_cfg_ParamInfo.iWidth;
		//pParam->iHeight = m_global_cfg_ParamInfo.iHeight;

		//pParam->bExposureAuto = (cfg_Exposure_modeto.nCurValue == 2) ? 0 : 1;     //0:Auto, 1:Timed;

		//pParam->bIsEnbAcquisitionFrameRate = cfg_bIsEnbAcquisitionFrameRate;
		//pParam->dbAcquisitionFrameRate = cfg_dbAcquisitionFrameRate.fCurValue;

		if (bDumpInf) {

			IPSLOG(1, "\n\n>>> Device information === >>> === >>>\n");
			IPSLOG(1, "\t	@ Cfg.bIsConnected---> %d\n", pParam->bIsConnected);
			//IPSLOG(1, "\t	@ Cfg.bIsEnbAcquisitionFrameRate---> %s\n", (pParam->bIsEnbAcquisitionFrameRate) ? "True" : "False");
			//IPSLOG(1, "\t	@ Cfg.dbAcquisitionFrameRate---> %5.3f\n", pParam->dbAcquisitionFrameRate);
			//IPSLOG(1, "\t	@ Cfg.bExposureAuto---> %s\n", (!pParam->bExposureAuto) ? "Auto" : "Off");
			//IPSLOG(1, "\t	@ Cfg.bPixelFormat---> %s\n", (pParam->bPixelFormat) ? "RGB8" : "Mono8");
			IPSLOG(1, "\t	@ Cfg.iMax_Width ---> %d\n", pParam->iSensor_Width);
			IPSLOG(1, "\t	@ Cfg.iMax_Height---> %d\n", pParam->iSensor_Height);
			//IPSLOG(1, "\t	@ Cfg.iBinning_Scale---> %d\n", pParam->iBinning_Scale);
			//IPSLOG(1, "\t	@ Cfg.bIsEnbCrop---> %d\n", pParam->bIsEnbCrop);
			//IPSLOG(1, "\t	  # Cfg.iOffset_X ---> %d\n", pParam->iOffset_X);
			//IPSLOG(1, "\t	  # Cfg.iOffset_Y---> %d\n", pParam->iOffset_Y);
			//IPSLOG(1, "\t	  # Cfg.iWidth ---> %d\n", pParam->iWidth);
			//IPSLOG(1, "\t	  # Cfg.iHeight---> %d\n", pParam->iHeight);

			IPSLOG(1, "<<< Device information === <<< === <<< === ===\n\n");

		}

	}
	else {

		*pParam = seGigECamConfig();

		return -1;

	}
#endif
	return nRet;

}

#if 0
int CMethod_V4L2CamCtrl::viv_private_ioctl(const int pCam, const char* cmd, Json::Value& jsonRequest, Json::Value& jsonResponse) 
{

	if (!cmd) {
		IPSLOG(0, "cmd should not be null!\n");
		return -1;
	}

	jsonRequest["id"] = cmd;
	jsonRequest["streamid"] = m_Streamid;

	struct v4l2_ext_controls ecs;
	struct v4l2_ext_control ec;
	CLEAR(ecs);
	CLEAR(ec);
	ec.string = new char[VIV_JSON_BUFFER_SIZE];
	ec.id = V4L2_CID_VIV_EXTCTRL;
	ec.size = 0;
	ecs.controls = &ec;
	ecs.count = 1;

	::ioctl(pCam, VIDIOC_G_EXT_CTRLS, &ecs);

	strcpy(ec.string, jsonRequest.toStyledString().c_str());

	int ret = ::ioctl(pCam, VIDIOC_S_EXT_CTRLS, &ecs);
	if (ret != 0) {
		IPSLOG(0, "failed to set ext ctrl\n");
		goto end;
	}
	else {
		::ioctl(pCam, VIDIOC_G_EXT_CTRLS, &ecs);
		Json::Reader reader;
		reader.parse(ec.string, jsonResponse, true);
		delete[] ec.string;
		ec.string = nullptr;
		return jsonResponse["MC_RET"].asInt();
	}

end:
	delete ec.string;
	ec.string = nullptr;
	return S_EXT_FLAG;
}
#endif


int CMethod_V4L2CamCtrl::v4l2_xioctl(int fh, int request, void* arg)
{
	int nRet = 0;

	do {
		nRet = ioctl(fh, request, arg);
	} while (-1 == nRet && EINTR == errno);

	return nRet;

}


int CMethod_V4L2CamCtrl::v4l2_open_device(int iId)
{
	int fd = -1;
	char szFile[64] = { '\0' };

	sprintf(szFile, "/dev/video%d", iId);

	fd = ::open(szFile, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		IPSLOG(0, "can't open video file %s\n", szFile);
		return -1;
	}

	m_strDeviceName = szFile;

	return fd;
}


int CMethod_V4L2CamCtrl::v4l2_init_device(const int pCam)
{
	int nRet = 0;

	int camera = pCam;

	if (camera) {

		IPSLOG(1, "@_@!! VIDIOC_REQBUFS ; v4l2_requestbuffers  === === >>>\n");
		IPSLOG(1, "  >> Request a video buffer from the device, that is, initialize the video buffer\n");

		m_Bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		m_Bufrequest.memory = V4L2_MEMORY_MMAP;
		m_Bufrequest.count = m_BUFFER_COUNT;

		if (ioctl(camera, VIDIOC_REQBUFS, &m_Bufrequest) < 0) {
			IPSLOG(0, "VIDIOC_REQBUFS: %s\n", strerror(errno));
			return -1;
		}


		IPSLOG(1, "@_@!! VIDIOC_QUERYBUF :  v4l2_buffer === === >>>\n");
		IPSLOG(1, "   >> Query the status of the buffer\n");

		for (int i = 0; i < m_BUFFER_COUNT; i++) {

			IPSLOG(1, "BUFFER_COUNT_%02d\n", i);

			CLEAR(m_Buffer);
			m_Buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			m_Buffer.memory = V4L2_MEMORY_MMAP;
			m_Buffer.index = i;

			if (ioctl(camera, VIDIOC_QUERYBUF, &m_Buffer) < 0) {
				IPSLOG(0, "VIDIOC_QUERYBUF: %s\n", strerror(errno));
				return -1;
			}

			IPSLOG(1, "@_@!! VIDIOC_QBUF_%d ; v4l2_buffer  === === >>>\n", i);
			IPSLOG(1, "   >> Get a frame of video data from the device\n");

			IPSLOG(0, "Buffer description:\n");
			IPSLOG(0, "  offset: %d\n", m_Buffer.m.offset);
			IPSLOG(0, "  length: %d\n", m_Buffer.length);
			if (ioctl(camera, VIDIOC_QBUF, &m_Buffer) < 0) {
				IPSLOG(0, "VIDIOC_QBUF: %s\n", strerror(errno));
				return -1;
			}

			IPSLOG(1, "@_@!! mmap()  === === >>>\n");
			IPSLOG(1, "  >> mmap __ map the file into memory and get handler of void* data\n");
			void* data = mmap(
				nullptr,
				m_Buffer.length,
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				camera,
				m_Buffer.m.offset
			);

			if (data == MAP_FAILED) {
				IPSLOG(0, "mmap: %s\n", strerror(errno));
				return -1;
			}

			IPSLOG(1, "  >> mmap __ push data of map buffer(%p) to std::vector()\n", data);
			m_vecData.push_back(data);
		}

		IPSLOG(1, "\n");
		IPSLOG(1, ">_>!! m_vecData.size() = %d\n", m_vecData.size());

	}
	else {
		/* En error happened, display the correspdonding message */
		IPSLOG(0, "error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}


int CMethod_V4L2CamCtrl::v4l2_start_capturing(const int pCam)
{
	int nRet = 0;

	int camera = pCam;

	if (camera) {

		CLEAR(m_Buffer);
		m_Buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		m_Buffer.memory = V4L2_MEMORY_MMAP;
		m_Buffer.index = 0;

		int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(camera, VIDIOC_STREAMON, &type) < 0) {
			IPSLOG(0, "VIDIOC_STREAMON: %s\n", strerror(errno));
			// return -1;
		}

	}
	else {
		/* En error happened, display the correspdonding message */
		IPSLOG(0, "error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}


int CMethod_V4L2CamCtrl::v4l2_stop_capturing(const int pCam)
{
	int nRet = 0;

	int camera = pCam;

	if (camera) {

		IPSLOG(1, "@_@!! VIDIOC_STREAMOFF  === === >>>\n");
		IPSLOG(1, "  >> Close stream I/O operations\n");

		int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		ioctl(camera, VIDIOC_STREAMOFF, &type);

	}
	else {
		/* En error happened, display the correspdonding message */
		IPSLOG(0, "error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}

int CMethod_V4L2CamCtrl::v4l2_uninit_device()
{
	int nRet = 0;

	for (int i = 0; i < m_BUFFER_COUNT; i++) 
	{
		munmap(m_vecData[i], m_Buffer.length);
	}

	return nRet;
}

int CMethod_V4L2CamCtrl::v4l2_close_device()
{
	int nRet = 0;

	close(m_pCam);
	m_pCam = -1;

	return nRet;
}



#define def_Debug (1)

int CMethod_V4L2CamCtrl::AcquireImages(const int pCam, string strFilePath)
{
	int nRet = 0;

	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;


	/* Connect to the first available camera */
	int camera = pCam;

	v4l2_start_capturing(pCam);

	if (camera) {


		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(camera, &fds);
		select(camera + 1, &fds, nullptr, nullptr, nullptr);

		IPSLOG(1, "@_@!! VIDIOC_DQBUF ; v4l2_buffer  === === >>>\n");
		IPSLOG(1, "  >> Return the video buffer to the device\n");


		if (ioctl(camera, VIDIOC_DQBUF, &m_Buffer) < 0) {

			IPSLOG(0, "VIDIOC_DQBUF: %s\n", strerror(errno));
			ioctl(camera, VIDIOC_QBUF, &m_Buffer);
			return -1;
		}
		else {

			IPSLOG(1, "  buffer.index = %02d\n", m_Buffer.index);

			IPSLOG(1, "yuyv2rgb_cvtr.RunCPU(..Path..)\n");
			yuyv2rgb_cvtr.RunCPU(static_cast<const unsigned char*>(m_vecData[m_Buffer.index]), strFilePath.c_str());


			IPSLOG(1, "@_@!! VIDIOC_QBUF ; v4l2_buffer  === === >>>\n");
			IPSLOG(1, "  >> Get a frame of video data from the device\n");

			ioctl(camera, VIDIOC_QBUF, &m_Buffer);

			IPSLOG(1, "\n");

		}   //else

		usleep(50000); /* delay 50  ms */ 

	}
	else {
		/* En error happened, display the correspdonding message */
		IPSLOG(0, "error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	v4l2_stop_capturing(pCam);

	return nRet;

}



int CMethod_V4L2CamCtrl::AcquireImages(const int pCam, cv::Mat& matImg)
{
	int nRet = 0;

	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;


	/* Connect to the first available camera */
	int camera = pCam;

	v4l2_start_capturing(pCam);

	if (camera) {

		cv::Mat mat_img;

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(camera, &fds);
		select(camera + 1, &fds, nullptr, nullptr, nullptr);

		IPSLOG(1, "@_@!! VIDIOC_DQBUF ; v4l2_buffer  === === >>>\n");
		IPSLOG(1, "  >> Return the video buffer to the device\n");

		if (ioctl(camera, VIDIOC_DQBUF, &m_Buffer) < 0) {

			IPSLOG(0, "VIDIOC_DQBUF: %s\n", strerror(errno));
			ioctl(camera, VIDIOC_QBUF, &m_Buffer);
			return -1;
		}
		else {

			IPSLOG(1, "  buffer.index = %02d\n", m_Buffer.index);

			IPSLOG(1, "yuyv2rgb_cvtr.RunCPU(..cv::Mat..)\n");
			yuyv2rgb_cvtr.RunCPU(static_cast<const unsigned char*>(m_vecData[m_Buffer.index]), mat_img);

			matImg = mat_img.clone();

			mat_img.release();

			IPSLOG(1, "@_@!! VIDIOC_QBUF ; v4l2_buffer  === === >>>\n");
			IPSLOG(1, "  >> Get a frame of video data from the device\n");

			ioctl(camera, VIDIOC_QBUF, &m_Buffer);

			IPSLOG(1, "\n");

		}   //else

		usleep(50000); /* delay 50  ms */

	}
	else {
		/* En error happened, display the correspdonding message */
		IPSLOG(0, "error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	v4l2_stop_capturing(pCam);

	return nRet;

}


int cnt = 0;
char name[1024] = {'\0'};
#if 0
int CMethod_V4L2CamCtrl::AcquireStreaming(const int pCam, cv::Mat& matImg)
{
	int nRet = 0;

	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;

	/* Connect to the first available camera */
	int camera = pCam;

	if (camera) {

		//start >>
		start = std::chrono::high_resolution_clock::now();

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(camera, &fds);
		select(camera + 1, &fds, nullptr, nullptr, nullptr);

		// # func()
		nRet = ioctl(camera, VIDIOC_DQBUF, &m_Buffer);

		// end <<
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		strtmp = std::to_string(duration.count());
		////IPSLOG(0, " #01 _ ioctl of VIDIOC_DQBUF_CycleTime : %s (ms)\n", strtmp.c_str());


		if (nRet < 0) {

			IPSLOG(0, " Error !!! > VIDIOC_DQBUF: %s\n", strerror(errno));
		}
		else {

			//start >>
			start = std::chrono::high_resolution_clock::now();
			
			// # func()
#if(1)

			// # CPU --> OpenCV
			yuyv2rgb_cvtr.RunCPU(m_vecData[m_Buffer.index], matImg);

			//  # GPU --> OpenCL
			// yuyv2rgb_cvtr.RunGPU(m_vecData[m_Buffer.index], matImg);

			// # GPU --> Nxp_G2D 
			//g2d_cvtr.RunG2D(m_vecData[m_Buffer.index], matImg);

#else

			// GPU --> OpenCL
			yuyv2rgb_cvtr.RunGPU(m_vecData[m_Buffer.index], matImg);
#endif

			// end <<
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			strtmp = std::to_string(duration.count());
			////IPSLOG(0, " #02 _ yuyv2rgb_cvtr_CycleTime : %s (ms)\n", strtmp.c_str());

			//start >>
			start = std::chrono::high_resolution_clock::now();

			// # func()
			ioctl(camera, VIDIOC_QBUF, &m_Buffer);

			// end <<
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			strtmp = std::to_string(duration.count());
			////IPSLOG(0, " #03 _ ioctl of VIDIOC_QBUF_CycleTime : %s (ms)\n", strtmp.c_str());
		}


		//usleep(40000); /* delay 40  ms */
		//IPSLOG(0, " # Warning : Increasing delay time( 40ms ) can reduce CPU usage!!!\n");
		//IPSLOG(0, " # Warning : Increasing delay time( 40ms ) can reduce CPU usage!!!\n");

	}
	else {
		/* En error happened, display the correspdonding message */
		IPSLOG(0, " Error !!! > OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}
#else
int CMethod_V4L2CamCtrl::AcquireStreaming(const int pCam, cv::Mat& matImg)
{
	int nRet = 0;

	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;

	cap_uvc.read(matImg);
    // 檢查是否成功讀取畫面
    if (matImg.empty()) {
        std::cerr << "Error: Unable to read cap_uvc." << std::endl;
    } else {
        memset(&m_Buffer, 0, sizeof(m_Buffer));
        m_Buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_Buffer.memory = V4L2_MEMORY_MMAP;

        m_Buffer.m.userptr = reinterpret_cast<unsigned long>(matImg.data);
        m_Buffer.length = matImg.total() * matImg.elemSize();

        #if 0
        {
            std::ofstream outputFile("/tmp/test.yuv", std::ios::binary);
            if (!outputFile.is_open()) {
                std::cerr << "Error: Could not open output file." << std::endl;
                return -1;
            }

            outputFile.write(reinterpret_cast<const char*>(matImg.data), matImg.total() * matImg.elemSize());
            outputFile.close();
        }

        {
            //auto img = cv::imread("/home/ubuntu/ubuntu/for_joe/1.png");
            // cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
            int width = 640;
            int height = 480;
            cv::Mat img(height, width, CV_8UC3);
            cv::cvtColor(matImg, img, cv::COLOR_YUV2RGB); //<-- Good~~

            //auto img = cv::imread("/home/ubuntu/ubuntu/for_joe/1.png");
            //cv::Mat resizedImage;
            //resize(img, resizedImage, targetSize); 
            //cv::resize(img, resizedImage, cv::Size(360, 360));

            int new_width = 320;
            int new_height = 320;

            float scale_x = static_cast<float>(new_width) / 640;
            float scale_y = static_cast<float>(new_height) / 480;

            cv::Mat resized_image;
            cv::resize(img, resized_image, cv::Size(), scale_x, scale_y);
        
            auto t1 = chrono::high_resolution_clock::now();
            auto t2 = chrono::high_resolution_clock::now();
            auto t = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
            //cout << "infer: " << t / 1000. << " ms, " << endl;
        }

        #endif
        
    }
	return nRet;
}
#endif

int CMethod_V4L2CamCtrl::AcquireStreaming_Prepare()
{
	int camera = m_pCam;

	CAMD("AcquireStreaming_Prepare(...) >>>\n");
	cout << " > tid = " << tid << endl;
	cout << " > tid = " << tid << endl;

	if (camera) {

		if (bIsCreated) {

			IPSLOG(1, "# The Thread already creatr()...\n");
			return 0;
		}

		notified_IsDone = false;
		notified_IsRun = false;
		IPSLOG(1, "Setting Thread default value.\n");
		IPSLOG(1, " > notified_IsDone = %d\n", notified_IsDone);
		IPSLOG(1, " > notified_IsRun = %d\n", notified_IsRun);

		seGigECamConfig pParam = seGigECamConfig();

		//std::thread t1(&CMethod_V4L2CamCtrl::Thread_Acquire, this, camera, &pParam);

		// Create a callback function that binds to the dumpinfo method of this instance
		std::function<void(const std::string&)> callback = std::bind(&CMethod_V4L2CamCtrl::Streaming_CallbackEx, this, std::placeholders::_1);

		std::thread t1(&CMethod_V4L2CamCtrl::Thread_Acquire, this, camera, &pParam, callback);
				
		tid = t1.get_id();
		t1.detach();

		bIsCreated = true;


		cout << " # >> tid = " << tid << endl;

	}

	CAMD("AcquireStreaming_Prepare(...) <<<\n");

	return 0;
}


int CMethod_V4L2CamCtrl::AcquireStreaming_Inquiry(seStreamingInfo* pStrmInfo)
{	
	std::unique_lock<std::mutex> lock(u_mutex);

	pStrmInfo->IsEnbStreaming = m_global_cfg_ParamInfo.bIsStreaming;

	IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());
	IPSLOG(0, " ## pStrmInfo->IsEnbStreaming = %d\n", pStrmInfo->IsEnbStreaming);

	return 0;
}


int CMethod_V4L2CamCtrl::AcquireStreaming_StartorStop(bool bflgEnableThd)
{
	IPSLOG(1, "AcquireStreaming_StartorStop(...) >>>\n");
	IPSLOG(1, " bflgEnableThd = %d\n", bflgEnableThd);

	std::unique_lock<std::mutex> lock(u_mutex);

	if (bflgEnableThd) {

		IPSLOG(1, " # Setting Thread ==> START~ START~ \n");
		notified_IsRun = true;

		IPSLOG(1, " > notified_IsDone = %d\n", notified_IsDone);
		IPSLOG(1, " > notified_IsRun = %d\n", notified_IsRun);

	}
	else {

		usleep(50000); /* delay 50  ms */

		IPSLOG(1, " # Setting Thread ==> STOP! STOP! \n");
		notified_IsRun = false;

		IPSLOG(1, " > notified_IsDone = %d\n", notified_IsDone);
		IPSLOG(1, " > notified_IsRun = %d\n", notified_IsRun);

	}

	// RexTYW_20231002_Changed the function to Thread_Acquire()
	//Streaming_Setting(notified_IsRun);
	//IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());


	m_iFrameRate_Cnt = 0;

	IPSLOG(1, " cond_var.notify_one()\n");
	cond_var.notify_one();

	IPSLOG(1, "AcquireStreaming_StartorStop(...) <<<\n");


	return 0;
}

// Old RingBuffer of OverWrite.
//int CMethod_V4L2CamCtrl::AcquireStreaming_Capture(cv::Mat& matImg)
//{
//	//std::unique_lock<std::mutex> lock(u_mutex);
//
//	int nRet = 0;
//
//	if (Streaming_Inquiry()) {
//
//		CAMD(" # @ GigECam_Strm_AcquireImages -> pop()\n");
//
//		if (g_owRingbuffer.pop(matImg)) {
//
//			CAMD(" OK ### g_owRingbuffer.pop is Successful. \n");
//
//			CAMD(" @ The Size of RingBuffer = %s\n", std::to_string(g_owRingbuffer.size()));
//		}
//		else {
//
//			IPSLOG(0, " ## Warning, g_owRingbuffer.pop of buffer is empty!!! \n");
//			printf(" ##[ %s ]; >> Warning, g_owRingbuffer.pop of buffer is empty!!! \n", __func__);
//			return 7; // 7 : Buffer is Empty()
//		}
//	}
//	else {
//
//		IPSLOG(0, "Warning: The Streaming is not running. Please check again.\n");
//		return -1;
//	}
//
//	return nRet;
//}



int CMethod_V4L2CamCtrl::AcquireStreaming_Capture(cv::Mat& matImg)
{
	//std::unique_lock<std::mutex> lock(u_mutex);

	int nRet = 0;

	if (Streaming_Inquiry()) {

		TimestampedValue<cv::Mat> tmpMat;

		CAMD(" # @ GigECam_Strm_AcquireImages -> pop()\n");

		if (g_timerRingbuffer.pop(tmpMat)) {

			CAMD(" OK[ timerRing ] ### g_timerRingbuffer.pop is Successful. \n");

			tmpMat.value.copyTo(matImg);

			CAMD(" @ The Size of timerRingBuffer = %s\n", std::to_string(g_timerRingbuffer.size()));

		}
		else {

			IPSLOG(0, " ## Warning[ timerRing ], g_timerRingbuffer.pop of buffer is empty!!! \n");
			printf(" ##[ %s ]; >> Warning[ timerRing ], g_timerRingbuffer.pop of buffer is empty!!! \n", __func__);
			return 7; // 7 : Buffer is Empty()
		}
	}
	else {

		IPSLOG(0, "Warning: The Streaming is not running. Please check again.\n");
		return -1;
	}

	return nRet;
}


int CMethod_V4L2CamCtrl::AcquireStreaming_Capture(cv::Mat& matImg, int iTimeVal)
{
	//std::unique_lock<std::mutex> lock(u_mutex);

	const int nlimitcnt_ByPass = 2;

	int nRet = 0;
	int nRes = 0;
	int nlimitcnt = 0;
	int nByPassCnt = 0;
	int nSkipTime_ms = 71;	// => 1 /fps(15) = 71ms
	int nSkipTime_us = 71 * 1000;	// => 1 /fps(15) = 71000us

	iTimeVal = (iTimeVal > 500) ? 500 : iTimeVal;

	if (Streaming_Inquiry()) {

		auto nowTimestamp = std::chrono::system_clock::now() + std::chrono::milliseconds(nSkipTime_ms);
		IPSLOG(0, " # [ timerRing ] >> nowTimestamp : %s\n", std::string(formatTimestamp(nowTimestamp)).c_str());

		auto delayTimestamp = nowTimestamp + std::chrono::milliseconds(iTimeVal);
		IPSLOG(0, " # [ timerRing ] >> delayTimestamp : %s\n", std::string(formatTimestamp(delayTimestamp)).c_str());

		TimestampedValue<cv::Mat> tmpMat;


		// Step 1: Skip outdated images of 2 frames using the current timestamp.
		while (nlimitcnt < (33))
		{

			//nRes = g_timerRingbuffer.pop(tmpMat);
			//usleep(nSkipTime_us);	// = 1 /fps(20) = 50ms
			nRes = g_timerRingbuffer.pop(tmpMat);

			IPSLOG(0, " # [ timerRing ] >> RingBuffer Time : %s\n", std::string(formatTimestamp(tmpMat.timestamp)).c_str());

			if ((tmpMat.timestamp > nowTimestamp) && (nRes)) {

				nByPassCnt++;
				IPSLOG(0, " # [ timerRing ] >> Step_1 >> nByPassCnt++ > ( %d )\n", nByPassCnt);

				if (nByPassCnt < nlimitcnt_ByPass)
				{
					usleep(nSkipTime_us);	// = 1 /fps(20) = 50ms

					IPSLOG(0, " # [ timerRing ] >> Step_1 >> nByPassCnt on going~~~ > ( %d )\n", nByPassCnt);

					nlimitcnt++;

					continue;
				}
				else
				{
					IPSLOG(0, " # [ timerRing ] >> Step_1 >> nByPassCnt is done. > ( %d )\n", nByPassCnt);
					IPSLOG(0, " # [ timerRing ] >> Step_1 >> RingBuffer Successful ~~~\n");

					break;
				}
			}
			else {

				usleep(nSkipTime_us); // = 1 /fps(20) = 50ms

				nlimitcnt++;

				IPSLOG(0, " # [ timerRing ] >> nlimitcnt is %d\n", nlimitcnt);
				IPSLOG(0, " # [ timerRing ] >> nRes is %d\n", nRes);
				IPSLOG(0, " # [ timerRing ] >> RingBuffer Next....\n");
				continue;
			}

		}

		// Step_2: Skip outdated images using the timestamp + delaytime.
		if ((tmpMat.timestamp < delayTimestamp) && (iTimeVal)) {

			nlimitcnt = 0;

			while (nlimitcnt < (30))
			{

				nRes = g_timerRingbuffer.pop(tmpMat);

				IPSLOG(0, " # [ timerRing ] >> RingBuffer Time : %s\n", std::string(formatTimestamp(tmpMat.timestamp)).c_str());

				if ((tmpMat.timestamp > delayTimestamp) && (nRes)) {

					IPSLOG(0, " # [ timerRing ] >> Step_2 >> RingBuffer Successful ~~~\n");

					break;
				}
				else {

					usleep(nSkipTime_us); // = 1 /fps(20) = 50ms

					nlimitcnt++;

					IPSLOG(0, " # [ timerRing ] >> nlimitcnt is %d\n", nlimitcnt);
					IPSLOG(0, " # [ timerRing ] >> nRes is %d\n", nRes);
					IPSLOG(0, " # [ timerRing ] >> RingBuffer Next....\n");
					continue;
				}

			}

		}
		else {
			// No need 
		}


		if (nRes) {

			CAMD(" OK[ timerRing ] ### g_timerRingbuffer.pop is Successful. \n");

			tmpMat.value.copyTo(matImg);

			CAMD(" @ The Size of timerRingBuffer = %s\n", std::to_string(g_timerRingbuffer.size()));
		}
		else {

			IPSLOG(0, " ## Warning[ timerRing ], g_timerRingbuffer.pop of buffer is empty!!! \n");
			printf(" ##[ %s ]; >> Warning[ timerRing ], g_timerRingbuffer.pop of buffer is empty!!! \n", __func__);
			return 7; // 7 : Buffer is Empty()
		}

	}
	else {

		IPSLOG(0, "Warning[ timerRing ]: The Streaming is not running. Please check again.\n");
		return -1;
	}

	return nRet;
}



int CMethod_V4L2CamCtrl::AcquireStreaming_Close()
{
	IPSLOG(1, "AcquireStreaming_Close(...) >>>\n");

	AcquireStreaming_StartorStop(false);

	{
		std::lock_guard<std::mutex> lock(u_mutex);

		notified_IsDone = true;
		notified_IsRun = true;

		IPSLOG(1, "Close the Thread..\n");
		IPSLOG(1, " > notified_IsDone = %d\n", notified_IsDone);
		IPSLOG(1, " > notified_IsRun = %d\n", notified_IsRun);
	}

	IPSLOG(1, " cond_var.notify_one()\n");
	cond_var.notify_all();

	bIsCreated = false;

	Streaming_Setting(false);
	IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

	std::this_thread::sleep_for(std::chrono::milliseconds(800));

	IPSLOG(1, "AcquireStreaming_Close(...) <<<\n");

	return 0;
}


int CMethod_V4L2CamCtrl::Streaming_Inquiry()
{
	std::lock_guard<std::mutex> lock(u_streaming_mutex);

	std::cout << " ## Streaming_Inquiry() ==>> bIsStreaming =" << std::to_string(m_global_cfg_ParamInfo.bIsStreaming) << std::endl;;

	return m_global_cfg_ParamInfo.bIsStreaming;
}


int CMethod_V4L2CamCtrl::Streaming_Setting(const int iflg)
{
	std::lock_guard<std::mutex> lock(u_streaming_mutex);

	m_global_cfg_ParamInfo.bIsStreaming = iflg;

	std::cout << " ## Streaming_Setting() <<== bIsStreaming =" << std::to_string(m_global_cfg_ParamInfo.bIsStreaming) << std::endl;;
	
	return 0;
}


void CMethod_V4L2CamCtrl::Streaming_CallbackEx(const std::string& strProj)
{
	// # No need.
	////IPSLOG(1, "StreamingCallbackEx_ ProjectName( %s )\n", strProj.c_str());

}


   
int CMethod_V4L2CamCtrl::Thread_Acquire(const int pCam, LpGigECamConfig pParamOut, std::function<void(const std::string&)> callback)
{
	IPSLOG(1, "Thread_Acquire(...) >>>\n");
	IPSLOG(1, "   pCam = %d\n", pCam);

	int nRet = 0;
	string strInfo;

	auto valS = std::chrono::high_resolution_clock::now();
	auto valE = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> valD(0);

	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;

	bool bIsAlreadyStart = 0;
	cv::VideoWriter cvWriter;

	cv::Mat matImg_Src;
	cv::Mat matImg_Crop;
	string strResponse;
	cv::Size frameSize;
	std::string pipeline;

	int crop_W = 0, crop_H = 0, resize_W = 0, resize_H = 0;

	while (!notified_IsDone)
	{

#if 1
		std::unique_lock<std::mutex> lock(u_mutex);
		while (!notified_IsRun)
		{
			// # End acquisition
			if (bIsAlreadyStart) {

				IPSLOG(1, " > # Thread_Acquire -> MV_CC_StopGrabbing() \n");

				// 停止取流 	// end grab image

				iCnt_0 = 0;
				iCnt_1 = 0;

				if (cvWriter.isOpened()) {
					cvWriter.release();
				}

				if (!matImg_Src.empty()) {
					matImg_Src.release();
				}

				if (!matImg_Crop.empty()) {
					matImg_Crop.release();
				}

				g_owRingbuffer.clear();
				g_timerRingbuffer.clear();

				bIsAlreadyStart = 0;

				v4l2_start_capturing(pCam);

				Streaming_Setting(false);
				IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

				usleep(10000); /* delay 50  ms */
			}

			IPSLOG(1, " > The worker_thread() wait\n");
			cond_var.wait(lock);
		}
		lock.unlock();
#endif
		// after the wait, we own the lock.
		//IPSLOG(1, "worker_thread() is processing data\n");
		//IPSLOG(1, "notified_IsDone = %d\n", notified_IsDone);
		//IPSLOG(1, "notified_IsRun = %d\n", notified_IsRun);

		if (!bIsAlreadyStart) {

			// 開始取流
			// start grab image

			// OpenCV VideoWriter object to write video to Gstreamer pipeline
#ifdef ALGO_Enable_StreamingBufOpt_EnableGStreamer_DEBUG

			int fourcc = cv::VideoWriter::fourcc('H', '2', '6', '4');	// H.264 codec
			double fps = 25;
			frameSize = cv::Size(m_global_cfg_ParamInfo.iSensor_Width, m_global_cfg_ParamInfo.iSensor_Height);
			bool bIsColor = true;

			std::stringstream ss;
			ss << "videocrop" <<
				" left=" << std::to_string(crop_W) <<
				" top=" << std::to_string(crop_H) <<
				" right=" << std::to_string(crop_W) <<
				" bottom=" << std::to_string(crop_H);

			string strGsmCrop_W = ss.str();
			ss.str("");

			ss << "video/x-raw" <<
				", width=" << std::to_string(resize_W) <<
				", height=" << std::to_string(resize_H);

			string strGsmReszie = ss.str();
#if 0
			pipeline = "appsrc ! videoconvert ! " + strGsmCrop_W + " ! videocrop-meta-enable=true ! " \
			    + strGsmReszie + " ! vpuenc_h264 ! video/x-h264 " \
			    "! rtspclientsink location=rtsp://localhost:8554/mystream";
#else
    // gst-launch-1.0 -v v4l2src device=/dev/video5 ! image/jpeg,width=640,height=480 
    //   ! jpegdec ! videoscale ! videoconvert \
    //   ! x264enc ! "video/x-h264,profile=baseline" \
    //   ! rtspclientsink location=rtsp://192.168.0.183:8554/mystream 
    //      --gst-debug-level=3
/*
[OK]
gst-launch-1.0 -v v4l2src device=/dev/video5 ! image/jpeg,width=640,height=480 \
   ! jpegdec ! videoscale ! video/x-raw,width=1920,height=1080  ! videoconvert \
   ! x264enc ! "video/x-h264,profile=baseline" \
   ! rtspclientsink location=rtsp://localhost:8554/mystream \
      --gst-debug-level=3
*/
/*			pipeline = "-v v4l2src device=/dev/video5 ! appsrc " \
    "! image/jpeg,width=640,height=480 " \
    "! jpegdec ! videoscale ! video/x-raw,width=1920,height=1080  ! videoconvert " \
    "! x264enc ! \"video/x-h264,profile=baseline\" "\
    "! rtspclientsink location=rtsp://localhost:8554/mystream --gst-debug-level=3";*/
/*
GStreamer_pipeline : 
 appsrc ! videoconvert 
 ! videocrop left=2147483008 top=2147483348 right=2147483008 bottom=2147483348 ! videocrop-meta-enable=true 
 ! video/x-raw, width=640, height=480 ! vpuenc_h264 ! video/x-h264 
 ! rtspclientsink location=rtsp://localhost:8554/mystream
*/
/*
			pipeline = "appsrc " \
    "! image/jpeg, width=640, height=480 " \
    "! jpegdec ! videoscale ! video/x-raw, width=1920, height=1080 ! videoconvert " \
    "! x264enc ! video/x-h264, profile=baseline " \
    "! rtspclientsink location=rtsp://localhost:8554/mystream --gst-debug-level=3";
        */
#if 1
//黑畫面
pipeline = "appsrc \
            ! videoconvert  \
            ! x264enc ! video/x-h264, profile=baseline \
            ! rtspclientsink location=rtsp://localhost:8554/mystream";
#else
pipeline = "appsrc \
            | image/jpeg,width=640,height=480 ! jpegdec ! videoscale ! video/x-raw,width=1920,height=1080 \
            ! videoconvert  \
            ! x264enc ! video/x-h264, profile=baseline \
            ! rtspclientsink location=rtsp://localhost:8554/mystream";
#endif


#endif
			//std::string pipeline = "appsrc \
			//						! videoconvert \
			//						! videocrop left=420 top=0 right=420 bottom=0 \
			//						! imxvideoconvert_g2d videocrop-meta-enable=true \
			//						! video/x-raw, width=540, height=540 \
			//						! vpuenc_h264 \
			//						! video/x-h264 \
			//						! rtspclientsink location=rtsp://localhost:8554/mystream";

			IPSLOG(1, " >> GStreamer_pipeline : \n %s\n\n", pipeline.c_str());

			//# Create the video writer using the GStreamer pipeline
			cvWriter.open(pipeline, fourcc, fps, frameSize, bIsColor);

#endif
			//Init the matImg
			matImg_Src = cv::Mat::zeros(m_global_cfg_ParamInfo.iSensor_Height, m_global_cfg_ParamInfo.iSensor_Width, CV_8UC3);

			//valS = std::chrono::high_resolution_clock::now();

			bIsAlreadyStart = 1;

			v4l2_start_capturing(pCam);

			Streaming_Setting(true);
			IPSLOG(0, " ## m_global_cfg_ParamInfo.bIsStreaming = %d\n", Streaming_Inquiry());

			// usleep(50000); /* delay 50  ms */
			usleep(10000); /* delay 50  ms */

		}

		iCnt_0++;

		//// # 01 >> # AcquireStreaming
		start = std::chrono::high_resolution_clock::now();
		// IPSLOG(0, "FrmaeNo.[ %d ] CycleTime : %s (ms)\n", iCnt_0, strtmp.c_str());

		// # func()
		nRet = AcquireStreaming(pCam, matImg_Src);


		// # 01 <<
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		strtmp = std::to_string(duration.count());

		//IPSLOG(0, " ### >>> ### >>> ### >>> ### >>> ### >>> ### >>> ### >>> ### >>> \n\n");

		//IPSLOG(0, " >> GStreamer_pipeline : \n >> %s\n", pipeline.c_str());
		//IPSLOG(0, " >> matImg_Src : rows( %d ), cols( %d )\n", matImg_Src.rows, matImg_Src.cols);
		//IPSLOG(0, " >> matImg_Crop : rows( %d ), cols( %d )\n", matImg_Crop.rows, matImg_Crop.cols);
		//IPSLOG(0, " >> m_global_cfg_ParamInfo.iSensor_ : W( %d ), H( %d )\n", m_global_cfg_ParamInfo.iSensor_Width, m_global_cfg_ParamInfo.iSensor_Height);

		//IPSLOG(0, " ### >>> ### >>> ### >>> ### >>> ### >>> ### >>> ### >>> ### >>> \n\n");

		if (matImg_Src.empty()) {
			usleep(1000);
			IPSLOG(0, " WARNING ~~~ > matImg_Src.empty()\n");
			continue;
		}


		if (nRet) {
			IPSLOG(0, " Error!!! > AcquireStreaming() in Thread_Acquire(...)\n");
			AcquireStreaming_StartorStop(false);
			usleep(50000); /* delay 50  ms */
		}

#ifdef ALGO_Enable_StreamingBufOpt_AddTimestamp_DEBUG

		if (m_iFrameRate_Cnt >= ULONG_MAX) {
			m_iFrameRate_Cnt = 0;
		}

		strInfo = "No." + std::to_string(m_iFrameRate_Cnt++) + "_" + getCurrentTime();
		//strInfo = "No." + std::to_string(m_iFrameRate_Cnt++);
		//IPSLOG(1, "%s\n", strInfo.c_str());

		int font_face = cv::FONT_HERSHEY_COMPLEX;	// FONT_HERSHEY_SIMPLEX;
		double font_scale = 1.5;
		int thickness = 2;
		int baseline;
		cv::Size text_size = cv::getTextSize(strInfo, font_face, font_scale, thickness, &baseline);

		cv::Point origin;
		origin.x = 20 + text_size.width / 2;
		origin.y = 60 + text_size.height / 2;

		cv::putText(matImg_Src, strInfo, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);

#endif

		// # 02 >> # cvWriter.write(matImg_Src)
		start = std::chrono::high_resolution_clock::now();

		/// > GStreamer ///
		// Check if video writer opened successfully
		// # Streaming by the GStreamer at VideoWriter of OpenCV.
#ifdef ALGO_Enable_StreamingBufOpt_EnableGStreamer_DEBUG

		if (cvWriter.isOpened()) {
			cvWriter.write(matImg_Src);
		}
		else {
			// IPSLOG(0, " WARNING ~~~ > Can't open the VideoWriter of OpenCV.\n");
		}

#endif

		// # 02 <<
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		strtmp = std::to_string(duration.count());
		////CAMM("FrmaeNo.[ %d ] 02.--.--> GStreamer video write()_CycleTime : %s (ms)\n", iCnt_0, strtmp.c_str());



		// # 03 >> # g_timerRingbuffer.push(matImg_Crop)
		start = std::chrono::high_resolution_clock::now();

		/// > Image Processing ///
		// # Cropping : if enable the Frame croppping flag.
		{

			//g_owRingbuffer.push(matImg_Src);
			g_timerRingbuffer.push(matImg_Src);

			// # 03 <<
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			strtmp = std::to_string(duration.count());
			////CAMM("FrmaeNo.[ %d ] 04.--.-->  Frame g_timerRingbuffer.push()_CycleTime : %s (ms)\n", iCnt_0, strtmp.c_str());

		}


		//// # 05 >>
		//start = std::chrono::high_resolution_clock::now();

		///// > callback("V4L2_Streaming") ///
		//valD = valE - valS;
		//if (!(static_cast<int>(valD.count()) % 5)) { //5000ms == 5sec

		//	callback("V4L2_Streaming"); // Call the callback function
		//}


		//// # 05 <<
		//end = std::chrono::high_resolution_clock::now();
		//duration = end - start;
		//strtmp = std::to_string(duration.count());
		////CAMM("FrmaeNo.[ %d ] 05.--.--> callback(　V4L2_Streaming　)_CycleTime : %s (ms)\n", iCnt_0, strtmp.c_str());

		/// > RESTful API ///
		// # RESTful API[POST] : post the Image to ELIC bythe flask of Python. 
//#ifdef ALGO_Enable_StreamingBufOpt_EnableRestfulOflask_DEBUG


//#endif	// # ALGO_Enable_StreamingBufOpt_EnableRestfulOflask_DEBUG

		// # cycletime_end
		//if (iCnt_0 >= ULONG_MAX) {
		//	iCnt_0 = 0;
		//}
		//end = std::chrono::high_resolution_clock::now();
		//duration = end - start;
		//std::cout << "function() took " << duration.count() << " milliseconds to execute.\n";
		//strtmp = std::to_string(duration.count());
		//CAMM("FrmaeNo.[ %d ] .--.--> Total_AcquireImages()_CycleTime : %s (ms)\n", iCnt_0, strtmp.c_str());

		usleep(1);

	}

	return 0;
}



int CMethod_V4L2CamCtrl::GigECam_DebugPrint()
{
	int nRet = EXIT_SUCCESS;

	IPSLOG(1, "GigECam_DebugPrint() --> TDB\n");

	return nRet;
} 
