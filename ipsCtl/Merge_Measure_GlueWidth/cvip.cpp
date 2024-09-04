#include "cvip.h"
#include <stdlib.h> // free, perror
#include <string.h> // strlen
#include <ctime>
#include <math.h>	// atan
#include <cmath>
#include <random>
#include <stdarg.h>

#if defined(_MSC_VER)
	
	#include <direct.h> // _getcwd	
	#include <io.h>

#elif defined (__GNUC__)


	#include <stdio.h>
	#include <unistd.h>	// getcwd	
	#include <sys/types.h>
	#include <sys/stat.h>

#endif //#if defined(_MSC_VER)


#define bThrsh_Enable_bitwise_AND	(1)


using namespace std;
using namespace cv;



CCVIPItem::CCVIPItem()
{}

CCVIPItem::~CCVIPItem()
{}

int CCVIPItem::Rect_IsProperSubset(seRect roiMain, seRect roiSub) // Sub⊂Main
{

	if (roiSub.left > roiMain.left &&
		roiSub.top > roiMain.top &&
		roiSub.right < roiMain.right &&
		roiSub.bottom < roiMain.bottom)
	{
		return ER_TRUE;
	}
	else {
		return ER_ABORT;
	}

}


int CCVIPItem::Circle_IsProperSubset(seCircle roiMain, seCircle roiSub) // Sub⊂Main
{

	double dbX = pow(((double)roiMain.cX - (double)roiSub.cX), 2);
	double dbY = pow(((double)roiMain.cY - (double)roiSub.cY), 2);
	double dbDist = sqrt(dbX + dbY);

	if (dbDist < fabs(roiMain.dbRadius - roiSub.dbRadius)) {

		return ER_TRUE;
	}
	else {
		return ER_ABORT;
	}

}


//int CCVIPItem::Uint8ToCvMat(const LPImageInfo pSrc, cv::Mat& mDest)
//{
//	if (pSrc->pbImgBuf == nullptr) {
//		return ER_ABORT;
//	}
//
//	int res		= 0;
//
//	int iW		= pSrc->iWidth;
//	int iH		= pSrc->iHeight;
//	int iChannels	= pSrc->iChannels;
//	int iColorSpace = pSrc->iColorSpace;
//	unsigned char* pBuf = pSrc->pbImgBuf;
//
//	if (mDest.empty()) {
//		if (iChannels == 3) {
//
//			mDest = cv::Mat::zeros(iH, iW, CV_8UC3);
//		}
//		else {
//
//			mDest = cv::Mat::zeros(iH, iW, CV_8UC1);
//		}
//	}
//	else {
//
//		if (mDest.rows != iH || 
//			mDest.cols != iW || 
//			mDest.channels() != iChannels) {
//
//			return ER_ABORT;
//		}
//	}
//
//	try {
//
//		int iRowSize = iW * iChannels;
//		unsigned char* pter_Mat = mDest.ptr<unsigned char>(0);
//		for (int y = 0; y < iH; y++) {
//
//			pter_Mat = mDest.ptr<unsigned char>(y);
//
//			memcpy(pter_Mat, &pBuf[y * iRowSize], sizeof(unsigned char) * iRowSize);
//		}
//
//	}
//	catch (std::exception& e) {
//
//		std::cout << "exception : memcpy() the char* InImg buffer to cv::Mat." << e.what() << std::endl;
//		return ER_ABORT;
//	}
//
//	return res;
//}


//int CCVIPItem::Uint8ToCvMat_HSLtoGray(const LPImageInfo pSrc, cv::Mat& mDest)
//{
//	if (pSrc->pbImgBuf == nullptr) {
//		return ER_ABORT;
//	}
//
//	int res = 0;
//
//	int iW = pSrc->iWidth;
//	int iH = pSrc->iHeight;
//	int iChannels = pSrc->iChannels;
//	int iColorSpace = pSrc->iColorSpace;
//	unsigned char* pBuf = pSrc->pbImgBuf;
//
//	if (mDest.empty()) {
//
//		mDest = cv::Mat::zeros(iH, iW, CV_8UC1);
//	}
//	else {
//
//		if (mDest.rows != iH ||
//			mDest.cols != iW ){
//
//			return ER_ABORT;
//		}
//	}
//
//	try {
//
//		int iRowSize = iW * iChannels;
//		unsigned char* pter_Mat = mDest.ptr<unsigned char>(0);
//		for (int y = 0; y < iH; y++) {
//
//			pter_Mat = mDest.ptr<unsigned char>(y);
//
//			for (int x = 0, i=0; x < iRowSize; x += 3, i++) {
//
//				pter_Mat[i] = pBuf[y * iRowSize + x];
//			}
//		}
//
//	}
//	catch (std::exception& e) {
//
//		std::cout << "exception : memcpy() the char* InImg buffer to cv::Mat." << e.what() << std::endl;
//		return ER_ABORT;
//	}
//
//	return res;
//}


//int CCVIPItem::Uint8ToCvMat_GrayScalar(const LPImageInfo pSrc, cv::Mat& mDest)
//{
//	int res = 0;
//
//	int iChannels = pSrc->iChannels;
//	int iColorSpace = pSrc->iColorSpace;
//
//	if (iColorSpace == (int)(emColorSpace::COLORSPACE_RGB)) {
//
//		if (Uint8ToCvMat(pSrc, mDest) < ER_OK) {
//			return ER_ABORT;
//		}
//
//		if (3 == iChannels) {
//
//			cv::cvtColor(mDest, mDest, COLOR_BGR2GRAY);
//		}	
//
//	}
//	else if (iColorSpace == (int)(emColorSpace::COLORSPACE_HSL)) {
//
//		//ColorSpace --> emColorSpace::COLORSPACE_HSL.
//		if (3 == iChannels) {
//
//			//std::vector< cv::Mat> vec_matPlan(iChannels);
//
//			//cv::split(matTmp, vec_matPlan);
//
//			//mDest = vec_matPlan.at(2).clone();	// L channels
//
//			Uint8ToCvMat_HSLtoGray(pSrc, mDest);
//
//		}
//		else {	
//
//			if (Uint8ToCvMat(pSrc, mDest) < ER_OK) {
//				return ER_ABORT;
//			}
//		}
//
//	}
//	else{
//
//		return ER_ABORT;
//	}
//
//	return res;
//}


//int CCVIPItem::Uint8ToCvMat_ColorScalar(const LPImageInfo pSrc, cv::Mat& mDest)
//{
//	int res = 0;
//
//	if (Uint8ToCvMat(pSrc, mDest) < ER_OK) {
//		return ER_ABORT;
//	}
//
//	if (3 != mDest.channels()) {
//
//		cv::cvtColor(mDest, mDest, COLOR_GRAY2BGR);
//	}
//
//	return res;
//}


int CCVIPItem::Uint8ToCvMat(const LPImageInfo pSrc, cv::Mat& mDest)
{
	if (pSrc->pbImgBuf == nullptr) {
		return ER_ABORT;
	}

#ifdef ALGO_Enable_CYCLE_TIME_DEBUG

	clock_t start, end;

	//cycletime_start
	start = clock();

#endif

	int res = 0;
	int iW = pSrc->iWidth;
	int iH = pSrc->iHeight;
	int iChannels = pSrc->iChannels;
	int iColorSpace = pSrc->iColorSpace;
	unsigned char* pBuf = pSrc->pbImgBuf;

	if (iChannels == 3) {

		mDest = cv::Mat(iH, iW, CV_8UC3, pBuf);
	}
	else {

		mDest = cv::Mat(iH, iW, CV_8UC1, pBuf);
	}

#ifdef ALGO_Enable_CYCLE_TIME_DEBUG

	//cycletime_end
	end = clock();

	double elapsed = double(end - start) / CLOCKS_PER_SEC;
	printD("[__%s__] : Cycle time = %6.4f (seconds) \n", __func__, elapsed);

	//double elapsed = double(end - start);
	//printD("[__%s__] : Cycle time = %5.2f (millisecond) \n", __func__, elapsed);

#endif

	return res;
}


int CCVIPItem::Uint8ToCvMat_GrayScalar(const LPImageInfo pSrc, cv::Mat& mDest)
{
	int res = 0;

	cv::Mat matTmp;
	if (Uint8ToCvMat(pSrc, matTmp) < ER_OK) {
		return ER_ABORT;
	}

	int iChannels = pSrc->iChannels;
	int iColorSpace = pSrc->iColorSpace;

	if (1 != matTmp.channels()) {

		//ColorSpace --> emColorSpace::COLORSPACE_HSL.
		if ((3 == iChannels) && (iColorSpace == (int)(emColorSpace::COLORSPACE_HSL))) {

			std::vector< cv::Mat> vec_matPlan(iChannels);

			cv::split(matTmp, vec_matPlan);

			mDest = vec_matPlan.at(2).clone();	// L channels
		}
		else {	//ColorSpace --> emColorSpace::COLORSPACE_RGB.

			cv::cvtColor(matTmp, mDest, COLOR_BGR2GRAY);
		}
	}
	else {

		mDest = matTmp.clone();
	}

	return res;
}


int CCVIPItem::Uint8ToCvMat_ColorScalar(const LPImageInfo pSrc, cv::Mat& mDest)
{
	int res = 0;

	cv::Mat matTmp;
	if (Uint8ToCvMat(pSrc, matTmp) < ER_OK) {
		return ER_ABORT;
	}

	if (3 != matTmp.channels()) {
		cv::cvtColor(matTmp, mDest, COLOR_GRAY2BGR);
	}
	else {
		mDest = matTmp.clone();
	}

	return res;
}


int CCVIPItem::CvMatToUint8(const cv::Mat mSrc, LPImageInfo pDest)
{

#ifdef ALGO_Enable_CYCLE_TIME_DEBUG

	clock_t start, end;

	//cycletime_start
	start = clock();

#endif

	int res = 0;

	int iW = pDest->iWidth;
	int iH = pDest->iHeight;
	int iChannels = pDest->iChannels;
	unsigned char* pBuf = pDest->pbImgBuf;

	if ((pDest->pbImgBuf == nullptr)	||
		(mSrc.cols != iW)	|| 
		(mSrc.rows != iH)	|| 
		(mSrc.channels() != iChannels) ){

		printD("Parameters Error!!!.\n");
		return ER_ABORT;
	}

	try {

		cv::Size size;
		size.width = iW * iChannels;
		size.height = iH;

		if (mSrc.isContinuous()) {			

			size.width *= size.height;
			size.height = 1;
		}

		const unsigned char* pter_Mat = mSrc.ptr<unsigned char>(0);

		int iRowSize = size.width;
		for (int y = 0; y < size.height; y++) {

			pter_Mat = mSrc.ptr<unsigned char>(y);

			memcpy(&pBuf[y * iRowSize], pter_Mat, sizeof(unsigned char) * iRowSize);
		}

	}
	catch (std::exception& e) {

		printD("exception : memcpy() the cv::Mat to char* OutImg buffer.\n");
		return ER_ABORT;
	}


#ifdef ALGO_Enable_CYCLE_TIME_DEBUG

	//cycletime_end
	end = clock();

	double elapsed = double(end - start) / CLOCKS_PER_SEC;
	printD("[__%s__] : Cycle time = %6.4f (seconds) \n", __func__, elapsed);

	//double elapsed = double(end - start);
	//printD("[__%s__] : Cycle time = %5.2f (millisecond) \n", __func__, elapsed);

#endif

	return 0;
}


