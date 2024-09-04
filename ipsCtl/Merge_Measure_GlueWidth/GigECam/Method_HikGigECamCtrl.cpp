#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "../cvip.h"
#include "Method_HikGigECamCtrl.h"



using namespace std;

static OverwriteRingBuffer<cv::Mat> g_owRingbuffer(6);

int CMethod_HikGigECamCtrl::g_exPrev_W = 0;
int CMethod_HikGigECamCtrl::g_exPrev_H = 0;
unsigned char* CMethod_HikGigECamCtrl::g_pDataForRGB = nullptr;

//////////////////////////////////////////////////////////////////////////////
///  Hik vision Gige camera controller function
/////////////////////////////////////////////////////////////////////////////

CMethod_HikGigECamCtrl::CMethod_HikGigECamCtrl()
	: m_pCam(nullptr)
	, m_stDevList({ 0 })
	, m_global_cfg_ParamInfo(seGigECamConfig())
	, m_fLimit_ExposureTime_Min(15.0)			//define by the MVS of HikVision
	, m_fLimit_ExposureTime_Max(9.9995e+06)		//define by the MVS of HikVision
	, m_iFrameRate_Cnt(0)
	, m_RetryCnt(10)
	, bflgIsBinningMode(false)
	, restful_client("http://127.0.0.1:8000/streaming")
{}


CMethod_HikGigECamCtrl::~CMethod_HikGigECamCtrl()
{
	GigECam_Release();
}


int CMethod_HikGigECamCtrl::GigeCam_Init()
{
	int nRet = 0;

	if (m_global_cfg_ParamInfo.bIsStreaming) {

		CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(5000); /* delay 5  ms */
	}


	// ## Enum device
	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
	if (MV_OK != nRet)
	{
		CAME("Error !!, EnumDevices fail [%x]\n", nRet);
		return -1;
	}

	//if (m_stDevList.nDeviceNum == 0)
	//{
	//	CAME("Error !!, no camera found!\n");
	//	return nRet;
	//}

	// ## list all device
	if (m_stDevList.nDeviceNum > 0)
	{
		for (int i = 0; i < m_stDevList.nDeviceNum; i++)
		{
			printf("[device %d]:\n", i);
			MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
			if (nullptr == pDeviceInfo)
			{
				break;
			}
			PrintDeviceInfo_HikVision(pDeviceInfo);
		}
	}
	else
	{
		CAME("Error !!, Find No Devices!\n");
		return -1;
	}


	int nDeviceIndex = -1;


#if(1)

	// select device and create handle
	for (int nIndex = 0; nIndex < m_stDevList.nDeviceNum; nIndex++) {

		nRet = MV_CC_CreateHandle(&m_pCam, m_stDevList.pDeviceInfo[nIndex]);
		if (MV_OK != nRet)
		{
			CAMW("Warning !!, CMV_CC_CreateHandle fail! nRet [%x]\n", nRet);
			continue;
		}
		else {

			nRet = MV_CC_OpenDevice(m_pCam);
			if (0x80000203 == nRet) {

				CAMW("Warning !!, OpenDevice has no access [%x], Next\n", nRet);
				continue;
			}
			else if (MV_OK != nRet) {

				CAME("Error !!, OpenDevice fail [%x]\n", nRet);
				return nRet;
			}
			else
			{
				nDeviceIndex = nIndex;
				CAMI(" ~^_^~  MV_CC_OpenDevice[ %d ]\n", nDeviceIndex);

				break;
			}
		}

	}

	if (m_pCam == nullptr || -1 == nDeviceIndex) {

		CAME("Error QQ, CreateHandle fail [%x]\n", nRet);
		return -1;
	}

#else

	// ## Attempt to find the first deviceand create its handle.
	nDeviceIndex = 0;

	MV_CC_DEVICE_INFO m_stDevInfo = { 0 };
	memcpy(&m_stDevInfo, m_stDevList.pDeviceInfo[nDeviceIndex], sizeof(MV_CC_DEVICE_INFO));

	// select device and create handle
	nRet = MV_CC_CreateHandle(&m_pCam, &m_stDevInfo);
	if (MV_OK != nRet)
	{
		CAME("Error !!, CreateHandle fail [%x]\n", nRet);
		return nRet;
	}

	nRet = MV_CC_OpenDevice(m_pCam);
	if (MV_OK != nRet)
	{
		CAME("Error !!, OpenDevice fail [%x]\n", nRet);
		return nRet;
	}

#endif 



	// 連接設備
	// nRet = MV_CC_OpenDevice(m_pCam, nAccessMode, nSwitchoverKey);
	/*
		nAccessMode = MV_ACCESS_Exclusive，設備訪問模式，默認獨占模式 MV_ACCESS_Exclusive：獨占權限，其他APP只允許讀CCP寄存器。
		nSwitchoverKey = 0 切換權限時的密鑰，默認為無，訪問模式支持權限切換（2/4/6模式）時有效。
	*/
	//nRet = MV_CC_OpenDevice(m_pCam);
	//if (MV_OK != nRet)
	//{
	//	CAME("Error !!, OpenDevice fail [%x]\n", nRet);
	//	return nRet;
	//}


	/////////////////////////////////////////
	// ==> Other function 
	/////////////////////////////////////////

	// ch:探測網絡最佳包大小(只對GigE相機有效) | en:Detection network optimal package size(It only works for the GigE camera)
	if (m_stDevList.pDeviceInfo[nDeviceIndex]->nTLayerType == MV_GIGE_DEVICE)
	{
		int nPacketSize = MV_CC_GetOptimalPacketSize(m_pCam);
		if (nPacketSize > 0)
		{
			nRet = MV_CC_SetIntValue(m_pCam, "GevSCPSPacketSize", nPacketSize);
			if (nRet != MV_OK)
			{
				CAMW("Warning: Set Packet Size fail nRet [0x%x]!\n", nRet);
			}
		}
		else
		{
			CAMW("Warning: Get Packet Size fail nRet [0x%x]!\n", nPacketSize);
		}
	}

	int nAcquisitionMode = 2;	//0 : SingleFrame, 2 : Continuous
	nRet = MV_CC_SetEnumValue(m_pCam, "AcquisitionMode", nAcquisitionMode);
	if (MV_OK != nRet)
	{
		CAMD("Set UserSetSelector 1! nRet [0x%x]\n", nRet);
	}

	nRet = MV_CC_SetEnumValue(m_pCam, "UserSetSelector", 1);
	if (MV_OK != nRet)
	{
		CAMD("Set UserSetSelector 1! nRet [0x%x]\n", nRet);
	}

	nRet = MV_CC_SetEnumValue(m_pCam, "UserSetDefault", 1);
	if (MV_OK != nRet)
	{
		CAMD("Set UserSetDefault 1! nRet [0x%x]\n", nRet);
	}


	if (Configure_Get(m_pCam, &m_global_cfg_ParamInfo, 1)) {

		CAMD("Configure_Get( m_pCam, \" m_global_cfg_ParamInfo \", 1 )\n");
	}


	return nRet;
}


int CMethod_HikGigECamCtrl::GigECam_SetConfig(const LpGigECamConfig pParamIn)
{

	int nRet = 0;

	if (m_pCam == nullptr) {
		CAME(" > Erroe!!! m_pCam <= 0\n");
		return -1;
	}

	if (!m_global_cfg_ParamInfo.bIsStreaming) {

		Configure_Default(m_pCam, pParamIn);

		if (bflgIsBinningMode) {
			if (!nRet) nRet = Configure_Binning(m_pCam, pParamIn);
		}
		else {
			if (!nRet) nRet = Configure_Decimation(m_pCam, pParamIn);
		}

		//Update camera config to global parameter.
		if (!nRet) nRet = Configure_Get(m_pCam, &m_global_cfg_ParamInfo);

		// Image format information
		if (!nRet) nRet = Configure_ImageFormat(m_pCam, pParamIn);

		//Enable Frame rate and setting.
		if (!nRet) nRet = Configure_FrameRate(m_pCam, pParamIn);

	}
	else {

		CAMW("This is streaming mode; settings cannot be configured..\n");
		CAMW("This is streaming mode; settings cannot be configured..\n");
	}

	// Exposure mode and time setting
	if (!nRet) nRet = Configure_Exposure(m_pCam, pParamIn);

	if (!m_global_cfg_ParamInfo.bIsStreaming) {

		// Enable Trigger Mode and Trigger Source.
		if (!nRet) nRet = Configure_TriggerMode(m_pCam, pParamIn);
	}


//	if (!nRet) nRet = Configure_FrameRate(m_pCam, pParamIn);


	//// Configure Inet Addr <==== no function
	//nRet = Configure_PersistentIP(m_pCam, pParamIn);
	//if (nRet < 0)
	//{
	//    return nRet;
	//}


	if (nRet < 0)
	{
		if (m_global_cfg_ParamInfo.bIsStreaming) {

			CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

			AcquireStreaming_StartorStop(false);
		}

		return nRet;
	}


	//設置Command型節點-發送參數保存命令-參數會保存到相機內部ROM裡面
	//當修改參數完成後，可以調用該節點進行參數保存
	//執行成功後，掉電不會消失
	nRet = MV_CC_SetCommandValue(m_pCam, "UserSetSave");
	if (MV_OK != nRet)
	{
		CAMW("Set UserSetSave fail [%x]\n", nRet);
	}

	return nRet;
}


