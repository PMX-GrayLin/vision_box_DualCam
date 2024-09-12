#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "../cvip.h"
#include "Method_BaslerGigECamCtrl.h"



using namespace std;

static OverwriteRingBuffer<cv::Mat> g_owRingbuffer(6);

//////////////////////////////////////////////////////////////////////////////
///  Basler vision Gige camera controller function
/////////////////////////////////////////////////////////////////////////////

CMethod_BaslerGigECamCtrl::CMethod_BaslerGigECamCtrl()
	: m_pCam(nullptr)
	//, m_stDevList({ 0 })
	, m_global_cfg_ParamInfo(seGigECamConfig())
	, m_fLimit_ExposureTime_Min(15.0)			//define by the MVS of BaslerVision
	, m_fLimit_ExposureTime_Max(9.9995e+06)		//define by the MVS of BaslerVision
	, m_iFrameRate_Cnt(0)
	, m_RetryCnt(10)
{}


CMethod_BaslerGigECamCtrl::~CMethod_BaslerGigECamCtrl()
{
	GigECam_Release();
}


int CMethod_BaslerGigECamCtrl::GigeCam_Init()
{
	int nRet = 0;

	if (m_global_cfg_ParamInfo.bIsStreaming) {

		CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(5000); /* delay 5  ms */
	}	

    // Before using any pylon methods, the pylon runtime must be initialized.
    PylonInitialize();


	// Create an instant camera object with the first found camera device.
	//CBaslerUniversalInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
	m_pCam = new CBaslerUniversalInstantCamera(CTlFactory::GetInstance().CreateFirstDevice());


	// Open the camera for accessing the parameters.
	m_pCam->Open();

	// Get camera device information.
	cout << "Camera Device Information" << endl
		<< "=========================" << endl;
	cout << "Vendor           : "
		<< m_pCam->DeviceVendorName.GetValue() << endl;
	cout << "Model            : "
		<< m_pCam->DeviceModelName.GetValue() << endl;
	cout << "Firmware version : "
		<< m_pCam->DeviceFirmwareVersion.GetValue() << endl << endl;


	if (Configure_Get(m_pCam, &m_global_cfg_ParamInfo, 1)) {

		printf("Configure_Get( m_pCam, \" m_global_cfg_ParamInfo \", 1 )\n");
	}


	return nRet;
}


int CMethod_BaslerGigECamCtrl::GigECam_SetConfig(const LpGigECamConfig pParamIn)
{

	int nRet = 0;

	if (m_pCam == nullptr) {
		cout << " > Erroe!!! m_pCam <= 0 " << endl;
		return -1;
	}

	if (!m_global_cfg_ParamInfo.bIsStreaming) {

		if (!nRet) nRet = Configure_Binning(m_pCam, pParamIn);

		// # ==> The GigE camera of Basler don't supply Decimation mode.
		// # if (!nRet) nRet = Configure_Decimation(m_pCam, pParamIn);			

		//Update camera config to global parameter.
		if (!nRet) nRet = Configure_Get(m_pCam, &m_global_cfg_ParamInfo);
			
		// Image format information
		if (!nRet) nRet = Configure_ImageFormat(m_pCam, pParamIn);

		//Enable Frame rete and setting.
		if (!nRet) nRet = Configure_FrameRate(m_pCam, pParamIn);

	}
	else {

		std::cout << "This is streaming mode; settings cannot be configured..\n";
		std::cout << "This is streaming mode; settings cannot be configured..\n";
	}


	// Exposure mode and time setting
	if (!nRet) nRet = Configure_Exposure(m_pCam, pParamIn);


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


	////// # 設置Command型節點-發送參數保存命令-參數會保存到相機內部ROM裡面
	////// # 當修改參數完成後，可以調用該節點進行參數保存
	////// # 執行成功後，掉電不會消失
	//cout << "UserSetSelector" << endl;
	if (!m_global_cfg_ParamInfo.bIsStreaming) {
		m_pCam->UserSetSelector.SetValue(UserSetSelector_UserSet1);
		m_pCam->UserSetSave.Execute();
	}

	return nRet;
}



int CMethod_BaslerGigECamCtrl::GigECam_GetConfig(LpGigECamConfig pParamOut)
{
	int nRet = 0;

	if (m_pCam == nullptr) {
		cout << " > Erroe!!! m_pCam <= 0 " << endl;
		return -1;
	}

	if (m_global_cfg_ParamInfo.bIsStreaming) {

		CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

		AcquireStreaming_StartorStop(false);
		usleep(50000);
	}

	// Configure information
	nRet = Configure_Get(m_pCam, pParamOut);

	if (nRet < 0)
	{
		return nRet;
	}

	return nRet;
}



