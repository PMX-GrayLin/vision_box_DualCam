#include <opencv2/opencv.hpp>
#include "Method_ImgProcessing.h"
#include <ctime>


using namespace std;
using namespace cv;


CMethod_ImgProcessing::CMethod_ImgProcessing()
{}

CMethod_ImgProcessing::~CMethod_ImgProcessing()
{}


int CMethod_ImgProcessing::IpThreshold_SetParameter(int iChannels, double* pThresh, double* pMaxVal, emThresholdTypes emTypes)
{
	if (iChannels <= 0) {
		return ER_ABORT;
	}
	
	IpThreshold_Clear();

	int res = 0;

	if (!m_vec_dbThresh.empty()) {
		m_vec_dbThresh.clear();
	//m_vec_dbThresh.reserve(iChannels);
	}
	if (!m_vec_dbMaxVal.empty()) {
		m_vec_dbMaxVal.clear();
	//m_vec_dbMaxVal.reserve(iChannels);
	}

	int i = 0;
	for (int i = 0; i < iChannels; i++) {

		m_vec_dbThresh.push_back( pThresh[i] );
		m_vec_dbMaxVal.push_back( pMaxVal[i] );
	}

	m_emTypes = emTypes;

	return res;
}


int CMethod_ImgProcessing::IpThreshold_TestIt(const LPImageInfo pIn)
{
	int res = 0;

	cv::Mat mat_img;
	if (Uint8ToCvMat(pIn, mat_img) < ER_OK) {
		return ER_ABORT;
	}

	CalcThreshold(mat_img, m_thresh_img, m_vec_dbThresh, m_vec_dbMaxVal, m_emTypes);

	return res;
}

int CMethod_ImgProcessing::IpThreshold_TestIt(const cv::Mat mat_img)
{
	int res = 0;

	if (mat_img.empty()) {
		return ER_ABORT;
	}

	CalcThreshold(mat_img, m_thresh_img, m_vec_dbThresh, m_vec_dbMaxVal, m_emTypes);

	return res;
}


int CMethod_ImgProcessing::IpThreshold_GetResult(LPImageInfo pOut)
{
	int res = 0;

	int iW = pOut->iWidth;
	int iH = pOut->iHeight;
	int iChannels = pOut->iChannels;

	if (pOut->pbImgBuf == nullptr ||
		iW != m_thresh_img.cols ||
		iH != m_thresh_img.rows ||
		iChannels != m_thresh_img.channels()) {

		return ER_ABORT;
	}

	if (CvMatToUint8(m_thresh_img, pOut) < ER_OK) {
		return ER_ABORT;
	}

	return res;
}

int CMethod_ImgProcessing::IpThreshold_GetResult(cv::Mat& mDest_Img)
{
	int res = 0;

	if (m_thresh_img.empty()) {
		return ER_ABORT;
	}
	m_thresh_img.copyTo(mDest_Img);

	return res;
}



int CMethod_ImgProcessing::IpHistogram_SetParameter(const LPImageInfo pIn)
{
	int res = 0;

	IpHistogram_Clear();

	cv::Mat matImg;
	if (Uint8ToCvMat(pIn, matImg) < ER_OK) {
		return ER_ABORT;
	}

	m_hist_Img = matImg.clone();

	return res;
}

int CMethod_ImgProcessing::IpHistogram_SetParameter(const LPImageInfo pIn, seAnnulus roiAnnulus)
{
	int res = 0;

	IpHistogram_Clear();

	cv::Mat matImg;
	if (Uint8ToCvMat(pIn, matImg) < ER_OK) {
		return ER_ABORT;
	}

	m_hist_Img = matImg.clone();

	m_emHist_BoxShape = emBoxShape::SHAPE_ANNULUS;
	m_hist_Annulus = roiAnnulus;

	return res;
}

int CMethod_ImgProcessing::IpHistogram_SetParameter(const cv::Mat matImg, seAnnulus roiAnnulus)
{
	int res = 0;

	IpHistogram_Clear();

	if(matImg.empty()) {
		return ER_ABORT;
	}

	m_hist_Img = matImg.clone();

	m_emHist_BoxShape = emBoxShape::SHAPE_ANNULUS;
	m_hist_Annulus = roiAnnulus;

	return res;
}


int CMethod_ImgProcessing::IpHistogram_SetParameter(const LPImageInfo pIn, seBoundingBox roiRect)
{
	int res = 0;

	IpHistogram_Clear();

	cv::Mat matImg;
	if (Uint8ToCvMat(pIn, matImg) < ER_OK) {
		return ER_ABORT;
	}

	m_hist_Img = matImg.clone();

	m_emHist_BoxShape = emBoxShape::SHAPE_RECT;
	m_hist_Rect = roiRect;

	return res;
}

