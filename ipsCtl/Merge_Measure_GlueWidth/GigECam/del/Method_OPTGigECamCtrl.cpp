#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "../cvip.h"
#include "Method_OPTGigECamCtrl.h"



using namespace std;

static OverwriteRingBuffer<cv::Mat> g_owRingbuffer(6);

int CMethod_OPTGigECamCtrl::g_exPrev_W = 0;
int CMethod_OPTGigECamCtrl::g_exPrev_H = 0;
OPT_EPixelType CMethod_OPTGigECamCtrl::g_exConvertFormat = gvspPixelRGB8;
unsigned char* CMethod_OPTGigECamCtrl::g_pDataForRGB = nullptr;

//////////////////////////////////////////////////////////////////////////////
///  Hik vision Gige camera controller function
/////////////////////////////////////////////////////////////////////////////

CMethod_OPTGigECamCtrl::CMethod_OPTGigECamCtrl()
	: m_pCam(nullptr)
	, m_stDevList({ 0 })
	, m_global_cfg_ParamInfo(seGigECamConfig())
	, m_fLimit_ExposureTime_Min(15.0)			//define by the MVS of HikVision
	, m_fLimit_ExposureTime_Max(9.9995e+06)		//define by the MVS of HikVision
	, m_iFrameRate_Cnt(0)
	, m_RetryCnt(10)
{}


CMethod_OPTGigECamCtrl::~CMethod_OPTGigECamCtrl()
{
	GigECam_Release();
}


int CMethod_OPTGigECamCtrl::GigeCam_Init()
{
	int nRet = 0;

	if (m_global_cfg_ParamInfo.bIsStreaming) {

		CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(5000); /* delay 5  ms */
	}


	// discover camera 
	//OPT_DeviceList m_stDevList;
	nRet = OPT_EnumDevices(&m_stDevList, interfaceTypeAll);
	if (OPT_OK != nRet)
	{
		("Error !!, Enumeration devices failed! ErrorCode[%d]\n", nRet);
		return -1;
	}

	if (m_stDevList.nDevNum < 1)
	{
		("Error !!, no camera\n");
		return -1;
	}


	// Print camera info (Index, Type, Vendor, Model, Serial number, DeviceUserID, IP Address) 
	PrintDeviceInfo(m_stDevList);


	// Select one camera to connect to  
	unsigned int nDeviceIndex = 0;

	nDeviceIndex = m_stDevList.nDevNum;

	CAMM(" # nDeviceIndex is [%d].\n", nDeviceIndex);

	// Create Device Handle
	nRet = OPT_CreateHandle(&m_pCam, OPT_ECreateHandleMode::modeByIndex, (void*)&nDeviceIndex);
	if (OPT_OK != nRet)
	{
		CAME("ERROR !!, Create m_pCam failed! ErrorCode[%d]\n", nRet);
		return -1;
	}


	// Open camera 
	nRet = OPT_Open(m_pCam);
	if (OPT_OK != nRet)
	{
		CAME("ERROR !!, Open camera failed! ErrorCode[%d]\n", nRet);
		return -1;
	}
	

	if (Configure_Get(m_pCam, &m_global_cfg_ParamInfo, 1)) {

		CAMD("Configure_Get( m_pCam, \" m_global_cfg_ParamInfo \", 1 )\n");
	}


	return nRet;
}