int CMethod_BaslerGigECamCtrl::GigECam_AcquireImages(string strFilePath)
{
	int nRet = 0;

	if (m_pCam == nullptr) {
		cout << " > Erroe!!! m_pCam <= 0 " << endl;
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



int CMethod_BaslerGigECamCtrl::GigECam_AcquireImages(cv::Mat& matImg)
{
	int nRet = 0;

	if (m_pCam == nullptr) {
		cout << " > Erroe!!! m_pCam <= 0 " << endl;
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



int CMethod_BaslerGigECamCtrl::GigECam_Release()
{
	int nRet = 0;

	if (m_pCam) {

		cout << "GigECam_Release() === === >>>" << endl;

		printf("\tRelease before, m_pCam Handle Address = 0x % llx\n", m_pCam);

		CAMD("GigECam_Release() === === >>>\n");
		if (m_global_cfg_ParamInfo.bIsStreaming) {

			CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

			AcquireStreaming_StartorStop(false);
			usleep(50000);
		}

		m_pCam->Close();

		// Releases all pylon resources.
		PylonTerminate();

		sleep(1);

		printf("\tRelease after, m_pCam Handle Address = 0x % llx\n", m_pCam);

		cout << "GigECam_Release() <<< === ===" << endl;

	}

	return nRet;
}


int CMethod_BaslerGigECamCtrl::GigECam_Strm_Prepare()
{
	int ret = 0;

	AcquireStreaming_Prepare();

	return ret;
}


int CMethod_BaslerGigECamCtrl::GigECam_Strm_Start()
{
	int ret = 0;

	AcquireStreaming_StartorStop(true);
	usleep(50000);

	return ret;
}


int CMethod_BaslerGigECamCtrl::GigECam_Strm_Stop()
{
	int ret = 0;

	AcquireStreaming_StartorStop(false);
	usleep(50000);

	return ret;
}


int CMethod_BaslerGigECamCtrl::GigECam_Strm_Close()
{
	int ret = 0;

	AcquireStreaming_Close();

	return ret;
}


int CMethod_BaslerGigECamCtrl::PrintDeviceInfo(CBaslerUniversalInstantCamera* pCam)
{
	int nRet = 0;

	/* Connect to the first available camera */
	CBaslerUniversalInstantCamera* camera = pCam;

	if (camera != nullptr) {

		seGigECamConfig pParamOut;

		Configure_Get(camera, &pParamOut, 1);

	}
	else {
		/* En error happened, display the correspdonding message */
		printf("error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}



int CMethod_BaslerGigECamCtrl::Configure_ImageFormat(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	CBaslerUniversalInstantCamera* camera = pCam;

	if (camera != nullptr) {

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


		// # Maximize the grabbed image area of interest (Image AOI).
		camera->OffsetX.TrySetToMinimum();
		camera->OffsetY.TrySetToMinimum();
		camera->Width.SetToMaximum();
		camera->Height.SetToMaximum();


		//寬高設置時需考慮步進(4)，即設置寬高需4的倍數
		//執行數據設定，順序 width -> offset_x ->height -> offset_y


		/// # ////////////////////////////////////////////
		// 1. >> config : iOffset_X, iWidth --. -->
		iDiff_X = abs((int)(pParamIn->iWidth + pParamIn->iOffset_X));
		printf(" Configure parameters is iOffset_X = %d, iWidth = %d\n", pParamIn->iOffset_X, pParamIn->iWidth);

		if (iDiff_X > iLimit_Max_Width) {

			printf(" Out Of Range(Max Width = %d) : iOffset_X + iWidth = %d\n", iLimit_Max_Width, iDiff_X);

			iWidth = iLimit_Max_Width;
			iOffsetX = 0;
		}
		else {

			iWidth = calcMultiplesofFour(pParamIn->iWidth);
			iOffsetX = calcMultiplesofFour(pParamIn->iOffset_X);

		}

		iRange_Max_Width = iWidth;
		iRange_Max_Offset_X = iOffsetX;	// (iLimit_Max_Width - iWidth);

		iDiff_X = abs(iRange_Max_Width + iRange_Max_Offset_X);
		if (iDiff_X > iLimit_Max_Width) {

			printf(" Out Of Range(Max Width = %d) : Range_Offset_X + Range_Width = %d\n", iLimit_Max_Width, iDiff_X);

			iRange_Max_Width = iLimit_Max_Width;
			iRange_Max_Offset_X = 0;
		}


		if (!nRet) {

			// # Maximize the grabbed image area of interest (Image AOI).
			//camera->OffsetX.SetValue(0);
			//camera->Width.SetValue(iLimit_Max_Width);

			printf(" Range parameters : iWidth = %d\n", iRange_Max_Width);

			camera->Width.SetValue(iRange_Max_Width);

			printf(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (!nRet) {

			printf(" Range parameters : iOffsetX = %d\n", iRange_Max_Offset_X);

			camera->OffsetX.SetValue(iRange_Max_Offset_X);

			printf(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}


		/// # ////////////////////////////////////////////
		// 2. >> Config : OffsetY, Height --. -->
		iDiff_Y = abs((int)(pParamIn->iHeight + pParamIn->iOffset_Y));
		printf(" Configure parameters is iOffset_Y = %d, iHeight = %d\n", pParamIn->iOffset_Y, pParamIn->iHeight);

		if (iDiff_Y > iLimit_Max_Height) {

			printf(" Out Of Range(Max Height = %d) : iOffset_Y + iHeight = %d\n", iLimit_Max_Height, iDiff_Y);

			iHeight = iLimit_Max_Height;
			iOffsetY = 0;
		}
		else {

			iHeight = calcMultiplesofTwo(pParamIn->iHeight);
			iOffsetY = calcMultiplesofTwo(pParamIn->iOffset_Y);
		}

		iRange_Max_Height = iHeight;
		iRange_Max_Offset_Y = iOffsetY;	// (iLimit_Max_Height - iHeight);

		iDiff_Y = abs((int)(iRange_Max_Height + iRange_Max_Offset_Y));
		printf(" Configure parameters is iOffset_Y = %d, iHeight = %d\n", pParamIn->iOffset_Y, pParamIn->iHeight);

		if (iDiff_Y > iLimit_Max_Height) {

			printf(" Out Of Range(Max Height = %d) : Range_Offset_Y + Range_Height = %d\n", iLimit_Max_Height, iDiff_Y);

			iRange_Max_Height = iLimit_Max_Height;
			iRange_Max_Offset_Y = 0;
		}

		if (!nRet) {

			// # Maximize the grabbed image area of interest (Image AOI).
			//camera->OffsetY.SetValue(0);
			//camera->Height.SetValue(iLimit_Max_Height);

			printf(" Range parameters : iHeight = %d\n", iRange_Max_Height);

			camera->Height.SetValue(iRange_Max_Height);

			printf(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		if (!nRet) {

			printf(" Range parameters : iOffsetY = %d\n", iRange_Max_Offset_Y);

			camera->OffsetY.SetValue(iRange_Max_Offset_Y);

			printf(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

		// config : PixelFormat  --. -->
		if (!nRet) {

			printf(" pixel_format = %d\n", pParamIn->bPixelFormat);

			if (pParamIn->bPixelFormat == 0) {  //0:PixelFormat_Mono8, 1:PixelFormat_BayerGR8
				camera->PixelFormat.SetValue(PixelFormat_Mono8);
			}
			else {
				camera->PixelFormat.SetValue(PixelFormat_RGB8Packed);
			}

			printf(" < Done~~~\n");

			usleep(500); /* delay 5  ms */
		}

	}
	else {
		/* En error happened, display the correspdonding message */
		printf("error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return result;
}


int CMethod_BaslerGigECamCtrl::Configure_Binning(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	/* Connect to the first available camera */
	CBaslerUniversalInstantCamera* camera = pCam;

	if (camera != nullptr) {

		int iScale = (pParamIn->iBinning_Scale <= 0) ? 1 : pParamIn->iBinning_Scale;

		if (!nRet) {

			camera->BinningHorizontalMode.SetValue(BinningHorizontalMode_Average);

			camera->BinningHorizontal.SetValue(iScale);

			usleep(500); /* delay 5  ms */
		}

		if (!nRet) {

			camera->BinningVerticalMode.SetValue(BinningVerticalMode_Average);

			camera->BinningVertical.SetValue(iScale);

			usleep(500); /* delay 5  ms */

		}

	}
	else {
		/* En error happened, display the correspdonding message */
		printf("error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return result;
}



int CMethod_BaslerGigECamCtrl::Configure_Decimation(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn)
{
	int result = 0;
	int nRet = 0;

	///* Connect to the first available camera */
	//CBaslerUniversalInstantCamera* camera = pCam;

	//if (camera != nullptr) {


	//	////config : BinningSelector, BinningVertical, BinningVertical -- . -->
	//	//if (!nRet) {

	//	//	printf(" Configure parameters : BinningSelector = %d\n", 0);

	//	//	nRet = MV_CC_SetEnumValue(m_pCam, "BinningSelector", 0);

	//	//	printf(" < Done~~~\n");

	//	//	usleep(500); /* delay 5  ms */
	//	//}

	//	int iScale = pParamIn->iBinning_Scale;

	//	if(iScale > 2) {
	//		iScale = 2; 
	//	}
	//	else if (iScale < 0) {
	//		iScale = 1;
	//	}

	//	if (!nRet) {

	//		printf(" Configure parameters : DecimationHorizontal = %d\n", iScale);

	//		nRet = MV_CC_SetEnumValue(m_pCam, "DecimationHorizontal", iScale);

	//		printf(" < Done~~~\n");

	//		usleep(500); /* delay 5  ms */
	//	}

	//	if (!nRet) {

	//		printf(" Configure parameters : DecimationVertical = %d\n", iScale);

	//		nRet = MV_CC_SetEnumValue(m_pCam, "DecimationVertical", iScale);

	//		printf(" < Done~~~\n");

	//		usleep(500); /* delay 5  ms */
	//	}

	//	if (MV_OK != nRet) {

	//		/* En error happened, display the correspdonding message */
	//		printf("Error: [%x]\n", nRet);
	//		return -1;
	//	}

	//}
	//else {
	//	/* En error happened, display the correspdonding message */
	//	printf("error: OpenDevice fail [%x]\n", nRet);
	//	return -1;
	//}

	return result;
}



/*
	設置自動曝光
	0 : Off
	1 : Once
	2 ：Continuous
*/
int CMethod_BaslerGigECamCtrl::Configure_Exposure(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;
	int nExposureMode = -1;
	double dbExposureTime_us = 0.0;

	/* Connect to the first available camera */
	CBaslerUniversalInstantCamera* camera = pCam;

	if (camera != nullptr) {


		m_fLimit_ExposureTime_Min = camera->AutoExposureTimeAbsLowerLimit.GetMin();
		m_fLimit_ExposureTime_Max = camera->AutoExposureTimeAbsUpperLimit.GetMax();
		cout << " > m_fLimit_ExposureTime_Min = " << m_fLimit_ExposureTime_Min << endl;
		cout << " > m_fLimit_ExposureTime_Max = " << m_fLimit_ExposureTime_Max << endl;


		if (pParamIn->bExposureAuto == static_cast<int>(emExposureAuto::Timed_Auto)) { //0:Auto, 1:Timed

			nExposureMode = 2;	//--> Continuous.
			dbExposureTime_us = 0;
		}
		else {

			nExposureMode = 0;	//--> Off
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
			printf(" Enable Timed ExposureMode ==> ExposureMode_Timed \n");
			camera->ExposureMode.SetValue(ExposureMode_Timed);
		}

		if (!nRet) {
			printf(" Exposure Auto ==>\n");
			printf(" > nExposureAuto = %d\n", nExposureMode);
			if (nExposureMode) {
				camera->ExposureAuto.SetValue(ExposureAuto_Continuous);
			}
			else {
				camera->ExposureAuto.SetValue(ExposureAuto_Off);
			}
		}

		if (!nRet && !nExposureMode) {
			
			printf(" Exposure Time ==>\n");

			//cout << "Initial exposure time = ";
			//cout << camera->ExposureTimeAbs.GetValue() << " us" << endl;

			//// Set the exposure time ranges for luminance control.
			//cout << "Set the exposure time ranges for luminance control";
			//camera->AutoExposureTimeAbsLowerLimit.SetToMinimum();
			//camera->AutoExposureTimeAbsUpperLimit.SetToMaximum();


			printf(" > dbExposureTime_us = %5.2f\n", dbExposureTime_us);
			cout << " > ExposureTime.SetValue( " << dbExposureTime_us << " us )\"" << endl;
			camera->ExposureTimeAbs.SetValue(dbExposureTime_us);
		}

		/* 設置自動增益
		0 : Off
		1 : Once
		2 ：Continuous  */
		if (!m_global_cfg_ParamInfo.bIsStreaming) {

			if (!nRet) {
				printf(" Gain auto ==>\n");
				camera->GainAuto.SetValue(GainAuto_Continuous);
			}
		}


	}
	else {
		/* En error happened, display the correspdonding message */
		printf("error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}


int CMethod_BaslerGigECamCtrl::Configure_FrameRate(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	/* Connect to the first available camera */
	CBaslerUniversalInstantCamera* camera = pCam;

	if (camera != nullptr) {

		// config : AcquisitionFrameRateEnable  --. -->
		if (!nRet) {

			bool nEnbAcqFrameRateEnable = true;

			printf(" AcquisitionFrameRateEnable = %d\n", nEnbAcqFrameRateEnable);

			camera->AcquisitionFrameRateEnable.SetValue(nEnbAcqFrameRateEnable);

			printf(" < Done~~~\n");

			usleep(500); /* delay 5  ms */

		}

		if (!nRet) {

			double dbAcqFrameRate = pParamIn->dbAcquisitionFrameRate;

			printf(" dbAcqFrameRate = %5.3f\n", dbAcqFrameRate);

			camera->AcquisitionFrameRateAbs.SetValue(dbAcqFrameRate);

			printf(" < Done~~~\n");

			usleep(500); /* delay 5  ms */

		}


	}


	return nRet;
}



int CMethod_BaslerGigECamCtrl::Configure_PersistentIP(CBaslerUniversalInstantCamera* pCam, const LpGigECamConfig pParamIn)
{
	int nRet = 0;

	/* Connect to the first available camera */
	CBaslerUniversalInstantCamera* camera = pCam;

	if (camera != nullptr) {

		//struct in_addr netAddr;

		const char* pIP = pParamIn->strPersistentIP.c_str();
		const char* pMask = "255.255.255.0";
		const char* pGateway = "0.0.0.0";

	}
	else {
		/* En error happened, display the correspdonding message */
		printf("error: OpenDevice fail [%x]\n", nRet);
		return -1;
	}

	return nRet;
}


int CMethod_BaslerGigECamCtrl::Configure_ResetConfigureCustomImageSettings(CBaslerUniversalInstantCamera* pCam)
{
	int result = 0;
	// TBD
	return result;
}


int CMethod_BaslerGigECamCtrl::Configure_ResetExposure(CBaslerUniversalInstantCamera* pCam)
{
	int result = 0;
	//TBD
	return result;
}


int CMethod_BaslerGigECamCtrl::Configure_Get(CBaslerUniversalInstantCamera* pCam, LpGigECamConfig pParamOut, bool bDumpInf)
{

	if (nullptr == pParamOut)
		return -1;

	int nRet = 0;

	/* Connect to the first available camera */
	CBaslerUniversalInstantCamera* camera = pCam;

	LpGigECamConfig pParam = pParamOut;

	if (camera != nullptr) {

		int cfg_Max_Width = { 0 }, cfg_Max_Height = { 0 };

		int cfg_OffsetX = { 0 }, cfg_OffsetY = { 0 };
		int cfg_Width = { 0 }, cfg_Height = { 0 };

		PixelFormatEnums cfg_PixelFormat;
		ExposureAutoEnums cfg_Exposure_modeto;

		int cfg_Binning_ScaleVal = { 0 };
		bool cfg_bIsEnbAcquisitionFrameRate = { 0 };
		double cfg_dbAcquisitionFrameRate = { 0 };


		cfg_Max_Width = camera->WidthMax.GetValue();
		usleep(500);
		cfg_Max_Height = camera->HeightMax.GetValue();
		usleep(500);
		cfg_Width = camera->Width.GetValue();
		usleep(500);
		cfg_Height = camera->Height.GetValue();
		usleep(500);
		cfg_OffsetX = camera->OffsetX.GetValue();
		usleep(500);
		cfg_OffsetY = camera->OffsetY.GetValue();
		usleep(500);

		// Remember the current pixel format.
		cfg_PixelFormat = camera->PixelFormat.GetValue();
		usleep(500);
		cfg_Exposure_modeto = camera->ExposureAuto.GetValue();
		usleep(500);

		cfg_Binning_ScaleVal = camera->BinningHorizontal.GetValue();
		usleep(500);

		cfg_bIsEnbAcquisitionFrameRate = camera->AcquisitionFrameRateEnable.GetValue();
		usleep(500);
		if (cfg_bIsEnbAcquisitionFrameRate) {

			//cfg_dbAcquisitionFrameRate = camera->AcquisitionFrameRate.GetValue();
			cfg_dbAcquisitionFrameRate = camera->AcquisitionFrameRateAbs.GetValue();

			usleep(500);
		}

		pParam->bIsConnected = true;

		pParam->iSensor_Width = cfg_Max_Width;
		pParam->iSensor_Height = cfg_Max_Height;

		pParam->iOffset_X = cfg_OffsetX;
		pParam->iOffset_Y = cfg_OffsetY;

		pParam->iWidth = cfg_Width;
		pParam->iHeight = cfg_Height;

		pParam->bPixelFormat = (cfg_PixelFormat == PixelFormat_Mono8) ? 0 : 1;  //0:PixelFormat_Mono8, 1:PixelFormat_BayerGR8
		pParam->bExposureAuto = (cfg_Exposure_modeto == ExposureAuto_Off) ? 1 : 0;     //0:Auto, 1:Timed_(Off);

		pParam->iBinning_Scale = cfg_Binning_ScaleVal;

		pParam->bIsEnbAcquisitionFrameRate = cfg_bIsEnbAcquisitionFrameRate;
		pParam->dbAcquisitionFrameRate = cfg_dbAcquisitionFrameRate;



		if (bDumpInf) {

			printf("\n\n>>> Device information === >>> === >>>\n");
			printf("\t	@ Cfg.bIsConnected---> %d\n", pParam->bIsConnected);
			printf("\t	@ Cfg.bIsEnbAcquisitionFrameRate---> %s\n", (pParam->bIsEnbAcquisitionFrameRate) ? "True" : "False");
			printf("\t	@ Cfg.dbAcquisitionFrameRate---> %5.3f\n", pParam->dbAcquisitionFrameRate);
			printf("\t	@ Cfg.bExposureAuto---> %s\n", (!pParam->bExposureAuto) ? "Auto" : "Off");
			//printf("\t	@ Cfg.ExposureTime(us)---> %5.3f\n", camera->ExposureTimeAbs.GetValue());
			printf("\t	@ Cfg.bPixelFormat---> %s\n", (pParam->bPixelFormat) ? "RGB8" : "Mono8");
			printf("\t	@ Cfg.iOffset_X ---> %d\n", pParam->iOffset_X);
			printf("\t	@ Cfg.iOffset_Y---> %d\n", pParam->iOffset_Y);
			printf("\t	@ Cfg.iWidth ---> %d\n", pParam->iWidth);
			printf("\t	@ Cfg.iHeight---> %d\n", pParam->iHeight);
			printf("\t	@ Cfg.iMax_Width ---> %d\n", pParam->iSensor_Width);
			printf("\t	@ Cfg.iMax_Height---> %d\n", pParam->iSensor_Height);
			printf("\t	@ Cfg.iBinning_Scale---> %d\n", pParam->iBinning_Scale);
			printf("<<< Device information === <<< === <<< === ===\n\n");

		}

	}
	else {

		*pParam = seGigECamConfig();

		return -1;

	}

	return nRet;

}



//int CMethod_BaslerGigECamCtrl::RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
//{
//	if (nullptr == pRgbData)
//	{
//		return MV_E_PARAMETER;
//	}
//
//	for (unsigned int j = 0; j < nHeight; j++)
//	{
//		for (unsigned int i = 0; i < nWidth; i++)
//		{
//			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
//			pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
//			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
//		}
//	}
//
//	return MV_OK;
//}
//
//
//
//int CMethod_BaslerGigECamCtrl::Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData, cv::Mat& mat_img)
//{
//	cv::Mat srcImage;
//	if (pstImageInfo->enPixelType == PixelType_Gvsp_Mono8)
//	{
//		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
//	}
//	else if (pstImageInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
//	{
//		RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
//		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
//	}
//	else
//	{
//		printf("unsupported pixel format\n");
//		return false;
//	}
//
//	if (nullptr == srcImage.data)
//	{
//		return false;
//	}
//
//	//    //save converted image in a local file
//	//    std::string filename = GetTimeAsFileName();
//	//    try {
//	//#if defined (VC9_COMPILE)
//	//        cvSaveImage(filename, &(IplImage(srcImage)));
//	//#else
//	//        //cv::imwrite(filename, srcImage);
//	//#endif
//	//    }
//	//    catch (cv::Exception& ex) {
//	//        fprintf(stderr, "Exception saving image to bmp format: %s\n", ex.what());
//	//    }
//	mat_img = srcImage.clone();
//	srcImage.release();
//
//	return true;
//}



int CMethod_BaslerGigECamCtrl::AcquireImages(CBaslerUniversalInstantCamera* pCam, string strFilePath)
{
	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;
	int nRet = 0;

	/* Connect to the first available camera */
	CBaslerUniversalInstantCamera* camera = pCam;

	try {

		cv::Mat mat_img;

		// Number of images to be grabbed.
		static const uint32_t c_countOfImagesToGrab = 1;

		// The parameter MaxNumBuffer can be used to control the count of buffers
		// allocated for grabbing. The default value of this parameter is 10.
		camera->MaxNumBuffer = 5;

		// Start the grabbing of c_countOfImagesToGrab images.
		// The camera device is parameterized with a default configuration which
		// sets up free-running continuous acquisition.
		camera->StartGrabbing(c_countOfImagesToGrab);

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;
		CImageFormatConverter formatConverter;
		CPylonImage pylonImage;

		// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
		// when c_countOfImagesToGrab images have been retrieved.
		while (camera->IsGrabbing())
		{
			// Wait for an image and then retrieve it. A timeout of 1000 ms is used.
			camera->RetrieveResult(1000, ptrGrabResult, TimeoutHandling_ThrowException);

			// Image grabbed successfully?
			if (ptrGrabResult->GrabSucceeded())
			{
				// Access the image data.
				int nWidth(0), nHeight(0);
				EPixelType emPixeType;

				nWidth = ptrGrabResult->GetWidth();
				nHeight = ptrGrabResult->GetHeight();
				emPixeType = ptrGrabResult->GetPixelType();

				const uint8_t* pImageBuffer = (uint8_t*)ptrGrabResult->GetBuffer();

				cout << "SizeX: " << nWidth << endl;
				cout << "SizeY: " << nHeight << endl;
				cout << "PixelType: " << emPixeType << endl;
				cout << "Gray value of first pixel: " << (uint32_t)pImageBuffer[0] << endl << endl;


				if (emPixeType == PixelType_Mono8)
				{
					formatConverter.OutputPixelFormat = PixelType_Mono8;
					formatConverter.Convert(pylonImage, ptrGrabResult);
					mat_img = cv::Mat(nHeight, nWidth, CV_8UC1, (uint8_t*)pylonImage.GetBuffer());
				}
				else if (emPixeType == PixelType_RGB8packed)
				{
					formatConverter.OutputPixelFormat = PixelType_RGB8packed;
					formatConverter.Convert(pylonImage, ptrGrabResult);
					mat_img = cv::Mat(nHeight, nWidth, CV_8UC3, (uint8_t*)pylonImage.GetBuffer());
				}
				else
				{
					printf("unsupported pixel format\n");
				}

				if (mat_img.empty()) {

					printf("The matrix is empty()!!!\n");
				}

				try {

					cv::imwrite(strFilePath, mat_img);

					mat_img.release();

					/* Display some informations about the retrieved buffer */
					printf("Acquired file path : %s\n", strFilePath.c_str());
				}
				catch (cv::Exception& ex) {
					fprintf(stderr, "Exception saving image to bmp format: %s\n", ex.what());
				}

			}
			else
			{
				cout << "Error: " << std::hex << ptrGrabResult->GetErrorCode() << std::dec << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}
	}
	catch (const GenericException& e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl << e.GetDescription() << endl;
		nRet = -1;
	}


	return nRet;

}


int CMethod_BaslerGigECamCtrl::AcquireImages(CBaslerUniversalInstantCamera* pCam, cv::Mat& matImg)
{
	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;
	int nRet = 0;

	/* Connect to the first available camera */
	CBaslerUniversalInstantCamera* camera = pCam;

	try {

		cv::Mat mat_img;

		// Number of images to be grabbed.
		static const uint32_t c_countOfImagesToGrab = 1;

		// The parameter MaxNumBuffer can be used to control the count of buffers
		// allocated for grabbing. The default value of this parameter is 10.
		camera->MaxNumBuffer = 5;

		// Start the grabbing of c_countOfImagesToGrab images.
		// The camera device is parameterized with a default configuration which
		// sets up free-running continuous acquisition.
		camera->StartGrabbing(c_countOfImagesToGrab);

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;
		CImageFormatConverter formatConverter;
		CPylonImage pylonImage;

		// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
		// when c_countOfImagesToGrab images have been retrieved.
		while (camera->IsGrabbing())
		{
			// Wait for an image and then retrieve it. A timeout of 1000 ms is used.
			camera->RetrieveResult(1000, ptrGrabResult, TimeoutHandling_ThrowException);

			// Image grabbed successfully?
			if (ptrGrabResult->GrabSucceeded())
			{
				// Access the image data.
				int nWidth(0), nHeight(0);
				EPixelType emPixeType;

				nWidth = ptrGrabResult->GetWidth();
				nHeight = ptrGrabResult->GetHeight();
				emPixeType = ptrGrabResult->GetPixelType();

				const uint8_t* pImageBuffer = (uint8_t*)ptrGrabResult->GetBuffer();

				cout << "SizeX: " << nWidth << endl;
				cout << "SizeY: " << nHeight << endl;
				cout << "PixelType: " << emPixeType << endl;
				cout << "Gray value of first pixel: " << (uint32_t)pImageBuffer[0] << endl << endl;


				if (emPixeType == PixelType_Mono8)
				{
					formatConverter.OutputPixelFormat = PixelType_Mono8;
					formatConverter.Convert(pylonImage, ptrGrabResult);
					mat_img = cv::Mat(nHeight, nWidth, CV_8UC1, (uint8_t*)pylonImage.GetBuffer());
				}
				else if (emPixeType == PixelType_RGB8packed)
				{
					formatConverter.OutputPixelFormat = PixelType_RGB8packed;
					formatConverter.Convert(pylonImage, ptrGrabResult);
					mat_img = cv::Mat(nHeight, nWidth, CV_8UC3, (uint8_t*)pylonImage.GetBuffer());
				}
				else
				{
					printf("unsupported pixel format\n");
				}

				if (!mat_img.empty()) {

					mat_img.copyTo(matImg);

					mat_img.release();
				}

			}
			else
			{
				cout << "Error: " << std::hex << ptrGrabResult->GetErrorCode() << std::dec << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}
	}
	catch (const GenericException& e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl << e.GetDescription() << endl;
		nRet = -1;
	}


	return nRet;

}

int CMethod_BaslerGigECamCtrl::AcquireStreaming(CBaslerUniversalInstantCamera* pCam, cv::Mat& matImg)
{
	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;
	int nRet = 0;

	cv::Mat mat_img;

	// Number of images to be grabbed.
	static const uint32_t c_countOfImagesToGrab = 1;

	// This smart pointer will receive the grab result data.
	CGrabResultPtr ptrGrabResult;
	CImageFormatConverter formatConverter;
	CPylonImage pylonImage;

	//try 
	{


		//cycletime_start
		start = clock();
		
		iCnt_1++;


		//for (uint32_t i = 0; i < c_countOfImagesToGrab && pCam->IsGrabbing(); ++i)
		for (uint32_t i = 0; i < c_countOfImagesToGrab; ++i)
		{
			// Wait for an image and then retrieve it. A timeout of 1000 ms is used.
			//cout << " > pCam->RetrieveResult ... \n";
			pCam->RetrieveResult(1000, ptrGrabResult, TimeoutHandling_ThrowException);

			//cout << " > ptrGrabResult->GrabSucceeded() ... \n";
			// Image grabbed successfully?
			if (ptrGrabResult->GrabSucceeded())
			{
				// Access the image data.
				int nWidth(0), nHeight(0);
				EPixelType emPixeType;

				nWidth = ptrGrabResult->GetWidth();
				nHeight = ptrGrabResult->GetHeight();
				emPixeType = ptrGrabResult->GetPixelType();

				const uint8_t* pImageBuffer = (uint8_t*)ptrGrabResult->GetBuffer();

				//cout << "  > SizeX: " << nWidth << endl;
				//cout << "  > SizeY: " << nHeight << endl;
				//cout << "  > PixelType: " << emPixeType << endl;
				//cout << "  > Gray value of first pixel: " << (uint32_t)pImageBuffer[0] << endl << endl;

				//cout << " > OpenCV() converter ... \n";
				//cout << " > emPixeType : " << emPixeType << endl;

				if (emPixeType == PixelType_Mono8)
				{
					//cout << "   > 01 formatConverter.OutputPixelFormat \n";
					formatConverter.OutputPixelFormat = PixelType_Mono8;

					//cout << "   > 02 formatConverter.Convert \n";
					formatConverter.Convert(pylonImage, ptrGrabResult);

					//cout << "   > 03 mat_img = cv::Mat(nHeight, nWidth, CV_8UC1, (uint8_t*)pylonImage.GetBuffer()) \n";
					mat_img = cv::Mat(nHeight, nWidth, CV_8UC1, (uint8_t*)pylonImage.GetBuffer());

				}
				else if (emPixeType == PixelType_RGB8packed)
				{
					formatConverter.OutputPixelFormat = PixelType_RGB8packed;
					formatConverter.Convert(pylonImage, ptrGrabResult);
					mat_img = cv::Mat(nHeight, nWidth, CV_8UC3, (uint8_t*)pylonImage.GetBuffer());
				}
				else
				{
					printf("unsupported pixel format\n");
				}

				if (!mat_img.empty()) {

					//cout << " > mat_img.copyTo(matImg) ... \n";

#ifdef ALGO_Enable_StreamingBufOpt_SpeedOptimization_DEBUG

					cv::resize(mat_img, matImg, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);
#else

					mat_img.copyTo(matImg);
#endif				

					mat_img.release();
				}

				////cycletime_end
				//end = clock();
				//elapsed = double(end - start) / CLOCKS_PER_SEC;
				//strtmp = std::to_string(elapsed);
				//cout << "1.[ " << iCnt_1 << " ]. --.--> HikVision_AcquireImages() Cycle time : " << strtmp.c_str() << " (seconds)" << endl;


			}
			else
			{
				cout << " > incompleteErr_timestamp : " << getCurrentTime() << endl;
				cout << "Error: " << std::hex << ptrGrabResult->GetErrorCode() << std::dec << " " << ptrGrabResult->GetErrorDescription() << endl;
			}


		}




	}
	//catch (const GenericException& e)
	//{
	//	// Error handling.
	//	cerr << "An exception occurred." << endl << e.GetDescription() << endl;
	//	nRet = -1;
	//}


	return nRet;

}



int CMethod_BaslerGigECamCtrl::AcquireStreaming_Prepare()
{
	CBaslerUniversalInstantCamera* camera = m_pCam;

	cout << "AcquireStreaming_Prepare(...) >>>\n";
	cout << "tid = " << tid << endl;
	cout << "tid = " << tid << endl;

	if (camera != nullptr) {

		if (bIsCreated) {

			cout << "thread already creatr()..." << endl;
			return 0;
		}

		seGigECamConfig pParam = seGigECamConfig();

		std::thread t1(&CMethod_BaslerGigECamCtrl::Thread_Acquire, this, camera, &pParam);

		tid = t1.get_id();
		t1.detach();

		bIsCreated = true;


		//cout << "   pCam =" << camera << endl;
		cout << "   tid =" << tid << endl;

	}

	cout << "AcquireStreaming_Prepare(...) <<<\n";

	return 0;
}


int CMethod_BaslerGigECamCtrl::AcquireStreaming_StartorStop(bool bflgEnableThd)
{
	cout << "AcquireStreaming_StartorStop(...) >>>\n";
	cout << "bflgEnableThd = "  << bflgEnableThd << endl;

	std::unique_lock<std::mutex> lock(u_mutex);

	if (bflgEnableThd) {

		cout << "AcquireStreaming_StartorStop(...) ==> START \n";
		notified_IsRun = true;

		std::cout << "notified_IsDone = " << notified_IsDone << endl;
		std::cout << "notified_IsRun = " << notified_IsRun << endl;

	}
	else {
		usleep(50000);

		cout << "AcquireStreaming_StartorStop(...) ==> STOP \n";
		notified_IsRun = false;

		std::cout << "notified_IsDone = " << notified_IsDone << endl;
		std::cout << "notified_IsRun = " << notified_IsRun << endl;

	}	

	m_global_cfg_ParamInfo.bIsStreaming = notified_IsRun;

	m_iFrameRate_Cnt = 0;

	cout << " cond_var.notify_one()\n";
	cond_var.notify_one();

	cout << "AcquireStreaming_StartorStop(...) <<<\n";


	return 0;
}


int CMethod_BaslerGigECamCtrl::Thread_Acquire(CBaslerUniversalInstantCamera* pCam, LpGigECamConfig pParamOut)
{

	cout << " > @ Thread_Acquire(...) >>>\n";
	//cout << "   pCam =" << pCam << endl;

	int ret = 0;
	string strInfo;
	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;
	bool bIsAlreadyStart = 0;

	while (!notified_IsDone)
	{


		//cycletime_start
		start = clock();


		std::unique_lock<std::mutex> lock(u_mutex);
		while (!notified_IsRun)
		{
			// # End acquisition
			if (bIsAlreadyStart) {

				bIsAlreadyStart = 0;

				iCnt_0 = 0;
				iCnt_1 = 0;

				usleep(50000);
			}

			std::cout << " > worker_thread() wait.start\n";

			//Stop the grabbing.
			pCam->StopGrabbing();

			std::cout << " > worker_thread() wait.end\n";
			cond_var.wait(lock);
		}
		lock.unlock();


		// after the wait, we own the lock.
		//std::cout << "worker_thread() is processing data\n";
		//std::cout << "notified_IsDone = " << notified_IsDone << endl;
		//std::cout << "notified_IsRun = " << notified_IsRun << endl;

		if (!bIsAlreadyStart) {

			std::cout << " > worker_thread() StartGrabbing...\n";

			//// The parameter MaxNumBuffer can be used to control the count of buffers allocated for grabbing.
			pCam->MaxNumBuffer = 5;

			//// Start the grabbing of c_countOfImagesToGrab images.
			//// The camera device is parameterized with a default configuration which sets up free-running continuous acquisition.
			pCam->StartGrabbing();

			bIsAlreadyStart = 1;
			usleep(5000);
		}

		iCnt_0++;
		cv::Mat matImg;

		ret = AcquireStreaming(pCam, matImg);

		if (matImg.empty()) {
			usleep(10);
			//cout << " > !! > matImg.empty()" << endl;
			continue;
		}

#ifdef ALGO_Enable_StreamingBufOpt_AddTimestamp_DEBUG

		//strInfo = "No." + std::to_string(m_iFrameRate_Cnt++) + "_" + getCurrentTime();
		strInfo = "No." + std::to_string(m_iFrameRate_Cnt++);

		cv::putText(matImg, strInfo, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 255, 255), 2);

#endif

		if (ret) {

			std::cout << "Error!!! AcquireStreaming() in Thread_Acquire(...)" << endl;
			AcquireStreaming_StartorStop(false);
			usleep(50000);
		}

		if (!matImg.empty()) {

			send_mat(matImg);

			matImg.release();
		}


		//// # cycletime_end
		//end = clock();
		//elapsed = double(end - start) / CLOCKS_PER_SEC;
		//strtmp = std::to_string(elapsed);
		//cout << "3.[ " << iCnt_0 << " ]. --.--> Total_AcquireImages() + send_mat(matImg)_CycleTime : " << strtmp.c_str() << " (seconds)" << endl;


		
		usleep(10);		
	}

	return 0;
}


int CMethod_BaslerGigECamCtrl::AcquireStreaming_Close()
{
	cout << "AcquireStreaming_Close(...) >>>\n";

	{
		std::lock_guard<std::mutex> lock(u_mutex);

		notified_IsDone = true;
		notified_IsRun = false;
	}

	cout << " cond_var.notify_one()\n";
	cond_var.notify_one();

	bIsCreated = false;


	m_global_cfg_ParamInfo.bIsStreaming = notified_IsRun;


	cout << "AcquireStreaming_Close(...) <<<\n";

	return 0;
}



static int CMethod_BaslerGigECamCtrl::GigECam_DebugPrint()
{
	int ret = EXIT_SUCCESS;

	cout << "GigECam_DebugPrint() --> TDB" << endl;

	return ret;
} 