double CCVIPItem::Similarity_CalcuSSIM(cv::Mat i1, cv::Mat i2) //SSIM（structural similarity）
{
	const double C1 = 6.5025, C2 = 58.5225;
	/***************************** INITS **********************************/
	int d = CV_32F;

	Mat I1, I2;
	i1.convertTo(I1, d); // cannot calculate on one byte large values 
	i2.convertTo(I2, d);

	Mat I1_2 = I1.mul(I1); // I1^2 	
	Mat I2_2 = I2.mul(I2); // I2^2 
	Mat I1_I2 = I1.mul(I2); // I1 * I2 

	/*************************** END INITS **********************************/

	Mat mu1, mu2; // PRELIMINARY COMPUTING 
	GaussianBlur(I1, mu1, Size(11, 11), 1.5);
	GaussianBlur(I2, mu2, Size(11, 11), 1.5);

	Mat mu1_2 = mu1.mul(mu1);
	Mat mu2_2 = mu2.mul(mu2);
	Mat mu1_mu2 = mu1.mul(mu2);

	Mat sigma1_2, sigma2_2, sigma12;

	GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
	sigma1_2 -= mu1_2;

	GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
	sigma2_2 -= mu2_2;

	GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
	sigma12 -= mu1_mu2;

	// FORMULA
	Mat t1, t2, t3;

	t1 = 2 * mu1_mu2 + C1;
	t2 = 2 * sigma12 + C2;
	t3 = t1.mul(t2); // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2)) 

	t1 = mu1_2 + mu2_2 + C1;
	t2 = sigma1_2 + sigma2_2 + C2;
	t1 = t1.mul(t2); // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2)) 

	Mat ssim_map;
	divide(t3, t1, ssim_map); // ssim_map = t3./t1; 

	Scalar mssim = mean(ssim_map); // mssim = average of ssim map 

	double ssim = mssim.val[0];

	return ssim;
}


int CCVIPItem::Histogram_CtvInteger(const cv::Mat valHist, std::vector<int>& vec1d_Data)
{
	int res = 0;

	if (valHist.empty()) { return ER_ABORT; }

	cv::Mat hist = valHist.clone();

	double minValue(0.0);		//直方圖中最小的bin的值
	double maxValue(0.0);		//直方圖中最大的bin的值
	cv::minMaxLoc(hist, &minValue, &maxValue, 0, 0);	//minMaxLoc可以計算最大值最小值以及其對應的位置 這裡求最大值

	double histHeight = 256.0;	//要繪製直方圖的最大高度
	int histSize = 256;			//直方圖每一維度bin個數
	int scale = 2;				//控制影象的寬大小


#ifdef _RexTY_DEBUG

	cv::Mat histImg = cv::Mat::zeros(cv::Size(histSize * scale, histSize), CV_8UC1);//用於顯示直方圖		
#endif

	float* p = hist.ptr<float>(0);
	for (size_t i = 0; i < histSize; i++)//進行直方圖的繪製
	{
		float bin_val = p[i];
		int intensity = cvRound((bin_val * histHeight) / maxValue);  //要繪製的高度 
		vec1d_Data.push_back(intensity);

#ifdef _RexTY_DEBUG

		//_fordebug
		for (size_t j = 0; j < scale; j++) //繪製直線 這裡用每scale條直線代表一個bin
		{
			cv::line(histImg, cv::Point(i * scale + j, histHeight - intensity), cv::Point(i * scale + j, histHeight - 1), 255);
		}
#endif

	}

#ifdef _RexTY_DEBUG

	cv::imshow("histImg", histImg);
	cv::waitKey(0);
	cv::destroyAllWindows();

#endif


	return res;
}

//std::vector<std::vector<int>>& vec2d_Info_Data
int CCVIPItem::CalcHistogram(const cv::Mat src, std::vector< std::vector<int>>& vec2d_histogram)
{
	int res = 0;

	bool blnGray = false;
	if (src.channels() == 1)
	{
		blnGray = true;
	}

	std::vector<Mat> bgr_plane;
	std::vector<Mat> gray_plane;
	std::vector<Mat> mergeSrc;

	const int bins[1] = { 256 };
	float hranges[2] = { 0, 255 };
	const float* ranges[1] = { hranges };
	cv::Mat b_hist, g_hist, r_hist, hist;


	//int grayImgNum = 1;
	//int grayChannels = 0;			//需要計算的通道號 單通道只有 通道號為0
	//const int grayHistDim = 1;		//長條圖維數
	//const int grayHistSize = 256;	//長條圖每一維度bin個數
	//float grayRanges[2] = { 0, 255 }; //灰度值的統計範圍
	//const float* grayHistRanges[1] = { grayRanges }; //灰度值統計範圍指針
	bool grayUniform = true;		//是否均勻
	//bool grayAccumulate = false;	//是否累積
	////計算灰度圖像的長條圖
	//cv::calcHist(&bgr_plane[0], grayImgNum, &grayChannels, cv::Mat(), hist, grayHistDim, &grayHistSize, grayHistRanges, grayUniform, grayAccumulate);


	if (blnGray)
	{
		split(src, gray_plane);
		calcHist(&gray_plane[0], 1, 0, cv::Mat(), hist, 1, bins, ranges, grayUniform);

		Histogram_CtvInteger(hist, vec2d_histogram[0]);
	}
	else
	{
		split(src, bgr_plane);
		calcHist(&bgr_plane[0], 1, 0, cv::Mat(), b_hist, 1, bins, ranges, grayUniform);
		calcHist(&bgr_plane[1], 1, 0, cv::Mat(), g_hist, 1, bins, ranges, grayUniform);
		calcHist(&bgr_plane[2], 1, 0, cv::Mat(), r_hist, 1, bins, ranges, grayUniform);

		Histogram_CtvInteger(r_hist, vec2d_histogram[0]);
		Histogram_CtvInteger(g_hist, vec2d_histogram[1]);
		Histogram_CtvInteger(b_hist, vec2d_histogram[2]);
	}

	return res;
}


int CCVIPItem::CalcHistogram(const cv::Mat src, const cv::Mat mask, std::vector< std::vector<int>>& vec2d_histogram)
{
	int res = 0;

	bool blnGray = false;
	if (src.channels() == 1)
	{
		blnGray = true;
	}

	std::vector<Mat> bgr_plane;
	std::vector<Mat> gray_plane;
	std::vector<Mat> mergeSrc;

	const int bins[1] = { 256 };
	float hranges[2] = { 0, 255 };
	const float* ranges[1] = { hranges };
	cv::Mat b_hist, g_hist, r_hist, hist;


	//int grayImgNum = 1;
	//int grayChannels = 0;				//需要計算的通道號 單通道只有 通道號為0
	//const int grayHistDim = 1;		//長條圖維數
	//const int grayHistSize = 256;		//長條圖每一維度bin個數
	//float grayRanges[2] = { 0, 255 };	//灰度值的統計範圍
	//const float* grayHistRanges[1] = { grayRanges }; //灰度值統計範圍指針
	bool grayUniform = true;			//是否均勻
	//bool grayAccumulate = false;		//是否累積
	////計算灰度圖像的長條圖
	//cv::calcHist(&bgr_plane[0], grayImgNum, &grayChannels, cv::Mat(), hist, grayHistDim, &grayHistSize, grayHistRanges, grayUniform, grayAccumulate);


	if (blnGray)
	{
		split(src, gray_plane);
		calcHist(&gray_plane[0], 1, 0, mask, hist, 1, bins, ranges, grayUniform);

		Histogram_CtvInteger(hist, vec2d_histogram[0]);
	}
	else
	{
		split(src, bgr_plane);
		calcHist(&bgr_plane[0], 1, 0, mask, b_hist, 1, bins, ranges, grayUniform);
		calcHist(&bgr_plane[1], 1, 0, mask, g_hist, 1, bins, ranges, grayUniform);
		calcHist(&bgr_plane[2], 1, 0, mask, r_hist, 1, bins, ranges, grayUniform);

		Histogram_CtvInteger(r_hist, vec2d_histogram[0]);
		Histogram_CtvInteger(g_hist, vec2d_histogram[1]);
		Histogram_CtvInteger(b_hist, vec2d_histogram[2]);
	}

	return res;
}


int CCVIPItem::CalcThreshold(const cv::Mat src, Mat& matMerge, std::vector<double> vecThresh, std::vector<double> vecMaxVal, emThresholdTypes emTypes)
{
	int res = 0;
	int iChannels = src.channels();

	std::vector<Mat> bgr_plane;
	std::vector<Mat> mergeSrc;
	cv::Mat thresh_img;

	split(src, bgr_plane);

	for (int i = 0; i < iChannels; i++) {

		if (!thresh_img.empty()) {
			thresh_img.release();
		}

		switch (emTypes) {

		case emThresholdTypes::THRSH_BINARY:
		default:
			//cv::threshold(bgr_plane[i], thresh_img, vecThresh[i], vecMaxVal[i], cv::THRESH_BINARY);
			vecThresh[i] = (vecThresh[i] < 0) ? 0 : vecThresh[i];
			vecMaxVal[i] = (vecMaxVal[i] < vecThresh[i]) ? vecThresh[i] : vecMaxVal[i];
			cv::inRange(bgr_plane[i], vecThresh[i], vecMaxVal[i], thresh_img);
			break;

		case emThresholdTypes::THRSH_OTSU:
			cv::threshold(bgr_plane[i], thresh_img, 0, 255, cv::THRESH_OTSU);
			break;

		case emThresholdTypes::THRSH_ADAPTIVE:
			int maxVal = 255;
			int blockSize = 5;
			double C = 0;
			cv::medianBlur(bgr_plane[i], bgr_plane[i], 5);
			cv::adaptiveThreshold(src, thresh_img, maxVal, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, blockSize, C);
			break;
		}

		mergeSrc.push_back(thresh_img);

	}

#if bThrsh_Enable_bitwise_AND

	matMerge = mergeSrc[0];

	for (int i = 1; i < mergeSrc.size(); i++) {

		cv::bitwise_and(matMerge, mergeSrc[i], matMerge);
	}

#else

	merge(mergeSrc, matMerge);

#endif

	return res;
}


int CCVIPItem::CalcuBoundingBox(seBoundingBox se_rect, seBoundingBox& se_out)
{
	int res = 0;

	if (se_rect.dbAngle != 0) {

		cv::Point posCenter(se_rect.cX, se_rect.cY);
		double dbAngle(se_rect.dbAngle);
		double dbScale(1.0);
		cv::Size szSize(se_rect.rectBox.width, se_rect.rectBox.height);

		// Create the rotated rectangle
		cv::RotatedRect rotatedRectangle(posCenter, szSize, dbAngle);

		se_rect.rectBox.left	= rotatedRectangle.boundingRect().tl().x;
		se_rect.rectBox.top		= rotatedRectangle.boundingRect().tl().y;
		se_rect.rectBox.right	= rotatedRectangle.boundingRect().br().x;
		se_rect.rectBox.bottom	= rotatedRectangle.boundingRect().br().y;

	}
	
	se_out = se_rect;	

	return res;
}

int CCVIPItem::CalcuBoundingBox(seCircle se_circle, seBoundingBox& se_out)
{
	int res = 0;

	se_out.cX = se_circle.cX;
	se_out.cY = se_circle.cY;
	se_out.dbAngle = 0;

	se_out.rectBox.left = se_circle.cX - static_cast<int>(se_circle.dbRadius);
	se_out.rectBox.top = se_circle.cY - static_cast<int>(se_circle.dbRadius);

	se_out.rectBox.right = se_circle.cX + static_cast<int>(se_circle.dbRadius);
	se_out.rectBox.bottom = se_circle.cY + static_cast<int>(se_circle.dbRadius);

	se_out.rectBox.width = abs(se_out.rectBox.right - se_out.rectBox.left);
	se_out.rectBox.height = abs(se_out.rectBox.bottom - se_out.rectBox.top);

	return res;
}

int CCVIPItem::CalcuBoundingBox(seAnnulus	se_annulus, seBoundingBox& se_out)
{
	int res = 0;

	se_out.cX = se_annulus.cX;
	se_out.cY = se_annulus.cY;
	se_out.dbAngle = 0;// se_annulus.dnAngle;

	se_out.rectBox.left	= se_annulus.cX - static_cast<int>(se_annulus.dbRadius_Outer);
	se_out.rectBox.top	= se_annulus.cY - static_cast<int>(se_annulus.dbRadius_Outer);

	se_out.rectBox.right	= se_annulus.cX + static_cast<int>(se_annulus.dbRadius_Outer);
	se_out.rectBox.bottom	= se_annulus.cY + static_cast<int>(se_annulus.dbRadius_Outer);

	se_out.rectBox.width = abs(se_out.rectBox.right - se_out.rectBox.left);
	se_out.rectBox.height = abs(se_out.rectBox.bottom - se_out.rectBox.top);

	return res;
}


