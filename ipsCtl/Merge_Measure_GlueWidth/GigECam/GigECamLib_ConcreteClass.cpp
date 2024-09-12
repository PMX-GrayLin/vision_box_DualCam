
#include "GigECamLib_ConcreteClass.h"
#define CVUI_IMPLEMENTATION
#include "../cvip.h"


using namespace std;
using namespace cv;


GigECam_C_Library::GigECam_C_Library()
{
	m_seConfig = seGigECamConfig();

	Cam_Log_Level();

        phdl_cam = &cam_ctrl_uvc;
}

GigECam_C_Library::~GigECam_C_Library()
{}


int GigECam_C_Library::Init()
{
	int res = 0;

	if(phdl_cam == nullptr) {
        	fprintf(stderr, "%s()%d: Error: phdl_cam is null.\n", __FUNCTION__, __LINE__);
    	}

	res = phdl_cam->GigeCam_Init();

	return res;
}


int GigECam_C_Library::SetConfig(const LpGigECamConfig pParamIn)
{
	int res = 0;

	m_seConfig = *pParamIn;

	if (m_seConfig.iOffset_X <= 0) {
		m_seConfig.iOffset_X = seGigECamConfig().iOffset_X;
	}

	if (m_seConfig.iOffset_Y <= 0) {
		m_seConfig.iOffset_Y = seGigECamConfig().iOffset_Y;
	}

	if (m_seConfig.iWidth <= 0) {
		m_seConfig.iWidth = seGigECamConfig().iWidth;
	}
	
	if (m_seConfig.iHeight <= 0) {
		m_seConfig.iHeight = seGigECamConfig().iHeight;
	}
	
	if (m_seConfig.dbExposureTime <= 0.0) {
		m_seConfig.dbExposureTime = seGigECamConfig().dbExposureTime;
	}

	if (m_seConfig.iBinning_Scale <= 0) {
		m_seConfig.iBinning_Scale = seGigECamConfig().iBinning_Scale;
	}

	res = phdl_cam->GigECam_SetConfig(&m_seConfig);

	return res;
}


int GigECam_C_Library::GetConfig(LpGigECamConfig pParamIn)
{
	int res = 0;

	if(nullptr == pParamIn)
		return -1;

	res = phdl_cam->GigECam_GetConfig(pParamIn);

	return res;
}



int GigECam_C_Library::AcquireImages(string strFilePath)
{
	int res = 0;

	if (strFilePath.empty())
		return -1;
	
	string strFileName;

#ifdef ALGO_Enable_ImgBufOpt_AddTimestamp_DEBUG

	strFileName = strFilePath + getCurrentTime() + ".jpg";

#else

	strFileName = strFilePath;

#endif

	cout << "strFileName ===> ===> " << strFileName.c_str() << endl;

	res = phdl_cam->GigECam_AcquireImages(strFileName);

	return res;
}



int GigECam_C_Library::AcquireImages(seImageInfo* pImgInfo, std::string strFilePath, bool bStaticImgMode)
{
	int res = 0;

	if (nullptr == pImgInfo || strFilePath.empty())
		return -1;

	cv::Mat tmpImg;

	if (bStaticImgMode) {

		tmpImg = cv::imread(strFilePath, cv::IMREAD_GRAYSCALE);
	}
	else {

		res = phdl_cam->GigECam_AcquireImages(tmpImg);

		if (!tmpImg.empty()) {

			string strFileName;

#ifdef ALGO_Enable_ImgBufOpt_AddTimestamp_DEBUG

			strFileName = strFilePath + getCurrentTime() + ".jpg";

#else

			strFileName = strFilePath;

#endif

			cout << "strFileName ===> ===> " << strFileName.c_str() << endl;

			cv::imwrite(strFileName, tmpImg);

		}
		else {
			return ER_ABORT;
		}

	}

//#endif	

	seImageInfo* pImg = pImgInfo;
	pImg->iWidth = tmpImg.cols;
	pImg->iHeight = tmpImg.rows;
	pImg->iChannels = tmpImg.channels();
	pImg->pbImgBuf = new unsigned char[pImg->iWidth * pImg->iHeight * pImg->iChannels];

	if (CCVIPItem::CvMatToUint8(tmpImg, pImg) < ER_OK) {
		return ER_ABORT;
	}

	return res;
}


int GigECam_C_Library::Release()
{
	int res = 0;

	res = phdl_cam->GigECam_Release();

	return res;
}



int GigECam_C_Library::Streaming_Prepare()
{
	int res = 0;

	res = phdl_cam->GigECam_Strm_Prepare();

	return res;
};


int GigECam_C_Library::Streaming_Start()
{
	int res = 0;

	res = phdl_cam->GigECam_Strm_Start();

	return res;
};

int GigECam_C_Library::Streaming_AcquireImages(std::string strFilePath)
{
	int res = 0;

	if (strFilePath.empty())
		return -1;

	string strFileName;

#ifdef ALGO_Enable_ImgBufOpt_AddTimestamp_DEBUG

	strFileName = strFilePath + getCurrentTime() + ".jpg";

#else

	strFileName = strFilePath;

#endif

	cout << "strFileName ===> ===> " << strFileName.c_str() << endl;

	res = phdl_cam->GigECam_Strm_AcquireImages(strFileName);


	return res;
}


int GigECam_C_Library::Streaming_AcquireImages(seImageInfo* pImgInfo, std::string strFilePath)
{
	int res = 0;

	if (nullptr == pImgInfo)	// || strFilePath.empty())
		return -1;

	string strFileName;

	if (!strFilePath.empty()) {

#ifdef ALGO_Enable_ImgBufOpt_AddTimestamp_DEBUG

		strFileName = strFilePath + getCurrentTime() + ".jpg";

#else

		strFileName = strFilePath;

#endif
	}

	cout << "strFileName ===> ===> " << strFileName.c_str() << endl;

	cv::Mat tmpImg;

	res = phdl_cam->GigECam_Strm_AcquireImages(tmpImg);

	if(!tmpImg.empty() && !res)
	{
		if (!strFilePath.empty()) {

			cv::imwrite(strFileName, tmpImg);
		}

		seImageInfo* pImg = pImgInfo;

		pImg->iWidth = tmpImg.cols;
		pImg->iHeight = tmpImg.rows;
		pImg->iChannels = tmpImg.channels();
		pImg->pbImgBuf = new unsigned char[pImg->iWidth * pImg->iHeight * pImg->iChannels];

		if (CCVIPItem::CvMatToUint8(tmpImg, pImg) < ER_OK) {
			cout << "Error !!! CCVIPItem::CvMatToUint8(tmpImg, pImg) faile." << endl;
			return ER_ABORT;
		}

		tmpImg.release();

	}
	else {
		cout << "Error !!! The cv::Mat of Image is Empty()" << endl;
	}


	return res;
}



int GigECam_C_Library::Streaming_Stop()
{
	int res = 0;

	res = phdl_cam->GigECam_Strm_Stop();

	return res;
};


int GigECam_C_Library::Streaming_Colse()
{
	int res = 0;

	res = phdl_cam->GigECam_Strm_Close();
	
	return res;
};




int GigECam_C_Library::DebugPrint()
{
	int res = 0;

	res = phdl_cam->GigECam_DebugPrint();

	return res;
}