int CMethod_HikGigECamCtrl::GigECam_GetConfig(LpGigECamConfig pParamOut)
{
	int nRet = 0;

	if (m_pCam == nullptr) {
		CAME(" > Erroe!!! m_pCam <= 0 \n");
		return -1;
	}

	if (m_global_cfg_ParamInfo.bIsStreaming) {

		CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(50000);
	}


#ifdef ALGO_Enable_OpenCV_getBuildInformation_DEBUG

	CAMD("\n");
	CAMD(" > ---. ---- > ---. ---- > ---. ---- > ---. ---- > ---. ---- \n");
	CAMD(" > OPENCV REGISTRT LIST. START ---- > ---. ---- > ---. ---- \n");
	CAMD(" > ---. ---- > ---. ---- > ---. ---- > ---. ---- > ---. ---- \n");

	std::cout << cv::getBuildInformation() << std::endl;

	CAMD(" > ---. ---- > ---. ---- > ---. ---- > ---. ---- > ---. ---- \n");
	CAMD(" > OPENCV REGISTRT LIST. END ---- > ---. ---- > ---. ---- \n");
	CAMD(" > ---. ---- > ---. ---- > ---. ---- > ---. ---- > ---. ---- \n");
	CAMD("\n");

#endif

	// Configure information
	nRet = Configure_Get(m_pCam, pParamOut);

	if (nRet < 0)
	{
		return nRet;
	}

	return nRet;
}



int CMethod_HikGigECamCtrl::GigECam_AcquireImages(string strFilePath)
{
	int nRet = 0;

	if (m_pCam == nullptr) {
		CAME(" > Erroe!!! m_pCam <= 0 \n");
		return -1;
	}

	if (m_global_cfg_ParamInfo.bIsStreaming) {

		CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(50000);
	}

	nRet = AcquireImages(m_pCam, strFilePath);

	return nRet;
}



int CMethod_HikGigECamCtrl::GigECam_AcquireImages(cv::Mat& matImg)
{
	int nRet = 0;

	if (m_pCam == nullptr) {
		CAME(" > Erroe!!! m_pCam <= 0 \n");
		return -1;
	}

	if (m_global_cfg_ParamInfo.bIsStreaming) {

		CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(50000);
	}

	nRet = AcquireImages(m_pCam, matImg);

	return nRet;
}



int CMethod_HikGigECamCtrl::GigECam_Release()
{
	int nRet = 0;

	if (m_pCam) {

		CAMD("GigECam_Release() === === >>>\n");
		if (m_global_cfg_ParamInfo.bIsStreaming) {

			CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

			AcquireStreaming_StartorStop(false);
			usleep(50000);
		}

		CAMD(" # >> Release before, m_pCam Handle Address = 0x % llx\n", m_pCam);

		nRet = MV_CC_CloseDevice(m_pCam);
		if (MV_OK != nRet)
		{
			CAME("Error !!, MV_CC_CloseDevice fail! nRet [%x]\n", nRet);
			//return nRet;
		}

		//销毁句柄，释放资源
		nRet = MV_CC_DestroyHandle(m_pCam);
		if (MV_OK != nRet)
		{
			CAME("Error !!, MV_CC_DestroyHandle fail [%x]\n", nRet);
			return nRet;
		}
		else {
			m_pCam = nullptr;
		}

		sleep(1);

		CAMD(" # << Release after, m_pCam Handle Address = 0x % llx\n", m_pCam);

		CAMD("MV_CC_DestroyHandle(m_pCam) is done!!!! \n");

		CAMD("GigECam_Release() <<< === === \n");

	}

	if (g_pDataForRGB) {

		free(g_pDataForRGB);
		g_pDataForRGB = nullptr;
	}

	return nRet;
}


int CMethod_HikGigECamCtrl::GigECam_Strm_Prepare()
{
	int ret = 0;

	AcquireStreaming_Prepare();

	return ret;
}


int CMethod_HikGigECamCtrl::GigECam_Strm_Start()
{
	int ret = 0;

	AcquireStreaming_StartorStop(true);
	usleep(50000);

	return ret;
}


int CMethod_HikGigECamCtrl::GigECam_Strm_AcquireImages(string strFilePath)
{
	int nRet = 0;

	CAMI(" # strFilePath : %s\n", strFilePath.c_str());

	cv::Mat tmpImg;

	nRet = AcquireStreaming_Capture(tmpImg);
	if (nRet) {
		CAME(" Error!! The method 'Streaming_AcquireImages' must be run during the streaming process.\n");
		return ER_ABORT;
	}

	if (!tmpImg.empty()) {

		cv::imwrite(strFilePath, tmpImg);
	
		tmpImg.release();
	}
	else {
	
		CAMW("Warning !! The cv::Mat of image is Empyty()\n");
	}

	return nRet;
}


int CMethod_HikGigECamCtrl::GigECam_Strm_AcquireImages(cv::Mat& matImg)
{
	int nRet = 0;

	cv::Mat tmpImg;

	nRet = AcquireStreaming_Capture(tmpImg);
	if (nRet) {
		CAME(" Error!! The method 'Streaming_AcquireImages' must be run during the streaming process.\n");
		return ER_ABORT;
	}

	if (!tmpImg.empty()) {

		tmpImg.copyTo(matImg);

		tmpImg.release();
	}
	else {

		CAMW("Warning !! The cv::Mat of image is Empyty()\n");
	}

	return nRet;
}


int CMethod_HikGigECamCtrl::GigECam_Strm_Stop()
{
	int ret = 0;

	AcquireStreaming_StartorStop(false);

	usleep(50000);

	return ret;
}


int CMethod_HikGigECamCtrl::GigECam_Strm_Close()
{
	int ret = 0;

	AcquireStreaming_StartorStop(false);
	usleep(50000);

	AcquireStreaming_Close();
	usleep(50000);

	return ret;
}


int CMethod_HikGigECamCtrl::PrintDeviceInfo(void* pCam)
{
	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		seGigECamConfig pParamOut;

		Configure_Get(camera, &pParamOut, 1);

	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error !!, OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}

bool CMethod_HikGigECamCtrl::PrintDeviceInfo_HikVision(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
	if (nullptr == pstMVDevInfo)
	{
		printf("The Pointer of pstMVDevInfo is nullptr!\n");
		return false;
	}
	if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
	{
		int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
		int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
		int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
		int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
		// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
		printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chModelName);
		printf("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
		printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
	{
		printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chModelName);
		printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
	}
	else
	{
		printf("Not support.\n");
	}
	return true;
}