int CCVIPItem::CreateMaskImg(seBoundingBox se_rect, cv::Mat& out, int iMasktype)
{
	int res = 0;

	cv::Point posCenter(se_rect.cX, se_rect.cY);
	double dbAngle(se_rect.dbAngle);
	double dbScale(1.0);
	cv::Size szSize(se_rect.rectBox.width, se_rect.rectBox.height);

	// Create the rotated rectangle
	cv::RotatedRect rotatedRectangle(posCenter, szSize, dbAngle);

	// We take the edges that OpenCV calculated for us
	cv::Point2f vertices2f[4];
	rotatedRectangle.points(vertices2f);

	if (1 == iMasktype) {

		// Convert them so we can use them in a fillConvexPoly
		cv::Point vertices[4];
		for (int i = 0; i < 4; ++i) {
			vertices[i] = vertices2f[i];
		}
		// Now we can fill the rotated rectangle with our specified color
		cv::fillConvexPoly(out, vertices, 4, cv::Scalar(255));
	}
	else if (2 == iMasktype) {

		// Convert them so we can use them in a fillConvexPoly
		std::vector<cv::Point> vecVertices;
		for (int i = 0; i < 4; ++i) {
			vecVertices.push_back(vertices2f[i]);
		}
		// Now we can drawing the rotated rectangle with our specified color
		cv::polylines(out, vecVertices, true, cv::Scalar(255), 2);
	}
	else {

		cv::Point posS(se_rect.rectBox.left, se_rect.rectBox.top);
		cv::Point posE(se_rect.rectBox.right, se_rect.rectBox.bottom);
		cv::rectangle(out, posS, posE, cv::Scalar(255), FILLED);
	}

	return res;
}

int CCVIPItem::CreateMaskImg(seCircle se_circle, cv::Mat& out)
 {
	int res =0;

	cv::Point posCenter(se_circle.cX, se_circle.cY);
	int iRadius = static_cast<int>(se_circle.dbRadius);
	cv::circle(out, posCenter, iRadius, cv::Scalar(255), FILLED);

	return res;
 }