int CMethod_OPTGigECamCtrl::GigECam_SetConfig(const LpGigECamConfig pParamIn)
{

	int nRet = 0;

	if (m_pCam == nullptr) {
		CAME(" > Erroe!!! m_pCam <= 0\n");
		return -1;
	}

	if (!m_global_cfg_ParamInfo.bIsStreaming) {

		// ture off triggermode, and setting the AcquisitionFrameRateEnable to false.
		Configure_Default(m_pCam, pParamIn);

		// Combine photo-sensitive cells together. This will reduces the resolution of the image.
		if (!nRet) nRet = Configure_Binning(m_pCam, pParamIn);

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


	if (nRet < 0)
	{
		if (m_global_cfg_ParamInfo.bIsStreaming) {

			CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

			AcquireStreaming_StartorStop(false);
		}

		return nRet;
	}

	return nRet;
}


int CMethod_OPTGigECamCtrl::GigECam_GetConfig(LpGigECamConfig pParamOut)
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



int CMethod_OPTGigECamCtrl::GigECam_AcquireImages(string strFilePath)
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



int CMethod_OPTGigECamCtrl::GigECam_AcquireImages(cv::Mat& matImg)
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



int CMethod_OPTGigECamCtrl::GigECam_Release()
{
	int nRet = 0;

	if (m_pCam) {

		CAMD("GigECam_Release() === === >>>\n");

		CAMD(" # >> Release before, m_pCam Handle Address = 0x % llx\n", m_pCam);

		// Close camera 
		nRet = OPT_Close(m_pCam);
		if (OPT_OK != nRet)
		{
			CAME("Error !!, OPT_Close fail! nRet [%x]\n", nRet);
			//return nRet;
		}

		if (m_pCam != nullptr) {

			// Destroy Device Handle
			OPT_DestroyHandle(m_pCam);

			m_pCam = nullptr;
		}


		sleep(1);

		CAMD(" # << Release after, m_pCam Handle Address = 0x % llx\n", m_pCam);

		CAMD("OPT_DestroyHandle(m_pCam) is done!!!! \n");

		CAMD("GigECam_Release() <<< === === \n");

	}

	if (g_pDataForRGB) {

		free(g_pDataForRGB);
		g_pDataForRGB = nullptr;
	}

	return nRet;
}


int CMethod_OPTGigECamCtrl::GigECam_Strm_Prepare()
{
	int nRet = 0;

	AcquireStreaming_Prepare();

	return nRet;
}


int CMethod_OPTGigECamCtrl::GigECam_Strm_Start()
{
	int nRet = 0;

	AcquireStreaming_StartorStop(true);
	usleep(50000);

	return nRet;
}


int CMethod_OPTGigECamCtrl::GigECam_Strm_AcquireImages(string strFilePath)
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


int CMethod_OPTGigECamCtrl::GigECam_Strm_AcquireImages(cv::Mat& matImg)
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


int CMethod_OPTGigECamCtrl::GigECam_Strm_Stop()
{
	int nRet = 0;

	AcquireStreaming_StartorStop(false);

	usleep(50000);

	return nRet;
}


int CMethod_OPTGigECamCtrl::GigECam_Strm_Close()
{
	int nRet = 0;

	AcquireStreaming_StartorStop(false);
	usleep(50000);

	AcquireStreaming_Close();
	usleep(50000);

	return nRet;
}



static int CMethod_OPTGigECamCtrl::PrintDeviceInfo(OPT_DeviceList deviceInfoList)
{
	int nRet = 0;

	OPT_DeviceInfo* pDevInfo = nullptr;
	unsigned int cameraIndex = 0;
	char vendorNameCat[11];
	char cameraNameCat[16];

	// Print title line 
	printf("\nIdx Type Vendor     Model      S/N             DeviceUserID    IP Address    \n");
	printf("------------------------------------------------------------------------------\n");

	for (cameraIndex = 0; cameraIndex < deviceInfoList.nDevNum; cameraIndex++)
	{
		pDevInfo = &deviceInfoList.pDevInfo[cameraIndex];

		// Camera index in device list, display in 3 characters 
		printf("%-3d", cameraIndex + 1);


		// Camera type 
		switch (pDevInfo->nCameraType)
		{
		case typeGigeCamera:printf(" GigE"); break;
		case typeU3vCamera:printf(" U3V "); break;
		case typeCLCamera:printf(" CL  "); break;
		case typePCIeCamera:printf(" PCIe"); break;
		default:printf("     "); break;
		}


		// Camera vendor name, display in 10 characters 
		if (strlen(pDevInfo->vendorName) > 10)
		{
			memcpy(vendorNameCat, pDevInfo->vendorName, 7);
			vendorNameCat[7] = '\0';
			strcat(vendorNameCat, "...");
			printf(" %-10.10s", vendorNameCat);
		}
		else
		{
			printf(" %-10.10s", pDevInfo->vendorName);
		}


		// Camera model name, display in 10 characters 
		printf(" %-10.10s", pDevInfo->modelName);


		// Camera serial number, display in 15 characters 
		printf(" %-15.15s", pDevInfo->serialNumber);


		// Camera user id, display in 15 characters 
		if (strlen(pDevInfo->cameraName) > 15)
		{
			memcpy(cameraNameCat, pDevInfo->cameraName, 12);
			cameraNameCat[12] = '\0';
			strcat(cameraNameCat, "...");
			printf(" %-15.15s", cameraNameCat);
		}
		else
		{
			printf(" %-15.15s", pDevInfo->cameraName);
		}


		// IP address of GigE camera 
		if (pDevInfo->nCameraType == typeGigeCamera)
		{
			printf(" %s", pDevInfo->DeviceSpecificInfo.gigeDeviceInfo.ipAddress);
		}

		printf("\n");
	}

	return nRet;
}


int CMethod_OPTGigECamCtrl::Configure_Default(OPT_HANDLE pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;


	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;

	bool bTriggerMode = pParamIn->bIsEnbTriggerMode;
	m_global_cfg_ParamInfo.bIsEnbTriggerMode = bTriggerMode;

	CAMM(" >> bTriggerMode = %d\n", bTriggerMode);
	CAMM(" >> setting m_global_cfg_ParamInfo.bIsEnbTriggerMode = %d\n", m_global_cfg_ParamInfo.bIsEnbTriggerMode);


	if (camera) {

		// config : Trigger Mode  --. -->
		if (!nRet) {

			CAMM(" >> setting AcquisitionFrameRateEnable = %d\n", false);

			nRet = OPT_SetBoolFeatureValue(camera, "AcquisitionFrameRateEnable", false);

			if (OPT_OK != nRet)
			{
				CAMM("set AcquisitionFrameRateEnable fail! nRet [%x]\n", nRet);
				nRet = -1;
			}

			usleep(5000); /* delay 5  ms */

		}

		if (!nRet) {

			bool bTriggerMode = 0; //off the trigger

			CAMW(" >> setting TriggerMode to Default value = %d\n", bTriggerMode);

			// set trigger mode as on(1)/off(0)
			OPT_SetEnumFeatureValue(camera, "TriggerMode", bTriggerMode);

			if (OPT_OK != nRet)
			{
				CAME("Error, Enum value of TriggerMode fail! nRet [%x]\n", nRet);
				nRet = -1;
			}

			CAMM("Done~~~\n");			

			usleep(5000); /* delay 5  ms */

		}

		if (OPT_OK != nRet) {

			/* En error happened, display the correspdonding message */
			CAME("Error: [%x]\n", nRet);
			return -1;
		}

	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error, OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}



int CMethod_OPTGigECamCtrl::Configure_ImageFormat(OPT_HANDLE pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;

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

			nRet = OPT_SetIntFeatureValue(m_pCam, "OffsetX", 0);
			nRet = OPT_SetIntFeatureValue(m_pCam, "Width", iLimit_Max_Width);

			CAMD(" Configure parameters : Range_iWidth = %d\n", iRange_Max_Width);

			nRet = OPT_SetIntFeatureValue(m_pCam, "Width", iRange_Max_Width);

			if (OPT_OK != nRet)
			{
				CAMM("set Int value ofOffsetX or Width fail! nRet [%x]\n", nRet);
				nRet = -1;
			}

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (!nRet) {

			CAMD(" Configure parameters : Range_iOffsetX = %d\n", iRange_Max_Offset_X);

			nRet = OPT_SetIntFeatureValue(m_pCam, "OffsetX", iRange_Max_Offset_X);

			if (OPT_OK != nRet)
			{
				CAMM("set Int value ofOffsetX fail! nRet [%x]\n", nRet);
				nRet = -1;
			}

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

			nRet = OPT_SetIntFeatureValue(m_pCam, "OffsetY", 0);
			nRet = OPT_SetIntFeatureValue(m_pCam, "Height", iLimit_Max_Height);

			CAMD(" Configure parameters : Range_iHeight = %d\n", iRange_Max_Height);

			nRet = OPT_SetIntFeatureValue(m_pCam, "Height", iRange_Max_Height);

			if (OPT_OK != nRet)
			{
				CAMM("set Int value OffsetY or Height fail! nRet [%x]\n", nRet);
				nRet = -1;
			}

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (!nRet) {

			CAMD(" Configure parameters : Range_iOffsetY = %d\n", iRange_Max_Offset_Y);

			nRet = OPT_SetIntFeatureValue(m_pCam, "OffsetY", iRange_Max_Offset_Y);

			if (OPT_OK != nRet)
			{
				CAMM("set Int value OffsetY fail! nRet [%x]\n", nRet);
				nRet = -1;
			}

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}



		/// # ////////////////////////////////////////////
		// 3. > config : PixelFormat  --. -->
		if (!nRet) {

			CAMD("pixel_format = %d\n", pParamIn->bPixelFormat);

			if (pParamIn->bPixelFormat == 0) {  //0:PixelFormat_Mono8, 1:PixelFormat_BayerGR8
				nRet = OPT_SetEnumFeatureValue(m_pCam, "PixelFormat", 0x01080001);   //0x01080001:Mono8

				if (OPT_OK != nRet)
				{
					CAMM("set Int value PixelFormat fail! nRet [%x]\n", nRet);
					nRet = -1;
				}
			}
			else {
				CAME("Error !!!, PixelFormat do not suuply Color mod. nRet [%x]\n", nRet);
				nRet = -1;
			}

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (OPT_OK != nRet) {

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


int CMethod_OPTGigECamCtrl::Configure_Binning(OPT_HANDLE pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;

	if (camera) {

		int iScale = pParamIn->iBinning_Scale;	// 0:Off, 1:X, 2:Y, 3:XY

		if (iScale > 3) {
			iScale = 3;
		}
		else if (iScale < 0) {
			iScale = 0;
		}

		if (!nRet) {

			CAMD(" Configure parameters : Binning = %d\n", iScale);

			nRet = OPT_SetEnumFeatureValue(m_pCam, "Binning", iScale);

			if (OPT_OK != nRet) {

				CAME("Error: The value of Binning is fail [%x]\n", nRet);
				nRet = -1;
			}

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (OPT_OK != nRet) {

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



int CMethod_OPTGigECamCtrl::Configure_Decimation(OPT_HANDLE pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;

	if (camera) {

		int iScale = pParamIn->iBinning_Scale;	// 0:Off, 1:X, 2:Y, 3:XY

		if(iScale > 2) {
			iScale = 2; 
		}
		else if (iScale < 0) {
			iScale = 0;
		}

		if (!nRet) {

			CAMD(" Configure parameters : DecimationHorizontal = %d\n", iScale);

			nRet = OPT_SetEnumFeatureValue(m_pCam, "Decimation", iScale);

			if (OPT_OK != nRet) {

				/* En Error happened, display the correspdonding message */
				CAME("Error: The value of Decimation is fail [%x]\n", nRet);
				return -1;
			}

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
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
int CMethod_OPTGigECamCtrl::Configure_Exposure(OPT_HANDLE pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;
	int nExposureMode = -1;
	double dbExposureTime_us = 0.0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;

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
			nRet = OPT_SetEnumFeatureValue(camera, "ExposureAuto", nExposureMode);

			if (OPT_OK != nRet)
			{
				CAMM("set Enum of ExposureAuto fail! nRet [%x]\n", nRet);
				nRet = -1;
			}
		}


		if (!nRet && !nExposureMode) {
			CAMD("Exposure Time ==>\n");
			CAMD("dbExposureTime_us = %5.2f\n", dbExposureTime_us);
			nRet = OPT_SetDoubleFeatureValue(camera, "ExposureTime", dbExposureTime_us);

			if (OPT_OK != nRet)
			{
				CAMM("set Double of ExposureTime fail! nRet [%x]\n", nRet);
				nRet = -1;
			}
		}

		if (OPT_OK != nRet) {

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


int CMethod_OPTGigECamCtrl::Configure_FrameRate(OPT_HANDLE pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;

	if (camera) {

		// config : AcquisitionFrameRateEnable  --. -->
		if (!nRet) {

			bool nEnbAcqFrameRateEnable = true;

			CAMD("AcquisitionFrameRateEnable = %d\n", nEnbAcqFrameRateEnable);
			
			nRet = OPT_SetBoolFeatureValue(camera, "AcquisitionFrameRateEnable", nEnbAcqFrameRateEnable);

			if (OPT_OK != nRet)
			{
				CAMM("set AcquisitionFrameRateEnable fail! nRet [%x]\n", nRet);
				nRet = -1;
			}

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */

		}

		if (!nRet) {

			double dbAcqFrameRate = pParamIn->dbAcquisitionFrameRate;

			CAMD("dbAcqFrameRate = %5.3f\n", dbAcqFrameRate);

			nRet = OPT_SetDoubleFeatureValue(camera, "AcquisitionFrameRate", dbAcqFrameRate);

			if (OPT_OK != nRet)
			{
				CAMM("set AcquisitionFrameRateEnable fail! nRet [%x]\n", nRet);
				nRet = -1;
			}

			CAMD(" < Done~~~\n");

			usleep(500); /* delay 5  ms */

		}

		if (OPT_OK != nRet) {

			/* En error happened, display the correspdonding message */
			CAME("Error: [%x]\n", nRet);
			return -1;
		}

	}


	return nRet;
}


int CMethod_OPTGigECamCtrl::Configure_TriggerMode(OPT_HANDLE pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;

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

			// config : Trigger source  --. -->
			//if (!nRet) 
			{
				CAMD(" >> setting iTriggerSrc = %d\n", iTriggerSrc);

				switch (iTriggerSrc) {
				case 2:	// Line1
					iTriggerSrc = 2;
					break;
				case 3:	// Line2
					iTriggerSrc = 3;
					break;
				case 0:	// Software
				default:
					iTriggerSrc = 0;
					break;
				}

				// set trigger source
				nRet = OPT_SetEnumFeatureSymbol(camera, "TriggerSource", iTriggerSrc);
				if (OPT_OK != nRet)
				{
					CAMM("Set triggerSource value failed! ErrorCode[%d]\n", nRet);
				}

				m_global_cfg_ParamInfo.iTriggerSrc = iTriggerSrc;


				CAMD(" < Done~~~\n");

				usleep(500); /* delay 5  ms */

			}


			// config : Trigger TriggerSelector  --. -->
			//if (!nRet) 
			{			
				// Set trigger selector to FrameStart 
				nRet = OPT_SetEnumFeatureSymbol(camera, "TriggerSelector", "FrameStart");
				if (OPT_OK != nRet)
				{
					CAMM("Set triggerSelector value failed! ErrorCode[%d]\n", nRet);
				}

				usleep(500); /* delay 5  ms */
			}

			// config : Trigger activation  --. -->
			//if (!nRet) 
			{
				// TriggerActivation : 0:RisingEdge, 1:FallingEdge, 2:TriggerWidth
				CAMD(" >> setting TriggerActivation = %d\n", iTriggerActivation);

				// TriggerActivation : 0:RisingEdge, 1:FallingEdge, 2:TriggerWidth
				nRet = OPT_SetEnumFeatureSymbol(camera, "TriggerActivation", iTriggerActivation);
				if (OPT_OK != nRet)
				{
					CAMM("Set triggerActivation value failed! ErrorCode[%d]\n", nRet);
				}

				CAMD(" < Done~~~\n");

				usleep(500); /* delay 5  ms */
			}

			// config : Register image callback function() --. -->
			//if (!nRet) 
			{

				// en:Register image callback
				CAMD(" >> setting Register image callback function() \n");

				nRet = OPT_AttachGrabbing(camera, AcquireCallBackEx, camera);
				if (OPT_OK != nRet)
				{
					CAMM("Attach grabbing failed! ErrorCode[%d]\n", nRet);
				}

				CAMD(" < Done~~~\n");

				usleep(500); /* delay 5  ms */

			}

			// config : Trigger Mode  --. -->
			//if (!nRet) 
			{

				CAMD(" >> setting bTriggerMode = %d\n", bTriggerMode);

				// set trigger mode as on(1)/off(0)
				nRet = OPT_SetEnumFeatureSymbol(camera, "TriggerMode", 1);
				if (OPT_OK != nRet)
				{
					CAMM("Set triggerMode value failed! ErrorCode[%d]\n", nRet);
				}

				CAMD(" < Done~~~\n");

				usleep(500); /* delay 5  ms */

			}

		}


	}


	return nRet;
}



int CMethod_OPTGigECamCtrl::Configure_PersistentIP(OPT_HANDLE pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;
	// TBD
	return nRet;
}


int CMethod_OPTGigECamCtrl::Configure_ResetConfigureCustomImageSettings(OPT_HANDLE pCam)
{
	int result = 0;
	// TBD
	return result;
}


int CMethod_OPTGigECamCtrl::Configure_ResetExposure(OPT_HANDLE pCam)
{
	int result = 0;
	// TBD
	return result;
}


int CMethod_OPTGigECamCtrl::Configure_Get(OPT_HANDLE pCam, LpGigECamConfig pParamOut, bool bDumpInf)
{

	if (nullptr == pParamOut)
		return -1;

	int nRet = 0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;

	LpGigECamConfig pParam = pParamOut;

	if (camera != nullptr) {

		int64_t cfg_Max_Width = { 0 }, cfg_Max_Height = { 0 };

		int64_t cfg_OffsetX = { 0 }, cfg_OffsetY = { 0 };
		int64_t cfg_Width = { 0 }, cfg_Height = { 0 };

		uint64_t cfg_PixelFormat = { 0 };
		uint64_t cfg_Exposure_modeto = { 0 };

		uint64_t cfg_Binning_ScaleVal = { 0 };

		bool cfg_bIsEnbAcquisitionFrameRate = { 0 };
		double cfg_dbAcquisitionFrameRate = { 0 };


		if (!nRet) nRet = OPT_GetIntFeatureValue(camera, "SensorWidth", &cfg_Max_Width);
		usleep(500);
		if (!nRet) nRet = OPT_GetIntFeatureValue(camera, "SensorHeight", &cfg_Max_Height);
		usleep(500);

		if (!nRet) nRet = OPT_GetIntFeatureValue(camera, "OffsetX", &cfg_OffsetX);
		usleep(500);
		if (!nRet) nRet = OPT_GetIntFeatureValue(camera, "OffsetY", &cfg_OffsetY);
		usleep(500);

		if (!nRet) nRet = OPT_GetIntFeatureValue(camera, "Width", &cfg_Width);
		usleep(500);
		if (!nRet) nRet = OPT_GetIntFeatureValue(camera, "Height", &cfg_Height);
		usleep(500);

		if (!nRet) nRet = OPT_GetEnumFeatureValue(camera, "PixelFormat", &cfg_PixelFormat);
		usleep(500);
		if (!nRet) nRet = OPT_GetEnumFeatureValue(camera, "ExposureAuto", &cfg_Exposure_modeto);
		usleep(500);

		if (!nRet) nRet = OPT_GetEnumFeatureValue(camera, "Binning", cfg_Binning_ScaleVal);
		usleep(500);
		//if (!nRet) nRet = OPT_GetEnumFeatureValue(camera, "Decimation", &cfg_Binning_ScaleVal);
		//usleep(500);

		if (!nRet) nRet = OPT_GetBoolFeatureValue(camera, "AcquisitionFrameRateEnable", &cfg_bIsEnbAcquisitionFrameRate);
		usleep(500);

		if (cfg_bIsEnbAcquisitionFrameRate) {
			if (!nRet) nRet = OPT_GetDoubleFeatureValue(camera, "AcquisitionFrameRate", &cfg_dbAcquisitionFrameRate);
			usleep(500);
		}



		if (OPT_OK != nRet) {

			/* En error happened, display the correspdonding message */
			CAMD("Error: [%x]\n", nRet);
			return -1;
		}

		pParam->bIsConnected = true;

		pParam->iSensor_Width = cfg_Max_Width;
		pParam->iSensor_Height = cfg_Max_Height;

		pParam->iOffset_X = cfg_OffsetX;
		pParam->iOffset_Y = cfg_OffsetY;

		pParam->iWidth = cfg_Width;
		pParam->iHeight = cfg_Height;

		pParam->bPixelFormat = (cfg_PixelFormat == (uint64_t)gvspPixelMono8) ? 0 : 1;  //0:PixelFormat_Mono8, 1:PixelFormat_BayerGR8
		pParam->bExposureAuto = (cfg_Exposure_modeto == (uint64_t)2) ? 0 : 1;     //0:Auto, 1:Timed;

		pParam->iBinning_Scale = (cfg_Binning_ScaleVal == (uint64_t)0) ? 0 : 1;		// 0:Off, 1:X, 2:Y, 3:XY

		pParam->bIsEnbAcquisitionFrameRate = cfg_bIsEnbAcquisitionFrameRate;
		pParam->dbAcquisitionFrameRate = cfg_dbAcquisitionFrameRate;



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



// Image convert
int CMethod_OPTGigECamCtrl::imageConvert(OPT_HANDLE camera, OPT_Frame frame, cv::Mat& matImg)
{
	int nRet = 0;

	OPT_PixelConvertParam stPixelConvertParam;
	unsigned char* pDstBuf = nullptr;
	unsigned int nDstBufSize = 0;
	const char* pConvertFormatStr = nullptr;

	//OPT_EPixelType convertFormat = gvspPixelMono8;
	OPT_EPixelType convertFormat = gvspPixelRGB8;

	switch (convertFormat)
	{
	case gvspPixelRGB8:
		nDstBufSize = sizeof(unsigned char) * frame.frameInfo.width * frame.frameInfo.height * 3;
		pConvertFormatStr = (const char*)"RGB8";
		break;

	case gvspPixelBGR8:
		nDstBufSize = sizeof(unsigned char) * frame.frameInfo.width * frame.frameInfo.height * 3;
		pConvertFormatStr = (const char*)"BGR8";
		break;
	case gvspPixelBGRA8:
		nDstBufSize = sizeof(unsigned char) * frame.frameInfo.width * frame.frameInfo.height * 4;
		pConvertFormatStr = (const char*)"BGRA8";
		break;
	case gvspPixelMono8:
	default:
		nDstBufSize = sizeof(unsigned char) * frame.frameInfo.width * frame.frameInfo.height;
		pConvertFormatStr = (const char*)"Mono8";
		break;
	}

	pDstBuf = (unsigned char*)malloc(nDstBufSize);
	if (nullptr == pDstBuf)
	{
		printf("malloc pDstBuf failed!\n");
		return;
	}

	// 图像转换成BGR8
	// convert image to BGR8
	memset(&stPixelConvertParam, 0, sizeof(stPixelConvertParam));
	stPixelConvertParam.nWidth = frame.frameInfo.width;
	stPixelConvertParam.nHeight = frame.frameInfo.height;
	stPixelConvertParam.ePixelFormat = frame.frameInfo.pixelFormat;
	stPixelConvertParam.pSrcData = frame.pData;
	stPixelConvertParam.nSrcDataLen = frame.frameInfo.size;
	stPixelConvertParam.nPaddingX = frame.frameInfo.paddingX;
	stPixelConvertParam.nPaddingY = frame.frameInfo.paddingY;
	stPixelConvertParam.eBayerDemosaic = demosaicNearestNeighbor;
	stPixelConvertParam.eDstPixelFormat = convertFormat;
	stPixelConvertParam.pDstBuf = pDstBuf;
	stPixelConvertParam.nDstBufSize = nDstBufSize;

	nRet = OPT_PixelConvert(camera, &stPixelConvertParam);
	if (OPT_OK == nRet) {


		cv::Mat tmpImg;

		CAMM("image convert to %s successfully! nDstDataLen (%u)\n", pConvertFormatStr, stPixelConvertParam.nDstBufSize);

		if (frame.frameInfo.pixelFormat == gvspPixelMono8) {

			tmpImg = cv::Mat(frame.frameInfo.height, frame.frameInfo.width, CV_8UC1, pDstBuf);
		}
		else if ((frame.frameInfo.pixelFormat == gvspPixelRGB8) ||
				(frame.frameInfo.pixelFormat == gvspPixelBGR8) ){

			tmpImg = cv::Mat(frame.frameInfo.height, frame.frameInfo.width, CV_8UC3, pDstBuf);
		}
		else
		{
			CAME("Error !!!, unsupported pixel format\n");
			goto Err;
		}

		if (!tmpImg.empty()) {

			tmpImg.copyTo(matImg);

			tmpImg.release();
		}
		else {
			nRet = -1;
		}

	}
	else {

		CAME("Error !!!, image convert to %s failed! ErrorCode[%d]\n", pConvertFormatStr, nRet);
	}

Err:

	if (pDstBuf)
	{
		free(pDstBuf);
		pDstBuf = nullptr;
	}

	return nRet;
}



int CMethod_OPTGigECamCtrl::AcquireImages(OPT_HANDLE pCam, string strFilePath)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;
	int nRet = 0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;
	OPT_Frame frame;

	if (strFilePath.empty()) {

		CAME("Error !!!, The strFilePath is Empty\n");
		return -1;
	}

	if (camera) {

		// Start grabbing 
		nRet = OPT_StartGrabbing(camera);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, Start grabbing failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

		// Get a frame image
		nRet = OPT_GetFrame(camera, &frame, 500);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, Get frame failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

		cv::Mat tmpImg;

		nRet = imageConvert(camera, frame, tmpImg);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, image convert to RGB failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

		try {
			
			cv::imwrite(strFilePath, tmpImg);
			tmpImg.release();

			/* Display some informations about the retrieved buffer */
			CAMD("Acquired file path : %s\n", strFilePath.c_str());
		}
		catch (cv::Exception& ex) {
			CAME("Error, Exception saving image to bmp format: %s\n", ex.what());
		}

		// Free image buffer
		nRet = OPT_ReleaseFrame(camera, &frame);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, Release frame failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	usleep(50000); /* delay 50  ms */
	
	return nRet;
}



int CMethod_OPTGigECamCtrl::AcquireImages(OPT_HANDLE pCam, cv::Mat& matImg)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;
	int nRet = 0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;
	OPT_Frame frame;

	if (camera) {
	
		// Start grabbing 
		nRet = OPT_StartGrabbing(camera);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, Start grabbing failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

		// Get a frame image
		nRet = OPT_GetFrame(camera, &frame, 500);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, Get frame failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}
			
		nRet = imageConvert(camera, frame, matImg);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, image convert to RGB failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

		// Free image buffer
		nRet = OPT_ReleaseFrame(camera, &frame);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, Release frame failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}
	
	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	usleep(50000); /* delay 50  ms */

	return nRet;
}



int CMethod_OPTGigECamCtrl::AcquireStreaming(OPT_HANDLE pCam, cv::Mat& matImg)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;
	int nRet = 0;

	/* Connect to the first available camera */
	OPT_HANDLE camera = (OPT_HANDLE)pCam;
	OPT_Frame frame;

	if (camera) {

		// Get a frame image
		nRet = OPT_GetFrame(camera, &frame, 500);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, Get frame failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

		nRet = imageConvert(camera, frame, matImg);
		if (OPT_OK != nRet)
		{
			CAME("Error !!!, image convert to RGB failed! ErrorCode[%d]\n", nRet);
			return nRet;
		}

	}
	else {
		/* En error happened, display the correspdonding message */
		CAME("Error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;

}



static void CMethod_OPTGigECamCtrl::AcquireCallBackEx(OPT_Frame* pFrame, void* pUser)
{
	int nRet = 0;
	int nDstBufSize = 0;
	OPT_PixelConvertParam stPixelConvertParam;

	if (pFrame)         // ch:用于保存图像的缓存 | en:Buffer to save image
	{
		CAMW(" > Get One Frame: Width[%d], Height[%d], nFrameNum[%llu]\n",
			pFrame->frameInfo.width, pFrame->frameInfo.height, pFrame->frameInfo.blockId);

		if (0 == g_exPrev_W) {
			g_exPrev_W = pFrame->frameInfo.width;
		}
		if (0 == g_exPrev_H) {
			g_exPrev_H = pFrame->frameInfo.height;
		}

		if (gvspPixelRGB8 == g_exConvertFormat) {
			g_exConvertFormat = pFrame->frameInfo.pixelFormat;
		}

		CAMW(" > Get One Frame: Width[%d], Height[%d], nFrameNum[%llu],  prev_W[%d], prev_H[%d], prev_Format[%x]\n",
			pFrame->frameInfo.width, pFrame->frameInfo.height, pFrame->frameInfo.blockId, g_exPrev_W, g_exPrev_H, g_exConvertFormat);

	}
	else {

		CAME("Error!!! AcquireCallBackEx fail \n");
		CAME("Error!!! AcquireCallBackEx fail \n");

		return;
	}

	switch (g_exConvertFormat)
	{
	case gvspPixelRGB8:
		nDstBufSize = sizeof(unsigned char) * pFrame->frameInfo.width * pFrame->frameInfo.height * 3;
		break;

	case gvspPixelBGR8:
		nDstBufSize = sizeof(unsigned char) * pFrame->frameInfo.width * pFrame->frameInfo.height * 3;
		break;
	case gvspPixelBGRA8:
		nDstBufSize = sizeof(unsigned char) * pFrame->frameInfo.width * pFrame->frameInfo.height * 4;
		break;
	case gvspPixelMono8:
	default:
		nDstBufSize = sizeof(unsigned char) * pFrame->frameInfo.width * pFrame->frameInfo.height;
		break;
	}

	if ((pFrame->frameInfo.width != g_exPrev_W) ||
		(pFrame->frameInfo.height != g_exPrev_H) ||
		(pFrame->frameInfo.pixelFormat != g_exConvertFormat) ||
		(nullptr == g_pDataForRGB)) {

		if (g_pDataForRGB) {

			free(g_pDataForRGB);
			g_pDataForRGB = nullptr;
		}

		g_pDataForRGB = (unsigned char*)malloc(nDstBufSize);
	}
	else {

		memset(g_pDataForRGB, 0x00, nDstBufSize);
	}


	// 图像转换成BGR8
// convert image to BGR8
	memset(&stPixelConvertParam, 0, sizeof(stPixelConvertParam));
	stPixelConvertParam.nWidth = pFrame->frameInfo.width;
	stPixelConvertParam.nHeight = pFrame->frameInfo.height;
	stPixelConvertParam.ePixelFormat = pFrame->frameInfo.pixelFormat;
	stPixelConvertParam.pSrcData = pFrame->pData;
	stPixelConvertParam.nSrcDataLen = pFrame->frameInfo.size;
	stPixelConvertParam.nPaddingX = pFrame->frameInfo.paddingX;
	stPixelConvertParam.nPaddingY = pFrame->frameInfo.paddingY;
	stPixelConvertParam.eBayerDemosaic = demosaicNearestNeighbor;
	stPixelConvertParam.eDstPixelFormat = g_exConvertFormat;
	stPixelConvertParam.pDstBuf = g_pDataForRGB;
	stPixelConvertParam.nDstBufSize = nDstBufSize;

	nRet = OPT_PixelConvert(pUser, &stPixelConvertParam);
	if (OPT_OK != nRet) {

		CAMW("Warning !!!, image convert to RGB failed! ErrorCode[%d]\n", nRet);
		return;
	}
	else {

		cv::Mat tmpImg;

		CAMM("image convert successfully! nDstDataLen (%u)\n", stPixelConvertParam.nDstBufSize);

		if (pFrame->frameInfo.pixelFormat == gvspPixelMono8) {

			tmpImg = cv::Mat(pFrame->frameInfo.height, pFrame->frameInfo.width, CV_8UC1, g_pDataForRGB);
		}
		else if ((pFrame->frameInfo.pixelFormat == gvspPixelRGB8)	|| 
				(pFrame->frameInfo.pixelFormat == gvspPixelBGR8)		){

			tmpImg = cv::Mat(pFrame->frameInfo.height, pFrame->frameInfo.width, CV_8UC3, g_pDataForRGB);
		}
		else
		{
			CAME("Error !!!, unsupported pixel format\n");
			return;
		}


#ifdef ALGO_Enable_StreamingBufOpt_SpeedOptimization_DEBUG

		cv::resize(tmpImg, tmpImg, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);

#endif
		//Only for Debug. --.-->
		//CAMD(" # Trigger, ==> /home/user/primax/vsb/grap/rexty_trigger.bmp\n");
		//cv::imwrite("/home/user/primax/vsb/grap/rexty_trigger.bmp", matImg);
		// <--.--

		CAMI("# g_owRingbuffer.push  ===.===> \n");

		g_owRingbuffer.push(tmpImg);
		CAMI(" ##  RingBuffer.szie() = %d\n", g_owRingbuffer.size());

		CAMI("# g_owRingbuffer.push  <===.=== \n");

		tmpImg.release();

	}

}



int CMethod_OPTGigECamCtrl::AcquireStreaming_Prepare()
{
	OPT_HANDLE camera = (OPT_HANDLE)m_pCam;

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

		std::thread t1;

		t1 = std::thread(&CMethod_OPTGigECamCtrl::Thread_Acquire, this, camera, &m_global_cfg_ParamInfo);

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


int CMethod_OPTGigECamCtrl::AcquireStreaming_StartorStop(bool bflgEnableThd)
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


int CMethod_OPTGigECamCtrl::AcquireStreaming_Capture(cv::Mat& matImg)
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


int CMethod_OPTGigECamCtrl::AcquireStreaming_Close()
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


int CMethod_OPTGigECamCtrl::Thread_Acquire(OPT_HANDLE pCam, LpGigECamConfig pParamOut)
{

	CAMW(" ## Thread_Acquire(...) >>> Start ~\n");
	CAMI(" > pCam = %d\n", pCam);

	int nRet = 0;
	string strInfo;

	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration(0);
	string strtmp;

	bool bIsAlreadyStart = 0;

	while (!notified_IsDone)
	{

		//cycletime_start >>
		start = std::chrono::high_resolution_clock::now();


		std::unique_lock<std::mutex> lock(u_mutex);
		while (!notified_IsRun)
		{
			// # End acquisition
			if (bIsAlreadyStart) {
				CAMW(" @ >> Thread_Acquire -> OPT_StopGrabbing() \n");
				
				// 停止取流
				// end grab image
				nRet = OPT_StopGrabbing(pCam);
				if (OPT_OK != nRet)
				{
					CAMW("OPT_StopGrabbing fail! nRet [%x]\n", nRet);
				}

				iCnt_0 = 0;
				iCnt_1 = 0;

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
			nRet = OPT_StartGrabbing(pCam);
			if (OPT_OK != nRet)
			{
				CAMW("OPT_StartGrabbing fail! nRet [%x]\n", nRet);
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
		cv::Mat matImg;

		nRet = AcquireStreaming(pCam, matImg);

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

		if (nRet) {

			CAME("Error!!! AcquireStreaming() in Thread_Acquire(...) \n");
			AcquireStreaming_StartorStop(false);
			usleep(50000);
		}

		if (!matImg.empty()) {

			send_mat(matImg);

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


static int CMethod_OPTGigECamCtrl::GigECam_DebugPrint()
{
	int nRet = EXIT_SUCCESS;

	CAMD("GigECam_DebugPrint() --> TDB\n");

	return nRet;
} 