int CMethod_ImgProcessing::IpHistogram_SetParameter(const cv::Mat matImg, seBoundingBox roiRect)
{
	int res = 0;

	IpHistogram_Clear();

	if (matImg.empty()) {
		return ER_ABORT;
	}

	m_hist_Img = matImg.clone();

	m_emHist_BoxShape = emBoxShape::SHAPE_RECT;
	m_hist_Rect = roiRect;

	return res;
}


int CMethod_ImgProcessing::IpHistogram_SetParameter(const LPImageInfo pIn, seCircle roiCircle)
{
	int res = 0;

	IpHistogram_Clear();

	cv::Mat matImg;
	if (Uint8ToCvMat(pIn, matImg) < ER_OK) {
		return ER_ABORT;
	}

	m_hist_Img = matImg.clone();

	m_emHist_BoxShape = emBoxShape::SHAPE_CIRCLE;
	m_hist_Circle = roiCircle;

	return res;
}

int CMethod_ImgProcessing::IpHistogram_SetParameter(const cv::Mat matImg, seCircle roiCircle)
{
	int res = 0;

	IpHistogram_Clear();

	if (matImg.empty()) {
		return ER_ABORT;
	}

	m_hist_Img = matImg.clone();

	m_emHist_BoxShape = emBoxShape::SHAPE_CIRCLE;
	m_hist_Circle = roiCircle;

	return res;
}



int CMethod_ImgProcessing::IpHistogram_TestIt()
{
	int res = 0;

	std::vector< std::vector<int>> vec2d_histval( m_hist_Img.channels() );
	cv::Mat m_Hist_Mask = cv::Mat::zeros(m_hist_Img.size(), CV_8UC1);

	switch (m_emHist_BoxShape) {

	case emShapeTypes::SHAPE_RECT:
		CreateMaskImg(m_hist_Rect, m_Hist_Mask);
		CalcHistogram(m_hist_Img, m_Hist_Mask, vec2d_histval);
		break;
	case emShapeTypes::SHAPE_CIRCLE:
		CreateMaskImg(m_hist_Circle, m_Hist_Mask);
		CalcHistogram(m_hist_Img, m_Hist_Mask, vec2d_histval);
		break;
	case emShapeTypes::SHAPE_ANNULUS:
		CreateMaskImg(m_hist_Annulus, m_Hist_Mask);
		CalcHistogram(m_hist_Img, m_Hist_Mask, vec2d_histval);
		break;
	default:
	CalcHistogram(m_hist_Img, vec2d_histval);
		break;
	}

	m_vec2d_hist_Val = vec2d_histval;

	return res;
}

int CMethod_ImgProcessing::IpHistogram_GetResult(double* p1DArray)
{
	int res = 0;

	if (p1DArray == nullptr) {

		return ER_ABORT;
	}

	try {
		
		for (int i = 0; i < m_vec2d_hist_Val.size(); i++) {

			std::copy(m_vec2d_hist_Val[i].begin(), m_vec2d_hist_Val[i].end(), p1DArray);
			p1DArray += m_vec2d_hist_Val[i].size();
		}

	}
	catch (std::exception& e) {

		std::cout << "exception : memcpy() the cv::Mat to char* OutImg buffer." << e.what() << std::endl;
		return ER_ABORT;
	}


	return res;
}


int CMethod_ImgProcessing::IpMorphology_SetParameter(emMorphShapes emShapes, int iKSize)
{
	int res = 0;

	IpMorphology_Clear();

	m_kernel_size = (iKSize <3 ) ? 3 : iKSize;
	m_morph_element = getStructuringElement((int)emShapes, cv::Size(m_kernel_size, m_kernel_size));

	return res;
}


int CMethod_ImgProcessing::IpMorphology_TestIt(const LPImageInfo pIn, emMorphOperation emOperation)
{
	int res = 0;

	cv::Mat matImg_gray;
	if (Uint8ToCvMat_GrayScalar(pIn, matImg_gray) < ER_OK) {
		return ER_ABORT;
	}

	morphologyEx(matImg_gray, m_morph_destImg, (int)emOperation, m_morph_element);

	return res;
}

int CMethod_ImgProcessing::IpMorphology_TestIt(const cv::Mat matImg_gray, emMorphOperation emOperation)
{
	int res = 0;

	if(matImg_gray.empty() || matImg_gray.channels() >1 ) {
		return ER_ABORT;
	}

	morphologyEx(matImg_gray, m_morph_destImg, (int)emOperation, m_morph_element);

	return res;
}