int CCVIPItem::CreateMaskImg(seAnnulus se_annulus, cv::Mat& out)
{
	int res = 0;

	double dbAngle_start(((se_annulus.dbStartAngle <= 0) || (se_annulus.dbStartAngle >= 360)) ? 0 : se_annulus.dbStartAngle);
	double dbAngle_end(((se_annulus.dbEndAngle <= 0) || (se_annulus.dbEndAngle >= 360)) ? 360 : se_annulus.dbEndAngle);
	//dbAngle_start = min(dbAngle_start, dbAngle_end);
	//dbAngle_end = max(dbAngle_start, dbAngle_end);
	if (dbAngle_start > dbAngle_end) {
		std::swap(dbAngle_start, dbAngle_end);
	}

	double dbAngle_All[2] = { dbAngle_start, dbAngle_end };	

	cv::Point posCenter(se_annulus.cX, se_annulus.cY);
	int iRadius_Outer = static_cast<int>(se_annulus.dbRadius_Outer);
	int iRadius_Inner = static_cast<int>(se_annulus.dbRadius_Inner);

	if ( 0 == dbAngle_start && 360 == dbAngle_end ) {	//Shape of Annulus.

		cv::circle(out, posCenter, iRadius_Outer, cv::Scalar(255), cv::FILLED);
		cv::circle(out, posCenter, iRadius_Inner, cv::Scalar(0), cv::FILLED);
	}
	else {	//Shape of Cirecle Sector_(扇形)

		int thickness = 2;	//線寬
		int lineType = 8;	//線型
		cv::Scalar scColor = cv::Scalar(255, 255, 255);

		cv::ellipse(out,				//將橢圓畫到img圖像上
			posCenter,					//橢圓中心點
			cv::Size(iRadius_Outer, iRadius_Outer),//橢圓長度
			0,							//橢圓旋轉角度
			dbAngle_start,				//擴展弧度從0度到360度
			dbAngle_end,
			scColor,	
			thickness);					

		cv::ellipse(out,				//將橢圓畫到img圖像上
			posCenter,					//橢圓中心點
			Size(iRadius_Inner, iRadius_Inner),//橢圓長度
			0,							//橢圓旋轉角度
			dbAngle_start,				//擴展弧度從0度到360度
			dbAngle_end,
			scColor,	
			thickness);					

		cv::Point second_begin, second_end; //設定刻度的起點、終點 

		for (int c = 0; c < sizeof(dbAngle_All)/sizeof(dbAngle_All[0]); c++) {

			second_begin.x = posCenter.x + iRadius_Inner * cos( dbAngle_All[c] * CV_PI / 180.0 );	//刻度的起點x坐標賦值
			second_begin.y = posCenter.y + iRadius_Inner * sin( dbAngle_All[c] * CV_PI / 180.0 );	//刻度的起點y坐標賦值 

			//Radius_Oute
			second_end.x = posCenter.x + iRadius_Outer * cos( dbAngle_All[c] * CV_PI / 180.0 );	//刻度的終點x坐標賦
			second_end.y = posCenter.y + iRadius_Outer * sin( dbAngle_All[c] * CV_PI / 180.0 );	//刻度的終點y坐標賦

			line(out, second_begin, second_end, scColor, thickness);					//連接起點與終點 
		}

		std::vector< std::vector< cv::Point>> contours;
		std::vector< cv::Vec4i> hierarchy;
		cv::findContours(out, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
		cv::drawContours(out, contours, -1, cv::Scalar(255), cv::FILLED);

	}

	return 0;
}


int CCVIPItem::CalcuCenterOffset(cv::Size szImage, seCoordBindBox se_coord, seBoundingBox se_rect, seBoundingBox& se_out)
{
	int res = 0;

	se_out = se_rect;

	if (0.0 == se_coord.FMark.dbAngle) {

		return 0;
	}
 
	int half_W = round(szImage.width * 0.5);
	int half_H = round(szImage.height * 0.5);

	double dbDegree = se_coord.FMark.dbAngle * CV_PI / 180.0;
	double dbCos = cos(dbDegree);
	double dbSin = sin(dbDegree);

	int iPos_FMark_cX = se_coord.FMark.cX;
	int iPos_FMark_cY = se_coord.FMark.cY;

	int iShift_IBox_W = se_coord.CalibCoord.iDelta_InspectBox_W;
	int iShift_IBox_H = se_coord.CalibCoord.iDelta_InspectBox_H;

	cv::Mat matRotate		= (cv::Mat_<double>(2, 2) << dbCos, -1.0 * dbSin, dbSin, dbCos);
	cv::Mat matPos_Center	= (cv::Mat_<double>(2, 1) << half_W, half_H);
	cv::Mat matPos_FMark	= (cv::Mat_<double>(2, 1) << iPos_FMark_cX, iPos_FMark_cY);
	cv::Mat matShift_IBox	= (cv::Mat_<double>(2, 1) << iShift_IBox_W, iShift_IBox_H);

	cv::Mat matValue = matRotate * (matPos_FMark - matPos_Center) + matPos_Center + matShift_IBox;

	se_out.cX = round(matValue.at<double>(0));
	se_out.cY = round(matValue.at<double>(1));

	return res;
}

int CCVIPItem::CalcuCenterOffset(cv::Size szImage, seCoordBindBox se_coord, seCircle se_circle, seCircle& se_out)
{
	int res = 0;

	se_out = se_circle;

	if (0.0 == se_coord.FMark.dbAngle) {

		return 0;
	}
 
	int half_W = round(szImage.width * 0.5);
	int half_H = round(szImage.height * 0.5);

	double dbDegree = se_coord.FMark.dbAngle * CV_PI / 180.0;
	double dbCos = cos(dbDegree);
	double dbSin = sin(dbDegree);

	int iPos_FMark_cX = se_coord.FMark.cX;
	int iPos_FMark_cY = se_coord.FMark.cY;

	int iShift_IBox_W = se_coord.CalibCoord.iDelta_InspectBox_W;
	int iShift_IBox_H = se_coord.CalibCoord.iDelta_InspectBox_H;

	cv::Mat matRotate = (cv::Mat_<double>(2, 2) << dbCos, -1.0 * dbSin, dbSin, dbCos);
	cv::Mat matPos_Center = (cv::Mat_<double>(2, 1) << half_W, half_H);
	cv::Mat matPos_FM = (cv::Mat_<double>(2, 1) << iPos_FMark_cX, iPos_FMark_cY);
	cv::Mat matShift_IB = (cv::Mat_<double>(2, 1) << iShift_IBox_W, iShift_IBox_H);

	cv::Mat matValue = matRotate * (matPos_FM - matPos_Center) + matPos_Center + matShift_IB;

	se_out.cX = round(matValue.at<double>(0));
	se_out.cY = round(matValue.at<double>(1));


	return res;
}

int CCVIPItem::CalcuCenterOffset(cv::Size szImage, seCoordBindBox se_coord, seAnnulus se_annulus, seAnnulus& se_out)
{
	int res = 0;

	se_out = se_annulus;
 
	if (0.0 == se_coord.FMark.dbAngle) {

		return 0;
	}

	int half_W = round(szImage.width * 0.5);
	int half_H = round(szImage.height * 0.5);

	double dbDegree = se_coord.FMark.dbAngle * CV_PI / 180.0;
	double dbCos = cos(dbDegree);
	double dbSin = sin(dbDegree);

	int iPos_FMark_cX = se_coord.FMark.cX;
	int iPos_FMark_cY = se_coord.FMark.cY;

	int iShift_IBox_W = se_coord.CalibCoord.iDelta_InspectBox_W;
	int iShift_IBox_H = se_coord.CalibCoord.iDelta_InspectBox_H;

	cv::Mat matRotate		= (cv::Mat_<double>(2, 2) << dbCos, -1.0 * dbSin, dbSin, dbCos);
	cv::Mat matPos_Center	= (cv::Mat_<double>(2, 1) << half_W, half_H);
	cv::Mat matPos_FM		= (cv::Mat_<double>(2, 1) << iPos_FMark_cX, iPos_FMark_cY);
	cv::Mat matShift_IB		= (cv::Mat_<double>(2, 1) << iShift_IBox_W, iShift_IBox_H);

	cv::Mat matValue = matRotate * (matPos_FM - matPos_Center) + matPos_Center + matShift_IB;

	se_out.cX = round( matValue.at<double>(0));
	se_out.cY = round( matValue.at<double>(1));

	return res;
}


//	bDirection	//0: Outside to Inside:由外向內搜尋; 1: Inside to Outside:由內向外搜尋
int CCVIPItem::Annulus_Degrees_RangeSetting(seAnnulus roiAnnuls, double dbStepSize, bool bDirection, std::vector<pairPos>& vec_List_PairPos)
{
	int res = 0;

	cv::Point2f posCenter(roiAnnuls.cX, roiAnnuls.cY);
	double fRadius[2]{ roiAnnuls.dbRadius_Inner, roiAnnuls.dbRadius_Outer };

	double dbAngle_start(((roiAnnuls.dbStartAngle < 0) || (roiAnnuls.dbStartAngle > 360)) ? 0 : roiAnnuls.dbStartAngle);
	double dbAngle_end(((roiAnnuls.dbEndAngle < 0) || (roiAnnuls.dbEndAngle > 360)) ? 360 : roiAnnuls.dbEndAngle);

	//dbAngle_start	= min(dbAngle_start, dbAngle_end);
	//dbAngle_end		= max(dbAngle_start, dbAngle_end);
	if (dbAngle_start > dbAngle_end) {
		std::swap(dbAngle_start, dbAngle_end);
	}

	double dbDegrees_Total		= (dbAngle_end - dbAngle_start);
	double dbDegrees_Divided	= (dbStepSize > dbDegrees_Total) ? dbDegrees_Total : dbStepSize;
	double dbDegrees_Section	= round(dbDegrees_Total / dbDegrees_Divided);

	int irange_start = static_cast<int>(dbAngle_start);
	int irange_End = static_cast<int>(dbDegrees_Section) + irange_start;

	std::vector<pairPos> vecCnt_GlueWidth;

	cv::Point second_begin, second_end; //刻度的起點，终點 
	double scale_long(0.0);				//刻度的長度 
	int scale_width(1);					//刻度的寬度 

	for (int second_Scale = irange_start; second_Scale < irange_End; second_Scale++) {

		//Radius_Inner
		second_begin.x = posCenter.x + fRadius[0] * std::cos(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的起點x坐標賦值
		second_begin.y = posCenter.y + fRadius[0] * std::sin(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的起點y坐標賦值 

		//Radius_Oute
		second_end.x = posCenter.x + (fRadius[1] - scale_long) * std::cos(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的終點x坐標賦
		second_end.y = posCenter.y + (fRadius[1] - scale_long) * std::sin(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的終點y坐標賦

		vecCnt_GlueWidth.push_back(std::make_pair(second_begin, second_end));
	}

	vec_List_PairPos.reserve(dbDegrees_Section);
	vec_List_PairPos.assign(vecCnt_GlueWidth.begin(), vecCnt_GlueWidth.end());

	return res;
}


int CCVIPItem::Annulus_Degrees_GetInfoByLine(const cv::Mat mat_gray_Img, std::vector<pairPos> vec_List_PairPos, std::vector<std::vector<int>>& vec2d_Info_Data)
{
	int res = 0;

	if (mat_gray_Img.empty() || mat_gray_Img.channels() != 1 || vec_List_PairPos.empty())
		return ER_ABORT;

	int iRow_RangeSize = static_cast<int>(vec_List_PairPos.size());

	std::vector<int> vec_data_count(iRow_RangeSize);
	std::vector< std::vector<int>> vec2d_data_intensity(iRow_RangeSize);

	for (int x = 0; x < iRow_RangeSize; x++) {

		LineIterator itor(mat_gray_Img, vec_List_PairPos[x].first, vec_List_PairPos[x].second);
		LineIterator it2 = itor;

		//record the value of intensity from image by two point of line.
		for (int i = 0; i < itor.count; i++, itor++)
		{
			uchar* ptr = *itor;
			vec2d_data_intensity[x].push_back(ptr[0]);
		}

		//calculate the count of maxval(>0) from the Binary image.
		int iValidCnt_InLine = 0;
		for (int i = 0; i < it2.count; i++, it2++) {

			uchar* ptr = *it2;
			iValidCnt_InLine += (static_cast<int>(ptr[0]) != 0) ? 1 : 0;
		}
		vec_data_count.at(x) = iValidCnt_InLine;

	}

	vec2d_Info_Data = vec2d_data_intensity;

	return res;
}


int CCVIPItem::Annulus_Degrees_GetInfoByLine(const cv::Mat mat_gray_Img, std::vector<pairPos> vec_List_PairPos, std::vector<std::vector<int>>& vec2d_Info_Data, std::vector<std::vector<cv::Point>>& vec2d_Pos_Data)
{
	int res = 0;

	if (mat_gray_Img.empty() || mat_gray_Img.channels() != 1 || vec_List_PairPos.empty())
		return ER_ABORT;

	int iRow_RangeSize = static_cast<int>(vec_List_PairPos.size());

	std::vector<int> vec_data_count(iRow_RangeSize);
	std::vector< std::vector<int>> vec2d_data_intensity(iRow_RangeSize);
	std::vector< std::vector<cv::Point>> vec2d_data_position(iRow_RangeSize);

	for (int x = 0; x < iRow_RangeSize; x++) {

		LineIterator itor(mat_gray_Img, vec_List_PairPos[x].first, vec_List_PairPos[x].second);
		LineIterator it2 = itor;

		//record the value of intensity from image by two point of line.
		for (int i = 0; i < itor.count; i++, itor++)
		{
			uchar* ptr = *itor;
			vec2d_data_intensity[x].push_back(ptr[0]);
			vec2d_data_position[x].push_back(itor.pos());
		}

		//calculate the count of maxval(>0) from the Binary image.
		int iValidCnt_InLine = 0;
		for (int i = 0; i < it2.count; i++, it2++) {

			uchar* ptr = *it2;
			iValidCnt_InLine += (static_cast<int>(ptr[0]) != 0) ? 1 : 0;
		}
		vec_data_count.at(x) = iValidCnt_InLine;

	}

	vec2d_Info_Data = vec2d_data_intensity;
	vec2d_Pos_Data = vec2d_data_position;

	return res;
}


int CCVIPItem::Annulus_Degrees_GetInfoByLine_New(const cv::Mat mat_gray_Img, bool bDirection, std::vector<pairPos> vec_List_PairPos, std::vector<std::vector<int>>& vec2d_Info_Data, std::vector<std::vector<cv::Point>>& vec2d_Pos_Data, int& vec2d_MaxSize)
{
	int res = 0;

	if (mat_gray_Img.empty() || mat_gray_Img.channels() != 1 || vec_List_PairPos.empty())
		return ER_ABORT;

	bool bflg_Direction = bDirection;		//0: Outside to Inside:由外向內搜尋; 1: Inside to Outside:由內向外搜尋

	int iFeature_TotalCnt = static_cast<int>(vec_List_PairPos.size());

	int iMax_Size = 0;

	std::vector<int> vec_data_count(iFeature_TotalCnt);
	std::vector< std::vector<int>> vec2d_data_intensity(iFeature_TotalCnt);
	std::vector< std::vector<cv::Point>> vec2d_data_position(iFeature_TotalCnt);


	for (int x = 0; x < iFeature_TotalCnt; x++) {

		LineIterator itor(mat_gray_Img, vec_List_PairPos[x].first, vec_List_PairPos[x].second);
		LineIterator it2 = itor;

		//record the value of intensity from image by two point of line.
		for (int i = 0; i < itor.count; i++, itor++)
		{
			uchar* ptr = *itor;
			vec2d_data_intensity[x].push_back(ptr[0]);
			vec2d_data_position[x].push_back(itor.pos());
		}

		//calculate the count of maxval(>0) from the Binary image.
		int iValidCnt_InLine = 0;
		for (int i = 0; i < it2.count; i++, it2++) {

			uchar* ptr = *it2;
			iValidCnt_InLine += (static_cast<int>(ptr[0]) != 0) ? 1 : 0;
		}
		vec_data_count.at(x) = iValidCnt_InLine;

		if (vec2d_data_intensity[x].size() > iMax_Size) {
			iMax_Size = vec2d_data_intensity[x].size();
		}

	}

	if (!bflg_Direction) {	//0: Outside to Inside:由外向內搜尋，將所有Intensity與Position數據反轉。

		for (int c = 0; c < iFeature_TotalCnt; c++) {

			std::reverse(vec2d_data_intensity[c].begin(), vec2d_data_intensity[c].end());
			std::reverse(vec2d_data_position[c].begin(), vec2d_data_position[c].end());
		}
	}

	vec2d_Info_Data = vec2d_data_intensity;
	vec2d_Pos_Data = vec2d_data_position;
	vec2d_MaxSize = iMax_Size;


	return res;
}


int CCVIPItem::Circle_Shape_Detector(const cv::Mat mat_gry_Imeg, seAnnulus roiAnnuls, bool bDirection, bool bPolarity, int iMinEdgeStrength, int iKernelSize, seCircle& resCircle)
{
	int res = 0;

	if (mat_gry_Imeg.empty() || mat_gry_Imeg.channels() != 1)
		return ER_ABORT;

	cv::Mat mat_gray;
	seAnnulus seRoi_Annuls = roiAnnuls;

	//The method refer from cvip::Auto_Canny(...).
	const float sigma = 0.33;
	//apply small amount of Gaussian blurring
	cv::GaussianBlur(mat_gry_Imeg, mat_gray, cv::Size(3, 3), 0, 0);
	//get the median value of the matrix
	cv:Mat Calcu = mat_gray.reshape(0, 1);	// spread Input Mat to single row
	std::vector<double> vecFromMat;
	Calcu.copyTo(vecFromMat); // Copy Input Mat to vector vecFromMat
	std::nth_element(vecFromMat.begin(), vecFromMat.begin() + vecFromMat.size() / 2, vecFromMat.end());
	double v = vecFromMat[vecFromMat.size() / 2];
	//generate the thresholds
	int thresh_lower = (int)std::max(0.0, (1, 0 - sigma) * v);
	int thresh_upper = (int)std::min(255.0, (1, 0 + sigma) * v);

	const int idp = 1;						//const value define by opencv
	const double dbRoundness = 30;			//const value define by opencv

	int iminRadius, imaxRadius = 0;
	int iksize			= iKernelSize;		//kernel size of default is 3 or 5
	int dbmaxThrsh		= thresh_upper;		// higher threshold forthe Canny edge detector.
	int iEdgeStrength	= iMinEdgeStrength;
	bool bflg_Direction	= bDirection;		//0: Outside to Inside:由外向內搜尋; 1: Inside to Outside:由內向外搜尋
	bool bflg_Polarity	= bPolarity;		//0: Rising Edges_由暗到亮的邊; 1: Falling Edges_由與亮到暗的邊

	if (bflg_Direction) {	//0: Outside to Inside:由外向內搜尋

		iminRadius = iEdgeStrength;	// static_cast<int>(seRoi_Annuls.dbRadius_Inner);
		imaxRadius = static_cast<int>(seRoi_Annuls.dbRadius_Outer);
	}
	else {		//1: Inside to Outside:由內向外搜尋

		iminRadius = static_cast<int>(seRoi_Annuls.dbRadius_Inner);
		imaxRadius = iEdgeStrength;
	}
	const double  dbminDist = iminRadius;	//	Minimum distance between the centers to the detected circles.

	std::vector<Vec3f> circles;

	cv::medianBlur(mat_gry_Imeg, mat_gray, iksize);

	cv::HoughCircles(mat_gray, circles, HOUGH_GRADIENT, idp,
				dbminDist,	
				dbmaxThrsh, dbRoundness, 
				iminRadius, imaxRadius	//(min_radius & max_radius) to detect larger circles   
	);

	for (size_t i = 0; i < circles.size(); i++)
	{
		cv::Vec3i c = circles[i];
		// circle center
		resCircle.cX = c[0];
		resCircle.cY = c[1];
		// circle outline
		resCircle.dbRadius = c[2];

		if (resCircle.dbRadius < iminRadius) {

			resCircle.dbRadius = iminRadius;
			resCircle.cX = roiAnnuls.cX;
			resCircle.cY = roiAnnuls.cY;			
		}

		//Point center = Point(c[0], c[1]);
		//// circle center
		//circle(src_color, center, 1, Scalar(0, 100, 100), 3, LINE_AA);
		//// circle outline
		//int radius = c[2];
		//circle(src_color, center, radius, Scalar(255, 0, 255), 3, LINE_AA);
	}

	return res;
}


int CCVIPItem::Circle_Shape_Detector(const cv::Mat mat_gry_Imeg, std::vector<std::vector<int>> vec2d_Info_Data, std::vector<std::vector<cv::Point>> vec2d_Pos_Data, bool bDirection, bool bPolarity, int iMinEdgeStrength, int iKernelSize, seCircle& resCircle)
{
	int res = 0;

	int iEdgeStrength = iMinEdgeStrength;
	bool bflg_Direction = bDirection;		//0: Outside to Inside:由外向內搜尋; 1: Inside to Outside:由內向外搜尋
	bool bflg_Polarity = bPolarity;			//0: Rising Edges_由暗到亮的邊; 1: Falling Edges_由與亮到暗的邊

	int thresh_upper = iEdgeStrength * 1;
	int thresh_lower = iEdgeStrength * -1;

	int iAllDataCnt = vec2d_Info_Data.size();

	typedef std::tuple< int, int, cv::Point > tuplesInfo;	//tuple :  { array_ID, sobel_X value, cv::Point(x,y) }
	std::vector< std::vector<tuplesInfo>> vec2d_info_Edge(iAllDataCnt);
	std::vector< cv::Point> edgePoints;

	cv::Mat matTmp = cv::Mat::zeros(mat_gry_Imeg.size(), CV_8UC1);

	// Polarity operator of Edge detection.
	auto myComp = [](int iPolarity, int iFirst, int iSecond) {
		if ( !iPolarity ) 
		{	
			//0: Rising Edges_由暗到亮的邊.
			return (iFirst == 0 && iSecond > 0);
		}
		else {
			//1: Falling Edges_由與亮到暗的邊.
			return (iFirst < 0 && iSecond == 0);
		}
	};

	// IOU operator 
	auto myIOU = [](cv::Rect bbox_1, cv::Rect bbox_2) {

		cv::Rect bbox_and;
		bbox_and = bbox_1 & bbox_2;

		if (0 == bbox_and.area()) {
			return 0.0;
		}

		double bbox_or = bbox_1.area() + bbox_2.area() - bbox_and.area();
		double bbox_iou = (bbox_and.area() * 1.0 / bbox_or);

		return bbox_iou;
	};

	//comparise operator
	auto mySort = [](std::pair<double, cv::Rect>a, std::pair<double, cv::Rect>b) {

		return a.first > b.first;
	};

	//filter out of range data by the thresh_upper and thresh_lower value.
	for (int c = 0; c < iAllDataCnt; c++) {

		if (!bflg_Direction) {

			std::reverse(vec2d_Info_Data[c].begin(), vec2d_Info_Data[c].end());
			std::reverse(vec2d_Pos_Data[c].begin(), vec2d_Pos_Data[c].end());
		}

		for (int i = 0; i < vec2d_Info_Data[c].size(); i++) {

			int iVal = (vec2d_Info_Data[c].at(i) > thresh_upper) ? 255 : ((vec2d_Info_Data[c].at(i) < thresh_lower) ? -255 : 0);

			cv::Point posVal(vec2d_Pos_Data[c].at(i));
			vec2d_info_Edge[c].push_back(std::make_tuple(i, iVal, posVal));
		}
	}


	// selection and drawing the edge data on the image     
	int iRadius = 1;
	int iThickness = 2;

	iAllDataCnt = vec2d_info_Edge.size();
	for (int c = 0; c < iAllDataCnt; c++) {

		for (int i = 1; i < vec2d_info_Edge[c].size(); i++) {

			int iPos1 = i - 1;
			int iPos2 = i;
			int iFirst = std::get< 1 >(vec2d_info_Edge[c].at(iPos1));	//tuple get<1>(  sobel_X value ) 
			int iSecond = std::get< 1 >(vec2d_info_Edge[c].at(iPos2));

			if ( myComp(bflg_Polarity, iFirst, iSecond) ) {

				int cX = std::get< 2 >(vec2d_info_Edge[c].at(iPos1)).x;	//tuple get<2>( cv::Point(x,y) )
				int cY = std::get< 2 >(vec2d_info_Edge[c].at(iPos1)).y;

				cv::Point posCenter(cX, cY);
				cv::circle(matTmp, posCenter, iRadius, cv::Scalar(255), iThickness);
				break;
			}
		}
	}

	//No any Edge information in the Image.
	if (0 == cv::countNonZero(matTmp)) {	
		return 0;
	}

	//Noise removal of Mropholoy processin 
	cv::Mat kernel_Morp_Cross = getStructuringElement(MORPH_CROSS, cv::Size(3, 3));
	cv::Mat kernel_Morp_Rect = getStructuringElement(MORPH_RECT, cv::Size(3, 3));
	int iterCnt = 3;
	cv::morphologyEx(matTmp, matTmp, cv::MORPH_ERODE, kernel_Morp_Rect, cv::Point(-1, -1), 1);
	cv::morphologyEx(matTmp, matTmp, cv::MORPH_CLOSE, kernel_Morp_Cross, cv::Point(-1, -1), iterCnt);

	//get edge information from image 
	uchar* ptr = matTmp.ptr<uchar>(0);
	for (int i = 0; i < matTmp.rows; i++) {

		ptr = matTmp.ptr<uchar>(i);
		for (int j = 0; j < matTmp.cols; j++) {

			if (ptr[j] != 0) {

				cv::Point edge = cv::Point(j, i);
				edgePoints.push_back(edge);
			}
		}
	}

	int cX(0), cY(0);
	double dbRadius(0.0);

	//Method_01: Circle Detection uses Edge contours.
	std::vector< std::vector< cv::Point> > contours;
	std::vector< cv::Vec4i> hierarchy;
	//cv::findContours(matTmp, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	cv::findContours(matTmp, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
	std::vector< std::vector< cv::Point> > contours_poly(contours.size());
	std::vector< cv::Point2f>center(contours.size());
	std::vector<float>radius(contours.size());
	
	typedef std::pair< double, cv::Rect > pairInfo;	//std::pair : { area, BoundingBox }
	std::vector<pairInfo> vec_BBoxInfo;
	double roundness(0.0), minVal(99.0);
	int igloden(0);
	int iLimit_Contours = contours.size();
	for (size_t i = 0; i < iLimit_Contours; i++)
	{
		if (contours[i].size() < 20) {
			continue;
		}

		approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true);
		minEnclosingCircle(contours_poly[i], center[i], radius[i]);

		cv::Rect tmpRect = boundingRect(cv::Mat(contours_poly[i]));		
		double tmpArea = contourArea(cv::Mat(contours[i]));

		vec_BBoxInfo.push_back( std::make_pair(tmpArea, tmpRect) );
	}


	//Method_02: Circle Detection uses RANSAC algorithm.
	int iterations = 100;
	std::vector<seCircle> bestCirclesRansac;
	RANSAC_CircleDetection(edgePoints, bestCirclesRansac, iterations);
	int iLimit_Ransca = bestCirclesRansac.size();
	for (int i = 0; i < iLimit_Ransca; ++i) {

		int cX = bestCirclesRansac[i].cX;
		int cY = bestCirclesRansac[i].cY;
		double radius = bestCirclesRansac[i].dbRadius;

		cv::Rect tmpRect;
		tmpRect.x = cX - radius;
		tmpRect.y = cY - radius;
		tmpRect.width = radius * 2;
		tmpRect.height = radius * 2;

		double tmpArea = radius * radius * CV_PI;
		vec_BBoxInfo.push_back(std::make_pair(tmpArea, tmpRect));
	}

	//Calculate and selection golden candidate from all rectangles.
	std::sort(vec_BBoxInfo.begin(), vec_BBoxInfo.end(), mySort);

	cv::Rect rectGolden;
	double maxArea = 0.0, tmpArea = 0.0;
	for (int c = 1; c < vec_BBoxInfo.size(); c++) {

		int iS = c - 1;
		int iE = c;

		tmpArea = myIOU(vec_BBoxInfo[iS].second, vec_BBoxInfo[iE].second);

		if (tmpArea > maxArea) {

			maxArea = tmpArea;
			rectGolden = vec_BBoxInfo[iS].second;
		}
	}

	//return results of circle shape info.
	cv::Point ptCenter((rectGolden.br() + rectGolden.tl()) * 0.5);
	cX = ptCenter.x;
	cY = ptCenter.y;
	dbRadius = rectGolden.width * 0.5;

	resCircle.cX = cX;
	resCircle.cY = cY;
	resCircle.dbRadius = dbRadius;

#ifdef _RexTY_DEBUG

	cv::Mat matColor = mat_gry_Imeg.clone();
	cv::cvtColor(matColor, matColor, COLOR_GRAY2BGR);

	cv::Mat matColor_Center = mat_gry_Imeg.clone();
	cv::cvtColor(matColor_Center, matColor_Center, COLOR_GRAY2BGR);

	circle(matColor_Center, cv::Point(resCircle.cX, resCircle.cY), 2, cv::Scalar(255, 0, 0), 4);
	circle(matColor_Center, cv::Point(resCircle.cX, resCircle.cY), resCircle.dbRadius, cv::Scalar(255, 0, 0), 4);

	RNG rng(12345);
	//drawing RANSCA info.
	//Mat drawing_RANSAC = Mat::zeros(matTmp.size(), CV_8UC3);
	Mat drawing_RANSAC = matColor.clone();
	for (int i = 0; i < iLimit_Ransca; ++i) {

		int cX = bestCirclesRansac[i].cX;
		int cY = bestCirclesRansac[i].cY;
		double radius = bestCirclesRansac[i].dbRadius;

		circle(drawing_RANSAC, cv::Point(cX, cY), radius, cv::Scalar(255, 0, 0), 4);	
		circle(drawing_RANSAC, cv::Point(cX, cY), 2, cv::Scalar(255, 0, 0), 2);


		//circle(matColor, cv::Point(cX, cY), radius, cv::Scalar(255, 0, 0), 4);

		//cv::Scalar rngColor = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//circle(matColor_Center, cv::Point(cX, cY), 2, rngColor, 2);
	}


	//drawing Edge contours info.
	//cv::Mat drawing_Contours = cv::Mat::zeros(matTmp.size(), CV_8UC3);
	cv::Mat drawing_Contours = matColor.clone();
	for (size_t i = 0; i < contours.size(); i++)
	{
		//drawContours(drawing_Contours, contours_poly, (int)i, cv::Scalar(0, 255, 0), 1, 8, vector<Vec4i>(), 0, Point());
		circle(drawing_Contours, center[i], (int)radius[i], cv::Scalar(0, 0, 255), 2, 8, 0);
		circle(drawing_Contours, center[i], 2, cv::Scalar(0, 0, 255), 2);


		//circle(matColor, center[i], (int)radius[i], cv::Scalar(0, 255, 0), 4);

		//cv::Scalar rngColor = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//circle(matColor_Center, center[i], 2, rngColor, 2);
	}

	//cv::imshow("matColor", matColor);
	//cv::imshow("matColor_Center", matColor_Center);
	cv::imshow("drawing_Contours", drawing_Contours);
	cv::imshow("drawing_RANSAC", drawing_RANSAC);
	cv::imshow("matTmp", matTmp);
	cv::waitKey(0);
	cv::destroyAllWindows();

#endif

	return res;
}


int CCVIPItem::Circle_Shape_Detector_New(const cv::Mat mat_gry_Imeg, int vec2d_MaxSize, std::vector<std::vector<float>> vec2d_Info_Data, std::vector<std::vector<cv::Point>> vec2d_Pos_Data, bool bDirection, bool bPolarity, int iMinEdgeStrength, int iKernelSize, seCircle& resCircle, std::vector<sePoint>& vecPos_Circle)
{
	int res = 0;

	int iFeature_TotalCnt = vec2d_Info_Data.size();
	int iFeature_MaxSize = vec2d_MaxSize;

	int iEdgeStrength = iMinEdgeStrength;
	bool bflg_Direction = bDirection;		//0: Outside to Inside:由外向內搜尋; 1: Inside to Outside:由內向外搜尋
	bool bflg_Polarity = bPolarity;			//0: Rising Edges_由暗到亮的邊; 1: Falling Edges_由與亮到暗的邊


	cv::Mat matTmp = cv::Mat::zeros(mat_gry_Imeg.size(), CV_8UC1);
	cv::Mat matVector = cv::Mat::zeros(iFeature_TotalCnt, iFeature_MaxSize, CV_8U);	//(rows, cols)


	//Noise removal of Mropholoy processin
	cv::Mat kernel_Morp_Rect = getStructuringElement(MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));
	cv::Mat kernel_Morp_Cross = getStructuringElement(MORPH_CROSS, cv::Size(3, 3), cv::Point(-1, -1));
	cv::Mat kernel_Morp_Ellips = getStructuringElement(MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(-1, -1));
	int iterCnt = 3;


	typedef std::tuple< int, int, cv::Point > tuplesInfo;	//tuple :  { array_ID, sobel_X value, cv::Point(x,y) }
	std::vector< std::vector<tuplesInfo>> vec2d_info_Edge(iFeature_TotalCnt);
	std::vector< cv::Point> edgePoints;

	//////      //////      //////      //////      //////      //////      //////      
	// # lamble function --> . -->
	// Polarity operator of Edge detection.
	auto myComp = [](int iPolarity, int iFirst, int iSecond) {
		if (!iPolarity)
		{
			//0: Rising Edges_由暗到亮的邊.
			return (iFirst == 0 && iSecond > 0);
		}
		else {
			//1: Falling Edges_由與亮到暗的邊.
			//return (iFirst < 0 && iSecond == 0);
			return (iFirst > 0 && iSecond == 0);
		}
	};

	// IOU operator 
	auto myIOU = [](cv::Rect bbox_1, cv::Rect bbox_2) {

		cv::Rect bbox_and;
		bbox_and = bbox_1 & bbox_2;

		if (0 == bbox_and.area()) {
			return 0.0;
		}

		double bbox_or = bbox_1.area() + bbox_2.area() - bbox_and.area();
		double bbox_iou = (bbox_and.area() * 1.0 / bbox_or);

		return bbox_iou;
	};

	//comparise operator
	auto mySort = [](std::pair<double, cv::Rect>a, std::pair<double, cv::Rect>b) {

		return a.first > b.first;
	};

	// # lamble function <-- . <--
	//////      //////      //////      //////      //////      //////      //////      


	// # //// # //// # //// # //// # //// # ////
	// Algorithm ---> start()
	// # //// # //// # //// # //// # //// # ////

	//filter out of range data by the thresh_upper and thresh_lower value.
	for (int c = 0; c < iFeature_TotalCnt; c++) {

		//if (!bflg_Direction) {	//0: Outside to Inside:由外向內搜尋，將所有Intensity與Position數據反轉。

		//	std::reverse(vec2d_Info_Data[c].begin(), vec2d_Info_Data[c].end());
		//	std::reverse(vec2d_Pos_Data[c].begin(), vec2d_Pos_Data[c].end());
		//}

		//Threshold
		for (int j = 0; j < vec2d_Info_Data[c].size(); ++j) {

			// 小於 threshold value 一律設置為 0
			int iVal = (vec2d_Info_Data[c][j] < iEdgeStrength) ? 0 : 255;

			matVector.at<char>(c, j) = iVal;

			cv::Point posVal(vec2d_Pos_Data[c].at(j));
			vec2d_info_Edge[c].push_back(std::make_tuple(j, iVal, posVal));
		}

	}


#ifdef _RexTY_DEBUG

	if (!matVector.empty()) {
		imwrite("C:/Users/USER/Downloads/01_matVector.png", matVector);
	}

#endif


	// selection and drawing the edge data on the image     
	int iRadius = 1;
	int iThickness = 2;

	for (int c = 0; c < iFeature_TotalCnt; c++) {

		for (int i = 1; i < vec2d_info_Edge[c].size(); i++) {

			int iPos1 = i - 1;
			int iPos2 = i;
			int iFirst = std::get< 1 >(vec2d_info_Edge[c].at(iPos1));	//tuple get<1>(  sobel_X value ) 
			int iSecond = std::get< 1 >(vec2d_info_Edge[c].at(iPos2));

			if (myComp(bflg_Polarity, iFirst, iSecond)) // bflg_Polarity --> 0: Rising Edges_由暗到亮的邊; 1: Falling Edges_由與亮到暗的邊
			{
				int iPos_Final = 0;
				if (bflg_Direction) {		// bflg_Direction --> 0: Outside to Inside:由外向內搜尋; 1: Inside to Outside:由內向外搜尋

					if (bflg_Polarity) {	// bflg_Polarity --> 0: Rising Edges_由暗到亮的邊; 1: Falling Edges_由與亮到暗的邊
						iPos_Final = iPos1;
					}
					else {
						iPos_Final = iPos2;
					}
				}
				else {

					if (bflg_Polarity) {	// bflg_Polarity --> 0: Rising Edges_由暗到亮的邊; 1: Falling Edges_由與亮到暗的邊
						iPos_Final = iPos1;
					}
					else {
						iPos_Final = iPos2;
					}
				}

				int cX = std::get< 2 >(vec2d_info_Edge[c].at(iPos_Final)).x;	//tuple get<2>( cv::Point(x,y) )
				int cY = std::get< 2 >(vec2d_info_Edge[c].at(iPos_Final)).y;

				cv::Point posCenter(cX, cY);
				cv::circle(matTmp, posCenter, iRadius, cv::Scalar(255), iThickness);

				sePoint posC;
				posC.x = cX;
				posC.y = cY;
				vecPos_Circle.push_back(posC);
				break;
			}
		}
	}

	//No any Edge information in the Image.
	if (0 == cv::countNonZero(matTmp)) {
		return 0;
	}


#ifdef _RexTY_DEBUG

	cv::imshow("matTmp", matTmp);
	cv::waitKey(0);
	cv::destroyAllWindows();

#endif


	// morphology_method_01
	cv::morphologyEx(matTmp, matTmp, cv::MORPH_ERODE, kernel_Morp_Rect, cv::Point(-1, -1), 1);
	cv::morphologyEx(matTmp, matTmp, cv::MORPH_CLOSE, kernel_Morp_Cross, cv::Point(-1, -1), iterCnt);

	// morphology_method_02
	//cv::morphologyEx(matTmp, matTmp, cv::MORPH_ERODE, kernel_Morp_Rect, cv::Point(-1, -1), 2);
	////cv::morphologyEx(matTmp, matTmp, cv::MORPH_CLOSE, kernel_Morp_Cross, cv::Point(-1, -1), iterCnt);
	//cv::morphologyEx(matTmp, matTmp, cv::MORPH_OPEN, kernel_Morp_Rect);


#ifdef _RexTY_DEBUG

	cv::imshow("matTmp", matTmp);
	cv::waitKey(0);
	cv::destroyAllWindows();

	if (!matTmp.empty()) {
		cv::imwrite("C:/Users/USER/Downloads/02_matTmp.png", matTmp);
	}

#endif


#ifdef _RexTY_DEBUG

	cv::Mat matVector_Color;
	cv::cvtColor(matVector, matVector_Color, COLOR_GRAY2RGB);

	for (int c = 0; c < iFeature_TotalCnt; c++) {

		//Threshold
		for (int j = 0; j < vec2d_Pos_Data[c].size(); ++j) {

			int cX = vec2d_Pos_Data[c].at(j).x;
			int cY = vec2d_Pos_Data[c].at(j).y;

			if (matTmp.at<char>(cY, cX) != 0) {

				cv::Point posVal(j, c);
				cv::circle(matVector_Color, posVal, 1, cv::Scalar(0, 255, 0), 1);
			}
		}
	}

	if (!matVector_Color.empty()) {
		imwrite("C:/Users/USER/Downloads/03_matVector_Color.png", matVector_Color);
	}

#endif




	int iCnt_NonZore = cv::countNonZero(matTmp);
	//No any Edge information in the Image.
	if (0 == cv::countNonZero(matTmp)) {
		return 0;
	}

	//get edge information from image 
	uchar* ptr = matTmp.ptr<uchar>(0);
	for (int i = 0; i < matTmp.rows; i++) {

		ptr = matTmp.ptr<uchar>(i);
		for (int j = 0; j < matTmp.cols; j++) {

			if (ptr[j] != 0) {

				cv::Point edge = cv::Point(j, i);
				edgePoints.push_back(edge);
			}
		}
	}

	int cX(0), cY(0);
	double dbRadius(0.0);

	//Method_01: Circle Detection uses Edge contours.
	std::vector< std::vector< cv::Point> > contours;
	std::vector< cv::Vec4i> hierarchy;
	//cv::findContours(matTmp, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	cv::findContours(matTmp, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
	std::vector< std::vector< cv::Point> > contours_poly(contours.size());
	std::vector< cv::Point2f>center(contours.size());
	std::vector<float>radius(contours.size());

	typedef std::pair< double, cv::Rect > pairInfo;	//std::pair : { area, BoundingBox }
	std::vector<pairInfo> vec_BBoxInfo;
	double roundness(0.0), minVal(99.0);
	int igloden(0);
	int iLimit_Contours = contours.size();
	for (size_t i = 0; i < iLimit_Contours; i++)
	{
		if (contours[i].size() < 20) {
			continue;
		}

		approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
		minEnclosingCircle(contours_poly[i], center[i], radius[i]);

		cv::Rect tmpRect = boundingRect(cv::Mat(contours_poly[i]));
		double tmpArea = contourArea(cv::Mat(contours[i]));

		vec_BBoxInfo.push_back(std::make_pair(tmpArea, tmpRect));
	}


	//Method_02: Circle Detection uses RANSAC algorithm.
	double dbConfidence = 0.98;
	double dbCorrect_rate = 0.6;
	int iFit_use_num = 3;
	// 計算出需要的樣本數iter_num，使得置信度水平為confidence，在使用樣本數為 fit_use_num 的情況下正確率不低於 correct_rate。
	int iter_num = int(log(1 - dbConfidence) / log(1 - pow(dbCorrect_rate, iFit_use_num)));

	int iterations = iter_num;	// 100;
	std::vector<seCircle> bestCirclesRansac;
	RANSAC_CircleDetection(edgePoints, bestCirclesRansac, iterations);
	int iLimit_Ransca = bestCirclesRansac.size();
	for (int i = 0; i < iLimit_Ransca; ++i) {

		int cX = bestCirclesRansac[i].cX;
		int cY = bestCirclesRansac[i].cY;
		double radius = bestCirclesRansac[i].dbRadius;

		cv::Rect tmpRect;
		tmpRect.x = cX - radius;
		tmpRect.y = cY - radius;
		tmpRect.width = radius * 2;
		tmpRect.height = radius * 2;

		double tmpArea = radius * radius * CV_PI;
		vec_BBoxInfo.push_back(std::make_pair(tmpArea, tmpRect));
	}


	//////////////////////////////////////////////////////////////////////////////
	//Method_03: Circle Detection uses Least sqares  algorithm.
	seCircle bestCirclesLeastSquares;
	LeastSquares_CircleDetection(edgePoints, bestCirclesLeastSquares);

	cX = bestCirclesLeastSquares.cX;
	cY = bestCirclesLeastSquares.cY;
	iRadius = round(bestCirclesLeastSquares.dbRadius);

	cv::Rect tmpRect;
	tmpRect.x = cX - iRadius;
	tmpRect.y = cY - iRadius;
	tmpRect.width = iRadius * 2;
	tmpRect.height = iRadius * 2;

	double area = iRadius * iRadius * CV_PI;
	vec_BBoxInfo.push_back(std::make_pair(area, tmpRect));

	//////////////////////////////////////////////////////////////////////////////



	//Calculate and selection golden candidate from all rectangles.
	std::sort(vec_BBoxInfo.begin(), vec_BBoxInfo.end(), mySort);

	cv::Rect rectGolden;
	double maxArea = 0.0, tmpArea = 0.0;
	for (int c = 1; c < vec_BBoxInfo.size(); c++) {

		int iS = c - 1;
		int iE = c;

		tmpArea = myIOU(vec_BBoxInfo[iS].second, vec_BBoxInfo[iE].second);

		if (tmpArea > maxArea) {

			maxArea = tmpArea;
			rectGolden = vec_BBoxInfo[iS].second;
		}
	}

	//return results of circle shape info.
	cv::Point ptCenter((rectGolden.br() + rectGolden.tl()) * 0.5);
	cX = ptCenter.x;
	cY = ptCenter.y;
	dbRadius = rectGolden.width * 0.5;

	resCircle.cX = cX;
	resCircle.cY = cY;
	resCircle.dbRadius = dbRadius;


#ifdef _RexTY_DEBUG

	cv::Mat matColor = mat_gry_Imeg.clone();
	cv::cvtColor(matColor, matColor, COLOR_GRAY2BGR);

	cv::Mat matColor_Center = mat_gry_Imeg.clone();
	cv::cvtColor(matColor_Center, matColor_Center, COLOR_GRAY2BGR);

	circle(matColor_Center, cv::Point(resCircle.cX, resCircle.cY), 2, cv::Scalar(255, 0, 0), 4);
	circle(matColor_Center, cv::Point(resCircle.cX, resCircle.cY), resCircle.dbRadius, cv::Scalar(255, 0, 0), 4);

	RNG rng(12345);
	//drawing RANSCA info.
	//Mat drawing_RANSAC = Mat::zeros(matTmp.size(), CV_8UC3);
	Mat drawing_RANSAC = matColor.clone();
	for (int i = 0; i < iLimit_Ransca; ++i) {

		int cX = bestCirclesRansac[i].cX;
		int cY = bestCirclesRansac[i].cY;
		double radius = bestCirclesRansac[i].dbRadius;

		circle(drawing_RANSAC, cv::Point(cX, cY), radius, cv::Scalar(255, 0, 0), 4);
		circle(drawing_RANSAC, cv::Point(cX, cY), 2, cv::Scalar(255, 0, 0), 2);


		//circle(matColor, cv::Point(cX, cY), radius, cv::Scalar(255, 0, 0), 4);

		//cv::Scalar rngColor = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//circle(matColor_Center, cv::Point(cX, cY), 2, rngColor, 2);
	}

	Mat drawing_LeastSquare = matColor.clone();
	cX = bestCirclesLeastSquares.cX;
	cY = bestCirclesLeastSquares.cY;
	iRadius = round(bestCirclesLeastSquares.dbRadius);
	circle(drawing_LeastSquare, cv::Point(cX, cY), iRadius, cv::Scalar(255, 255, 0), 4);
	circle(drawing_LeastSquare, cv::Point(cX, cY), 2, cv::Scalar(255, 255, 0), 2);


	//drawing Edge contours info.
	//cv::Mat drawing_Contours = cv::Mat::zeros(matTmp.size(), CV_8UC3);
	cv::Mat drawing_Contours = matColor.clone();
	for (size_t i = 0; i < contours.size(); i++)
	{
		//drawContours(drawing_Contours, contours_poly, (int)i, cv::Scalar(0, 255, 0), 1, 8, vector<Vec4i>(), 0, Point());
		circle(drawing_Contours, center[i], (int)radius[i], cv::Scalar(0, 0, 255), 2, 8, 0);
		circle(drawing_Contours, center[i], 2, cv::Scalar(0, 0, 255), 2);


		//circle(matColor, center[i], (int)radius[i], cv::Scalar(0, 255, 0), 4);

		//cv::Scalar rngColor = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//circle(matColor_Center, center[i], 2, rngColor, 2);
	}

	//cv::imshow("matColor", matColor);
	//cv::imshow("matColor_Center", matColor_Center);
	cv::imshow("drawing_Contours", drawing_Contours);
	cv::imshow("drawing_RANSAC", drawing_RANSAC);
	cv::imshow("drawing_LeastSquare", drawing_LeastSquare);

	cv::imshow("matTmp", matTmp);

	imwrite("C:/Users/USER/Downloads/04_FindCireclue.png", drawing_Contours);


	cv::waitKey(0);
	cv::destroyAllWindows();

#endif




	return res;
}


int CCVIPItem::RANSAC_CircleDetection(std::vector<cv::Point> edgePoints, std::vector<seCircle>& bestCircles, int iterations)
{

	int res = 0;

	//Define three points to define a circle
	cv::Point A, B, C, D;

	//Define slopes, intercepts between points
	double slope_AB, slope_BC, intercept_AB, intercept_BC;

	//Define midpoints
	cv::Point midpt_AB, midpt_BC;

	//Define slopes, intercepts midpoints perpendicular
	double slope_midptAB, slope_midptBC, intercept_midptAB, intercept_midptBC;

	RNG random;
	cv::Point center;
	double radius;
	double circumference;
	seCircle circleFound;

	for (int i = 0; i < iterations; i++) {

		//Select three points at random among edgePoints vector
		A = edgePoints[random.uniform((int)0, (int)edgePoints.size())];
		B = edgePoints[random.uniform((int)0, (int)edgePoints.size())];
		C = edgePoints[random.uniform((int)0, (int)edgePoints.size())];

		//Calculate midpoints
		midpt_AB = (A + B) * 0.5;
		midpt_BC = (B + C) * 0.5;

		//Calculate slopes and intercepts
		slope_AB = (B.y - A.y) / (B.x - A.x + 0.000000001);
		intercept_AB = A.y - slope_AB * A.x;
		slope_BC = (C.y - B.y) / (C.x - B.x + 0.000000001);
		intercept_BC = C.y - slope_BC * C.x;

		//Calculate perpendicular slopes and intercepts
		slope_midptAB = -1.0 / slope_AB;
		slope_midptBC = -1.0 / slope_BC;
		intercept_midptAB = midpt_AB.y - slope_midptAB * midpt_AB.x;
		intercept_midptBC = midpt_BC.y - slope_midptBC * midpt_BC.x;

		//Calculate intersection of perpendiculars to find center of circle and radius
		double centerX = (intercept_midptBC - intercept_midptAB) / (slope_midptAB - slope_midptBC);
		double centerY = slope_midptAB * centerX + intercept_midptAB;
		center = cv::Point(centerX, centerY);
		cv::Point diffradius = center - A;
		radius = sqrt(diffradius.x * diffradius.x + diffradius.y * diffradius.y);
		circumference = 2.0 * CV_PI * radius;

		std::vector<int> onCircle;
		std::vector<int> notOnCircle;
		int radiusThreshold = 3;

		//Find edgePoints that fit on circle radius
		for (int i = 0; i < edgePoints.size(); i++) {
			cv::Point diffCenter = edgePoints[i] - center;
			double distanceToCenter = sqrt(diffCenter.x * diffCenter.x + diffCenter.y * diffCenter.y);
			if (abs(distanceToCenter - radius) < radiusThreshold) {
				onCircle.push_back(i);
			}
			else {
				notOnCircle.push_back(i);
			}
		}

		//If number of edgePoints more than circumference, we found a correct circle
		if ((double)onCircle.size() >= circumference)
		{
			circleFound.cX = center.x;
			circleFound.cY = center.y;
			circleFound.dbRadius = radius;

			bestCircles.push_back(circleFound);

			//remove edgePoints if circle found (only keep non-voting edgePoints)
			std::vector<cv::Point> toKeep;
			for (int i = 0; i < (int)notOnCircle.size(); i++)
			{
				toKeep.push_back(edgePoints[notOnCircle[i]]);
			}
			edgePoints.clear();
			edgePoints = toKeep;
		}

		//stop iterations when there is not enough edgePoints
		if (edgePoints.size() < 100) {
			break;
		}
	}

	return res;
}



int  CCVIPItem::LeastSquares_CircleDetection(std::vector<cv::Point> edgePoints, seCircle& bestCircles)
{
	int res = 0;

	double sumX = 0, sumY = 0;
	double sumXX = 0, sumYY = 0, sumXY = 0;
	double sumXXX = 0, sumXXY = 0, sumXYY = 0, sumYYY = 0;

	for (int i = 0; i < edgePoints.size(); i++) {
	
		cv::Point posDot = edgePoints[i];

		sumX += posDot.x;
		sumY += posDot.y;
		sumXX += posDot.x * posDot.x;
		sumYY += posDot.y * posDot.y;
		sumXY += posDot.x * posDot.y;
		sumXXX += posDot.x * posDot.x * posDot.x;
		sumXXY += posDot.x * posDot.x * posDot.y;
		sumXYY += posDot.x * posDot.y * posDot.y;
		sumYYY += posDot.y * posDot.y * posDot.y;
	}

	int pCount = edgePoints.size();
	double M1 = pCount * sumXY - sumX * sumY;
	double M2 = pCount * sumXX - sumX * sumX;
	double M3 = pCount * (sumXXX + sumXYY) - sumX * (sumXX + sumYY);
	double M4 = pCount * sumYY - sumY * sumY;
	double M5 = pCount * (sumYYY + sumXXY) - sumY * (sumXX + sumYY);

	double a = (M1 * M5 - M3 * M4) / (M2 * M4 - M1 * M1);
	double b = (M1 * M3 - M2 * M5) / (M2 * M4 - M1 * M1);
	double c = -(a * sumX + b * sumY + sumXX + sumYY) / pCount;

	double xCenter = -0.5 * a;
	double yCenter = -0.5 * b;
	double radius = 0.5 * sqrt(a * a + b * b - 4 * c);

	seCircle circleFound;
	circleFound.cX = xCenter;
	circleFound.cY = yCenter;
	circleFound.dbRadius = radius;

	bestCircles = circleFound;

	return res;
}



int CCVIPItem::RANSAC_FitLine(std::vector<cv::Point> edgePoints, int iDistance, double dbConfidence, double dbCorrect_rate, int iFit_use_num, cv::Vec4f& vec4LineRet)
{
	int ret = 0;

	int distance = iDistance;
	int fit_use_num = iFit_use_num;
	double confidence = dbConfidence;
	double correct_rate = dbCorrect_rate;

	// 計算出需要的樣本數iter_num，使得置信度水平為confidence，在使用樣本數為 fit_use_num 的情況下正確率不低於 correct_rate。
	int iter_num = int(log(1 - confidence) / log(1 - pow(correct_rate, fit_use_num)));

	//RNG random;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, edgePoints.size() - 1);

	int inliers_num = 0;
	std::vector<int> inliers;

	for (int i = 0; i < iter_num; i++) {

		std::vector<cv::Point2f> points;
		cv::Vec4f line = cv::Vec4f();

		points.push_back( edgePoints[dis(gen)] );	// point A = edgePoints[random.uniform((int)0, (int)edgePoints.size()-1)];
		points.push_back( edgePoints[dis(gen)] );	// point B = edgePoints[random.uniform((int)0, (int)edgePoints.size()-1)];

		cv::fitLine(points, line, cv::DIST_L2, 0, 0.01, 0.01);

		double vx = line[0], vy = line[1], x0 = line[2], y0 = line[3];

		if (0.0 == vx && 0.0 == vy) {
			continue;
		}

		cv::Point2f p0(x0, y0);
		std::vector<cv::Point> p1_list = edgePoints;
		std::vector<double> vecDistance_array;

		int tmp_inliers_num = 0;
		std::vector<int> tmp_inliers;

		//# 求cos(θ)
		double cos_theta = vx / sqrt(vx * vx + vy * vy);  

		//# 計算點與直線的距離
		for (int i = 0; i < p1_list.size(); i++) {

			double dbkeyVal = std::abs((p1_list[i].x - p0.x) * vy / vx + p0.y - p1_list[i].y) * cos_theta;
			vecDistance_array.push_back(dbkeyVal);
		}

		for (int i = 0; i < p1_list.size(); i++) {

			//if (vecDistance_array.at<float>(i) <= distance) {
			if (vecDistance_array[i] <= distance) {

				tmp_inliers_num++;
				tmp_inliers.push_back(1);
			}
			else {

				tmp_inliers.push_back(0);
			}
		}

		if (inliers_num < tmp_inliers_num) { //:  # 如果當前這次抽樣的局內項更大，那就採用當前這一次

			if (!inliers.empty()) {
				inliers.clear();
			}

			inliers_num = tmp_inliers_num;

			//inliers = tmp_inliers;
			inliers.assign(tmp_inliers.begin(), tmp_inliers.end());
		}

	}

	//# 之前兩個點的擬合只是為了排除局外項，再用所有局內項擬合一次，結果更精確
	std::vector<cv::Point> tmp_pts;
	for (int i = 0; i < edgePoints.size(); i++) {

		if (inliers[i] ) {
			tmp_pts.push_back(edgePoints[i]);
		}
	}

	cv::Vec4f line_ret = cv::Vec4f();
	cv::fitLine(tmp_pts, line_ret, cv::DIST_L2, 0, 0.01, 0.01);

	//double ret_vx = line_ret[0], ret_vy = line_ret[1], ret_x0 = line_ret[2], ret_y0 = line_ret[3];
	//printf("Final Ret :\n\t ret_vx(%5.3f), ret_vy(%5.3f), ret_x0(%5.3f), ret_y0(%5.3f)\n", ret_vx, ret_vy, ret_x0, ret_y0);

	vec4LineRet = line_ret;

	return ret;
}


int CCVIPItem::myGetAppPath(std::string& strAppPath)
{
	int res = 0;

#if defined(_MSC_VER)
	
	char* buf = _getcwd(nullptr, 0);

#elif defined(__GNUC__)

	char* buf = getcwd(nullptr, 0);

#endif	

	char* p = nullptr;

	if (!buf)
	{
		perror("_getcwd error, No current working directory.\n");
		return ER_ABORT;
	}

	// make sure the drive letter is capital
	if (strlen(buf) > 1 && buf[1] == ':')
	{
		if (buf[0] == 'c') {
			buf[0] = 'd';
		}
		buf[0] = toupper(buf[0]);
	}
	for (p = buf; *p; ++p)
	{
		if (*p == '\\')
		{
			*p = '/';
		}
	}

	strAppPath.assign(buf, strlen(buf));

	if (buf)
		free(buf);


//#elif defined(__GNUC__)
//
//
//	string dir = string(".");
//	vector<string> files = vector<string>();
//
//	CGeneralFunc::getdir(dir, files);
//
//	strAppPath.assign(files.begin(), files.end());
//
//
//#endif

	return res;
}


int CCVIPItem::myCreateDirectory(const std::string folder)
{
	int res = 0;

	std::string folder_builder;
	std::string sub;
	sub.reserve(folder.size());

	for (auto it = folder.begin(); it != folder.end(); ++it)
	{
		//cout << *(folder.end()-1) << endl;
		const char c = *it;
		sub.push_back(c);
		if (c == '\\' || it == folder.end() - 1)
		{
#if defined(_MSC_VER)

			folder_builder.append(sub);
			if (0 != ::_access(folder_builder.c_str(), 0))
			{
				// this folder not exist
				if (0 != ::_mkdir(folder_builder.c_str()))
				{
					// create failed
					return ER_ABORT;
				}
			}
			 
#elif  defined(__GNUC__)

			folder_builder.append(sub);
			if (0 != ::access(folder_builder.c_str(), 0))
			{
				// this folder not exist
				if (0 != ::mkdir(folder_builder.c_str(), 0777))
				{
					// create failed
					return ER_ABORT;
				}
			}

#endif


			sub.clear();
		}
	}

	return res;
}


int CCVIPItem::SaveTestImage(std::string strFunctionname, std::string strImageName, const LPImageInfo aryImageDataBuf, std::string& strFullPath)
{

#if( !_Enb_DebugMode)
	return ER_OK;
#endif

	int res = 0;

	if (strFunctionname.empty() || strImageName.empty() || nullptr == aryImageDataBuf->pbImgBuf)
	{
		return ER_ABORT;
	}

	std::string strPath_CurApp;
	std::string strPath_Folder;
	std::string strPath_FileName;

	//get current working directory .
	if (myGetAppPath(strPath_CurApp) < ER_OK) {
		return ER_ABORT;
	}

	//careate folder.
	strPath_Folder = strPath_CurApp + "/" + strFunctionname;
	if (myCreateDirectory(strPath_Folder) < ER_OK) {
		return ER_ABORT;
	}

	//Date and Time.
	time_t timer = time(0);
	tm ltm{};	// = localtime(&now);

#if defined(__unix__)
	localtime_r(&timer, &ltm);
#elif defined(_MSC_VER)
	localtime_s(&ltm, &timer);
#else
	static std::mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	ltm = *std::localtime(&timer);
#endif

	int yy	= ltm.tm_year + 1900;
	int mm	= ltm.tm_mon + 1;
	int dd	= ltm.tm_mday;
	int hour= ltm.tm_hour;
	int min	= ltm.tm_min;
	int sec	= ltm.tm_sec;

	char szTmp[2048] = {""};
	//sprintf_s(szTmp, "%s/%s_%02d%02d%02d_%02d%02d.png", 
	//	strPath_Folder.c_str(),
	//	strImageName.c_str(),
	//	yy, mm, dd, 
	//	hour, min);
	CGeneralFunc::format(szTmp, strlen(szTmp), "%s/%s_%02d%02d%02d_%02d%02d.png",
		strPath_Folder.c_str(),
		strImageName.c_str(),
		yy, mm, dd,
		hour, min);
	strPath_FileName.assign(szTmp, strlen(szTmp));

	//LPImageInfo convert to CV::Mat.  
	cv::Mat matDst;
	if (Uint8ToCvMat(aryImageDataBuf, matDst) < ER_OK) {

		return ER_ABORT;
	}

	strFullPath = strPath_FileName;

	cv::imwrite(strPath_FileName.c_str(), matDst);

	return res;
}


int CCVIPItem::SaveTestImage(std::string strFunctionname, std::string strImageName, const cv::Mat matImageDataBuf, std::string& strFullPath)
{

#if( !_Enb_DebugMode)
	return ER_OK;
#endif

	int res = 0;

	if (strFunctionname.empty() || strImageName.empty() || matImageDataBuf.empty())
	{
		return ER_ABORT;
	}

	std::string strPath_CurApp;
	std::string strPath_Folder;
	std::string strPath_FileName;

	//get current working directory .
	if (myGetAppPath(strPath_CurApp) < ER_OK) {
		return ER_ABORT;
	}

	//careate folder.
	strPath_Folder = strPath_CurApp + "/" + strFunctionname;
	if (myCreateDirectory(strPath_Folder) < ER_OK) {
		return ER_ABORT;
	}

	//Date and Time.
	time_t timer = time(0);
	tm ltm{};	// = localtime(&now);

#if defined(__unix__)
	localtime_r(&timer, &ltm);
#elif defined(_MSC_VER)
	localtime_s(&ltm, &timer);
#else
	static std::mutex mtx;
	std::lock_guard<std::mutex> lock(mtx);
	ltm = *std::localtime(&timer);
#endif

	int yy = ltm.tm_year + 1900;
	int mm = ltm.tm_mon + 1;
	int dd = ltm.tm_mday;
	int hour = ltm.tm_hour;
	int min = ltm.tm_min;
	int sec = ltm.tm_sec;

	char szTmp[2048] = { "" };
	//sprintf_s(szTmp, "%s/%s_%02d%02d%02d_%02d%02d.png",
	//	strPath_Folder.c_str(),
	//	strImageName.c_str(),
	//	yy, mm, dd,
	//	hour, min);
	CGeneralFunc::format(szTmp, strlen(szTmp), "%s/%s_%02d%02d%02d_%02d%02d.png",
		strPath_Folder.c_str(),
		strImageName.c_str(),
		yy, mm, dd,
		hour, min);
	strPath_FileName.assign(szTmp, strlen(szTmp));

	strFullPath = strPath_FileName;

	cv::imwrite(strPath_FileName.c_str(), matImageDataBuf);

	return res;
}


int CCVIPItem::Auto_Canny(cv::Mat image, cv::Mat& output, int& thresh_lower, int& thresh_upper)
{
	int res = 0;

	const float sigma = 0.33;
	//convert to grey colour space
	if (1 != image.channels()) {
		cv::cvtColor(image, output, cv::COLOR_BGR2GRAY);
	}
	else {
		output = image.clone();
	}

	//apply small amount of Gaussian blurring
	cv::GaussianBlur(output, output, cv::Size(3, 3), 0, 0);

	//get the median value of the matrix
	cv:Mat Calcu = output.reshape(0, 1);	// spread Input Mat to single row
	std::vector<double> vecFromMat;
	Calcu.copyTo(vecFromMat); // Copy Input Mat to vector vecFromMat
	std::nth_element(vecFromMat.begin(), vecFromMat.begin() + vecFromMat.size() / 2, vecFromMat.end());
	double v = vecFromMat[vecFromMat.size() / 2];

	//generate the thresholds
	int lower = (int)std::max(0.0, (1, 0 - sigma) * v);
	int upper = (int)std::min(255.0, (1, 0 + sigma) * v);

	//apply canny operator
	thresh_lower = lower;
	thresh_upper = upper;
	cv::Canny(output, output, lower, upper, 3);

	return res;
}


int CCVIPItem::Trigonometry_Angle(cv::Point ptCenter, cv::Point pt1, cv::Point pt2, double& dbAngle)
{
	int res = 0;

	double dx1 = (pt1.x - ptCenter.x);
	double dy1 = (pt1.y - ptCenter.y);
	double dx2 = (pt2.x - ptCenter.x);
	double dy2 = (pt2.y - ptCenter.y);

	double angle_line = (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);

	dbAngle = acos(angle_line) * 180.0 / CV_PI;

	return res;
}


// 將角度轉換為弧度
double CCVIPItem::to_radians(double degrees) {
	return degrees * CV_PI / 180.0;
}

// 以中心點旋轉二維座標點
cv::Point2f CCVIPItem::ptRotatePt2f(Point2f ptInput, Point2f ptOrg, double dAngle)
{
	double radians = to_radians(dAngle);
	double dWidth = ptOrg.x * 2;
	double dHeight = ptOrg.y * 2;
	double dY1 = dHeight - ptInput.y, dY2 = dHeight - ptOrg.y;

	double dX = (ptInput.x - ptOrg.x) * cos(radians) - (dY1 - ptOrg.y) * sin(radians) + ptOrg.x;
	double dY = (ptInput.x - ptOrg.x) * sin(radians) + (dY1 - ptOrg.y) * cos(radians) + dY2;

	dY = -dY + dHeight;
	return Point2f((float)dX, (float)dY);
}


int CCVIPItem::RotateImage(const cv::Mat& src, cv::Mat& dst, const double angle, const int mode)
{
	//mode = 0 ,Keep the original image size unchanged
	//mode = 1, Change the original image size to fit the rotated scale, padding with zero

	if (src.empty())
	{
		std::cout << "Damn, the input image is empty!\n";
		return -1;
	}

	if (mode == 0)
	{
		cv::Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
		cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
		cv::warpAffine(src, dst, rot, src.size());//the original size
	}
	else {

		double alpha = -angle * CV_PI / 180.0;//convert angle to radian format 

		cv::Point2f srcP[3];
		cv::Point2f dstP[3];
		srcP[0] = cv::Point2f(0, src.rows);
		srcP[1] = cv::Point2f(src.cols, 0);
		srcP[2] = cv::Point2f(src.cols, src.rows);

		//rotate the pixels
		for (int i = 0; i < 3; i++)
			dstP[i] = cv::Point2f(srcP[i].x * cos(alpha) - srcP[i].y * sin(alpha), srcP[i].y * cos(alpha) + srcP[i].x * sin(alpha));
		double minx, miny, maxx, maxy;
		minx = std::min(std::min(std::min(dstP[0].x, dstP[1].x), dstP[2].x), float(0.0));
		miny = std::min(std::min(std::min(dstP[0].y, dstP[1].y), dstP[2].y), float(0.0));
		maxx = std::max(std::max(std::max(dstP[0].x, dstP[1].x), dstP[2].x), float(0.0));
		maxy = std::max(std::max(std::max(dstP[0].y, dstP[1].y), dstP[2].y), float(0.0));

		int w = maxx - minx;
		int h = maxy - miny;

		//translation
		for (int i = 0; i < 3; i++)
		{
			if (minx < 0)
				dstP[i].x -= minx;
			if (miny < 0)
				dstP[i].y -= miny;
		}

		cv::Mat warpMat = cv::getAffineTransform(srcP, dstP);
		cv::warpAffine(src, dst, warpMat, cv::Size(w, h));//extend size

	}//end else

	return 0;
}




int CGeneralFunc::format(char* dest, size_t destlen, const char* fmt, ...)
{
	int res = 0;

	va_list args;
	va_start(args, fmt);

#if defined(_MSC_VER)
	_vsnprintf_s(dest, destlen, destlen, fmt, args);
#else
	vsnprintf(dest, destlen, fmt, args);
#endif

	va_end(args);

	return res;
}


//#if defined(__GNUC__)
//
//int CGeneralFunc::getdir(std::string dir, std::vector<std::string>& files)
//{
//	DIR* dp;
//	struct dirent* dirp;
//	if ((dp = opendir(dir.c_str())) == nullptr) {
//		cout << "Error(" << errno << ") opening " << dir << endl;
//		return errno;
//	}
//
//	while ((dirp = readdir(dp)) != nullptr) {
//		files.push_back(string(dirp->d_name));
//	}
//	closedir(dp);
//	return 0;
//}
//
//#endif //#if defined(__GNUC__)