int CMethod_HikGigECamCtrl::Configure_Default(void* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;


	/* Connect to the first available camera */
	void* camera = pCam;

	bool bTriggerMode = pParamIn->bIsEnbTriggerMode;
	m_global_cfg_ParamInfo.bIsEnbTriggerMode = bTriggerMode;

	CAMM(" >> bTriggerMode = %d\n", bTriggerMode);
	CAMM(" >> setting m_global_cfg_ParamInfo.bIsEnbTriggerMode = %d\n", m_global_cfg_ParamInfo.bIsEnbTriggerMode);


	if (camera) {

		// config : Trigger Mode  --. -->
		if (!nRet) {

			CAMM(" >> setting AcquisitionFrameRateEnable = %d\n", false);

			nRet = MV_CC_SetBoolValue(camera, "AcquisitionFrameRateEnable", false);

			if (MV_OK != nRet)
			{
				CAMM("set AcquisitionFrameRateEnable fail! nRet [%x]\n", nRet);
			}

			usleep(5000); /* delay 5  ms */

		}

		if (!nRet) {

			bool bTriggerMode = 0; //off the trigger

			CAMW(" >> setting TriggerMode to Default value = %d\n", bTriggerMode);

			// set trigger mode as on(1)/off(0)
			//nRet = MV_CC_SetEnumValue(camera, "TriggerMode", bTriggerMode);
			MV_CC_SetEnumValue(camera, "TriggerMode", bTriggerMode);

			if (MV_OK != nRet)
			{
				CAME("Error, MV_CC_SetTriggerMode fail! nRet [%x]\n", nRet);
			}

			CAMM("Done~~~\n");
			

			usleep(5000); /* delay 5  ms */

		}
	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error, OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}



int CMethod_HikGigECamCtrl::Configure_ImageFormat(void* pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		int iRange_Max_Offset_X = 0, iRange_Max_Offset_Y = 0;
		int iRange_Max_Width = 0, iRange_Max_Height = 0;

		int iLimit_Max_Width = 0, iLimit_Max_Height = 0;

		iLimit_Max_Width = m_global_cfg_ParamInfo.iSensor_Width;
		iLimit_Max_Height = m_global_cfg_ParamInfo.iSensor_Height;


		if (iLimit_Max_Width == 0 || iLimit_Max_Height == 0) {

			return -1;
		}


		int iOffsetX = 0, iOffsetY = 0;
		int iWidth = 0, iHeight = 0;
		int iDiff_X = 0, iDiff_Y = 0;


		//寬高設置時需考慮步進(4)，即設置寬高需4的倍數
		//執行數據設定，順序 width -> offset_x ->height -> offset_y


        /// # ////////////////////////////////////////////
		// 1. > config : iOffset_X, iWidth --. -->
		iDiff_X = abs((int)(pParamIn->iWidth + pParamIn->iOffset_X));
		CAMD(" Configure parameters is iOffset_X = %d, iWidth = %d\n", pParamIn->iOffset_X, pParamIn->iWidth);

		if (iDiff_X > iLimit_Max_Width) {

			CAMD(" Out Of Range(Max Width = %d) : iOffset_X + iWidth = %d\n", iLimit_Max_Width, iDiff_X);

			iWidth = iLimit_Max_Width;
			iOffsetX = 0;
		}
		else {

			iWidth = calcMultiplesofFour(pParamIn->iWidth);
			iOffsetX = calcMultiplesofFour(pParamIn->iOffset_X);

		}

		iRange_Max_Width = iWidth;
		iRange_Max_Offset_X = iOffsetX;	// (iLimit_Max_Width - iWidth);

		iDiff_X = abs((int)(iRange_Max_Width + iRange_Max_Offset_X));
		CAMD(" parameters is Range_iOffset_X = %d, Range_iWidth = %d\n", iRange_Max_Offset_X, iRange_Max_Width);

		if (iDiff_X > iLimit_Max_Width) {

			CAMD(" Out Of Range(Max Width = %d) : Range_iOffset_X + Range_iWidth = %d\n", iLimit_Max_Width, iDiff_X);

			iRange_Max_Width = iLimit_Max_Width;
			iRange_Max_Offset_X = 0;
		}


		if (!nRet) {

			nRet = MV_CC_SetIntValue(m_pCam, "OffsetX", 0);
			nRet = MV_CC_SetIntValue(m_pCam, "Width", iLimit_Max_Width);

			CAMD(" Configure parameters : Range_iWidth = %d\n", iRange_Max_Width);

			nRet = MV_CC_SetIntValue(m_pCam, "Width", iRange_Max_Width);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (!nRet) {

			CAMD(" Configure parameters : Range_iOffsetX = %d\n", iRange_Max_Offset_X);

			nRet = MV_CC_SetIntValue(m_pCam, "OffsetX", iRange_Max_Offset_X);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}



		/// # ////////////////////////////////////////////
		// 2. > config : OffsetY, Height --. -->
		iDiff_Y = abs((int)(pParamIn->iHeight + pParamIn->iOffset_Y));
		CAMD(" Configure parameters is iOffset_Y = %d, iHeight = %d\n", pParamIn->iOffset_Y, pParamIn->iHeight);

		if (iDiff_Y > iLimit_Max_Height) {

			CAMD(" Out Of Range(Max Height = %d) : iOffset_Y + iHeight = %d\n", iLimit_Max_Height, iDiff_Y);

			iHeight = iLimit_Max_Height;
			iOffsetY = 0;
		}
		else {

			iHeight = calcMultiplesofTwo(pParamIn->iHeight);
			iOffsetY = calcMultiplesofTwo(pParamIn->iOffset_Y);
		}

		iRange_Max_Height = iHeight;
		iRange_Max_Offset_Y = iOffsetY;	// (iLimit_Max_Height - iHeight);

		iDiff_Y = abs((int)(iRange_Max_Height + iRange_Max_Offset_X));
		CAMD(" parameters is Range_iOffset_Y = %d, Range_iHeight = %d\n", iRange_Max_Offset_Y, iRange_Max_Height);

		if (iDiff_Y > iLimit_Max_Height) {

			CAMD(" Out Of Range(Max Height = %d) : Range_iOffset_Y + Range_iHeight = %d\n", iLimit_Max_Height, iDiff_Y);

			iRange_Max_Height = iLimit_Max_Height;
			iRange_Max_Offset_Y = 0;
		}


		if (!nRet) {

			nRet = MV_CC_SetIntValue(m_pCam, "OffsetY", 0);
			nRet = MV_CC_SetIntValue(m_pCam, "Height", iLimit_Max_Height);

			CAMD(" Configure parameters : Range_iHeight = %d\n", iRange_Max_Height);

			nRet = MV_CC_SetIntValue(m_pCam, "Height", iRange_Max_Height);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (!nRet) {

			CAMD(" Configure parameters : Range_iOffsetY = %d\n", iRange_Max_Offset_Y);

			nRet = MV_CC_SetIntValue(m_pCam, "OffsetY", iRange_Max_Offset_Y);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}



		/// # ////////////////////////////////////////////
		// 3. > config : PixelFormat  --. -->
		if (!nRet) {

			CAMD("pixel_format = %d\n", pParamIn->bPixelFormat);

			if (pParamIn->bPixelFormat == 0) {  //0:PixelFormat_Mono8, 1:PixelFormat_BayerGR8
				nRet = MV_CC_SetEnumValue(m_pCam, "PixelFormat", 0x01080001);   //0x01080001:Mono8
			}
			else {
				nRet = MV_CC_SetEnumValue(m_pCam, "PixelFormat", 0x02180014);  //0x02180014:RGB8Packed
				//nRet = MV_CC_SetEnumValue(m_pCam, "PixelFormat", 0x02180015);  //0x02180015:BGR8Packed
			}

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}




		if (MV_OK != nRet) {

			/* En error happened, display the correspdonding message */
			CAME("Error: [%x]\n", nRet);
			return -1;
		}

	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return result;
}


int CMethod_HikGigECamCtrl::Configure_Binning(void* pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		//// config : AcquisitionFrameRateEnable  --. -->
		//if (!nRet) {

		//	bool nEnbAcqFrameRateEnable = true;

		//	CAMD("AcquisitionFrameRateEnable = %d\n", nEnbAcqFrameRateEnable);

		//	nRet = MV_CC_SetBoolValue(m_pCam, "AcquisitionFrameRateEnable", nEnbAcqFrameRateEnable);

		//	CAMD("Done~~~\n");

		//	usleep(500); /* delay 5  ms */
		//}


		//config : BinningSelector, BinningVertical, BinningVertical -- . -->
		if (!nRet) {

			CAMD(" Configure parameters : BinningSelector = %d\n", 0);

			nRet = MV_CC_SetEnumValue(m_pCam, "BinningSelector", 0);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		int iScale = pParamIn->iBinning_Scale;

		if (!nRet) {

			CAMD(" Configure parameters : BinningHorizontal = %d\n", iScale);

			nRet = MV_CC_SetEnumValue(m_pCam, "BinningHorizontal", iScale);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (!nRet) {

			CAMD(" Configure parameters : BinningVertical = %d\n", iScale);

			nRet = MV_CC_SetEnumValue(m_pCam, "BinningVertical", iScale);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (MV_OK != nRet) {

			/* En error happened, display the correspdonding message */
			CAME("Error: [%x]\n", nRet);
			return -1;
		}

	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return result;
}



int CMethod_HikGigECamCtrl::Configure_Decimation(void* pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		int iScale = pParamIn->iBinning_Scale;

		if(iScale > 2) {
			iScale = 2; 
		}
		else if (iScale < 0) {
			iScale = 1;
		}

		if (!nRet) {

			CAMD(" Configure parameters : DecimationHorizontal = %d\n", iScale);

			nRet = MV_CC_SetEnumValue(m_pCam, "DecimationHorizontal", iScale);
			//MV_CC_SetEnumValue(m_pCam, "DecimationHorizontal", iScale);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}
		if (MV_OK != nRet) {

			/* En Error happened, display the correspdonding message */
			CAME("Error: [%x]\n", nRet);
			return -1;
		}

		if (!nRet) {

			CAMD(" Configure parameters : DecimationVertical = %d\n", iScale);

			nRet = MV_CC_SetEnumValue(m_pCam, "DecimationVertical", iScale);
			//MV_CC_SetEnumValue(m_pCam, "DecimationVertical", iScale);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}
		if (MV_OK != nRet) {

			/* En Error happened, display the correspdonding message */
			CAME("Error: [%x]\n", nRet);
			return -1;
		}

	}
	else {
		/* En error happened, display the correspdonding message */
		CAMD("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return result;
}



/*
	設置自動曝光
	0 : Off
	1 : Once
	2 ：Continuous
*/
int CMethod_HikGigECamCtrl::Configure_Exposure(void* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;
	int nExposureMode = -1;
	double dbExposureTime_us = 0.0;

	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		if (pParamIn->bExposureAuto == static_cast<int>(emExposureAuto::Timed_Auto)) { //0:Timed_Auto, 1:Timed

			nExposureMode = 2;
			dbExposureTime_us = 0;
		}
		else {

			nExposureMode = 0;
			dbExposureTime_us = pParamIn->dbExposureTime;

			if (dbExposureTime_us < m_fLimit_ExposureTime_Min) {
				dbExposureTime_us = m_fLimit_ExposureTime_Min;
			}
			else if (dbExposureTime_us > m_fLimit_ExposureTime_Max) {
				dbExposureTime_us = m_fLimit_ExposureTime_Max;
			}
			else {
				dbExposureTime_us = dbExposureTime_us;
			}

		}


		/* 設置自動曝光
		0 : Off
		1 : Once
		2 ：Continuous  */
		if (!nRet) {
			CAMD("Exposure Mode ==>\n");
			CAMD("nExposureMode = %d\n", nExposureMode);
			nRet = MV_CC_SetEnumValue(camera, "ExposureAuto", nExposureMode);
		}


		if (!nRet && !nExposureMode) {
			CAMD("Exposure Time ==>\n");
			CAMD("dbExposureTime_us = %5.2f\n", dbExposureTime_us);
			nRet = MV_CC_SetFloatValue(camera, "ExposureTime", dbExposureTime_us);
		}

		/* 設置自動增益
		0 : Off
		1 : Once
		2 ：Continuous  */
		if (!nRet) {
			CAMD("Gain auto ==>\n");
			nRet = MV_CC_SetEnumValue(camera, "GainAuto", 2);
		}

		///* 設置自動白平衡
		//0 : Off
		//2 : Once
		//1 ：Continuous  */
		//if (!nRet) {
		//    CAMD("Balance white auto ==>\n");
		//    nRet = MV_CC_SetEnumValue(camera, "BalanceWhiteAuto", 1);
		//}

		if (MV_OK != nRet) {

			CAMD("Error: fail [%x]\n", nRet);
			return nRet;
		}
	}
	else {
		/* En error happened, display the correspdonding message */
		CAMD("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}


int CMethod_HikGigECamCtrl::Configure_FrameRate(void* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		// config : AcquisitionFrameRateEnable  --. -->
		if (!nRet) {

			bool nEnbAcqFrameRateEnable = true;

			CAMD("AcquisitionFrameRateEnable = %d\n", nEnbAcqFrameRateEnable);

			nRet = MV_CC_SetBoolValue(camera, "AcquisitionFrameRateEnable", nEnbAcqFrameRateEnable);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */

		}

		if (!nRet) {

			double dbAcqFrameRate = pParamIn->dbAcquisitionFrameRate;

			CAMD("dbAcqFrameRate = %5.3f\n", dbAcqFrameRate);

			nRet = MV_CC_SetFloatValue(camera, "AcquisitionFrameRate", dbAcqFrameRate);

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */

		}


	}


	return nRet;
}


int CMethod_HikGigECamCtrl::Configure_TriggerMode(void* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	bool bTriggerMode = pParamIn->bIsEnbTriggerMode;
	int iTriggerSrc = pParamIn->iTriggerSrc;
	int iTriggerActivation = pParamIn->iTriggerActivation;	//0:RisingEdge, 1:FallingEdge, 2:LevelHigh, 3:LevelLow

	m_global_cfg_ParamInfo.bIsEnbTriggerMode = bTriggerMode;
	m_global_cfg_ParamInfo.iTriggerActivation = iTriggerActivation;

	CAMD(" > bTriggerMode = %d\n", bTriggerMode);
	CAMD(" > iTriggerActivation = %d\n", iTriggerActivation);
	CAMD(" # m_global_cfg_ParamInfo.bIsEnbTriggerMode = %d\n", m_global_cfg_ParamInfo.bIsEnbTriggerMode);
	CAMD(" # m_global_cfg_ParamInfo.iTriggerActivation = %d\n", m_global_cfg_ParamInfo.iTriggerActivation);


	if (camera) {

		if (bTriggerMode == 1) {

			if (!nRet) {

				CAMM(" >> setting AcquisitionFrameRateEnable = %d\n", false);

				nRet = MV_CC_SetBoolValue(camera, "AcquisitionFrameRateEnable", false);
				if (MV_OK != nRet)
				{
					CAMM("set AcquisitionFrameRateEnable fail! nRet [%x]\n", nRet);
				}
			}

			// config : Trigger Mode  --. -->
			//if (!nRet) 
			{

				CAMD(" >> setting bTriggerMode = %d\n", bTriggerMode);

				// set trigger mode as on(1)/off(0)
				nRet = MV_CC_SetEnumValue(camera, "TriggerMode", MV_TRIGGER_MODE_ON);
				if (MV_OK != nRet)
				{
					CAMM("MV_CC_SetTriggerMode fail! nRet [%x]\n", nRet);
				}

				CAMD(" < Done~~~\n");

				usleep(500); /* delay 5  ms */

			}


			//if (bTriggerMode) 
			{
				// config : Trigger source  --. -->
				//if (!nRet) 
				{

					CAMD(" >> setting iTriggerSrc = %d\n", iTriggerSrc);

					switch (iTriggerSrc) {
					case 0:	// Line0 : MV_TRIGGER_SOURCE_LINE0
						iTriggerSrc = MV_TRIGGER_SOURCE_LINE0;
						break;
					case 1:	// Line1 : MV_TRIGGER_SOURCE_LINE1
						iTriggerSrc = MV_TRIGGER_SOURCE_LINE1;
						break;
					case 2:	// Software : MV_TRIGGER_SOURCE_SOFTWARE
					default:
						iTriggerSrc = MV_TRIGGER_SOURCE_SOFTWARE;
						break;
					}

					// set trigger source
					nRet = MV_CC_SetEnumValue(camera, "TriggerSource", iTriggerSrc);
					if (MV_OK != nRet)
					{
						CAMM("MV_CC_SetTriggerSource fail! nRet [%x]\n", nRet);
					}

					m_global_cfg_ParamInfo.iTriggerSrc = iTriggerSrc;


					CAMD(" < Done~~~\n");

					usleep(500); /* delay 5  ms */

				}

				// config : Trigger activation  --. -->
				//if (!nRet) 
				{

					CAMD(" >> setting TriggerActivation = %d\n", iTriggerActivation);

					// TriggerActivation : 0:RisingEdge, 1:FallingEdge, 2:LevelHigh, 3:LevelLow
					nRet = MV_CC_SetEnumValue(camera, "TriggerActivation", iTriggerActivation);
					if (MV_OK != nRet)
					{
						CAMM("MV_CC_SetTriggerActivation fail! nRet [%x]\n", nRet);
					}

					CAMD(" < Done~~~\n");

					usleep(500); /* delay 5  ms */
				}

				// config : Register image callback function() --. -->
				//if (!nRet) 
				{

					// en:Register image callback
					CAMD(" >> setting Register image callback function() \n");

					nRet = MV_CC_RegisterImageCallBackEx(camera, AcquireCallBackEx, camera);
					if (MV_OK != nRet)
					{
						CAMM("Register Image CallBack fail! nRet [0x%x]\n", nRet);
					}

					CAMD(" < Done~~~\n");

					usleep(500); /* delay 5  ms */

				}

			}

		}


	}


	return nRet;
}



int CMethod_HikGigECamCtrl::Configure_PersistentIP(void* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		//struct in_addr netAddr;

		const char* pIP = pParamIn->strPersistentIP.c_str();
		const char* pMask = "255.255.255.0";
		const char* pGateway = "0.0.0.0";

		//if (!nRet) nRet = MV_CC_SetIntValue(camera, "GevPersistentIPAddress", iOffsetX);
		//if (!nRet) nRet = MV_CC_SetIntValue(camera, "GevPersistentSubnetMask", iOffsetX);
		//if (!nRet) nRet = MV_CC_SetIntValue(camera, "GevPersistentDefaultGateway", iOffsetX);

		if (MV_OK != nRet) {

			/* En Error happened, display the correspdonding message */
			CAMD("Error: [%x]\n", nRet);
			return nRet;
		}

	}
	else {
		/* En error happened, display the correspdonding message */
		CAMD("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}


int CMethod_HikGigECamCtrl::Configure_ResetConfigureCustomImageSettings(void* pCam)
{
	int result = 0;
	// TBD
	return result;
}


int CMethod_HikGigECamCtrl::Configure_ResetExposure(void* pCam)
{
	int result = 0;
	//TBD
	return result;
}


int CMethod_HikGigECamCtrl::Configure_Get(void* pCam, LpGigECamConfig pParamOut, bool bDumpInf)
{

	if (nullptr == pParamOut)
		return -1;

	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	LpGigECamConfig pParam = pParamOut;

	if (camera != nullptr) {

		MVCC_INTVALUE cfg_Max_Width = { 0 }, cfg_Max_Height = { 0 };

		MVCC_INTVALUE cfg_OffsetX = { 0 }, cfg_OffsetY = { 0 };
		MVCC_INTVALUE cfg_Width = { 0 }, cfg_Height = { 0 };

		MVCC_ENUMVALUE cfg_PixelFormat = { 0 };
		MVCC_ENUMVALUE cfg_Exposure_modeto = { 0 };

		MVCC_ENUMVALUE cfg_Binning_ScaleVal = { 0 };

		bool cfg_bIsEnbAcquisitionFrameRate = { 0 };
		MVCC_FLOATVALUE cfg_dbAcquisitionFrameRate = { 0 };


		if (!nRet) nRet = MV_CC_GetIntValue(camera, "WidthMax", &cfg_Max_Width);
		usleep(500);
		if (!nRet) nRet = MV_CC_GetIntValue(camera, "HeightMax", &cfg_Max_Height);
		usleep(500);

		if (!nRet) nRet = MV_CC_GetIntValue(camera, "OffsetX", &cfg_OffsetX);
		usleep(500);
		if (!nRet) nRet = MV_CC_GetIntValue(camera, "OffsetY", &cfg_OffsetY);
		usleep(500);

		if (!nRet) nRet = MV_CC_GetIntValue(camera, "Width", &cfg_Width);
		usleep(500);
		if (!nRet) nRet = MV_CC_GetIntValue(camera, "Height", &cfg_Height);
		usleep(500);

		if (!nRet) nRet = MV_CC_GetEnumValue(camera, "PixelFormat", &cfg_PixelFormat);
		usleep(500);
		if (!nRet) nRet = MV_CC_GetEnumValue(camera, "ExposureAuto", &cfg_Exposure_modeto);
		usleep(500);

		if (!nRet) nRet = MV_CC_GetEnumValue(camera, "DecimationHorizontal", &cfg_Binning_ScaleVal);
		if (0x80000106 == nRet) //0x80000106 == The node access condition is wrong, so we try "BinningHorizontal"; 
		{
			CAMW(" # Waring !!, Can not get info of \"DecimationHorizontal\"[0x%x], try to get \"BinningHorizontal\" node info.\n", nRet);
			nRet = MV_CC_GetEnumValue(camera, "BinningHorizontal", &cfg_Binning_ScaleVal);
			bflgIsBinningMode = true;
		}
		else {
			CAMI(" @ OK ~~, Can get info of \"DecimationHorizontal\"[0x%x]\n", nRet);
			bflgIsBinningMode = false;
		}
		usleep(500);

		if (!nRet) nRet = MV_CC_GetBoolValue(camera, "AcquisitionFrameRateEnable", &cfg_bIsEnbAcquisitionFrameRate);
		usleep(500);

		if (cfg_bIsEnbAcquisitionFrameRate) {
			if (!nRet) nRet = MV_CC_GetFloatValue(camera, "AcquisitionFrameRate", &cfg_dbAcquisitionFrameRate);
			usleep(500);
		}



		//if (MV_OK != nRet) {

		//	/* En error happened, display the correspdonding message */
		//	CAMD("Error: [%x]\n", nRet);
		//	return -1;
		//}

		pParam->bIsConnected = true;

		pParam->iSensor_Width = cfg_Max_Width.nCurValue;
		pParam->iSensor_Height = cfg_Max_Height.nCurValue;

		pParam->iOffset_X = cfg_OffsetX.nCurValue;
		pParam->iOffset_Y = cfg_OffsetY.nCurValue;

		pParam->iWidth = cfg_Width.nCurValue;
		pParam->iHeight = cfg_Height.nCurValue;

		pParam->bPixelFormat = (cfg_PixelFormat.nCurValue == 0x01080001) ? 0 : 1;  //0:PixelFormat_Mono8, 1:PixelFormat_BayerGR8
		pParam->bExposureAuto = (cfg_Exposure_modeto.nCurValue == 2) ? 0 : 1;     //0:Auto, 1:Timed;

		pParam->iBinning_Scale = cfg_Binning_ScaleVal.nCurValue;    

		pParam->bIsEnbAcquisitionFrameRate = cfg_bIsEnbAcquisitionFrameRate;
		pParam->dbAcquisitionFrameRate = cfg_dbAcquisitionFrameRate.fCurValue;



		if (bDumpInf) {

			CAMD("\n\n>>> Device information === >>> === >>>\n");
			CAMD("\t	@ Cfg.bIsConnected---> %d\n", pParam->bIsConnected);
			CAMD("\t	@ Cfg.bIsEnbAcquisitionFrameRate---> %s\n", (pParam->bIsEnbAcquisitionFrameRate) ? "True" : "False");
			CAMD("\t	@ Cfg.dbAcquisitionFrameRate---> %5.3f\n", pParam->dbAcquisitionFrameRate);
			CAMD("\t	@ Cfg.bExposureAuto---> %s\n", (!pParam->bExposureAuto) ? "Auto" : "Off");
			CAMD("\t	@ Cfg.bPixelFormat---> %s\n", (pParam->bPixelFormat) ? "RGB8" : "Mono8");
			CAMD("\t	@ Cfg.iOffset_X ---> %d\n", pParam->iOffset_X);
			CAMD("\t	@ Cfg.iOffset_Y---> %d\n", pParam->iOffset_Y);
			CAMD("\t	@ Cfg.iWidth ---> %d\n", pParam->iWidth);
			CAMD("\t	@ Cfg.iHeight---> %d\n", pParam->iHeight);
			CAMD("\t	@ Cfg.iMax_Width ---> %d\n", pParam->iSensor_Width);
			CAMD("\t	@ Cfg.iMax_Height---> %d\n", pParam->iSensor_Height);
			CAMD("\t	@ Cfg.iBinning_Scale---> %d\n", pParam->iBinning_Scale);
			CAMD("<<< Device information === <<< === <<< === ===\n\n");

		}

	}
	else {

		*pParam = seGigECamConfig();

		return -1;

	}

	return nRet;

}



int CMethod_HikGigECamCtrl::RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
	if (nullptr == pRgbData)
	{
		return MV_E_PARAMETER;
	}

	for (unsigned int j = 0; j < nHeight; j++)
	{
		for (unsigned int i = 0; i < nWidth; i++)
		{
			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
			pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
		}
	}

	return MV_OK;
}



int CMethod_HikGigECamCtrl::Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData, cv::Mat& mat_img)
{
	cv::Mat srcImage;
	if (pstImageInfo->enPixelType == PixelType_Gvsp_Mono8)
	{
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
	}
	else if (pstImageInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
	{
		RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
	}
	else
	{
		CAME("Error, unsupported pixel format\n");
		return false;
	}

	if (nullptr == srcImage.data)
	{
		return false;
	}

	//    //save converted image in a local file
	//    std::string filename = GetTimeAsFileName();
	//    try {
	//#if defined (VC9_COMPILE)
	//        cvSaveImage(filename, &(IplImage(srcImage)));
	//#else
	//        //cv::imwrite(filename, srcImage);
	//#endif
	//    }
	//    catch (cv::Exception& ex) {
	//        fCAMI(stderr, "Exception saving image to bmp format: %s\n", ex.what());
	//    }
	mat_img = srcImage.clone();
	srcImage.release();

	return true;
}



int CMethod_HikGigECamCtrl::AcquireImages(void* pCam, string strFilePath)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;

	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		cv::Mat mat_img;

		unsigned char* pData = nullptr;
		unsigned char* pDataForRGB = nullptr;

		// # cycletime_start >>
		start = std::chrono::high_resolution_clock::now();

		// ch:獲取數據包大小 | en:Get payload size
		MVCC_INTVALUE stParam;
		memset(&stParam, 0, sizeof(MVCC_INTVALUE));
		nRet = MV_CC_GetIntValue(camera, "PayloadSize", &stParam);
		if (MV_OK != nRet)
		{
			CAME("Error, Get PayloadSize fail! nRet [0x%x]\n", nRet);
			return nRet;
		}

		// 開始取流
		// start grab image
		nRet = MV_CC_StartGrabbing(camera);
		if (MV_OK != nRet)
		{
			CAME("Error, MV_CC_StartGrabbing fail! nRet [%x]\n", nRet);
			return nRet;
		}

		MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
		memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
		pData = (unsigned char*)malloc(sizeof(unsigned char) * stParam.nCurValue);
		if (nullptr == pData)
		{
			return -1;
		}
		unsigned int nDataSize = stParam.nCurValue;

		nRet = MV_CC_GetOneFrameTimeout(camera, pData, nDataSize, &stImageInfo, 1000);

		if (nRet == MV_OK) {

			pDataForRGB = (unsigned char*)malloc(stImageInfo.nWidth * stImageInfo.nHeight * 4 + 2048);
			if (nullptr == pDataForRGB)
			{
				return -1;
			}
			// 像素格式转换
			// convert pixel format 
			MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };
			// 從上到下依次是：圖像寬，圖像高，輸入數據緩存，輸入數據大小，源像素格式，
			// 目標像素格式，輸出數據緩存，提供的輸出緩衝區大小
			// Top to bottom are：image width, image height, input data buffer, input data size, source pixel format, 
			// destination pixel format, output data buffer, provided output buffer size
			stConvertParam.nWidth = stImageInfo.nWidth;
			stConvertParam.nHeight = stImageInfo.nHeight;
			stConvertParam.pSrcData = pData;
			stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;
			stConvertParam.enSrcPixelType = stImageInfo.enPixelType;
			if (stImageInfo.enPixelType == PixelType_Gvsp_Mono8) {
				stConvertParam.enDstPixelType = PixelType_Gvsp_Mono8;
			}
			else {
				stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;
			}
			stConvertParam.pDstBuffer = pDataForRGB;
			stConvertParam.nDstBufferSize = stImageInfo.nWidth * stImageInfo.nHeight * 4 + 2048;
			nRet = MV_CC_ConvertPixelType(camera, &stConvertParam);
			if (MV_OK != nRet)
			{
				CAME("Error, MV_CC_ConvertPixelType fail! nRet [%x]\n", nRet);
				goto Err;
			}

			if (stImageInfo.enPixelType == PixelType_Gvsp_Mono8)
			{
				mat_img = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC1, pDataForRGB);
			}
			else if (stImageInfo.enPixelType == PixelType_Gvsp_RGB8_Packed)
			{
				mat_img = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3, pDataForRGB);
			}
			else
			{
				CAME("Error, unsupported pixel format\n");
				goto Err;
			}

			if (mat_img.empty()) {

				CAME("Error, The matrix is empty()!!!\n");
				goto Err;
			}

			// # cycletime_end << 
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			strtmp = std::to_string(duration.count());
			CAMI("HikVisionCam_AcquireImages() >> Capture _CycleTime : % s(ms)\n", strtmp.c_str());

			// # cycletime_start >>
			start = std::chrono::high_resolution_clock::now();

			try {

				cv::imwrite(strFilePath, mat_img);

				mat_img.release();

				// # cycletime_end << 
				end = std::chrono::high_resolution_clock::now();
				duration = end - start;
				strtmp = std::to_string(duration.count());
				CAMI("HikVisionCam_AcquireImages() >> SaveImage _CycleTime : % s(ms)\n", strtmp.c_str());

				/* Display some informations about the retrieved buffer */
				CAMD("Acquired file path : %s\n", strFilePath.c_str());
			}
			catch (cv::Exception& ex) {
				CAME("Error, Exception saving image to bmp format: %s\n", ex.what());
			}

			// 停止取流
			// end grab image
			nRet = MV_CC_StopGrabbing(camera);
			if (MV_OK != nRet)
			{
				CAME("Error, MV_CC_StopGrabbing fail! nRet [%x]\n", nRet);
				goto Err;
			}

		}
		else {
			/* En error happened, display the correspdonding message */
			CAMD("Error: OpenDevice fail [%x]\n", nRet);
		}

	Err:

		if (pData) {

			free(pData);
			pData = nullptr;
		}
		if (pDataForRGB) {

			free(pDataForRGB);
			pDataForRGB = nullptr;
		}

		usleep(50000); /* delay 50  ms */

	}
	else {
		/* En error happened, display the correspdonding message */
		CAMD("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}


	return nRet;

}



int CMethod_HikGigECamCtrl::AcquireImages(void* pCam, cv::Mat& matImg)
{
	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;
	int nRet = 0;

	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		cv::Mat mat_img;

		unsigned char* pData = nullptr;
		unsigned char* pDataForRGB = nullptr;

		//cycletime_start
		start = clock();

		// ch:獲取數據包大小 | en:Get payload size
		MVCC_INTVALUE stParam;
		memset(&stParam, 0, sizeof(MVCC_INTVALUE));
		nRet = MV_CC_GetIntValue(camera, "PayloadSize", &stParam);
		if (MV_OK != nRet)
		{
			CAME("Error, Get PayloadSize fail! nRet [0x%x]\n", nRet);
			return nRet;
		}

		// 開始取流
		// start grab image
		nRet = MV_CC_StartGrabbing(camera);
		if (MV_OK != nRet)
		{
			CAME("Error, MV_CC_StartGrabbing fail! nRet [%x]\n", nRet);
			return nRet;
		}

		MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
		memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
		pData = (unsigned char*)malloc(sizeof(unsigned char) * stParam.nCurValue);
		if (nullptr == pData)
		{
			return -1;
		}
		unsigned int nDataSize = stParam.nCurValue;

		nRet = MV_CC_GetOneFrameTimeout(camera, pData, nDataSize, &stImageInfo, 1000);

		if (nRet == MV_OK) {

			// === >>> === >>> === >>>
			// 
			//数据去转换
			//int nConvertRet = 0;            
			//nConvertRet = Convert2Mat(&stImageInfo, pData, mat_img);
			// 
			// === >>> === >>> === >>>



			pDataForRGB = (unsigned char*)malloc(stImageInfo.nWidth * stImageInfo.nHeight * 4 + 2048);
			if (nullptr == pDataForRGB)
			{
				return -1;
			}
			// 像素格式转换
			// convert pixel format 
			MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };
			// 從上到下依次是：圖像寬，圖像高，輸入數據緩存，輸入數據大小，源像素格式，
			// 目標像素格式，輸出數據緩存，提供的輸出緩衝區大小
			// Top to bottom are：image width, image height, input data buffer, input data size, source pixel format, 
			// destination pixel format, output data buffer, provided output buffer size
			stConvertParam.nWidth = stImageInfo.nWidth;
			stConvertParam.nHeight = stImageInfo.nHeight;
			stConvertParam.pSrcData = pData;
			stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;
			stConvertParam.enSrcPixelType = stImageInfo.enPixelType;
			if (stImageInfo.enPixelType == PixelType_Gvsp_Mono8) {
				stConvertParam.enDstPixelType = PixelType_Gvsp_Mono8;
			}
			else {
				stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;
			}
			stConvertParam.pDstBuffer = pDataForRGB;
			stConvertParam.nDstBufferSize = stImageInfo.nWidth * stImageInfo.nHeight * 4 + 2048;
			nRet = MV_CC_ConvertPixelType(camera, &stConvertParam);
			if (MV_OK != nRet)
			{
				CAME("Error, MV_CC_ConvertPixelType fail! nRet [%x]\n", nRet);
				goto Err;
			}

			if (stImageInfo.enPixelType == PixelType_Gvsp_Mono8)
			{
				mat_img = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC1, pDataForRGB);
			}
			else if (stImageInfo.enPixelType == PixelType_Gvsp_RGB8_Packed)
			{
				mat_img = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3, pDataForRGB);
			}
			else
			{
				CAME("Error, unsupported pixel format\n");
				goto Err;
			}

			if (!mat_img.empty()) {

				mat_img.copyTo(matImg);

				mat_img.release();

			}

			// 停止取流
			// end grab image
			nRet = MV_CC_StopGrabbing(camera);
			if (MV_OK != nRet)
			{
				CAME("Error, MV_CC_StopGrabbing fail! nRet [%x]\n", nRet);
				goto Err;
			}

		}
		else {
			/* En error happened, display the correspdonding message */
			CAME("Error, OpenDevice fail [%x]\n", nRet);
		}


	Err:
		////cycletime_end
		//end = clock();
		//elapsed = double(end - start) / CLOCKS_PER_SEC;
		//strtmp = std::to_string(elapsed);
		//cout << "1. --.--> HikVision_AcquireImages() Cycle time : " << strtmp.c_str() << " (seconds)" << endl;


		if (pData) {

			free(pData);
			pData = nullptr;
		}
		if (pDataForRGB) {

			free(pDataForRGB);
			pDataForRGB = nullptr;
		}

		usleep(50000); /* delay 50  ms */

	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}


	return nRet;

}



int CMethod_HikGigECamCtrl::AcquireStreaming(void* pCam, cv::Mat& matImg)
{
	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;
	int nRet = 0;

	cv::Mat mat_img;


	/* Connect to the first available camera */
	void* camera = pCam;

	if (camera) {

		//cycletime_start
		start = clock();

		iCnt_1++;

		unsigned char* pData = nullptr;
		unsigned char* pDataForRGB = nullptr;

		////cycletime_start
		//start = clock();

		// ch:獲取數據包大小 | en:Get payload size
		MVCC_INTVALUE stParam;
		memset(&stParam, 0, sizeof(MVCC_INTVALUE));
		nRet = MV_CC_GetIntValue(camera, "PayloadSize", &stParam);
		if (MV_OK != nRet)
		{
			CAME("Error, Get PayloadSize fail! nRet [0x%x]\n", nRet);
			return nRet;
		}

		MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
		memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
		pData = (unsigned char*)malloc(sizeof(unsigned char) * stParam.nCurValue);
		if (nullptr == pData)
		{
			return -1;
		}
		unsigned int nDataSize = stParam.nCurValue;

		nRet = MV_CC_GetOneFrameTimeout(camera, pData, nDataSize, &stImageInfo, 1000);

		if (nRet == MV_OK) {

			MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };

			pDataForRGB = (unsigned char*)malloc(stImageInfo.nWidth * stImageInfo.nHeight * 4 + 2048);
			if (nullptr == pDataForRGB)
			{
				nRet = -1;
				goto Err;
			}
			// 像素格式转换
			// convert pixel format 
			// 從上到下依次是：圖像寬，圖像高，輸入數據緩存，輸入數據大小，源像素格式，
			// 目標像素格式，輸出數據緩存，提供的輸出緩衝區大小
			// Top to bottom are：image width, image height, input data buffer, input data size, source pixel format, 
			// destination pixel format, output data buffer, provided output buffer size
			stConvertParam.nWidth = stImageInfo.nWidth;
			stConvertParam.nHeight = stImageInfo.nHeight;
			stConvertParam.pSrcData = pData;
			stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;
			stConvertParam.enSrcPixelType = stImageInfo.enPixelType;
			if (stImageInfo.enPixelType == PixelType_Gvsp_Mono8) {
				stConvertParam.enDstPixelType = PixelType_Gvsp_Mono8;
			}
			else {
				stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;
			}
			stConvertParam.pDstBuffer = pDataForRGB;
			stConvertParam.nDstBufferSize = stImageInfo.nWidth * stImageInfo.nHeight * 4 + 2048;
			nRet = MV_CC_ConvertPixelType(camera, &stConvertParam);
			if (MV_OK != nRet)
			{
				CAME("Error, MV_CC_ConvertPixelType fail! nRet [%x]\n", nRet);
				goto Err;
			}

			if (stImageInfo.enPixelType == PixelType_Gvsp_Mono8)
			{
				mat_img = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC1, pDataForRGB);
			}
			else if (stImageInfo.enPixelType == PixelType_Gvsp_RGB8_Packed)
			{
				mat_img = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3, pDataForRGB);
				cv::cvtColor(mat_img, mat_img, cv::COLOR_BGR2RGB);
			}
			else
			{
				CAME("Error, unsupported pixel format\n");
				goto Err;
			}

			if (!mat_img.empty()) {

#ifdef ALGO_Enable_StreamingBufOpt_SpeedOptimization_DEBUG

				cv::resize(mat_img, matImg, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);
#else

				mat_img.copyTo(matImg);
#endif

				mat_img.release();

			}

			//// # cycletime_end
			//if (iCnt_1 >= ULONG_MAX) {
			//	iCnt_1 = 0;
			//}
			//end = clock();
			//elapsed = double(end - start) / CLOCKS_PER_SEC;
			//strtmp = std::to_string(elapsed);
			//cout << "1.[ " << iCnt_1 << " ].  --.--> HikVision_AcquireImages() Cycle time : " << strtmp.c_str() << " (seconds)" << endl;


		}
		else {

			CAMW("Warning, > incompleteErr_timestamp : %s\n", getCurrentTime());

			/* En error happened, display the correspdonding message */
			CAMW("Warning, OpenDevice fail [%x]\n", nRet);

		}


	Err:

		if (pData) {

			free(pData);
			pData = nullptr;
		}
		if (pDataForRGB) {

			free(pDataForRGB);
			pDataForRGB = nullptr;
		}

		// # usleep(20000); /* delay 50  ms */

	}
	else {
		/* En error happened, display the correspdonding message */
		CAMD("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}


	return nRet;

}



void __stdcall CMethod_HikGigECamCtrl::AcquireCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{

	int nRet = 0;
	int nBufSize = 0;


	if (pFrameInfo)         // ch:用于保存图像的缓存 | en:Buffer to save image
	{
		CAMW(" > Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);


		if (0 == g_exPrev_W) {
			g_exPrev_W = pFrameInfo->nWidth;
		}
		if (0 == g_exPrev_H) {
			g_exPrev_H = pFrameInfo->nHeight;
		}

		CAMW(" > Get One Frame: Width[%d], Height[%d], nFrameNum[%d],  prev_W[%d], prev_H[%d]\n",
			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum, g_exPrev_W, g_exPrev_H);

	}
	else {

		CAME("Error!!! AcquireCallBackEx fail \n");
		CAME("Error!!! AcquireCallBackEx fail \n");

		return;
	}

	nBufSize = pFrameInfo->nWidth * pFrameInfo->nHeight * 4 + 2048;

	if ((pFrameInfo->nWidth != g_exPrev_W)	|| 
		(pFrameInfo->nHeight != g_exPrev_H)	||
		(nullptr == g_pDataForRGB)			){

		if (g_pDataForRGB) {

			free(g_pDataForRGB);
			g_pDataForRGB = nullptr;
		}

		g_pDataForRGB = (unsigned char*)malloc(nBufSize);
	}
	else {
	
		memset(g_pDataForRGB, 0x00, nBufSize);
	}

	MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };

	// ch:设置对应的相机参数 | en:Set camera parameter
	if (nullptr != g_pDataForRGB)
	{
		cv::Mat mat_img;

		stConvertParam.nWidth = pFrameInfo->nWidth;
		stConvertParam.nHeight = pFrameInfo->nHeight;
		stConvertParam.pSrcData = pData;
		stConvertParam.nSrcDataLen = pFrameInfo->nFrameLen;
		stConvertParam.enSrcPixelType = pFrameInfo->enPixelType;

		if (pFrameInfo->enPixelType == PixelType_Gvsp_Mono8) {
			stConvertParam.enDstPixelType = PixelType_Gvsp_Mono8;
		}
		else {
			stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;
		}

		stConvertParam.pDstBuffer = g_pDataForRGB;
		stConvertParam.nDstBufferSize = nBufSize;

		nRet = MV_CC_ConvertPixelType(pUser, &stConvertParam);
		if (MV_OK != nRet)
		{
			CAMW("Warning, MV_CC_ConvertPixelType fail! nRet [%x]\n", nRet);
			//goto Err;
		}

		if (pFrameInfo->enPixelType == PixelType_Gvsp_Mono8)
		{
			mat_img = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, g_pDataForRGB);
		}
		else if (pFrameInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
		{
			mat_img = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3, g_pDataForRGB);
			cv::cvtColor(mat_img, mat_img, cv::COLOR_BGR2RGB);
		}
		else
		{
			CAMW("Warning, unsupported pixel format\n");
			//goto Err;
		}

		if (!mat_img.empty()) {

#ifdef ALGO_Enable_StreamingBufOpt_SpeedOptimization_DEBUG

			cv::resize(mat_img, mat_img, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);

#endif
			//Only for Debug. --.-->
			//CAMD(" # Trigger, ==> /home/user/primax/vsb/grap/rexty_trigger.bmp\n");
			//cv::imwrite("/home/user/primax/vsb/grap/rexty_trigger.bmp", matImg);
			// <--.--

			CAMI("# g_owRingbuffer.push  ===.===> \n");

			g_owRingbuffer.push(mat_img);
			CAMI(" ##  RingBuffer.szie() = %d\n", g_owRingbuffer.size());

			CAMI("# g_owRingbuffer.push  <===.=== \n");

			mat_img.release();

		}

	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error !!! OpenDevice fail [%x]\n", nRet);
		return;
	}

}




//void __stdcall CMethod_HikGigECamCtrl::AcquireCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
//{
//	if (pFrameInfo)         // ch:用于保存图像的缓存 | en:Buffer to save image
//	{
//		CAMW(" > Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
//			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
//		CAMW(" > Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
//			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
//	}
//
//	int nRet = 0;
//
//	unsigned char* pDataForRGB = nullptr;
//	MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };
//
//	// ch:设置对应的相机参数 | en:Set camera parameter
//	if (nullptr == pDataForRGB)
//	{
//		//cv::Mat matImg;
//		cv::Mat mat_img;
//
//		int nBufSize = pFrameInfo->nWidth * pFrameInfo->nHeight * 4 + 2048;
//		pDataForRGB = (unsigned char*)malloc(nBufSize);
//
//		stConvertParam.nWidth		= pFrameInfo->nWidth;
//		stConvertParam.nHeight		= pFrameInfo->nHeight;
//		stConvertParam.pSrcData		= pData;
//		stConvertParam.nSrcDataLen	= pFrameInfo->nFrameLen;
//		stConvertParam.enSrcPixelType = pFrameInfo->enPixelType;
//
//		if (pFrameInfo->enPixelType == PixelType_Gvsp_Mono8) {
//			stConvertParam.enDstPixelType = PixelType_Gvsp_Mono8;
//		}
//		else {
//			stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;
//		}
//
//		stConvertParam.pDstBuffer = pDataForRGB;
//		stConvertParam.nDstBufferSize = nBufSize;
//
//		nRet = MV_CC_ConvertPixelType(pUser, &stConvertParam);
//		if (MV_OK != nRet)
//		{
//			CAMW("Warning, MV_CC_ConvertPixelType fail! nRet [%x]\n", nRet);
//			//goto Err;
//		}
//
//		if (pFrameInfo->enPixelType == PixelType_Gvsp_Mono8)
//		{
//			mat_img = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pDataForRGB);
//		}
//		else if (pFrameInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
//		{
//			mat_img = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3, pDataForRGB);
//			cv::cvtColor(mat_img, mat_img, cv::COLOR_BGR2RGB);
//		}
//		else
//		{
//			CAMW("Warning, unsupported pixel format\n");
//			//goto Err;
//		}
//
//		if (!mat_img.empty()) {
//
//#ifdef ALGO_Enable_StreamingBufOpt_SpeedOptimization_DEBUG
//
//			cv::resize(mat_img, mat_img, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);
//
////#else
////
////			mat_img.copyTo(matImg);
//
//#endif
//			//Only for Debug. --.-->
//			//CAMD(" # Trigger, ==> /home/user/primax/vsb/grap/rexty_trigger.bmp\n");
//			//cv::imwrite("/home/user/primax/vsb/grap/rexty_trigger.bmp", matImg);
//			// <--.--
//
//			CAMD("# g_owRingbuffer.push  ===.===> \n");
//
//			//g_owRingbuffer.push(matImg);
//			g_owRingbuffer.push(mat_img);
//			CAMD(" ##  RingBuffer.szie() = %d\n", g_owRingbuffer.size());
//
//			CAMD("# g_owRingbuffer.push  <===.=== \n");
//
//			mat_img.release();
//			//matImg.release();
//
//		}
//
//Err:
//
//		if (pDataForRGB) {
//
//			free(pDataForRGB);
//			pDataForRGB = nullptr;
//		}
//
//	}
//	else {
//		/* En error happened, display the correspdonding message */
//		CAME("Error: OpenDevice fail [%x]\n", nRet);
//		return -1;
//	}
//
//}



int CMethod_HikGigECamCtrl::AcquireStreaming_Prepare()
{
	void* camera = m_pCam;

	CAMD("AcquireStreaming_Prepare(...) >>>\n");
	cout << " > tid = " << tid << endl;
	cout << " > tid = " << tid << endl;

	CAMW(" ## m_global_cfg_ParamInfo.bIsEnbTriggerMode = %d\n", m_global_cfg_ParamInfo.bIsEnbTriggerMode);
	if (m_global_cfg_ParamInfo.bIsEnbTriggerMode) {
		notified_IsTrigger = true;
	}
	else {
		notified_IsTrigger = false;
	}
	CAMI(" > notified_IsTrigger = %d\n", notified_IsTrigger);


	if (camera) {

		if (bIsCreated) {

			CAMW("# The Thread already creatr()...\n");
			return 0;
		}

		notified_IsDone = false;
		notified_IsRun = false;
		CAMI("Setting Thread default value.\n");
		CAMI(" > notified_IsDone = %d\n", notified_IsDone);
		CAMI(" > notified_IsRun = %d\n", notified_IsRun);

		seGigECamConfig pParam = seGigECamConfig();

		//// Because there is only one handle of camera control, only one thread can be launched here.
		std::thread t1;

		//CAMW(" ## m_global_cfg_ParamInfo.bIsEnbTriggerMode = %d\n", m_global_cfg_ParamInfo.bIsEnbTriggerMode);
		//if (m_global_cfg_ParamInfo.bIsEnbTriggerMode) {
		//	CAMW(" >> std::thread of Thread_TriggerMod\n");
		//	//t1 = std::thread(&CMethod_HikGigECamCtrl::Thread_TriggerMode, this, camera, &m_global_cfg_ParamInfo);
		//} 
		//else {
		//	CAMW(" >> std::thread of Thread_Acquire\n");
		//	//t1 = std::thread(&CMethod_HikGigECamCtrl::Thread_Acquire, this, camera, &m_global_cfg_ParamInfo);
		//}

		t1 = std::thread(&CMethod_HikGigECamCtrl::Thread_Acquire, this, camera, &m_global_cfg_ParamInfo);

		tid = t1.get_id();
		t1.detach();

		bIsCreated = true;


		cout << " # >> tid = " << tid << endl;

	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error: OpenDevice fail [%x]\n", camera);
		return -1;
	}

	CAMD("AcquireStreaming_Prepare(...) <<<\n");

	return 0;
}


int CMethod_HikGigECamCtrl::AcquireStreaming_StartorStop(bool bflgEnableThd)
{
	CAMI("AcquireStreaming_StartorStop(...) >>>\n");
	CAMI(" bflgEnableThd = %d\n", bflgEnableThd);

	std::unique_lock<std::mutex> lock(u_mutex);

	if (bflgEnableThd) {

		CAMI(" # Setting Thread ==> START~ START~ \n");
		notified_IsRun = true;

		if (m_global_cfg_ParamInfo.bIsEnbTriggerMode) {
			notified_IsTrigger = true;
		}
		else {
			notified_IsTrigger = false;
		}

		CAMI(" > notified_IsDone = %d\n", notified_IsDone);
		CAMI(" > notified_IsRun = %d\n", notified_IsRun);
		CAMI(" > notified_IsTrigger = %d\n", notified_IsTrigger);

	}
	else {
		usleep(50000);

		CAMI(" # Setting Thread ==> STOP! STOP! \n");
		notified_IsRun = false;
		notified_IsTrigger = false;

		CAMI(" > notified_IsDone = %d\n", notified_IsDone);
		CAMI(" > notified_IsRun = %d\n", notified_IsRun);
		CAMI(" > notified_IsTrigger = %d\n", notified_IsTrigger);

	}

	m_global_cfg_ParamInfo.bIsStreaming = notified_IsRun;

	m_iFrameRate_Cnt = 0;

	CAMI(" cond_var.notify_one()\n");
	cond_var.notify_one();

	CAMI(" cv_trigger.notify_one()\n");
	cv_trigger.notify_one();

	CAMI("AcquireStreaming_StartorStop(...) <<<\n");


	return 0;
}


int CMethod_HikGigECamCtrl::AcquireStreaming_Capture(cv::Mat& matImg)
{
	int nRet = 0;

	if (m_global_cfg_ParamInfo.bIsStreaming) {

		CAMD(" # @ GigECam_Strm_AcquireImages -> pop()\n");

		cv::Mat tmpMat;

		if (g_owRingbuffer.pop(tmpMat)) {

			CAMD(" OK ### g_owRingbuffer.pop is Successful. \n");
			tmpMat.copyTo(matImg);

			if (!tmpMat.empty()) {
				tmpMat.release();
			}

			CAMD(" @ The Size of RingBuffer = %s\n", std::to_string(g_owRingbuffer.size()));
		}
		else {

			CAMW("Warning, g_owRingbuffer.pop is Fail!!! \n");
		}
	}
	else {

		CAMW("Warning: The Streaming is not running. Please check again.\n");
		return -1;
	}

	return nRet;
}


int CMethod_HikGigECamCtrl::AcquireStreaming_Close()
{
	CAMW("# AcquireStreaming_Close(...) >>>\n");

	{
		std::lock_guard<std::mutex> lock(u_mutex);

		notified_IsDone = true;
		notified_IsRun = true;
		notified_IsTrigger = false;

		CAMI("Close the Thread..\n");
		CAMI(" > notified_IsDone = %d\n", notified_IsDone);
		CAMI(" > notified_IsRun = %d\n", notified_IsRun);
		CAMI(" > notified_IsTrigger = %d\n", notified_IsTrigger);
	}

	//record the bIsEnbTriggerMode status.
	//int prev_Record_TriggerMode = m_global_cfg_ParamInfo.bIsEnbTriggerMode;


	CAMW(" >> cond_var.notify_all()\n");
	cond_var.notify_all();

	CAMW(" >> cv_trigger.notify_all()\n");
	cv_trigger.notify_all();

	m_global_cfg_ParamInfo.bIsEnbTriggerMode = false;

	bIsCreated = false;

	m_global_cfg_ParamInfo.bIsStreaming = false;


	std::unique_lock<std::mutex> lck_end(mtx_triggrt_end);
	cv_trigger_end.wait(lck_end);

	//if (prev_Record_TriggerMode) {
	
		GigECam_Release();
		usleep(2000);
		GigeCam_Init();

	//	prev_Record_TriggerMode = 0;
	//}

	CAMW("# AcquireStreaming_Close(...) <<<\n");

	return 0;
}


int CMethod_HikGigECamCtrl::Thread_Acquire(void* pCam, LpGigECamConfig pParamOut)
{

	CAMW(" ## Thread_Acquire(...) >>> Start ~\n");
	CAMI(" > pCam = %d\n", pCam);

	int ret = 0;
	string strInfo;

	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;

	bool bIsAlreadyStart = 0;

	cv::Mat matImg;
	string strResponse;

	while (!notified_IsDone)
	{

		//cycletime_start >>
		start = std::chrono::high_resolution_clock::now();


		std::unique_lock<std::mutex> lock(u_mutex);
		while (!notified_IsRun)
		{
			// # End acquisition
			if (bIsAlreadyStart) {
				CAMW(" @ >> Thread_Acquire -> MV_CC_StopGrabbing() \n");
				
				// 停止取流
				// end grab image
				ret = MV_CC_StopGrabbing(pCam);
				if (MV_OK != ret)
				{
					CAMW("MV_CC_StopGrabbing fail! nRet [%x]\n", ret);
				}

				iCnt_0 = 0;
				iCnt_1 = 0;


				if (!matImg.empty()) {
					matImg.release();
				}

				bIsAlreadyStart = 0;

				usleep(50000);
			}

			CAMW(" @ >> Thread_Acquire() wait\n");

			cond_var.wait(lock);
		}
		lock.unlock();

		//// # Only for Debug ~~~
		//std::cout << "worker_thread() is processing data\n";
		//std::cout << "notified_IsDone = " << notified_IsDone << endl;
		//std::cout << "notified_IsRun = " << notified_IsRun << endl;

		if (!bIsAlreadyStart) {

			// 開始取流
			// start grab image
			ret = MV_CC_StartGrabbing(pCam);
			if (MV_OK != ret)
			{
				CAMW("MV_CC_StartGrabbing fail! nRet [%x]\n", ret);
			}

			bIsAlreadyStart = 1;
			usleep(50000);
		}

		std::unique_lock<std::mutex> lck(mtx_triggrt);
		while (notified_IsTrigger) { // 使用 while 迴圈檢查條件

			CAMW(" # >> Thread_TriggerMode() lock \n");

			cv_trigger.wait(lck);
		}


		iCnt_0++;

		ret = AcquireStreaming(pCam, matImg);

		if (matImg.empty()) {
			usleep(10);
			//cout << " > !! > matImg.empty()" << endl;
			continue;
		}

#ifdef ALGO_Enable_StreamingBufOpt_AddTimestamp_DEBUG

		if (m_iFrameRate_Cnt >= ULONG_MAX) {
			m_iFrameRate_Cnt = 0;
		}

		//strInfo = "No." + std::to_string(m_iFrameRate_Cnt++) + "_" + getCurrentTime();
		strInfo = "No." + std::to_string(m_iFrameRate_Cnt++);

		cv::putText(matImg, strInfo, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 255, 255), 2);

#endif

		if (ret) {

			CAME("Error!!! AcquireStreaming() in Thread_Acquire(...) \n");
			AcquireStreaming_StartorStop(false);
			usleep(50000);
		}

		if (!matImg.empty()) {

			send_mat(matImg);
			restful_client.postImage(matImg, strResponse);

			matImg.release();
		}


		//// # cycletime_end
		//if (iCnt_0 >= ULONG_MAX) {
		//	iCnt_0 = 0;
		//}

		//cycletime_end << 
		//end = std::chrono::high_resolution_clock::now();
		//duration = end - start;
		//strtmp = std::to_string(duration.count());
		//CAMM("FrmaeNo.[ %d ] .--.--> Total_AcquireImages()_CycleTime : %s (ms)\n", iCnt_0, strtmp.c_str());

		
		usleep(10);		
	}

	CAMW(" ## Thread_Acquire(...) <<< End!!\n");

	cv_trigger_end.notify_all();

	return 0;
}


int CMethod_HikGigECamCtrl::Thread_TriggerMode(void* pCam, LpGigECamConfig pParamOut)
{

	CAMW(" ## Thread_TriggerMode(...) >>> Start ~\n");
	CAMI(" > pCam = %d\n", pCam);

	int ret = 0;
	string strInfo;
	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;
	bool bIsAlreadyStart = 0;


	while (!notified_IsDone)
	{

		std::unique_lock<std::mutex> lock(u_mutex);
		while (!notified_IsRun)
		{
			// # End acquisition
			if (bIsAlreadyStart) {
				CAMI(" # >> Thread_Acquire -> MV_CC_StopGrabbing() \n");

				// 停止取流
				// end grab image
				ret = MV_CC_StopGrabbing(pCam);
				if (MV_OK != ret)
				{
					CAMW("Warning, MV_CC_StopGrabbing fail! nRet [%x]\n", ret);
				}

				iCnt_0 = 0;
				iCnt_1 = 0;

				bIsAlreadyStart = 0;

				usleep(50000);
			}

			CAMW(" # >> Thread_TriggerMode() wait\n");
			cond_var.wait(lock);
		}
		lock.unlock();

		if (!bIsAlreadyStart) {

			// 開始取流
			// start grab image
			ret = MV_CC_StartGrabbing(pCam);
			if (MV_OK != ret)
			{
				CAMW("MV_CC_StartGrabbing fail! nRet [%x]\n", ret);
			}

			bIsAlreadyStart = 1;
			usleep(50000);
		}

		std::unique_lock<std::mutex> lck(mtx_triggrt);
		while (pParamOut->bIsEnbTriggerMode) { // 使用 while 迴圈檢查條件

			CAMW(" # >> Thread_TriggerMode() lock \n");

			cv_trigger.wait(lck);
		}

		usleep(10);
	}

	CAMW(" ## Thread_TriggerMode(...) <<< End!!\n");
	CAMW(" ## cv_trigger_end.notify_all() \n");

	cv_trigger_end.notify_all();


	return 0;
}




int CMethod_HikGigECamCtrl::GigECam_DebugPrint()
{
	int ret = EXIT_SUCCESS;

	CAMD("GigECam_DebugPrint() --> TDB\n");

	return ret;
} 