int CMethod_ImgProcessing::IpMorphology_GetResult(LPImageInfo pOut)
{
	int res = 0;

	int iW = pOut->iWidth;
	int iH = pOut->iHeight;
	int iChannels = pOut->iChannels;

	if (pOut->pbImgBuf == nullptr ||
		iW != m_morph_destImg.cols ||
		iH != m_morph_destImg.rows ||
		iChannels != m_morph_destImg.channels()) {

		return ER_ABORT;
	}

	if (CvMatToUint8(m_morph_destImg, pOut) < ER_OK) {
		return ER_ABORT;
	}

	return res;
}

int CMethod_ImgProcessing::IpMorphology_GetResult(cv::Mat& mDest_Img)
{
	int res = 0;

	if(m_morph_destImg.empty()) {
		return ER_ABORT;
	}

	m_morph_destImg.copyTo(mDest_Img);

	return res;
}


int CMethod_ImgProcessing::IpNoiseRemoval_SetParameter(double dbLimit_min, double dbLimit_max)
{
	int res = 0;

	IpNoiseRemoval_Clear();

	if (dbLimit_max < dbLimit_min) {
		return ER_ABORT;
	}

	m_dbLimit_min = dbLimit_min;
	m_dbLimit_max = dbLimit_max;


	return res;
}


int CMethod_ImgProcessing::IpNoiseRemoval_TestIt(const LPImageInfo pIn)
{
	int res = 0;

	cv::Mat matImg_gray;
	if (Uint8ToCvMat_GrayScalar(pIn, matImg_gray) < ER_OK) {
		return ER_ABORT;
	}

	Algo_NoiseRemoval_Arae(matImg_gray, m_object_binImg, m_dbLimit_min, m_dbLimit_max);

	return res;
}

int CMethod_ImgProcessing::IpNoiseRemoval_TestIt(const cv::Mat matImg_gray)
{
	int res = 0;

	if (matImg_gray.empty()) {
		return ER_ABORT;
	}

	cv::Mat srcImg = matImg_gray.clone();

	Algo_NoiseRemoval_Arae(srcImg, m_object_binImg, m_dbLimit_min, m_dbLimit_max);

	return res;
}


int CMethod_ImgProcessing::IpNoiseRemoval_GetResult(LPImageInfo pOut)
{
	int res = 0;

	int iW = pOut->iWidth;
	int iH = pOut->iHeight;
	int iChannels = pOut->iChannels;

	if (pOut->pbImgBuf == nullptr ||
		iW != m_object_binImg.cols ||
		iH != m_object_binImg.rows ||
		iChannels != m_object_binImg.channels()) {

		return ER_ABORT;
	}

	if (CvMatToUint8(m_object_binImg, pOut) < ER_OK) {
		return ER_ABORT;
	}

	return res;
}

int CMethod_ImgProcessing::IpNoiseRemoval_GetResult(cv::Mat& mDest_Img)
{
	int res = 0;

	if (m_object_binImg.empty()) {
		return ER_ABORT;
	}

	m_object_binImg.copyTo(mDest_Img);

	return res;
}


int CMethod_ImgProcessing::Algo_NoiseRemoval_Arae(const cv::Mat src_binImg, cv::Mat& dst_binImg, double dbLimit_min, double dbLimit_max)
{
	int res = 0;

	//Mrophology
	cv::Mat kernel_Morp = getStructuringElement(MORPH_CROSS, cv::Size(3, 3));
	cv::Mat imgMorphology;
	cv::morphologyEx(src_binImg, imgMorphology, cv::MORPH_OPEN, kernel_Morp);		//cv::MORPH_OPEN == erode() --> dilate()
	cv::morphologyEx(imgMorphology, imgMorphology, cv::MORPH_CLOSE, kernel_Morp);	//cv::MORPH_CLOSE == dilate() --> erode()


	//Removw noise
	std::vector< std::vector< cv::Point>> contours;
	std::vector< cv::Vec4i> hierarchy;
	//cv::findContours(imgMorphology, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
	cv::findContours(imgMorphology, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_NONE);	

	//remove the out of limit area contours.
#if (1)

	double tmpArea = 0.0;
	for (int i = 0; i < contours.size();)// i++)
		{
		tmpArea = contourArea(Mat(contours[i]));

		if ((tmpArea >= dbLimit_min) && (tmpArea <= dbLimit_max)) {

			cv::drawContours(imgMorphology, contours, i, cv::Scalar(0), cv::FILLED);
			//contours[i].erase(contours[i].begin(), contours[i].end());
			//contours[i].clear();
			contours.erase(contours.begin() + i);
		}
		else {
			++i;
		}
	}

#ifdef _RexTY_DEBUG

	cv::imshow("imgMorphology_01", imgMorphology);
	cv::waitKey(0);
	cv::destroyAllWindows();
#endif

#else

//Method_02
	typedef std::pair<int, double>pair;
	std::vector<pair> vec;

	//calc and save all area value of contours.
	for (int i = 0; i < contours.size(); i++)
	{
		double contArea = contourArea(contours[i]);
		vec.push_back(make_pair(i, contArea));
	}

	//sort all area value of contours.
	std::sort(vec.begin(), vec.end(), 
		[](std::pair<int, double>a, std::pair<int, double>b) 
		{ 
			return a.second > b.second; 
		}
	);

	//remove the out of limit area contours.
	//Method_01
	int i = 0;
	while (i < vec.size()) {

		if( (vec[i].second < dbLimit_min) ||
			(vec[i].second > dbLimit_max) ){

			vec.erase(vec.begin() + i);
		}
		else {
			i++;
		}
	}

#endif

	cv::drawContours(imgMorphology, contours, -1, cv::Scalar(255), cv::FILLED);

#ifdef _RexTY_DEBUG

	cv::imshow("imgMorphology_02", imgMorphology);
	cv::waitKey(0);
	cv::destroyAllWindows();
#endif

	int iterations = 3;
	cv::morphologyEx(imgMorphology, imgMorphology, cv::MORPH_CLOSE, kernel_Morp, cv::Point(-1, -1), iterations);

#ifdef _RexTY_DEBUG

	cv::imshow("imgMorphology_03", imgMorphology);
	cv::waitKey(0);
	cv::destroyAllWindows();
#endif

	m_object_binImg = imgMorphology.clone();


	return res;
}


int CMethod_ImgProcessing::IpDataAugmeatation_SetParameter(seDataAugmentationInfo se_DA_Param)
{
	int res = 0;

	m_DA_Param = se_DA_Param;

	return res;
}



int CMethod_ImgProcessing::IpDataAugmentation_TestIt(const cv::Mat matImg_gray)
{
	int res = 0;

	if (matImg_gray.empty() || m_DA_Param.strFileName.empty()) {
		return -1;
	}

	const char* szFunc_Flip[] = { "_flip_Xasix_", "_flip_Yasix_", "_flip_XYasix_" };
	const char* szFunc_Rotate[] = { "_rotate_" };
	const char* szFunc_Brightness[] = { "_brightness_" };

	cv::Mat srcImg = matImg_gray.clone();

	string strName = m_DA_Param.strFileName;

	cv::Mat tmpImg;

	if (m_DA_Param.bEnb_Flip_Xasix) {

		string strTmp = strName + szFunc_Flip[0] + getCurrentTime() + ".png";
		
		cv::flip(srcImg, tmpImg, 0);

		cv::imwrite(strTmp, tmpImg);

		if(tmpImg.empty()){
			tmpImg.release();
		}
	}

	if (m_DA_Param.bEnb_Flip_Xasix) {
	
		string strTmp = strName + szFunc_Flip[1] + getCurrentTime() + ".png";

		cv::flip(srcImg, tmpImg, 1);

		cv::imwrite(strTmp, tmpImg);

		if (tmpImg.empty()) {
			tmpImg.release();
		}
	}

	if (m_DA_Param.bEnb_Flip_XYasix) {
	
		string strTmp = strName + szFunc_Flip[2] + getCurrentTime() + ".png";

		cv::flip(srcImg, tmpImg, -1);

		cv::imwrite(strTmp, tmpImg);

		if (tmpImg.empty()) {
			tmpImg.release();
		}
	}

	if (m_DA_Param.dbRotateAngle) {	
		//TBD
	}

	if (m_DA_Param.iVal_Brightness[0] ||
		m_DA_Param.iVal_Brightness[1] ||
		m_DA_Param.iVal_Brightness[2] ){

		int iVal[3] = { m_DA_Param.iVal_Brightness[0] , m_DA_Param.iVal_Brightness[1], m_DA_Param.iVal_Brightness[2] };

		string strTmp = strName + szFunc_Brightness[0] + getCurrentTime() + ".png";

		tmpImg = srcImg + cv::Scalar( iVal[0], iVal[1], iVal[2] );

		cv::imwrite(strTmp, tmpImg);

		if (tmpImg.empty()) {
			tmpImg.release();
		}
		
	}


	return res;
}


int CMethod_ImgProcessing::IpDataAugmentation_GetResult()
{
	int res = 0;
	//TBD
	return res;
}