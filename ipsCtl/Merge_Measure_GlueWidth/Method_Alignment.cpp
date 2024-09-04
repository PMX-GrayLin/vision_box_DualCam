#include <opencv2/opencv.hpp>
#include "Method_Alignment.h"



using namespace std;
using namespace cv;

CMethod_Alignment::CMethod_Alignment()
	:m_dbScore(0.0)
{}

CMethod_Alignment::~CMethod_Alignment()
{}


int CMethod_Alignment::Align_Calibration_SetParameter(LPImageInfo pCalibPatterntIn, seRect roiRect_01, seRect roiRect_02)
{
	int res = 0;

	Align_Image_Calibration();

	if (Uint8ToCvMat_GrayScalar(pCalibPatterntIn, m_CalibPattern_grayImg) < ER_OK) {
		return ER_ABORT;
	}
	if (m_CalibPattern_grayImg.channels() != 1) {
		cv::cvtColor(m_CalibPattern_grayImg, m_CalibPattern_grayImg, COLOR_RGB2GRAY);
	}


	m_rect_01 = roiRect_01;	//NoNeed
	m_rect_02 = roiRect_02;	//NoNeed


	return res;
}

int CMethod_Alignment::Align_Calibration_SetParameter(const cv::Mat m_CalibPattern_grayImg, seRect roiRect_01, seRect roiRect_02)
{
	int res = 0;

	Align_Image_Calibration();

	if (m_CalibPattern_grayImg.empty()) {
		return ER_ABORT;
	}

	if (m_CalibPattern_grayImg.channels() != 1) {
		cv::cvtColor(m_CalibPattern_grayImg, m_CalibPattern_grayImg, COLOR_RGB2GRAY);
	}


	m_rect_01 = roiRect_01;	//NoNeed
	m_rect_02 = roiRect_02;	//NoNeed


	return res;
}



int CMethod_Alignment::Align_Calibration_TestIt()
{
	int res = 0;


	res = Algo_CalcuPixelCnt_FullImage(m_CalibPattern_grayImg, m_PixelCount_X, m_PixelCount_Y);

	//res = Algo_CalcuPixelCnt_TwoPoints(m_CalibPattern_grayImg, m_rect_01, m_rect_02, mPixelCount_X);


	return res;
}


int CMethod_Alignment::Align_Calibration_GetResult(LPImageCalibration_Results myResultSet)
{
	int res = 0;

	seImageCalibration_Results myRes;

	myRes.dbPixelCount_X = m_PixelCount_X;
	myRes.dbPixelCount_Y = m_PixelCount_Y;

	myRes.circle_01 = m_Circle_01;
	myRes.circle_02 = m_Circle_02;
	myRes.circle_03 = m_Circle_03;

	*myResultSet = myRes;

	return res;
}



int CMethod_Alignment::Align_Pattern_SetParameter(seRect roiSearch, const LPImageInfo pTemplatIn)
{
	int res = 0;

	Align_Pattern_Clear();

	if (Uint8ToCvMat_GrayScalar(pTemplatIn, m_template_grayImg) < ER_OK) {
		return ER_ABORT;
	}
	if (m_template_grayImg.channels() != 1) {
		cv::cvtColor(m_template_grayImg, m_template_grayImg, COLOR_RGB2GRAY);
	}

	m_rectSearch = roiSearch;

	//New Algorithm.
	m_tpl_matchpattern.setDstFile(m_template_grayImg);	//Load image of template.
	res = m_tpl_matchpattern.LearnPattern();	//Learn Pattern feature of Template.


	return res;
}


int CMethod_Alignment::Align_Pattern_TestIt(const LPImageInfo pIn)
{
	int res = 0;
	cv::Mat ref_src_gray, ref_locate_gray;
	if(Uint8ToCvMat_GrayScalar(pIn, ref_src_gray) < ER_OK) {
		return ER_ABORT;
	}

	//crop image from image by the search box setting.
	ref_locate_gray = ref_src_gray(
		cv::Rect(
			cv::Point(m_rectSearch.left, m_rectSearch.top),
			cv::Point(m_rectSearch.right, m_rectSearch.bottom)
		)
	);

	//cropped image need the shift position for tamplate match calculate.
	sePoint posShift; 
	posShift.x = m_rectSearch.left;
	posShift.y = m_rectSearch.top;

	//New Algorithm.
	m_tpl_matchpattern.setSrcFile(ref_locate_gray);	// Load image of source image of crop size. 

	m_tpl_matchpattern.Match();

	//Old Algorithm.
	//res = Algo_PatternMatch(ref_locate_gray, m_template_grayImg, posShift, m_FMarkBox);



	return res;
}

int CMethod_Alignment::Align_Pattern_TestIt(const cv::Mat ref_src_gray)
{
	int res = 0;

	cv::Mat ref_locate_gray;

	if (ref_src_gray.empty()) {
		return ER_ABORT;
	}

	//crop image from image by the search box setting.
	ref_locate_gray = ref_src_gray(
		cv::Rect(
			cv::Point(m_rectSearch.left, m_rectSearch.top),
			cv::Point(m_rectSearch.right, m_rectSearch.bottom)
		)
	);

	//cropped image need the shift position for tamplate match calculate.
	sePoint posShift;
	posShift.x = m_rectSearch.left;
	posShift.y = m_rectSearch.top;

	//New Algorithm.
	m_tpl_matchpattern.setSrcFile(ref_locate_gray);	// Load image of source image of crop size. 

	m_tpl_matchpattern.Match();

	//Old Algorithm.
	//Algo_PatternMatch(ref_locate_gray, m_template_grayImg, posShift, m_FMarkBox);


	return res;
}



int CMethod_Alignment::Algo_PatternMatch(cv::Mat& ref_gray, cv::Mat& tpl_gray, sePoint posShift, seBoundingBox& seBBox)
{
	int res = 0;

	int match_method = cv::TM_SQDIFF;
	cv::matchTemplate(ref_gray, tpl_gray, m_matched_grayImg, match_method);

	double minValue, maxValue;
	cv::Point minLocP;	cv::Point maxLocP;
	cv::Point matchLoc;

	minMaxLoc(m_matched_grayImg, &minValue, &maxValue, &minLocP, &maxLocP, cv::Mat());

	if (match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
	{
		matchLoc = minLocP;
	}
	else
	{
		matchLoc = maxLocP;
	}

	//crop image from image by the search box setting.
	cv::Mat imgLocate = ref_gray(
		cv::Rect(minLocP.x, minLocP.y, tpl_gray.cols, tpl_gray.rows)
	);

	m_dbScore = Similarity_CalcuSSIM(tpl_gray, imgLocate) * 100.0;

	////find angle algorithm of Log Polar FFT;
	double dbAngle_1(0.0), dbAngle_2(0.0);
	res = Algo_CalcuAngle(tpl_gray, dbAngle_1);
	if (res) {
		return -1;
	}
	res = Algo_CalcuAngle(imgLocate, dbAngle_2);
	if (res) {
		return -1;
	}
	double dbRes_Angle = dbAngle_2 - dbAngle_1;

	matchLoc.x += posShift.x;
	matchLoc.y += posShift.y;

	double db_W = m_rectSearch.left - matchLoc.x;
	double db_H = m_rectSearch.top - matchLoc.y;
	double dbAngle = dbRes_Angle;

	seBBox.dbAngle = dbAngle;
	seBBox.cX = matchLoc.x + tpl_gray.cols / 2;	// +rectRatate.center.x;
	seBBox.cY = matchLoc.y + tpl_gray.rows / 2;	// +rectRatate.center.y;

	seBBox.rectBox.left = matchLoc.x;
	seBBox.rectBox.top = matchLoc.y;
	seBBox.rectBox.right = matchLoc.x + tpl_gray.cols;
	seBBox.rectBox.bottom = matchLoc.y + tpl_gray.rows;
	seBBox.rectBox.width = tpl_gray.cols;
	seBBox.rectBox.height = tpl_gray.rows;

	
	return res;
}


int CMethod_Alignment::Algo_CalcuAngle(cv::Mat& img_Source, double& dbAngle)
{
	int res = 0;
	cv::Mat img_gray(img_Source), img_Color(img_Source);

	if (1 != img_gray.channels() ) {

		cvtColor(img_gray, img_gray, cv::COLOR_BGR2GRAY); // Convert to gray
	}

	if (3 != img_Color.channels()) {

		cvtColor(img_Color, img_Color, cv::COLOR_GRAY2RGB); // Convert to gray
	}

	threshold(img_gray, img_gray, 125, 255, cv::THRESH_BINARY_INV); // Threshold the gray

	int largest_area = 0;
	int largest_contour_index = 0;

	typedef std::tuple<int, double, cv::Rect>tuples;
	std::vector<tuples> vecTuples;

	vector<vector<Point>> contours; // Vector for storing contour
	vector<Vec4i> hierarchy;

	findContours(img_gray, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE); // Find the contours in the image

	Scalar color(0, 0, 255);

	for (int i = 0; i < contours.size(); i++) // Iterate through each contour
	{
		//calc and save all area value of contours.
		double contArea = contourArea(contours[i]); // Find the area of contour
		vecTuples.push_back(make_tuple(i, contArea, boundingRect(contours[i])));

		//cv::drawContours(img_Color, contours, i, cv::Scalar(255, 0, 255), 2);
	}

	std::sort(vecTuples.begin(), vecTuples.end(),
		[](std::tuple<int, double, cv::Rect>a, std::tuple<int, double, cv::Rect>b)
		{
			return std::get< 1 >(a) > std::get< 1 >(b);
		}
	);
	
	if (0 == contours.size()) {

		dbAngle = -999.0;
		return -1;
	}

	cv::Rect rect1 = std::get< 2 >(vecTuples[1]);
	cv::Rect rect2 = std::get< 2 >(vecTuples[0]);

	cv::Point ptCenter((rect2.br() + rect2.tl()) * 0.5);
	cv::Point pt1( (rect1.br() + rect1.tl()) * 0.5);
	cv::Point pt2( ptCenter.x + 20, ptCenter.y );

	Trigonometry_Angle(ptCenter, pt1, pt2, dbAngle);

	//cv::imshow("img_Color", img_Color);
	//cv::waitKey(0);
	//cv::destroyAllWindows();

	return res;
}



#define Enb_PatternCalibration (0) 

int CMethod_Alignment::Algo_CalcuPixelCnt_TwoPoints(cv::Mat& img_Source, seRect rect_01, seRect rect_02, double& dbPixelCount_X, double& dbPixelCount_Y)
{
	int res = 0;
	cv::Mat img_gray(img_Source), img_Color(img_Source);
	cv::Mat binimg_locate_gray_01, binimg_locate_gray_02;

	if (1 != img_gray.channels()) {

		cvtColor(img_gray, img_gray, cv::COLOR_BGR2GRAY); // Convert to gray
	}


	if (3 != img_Color.channels()) {

		cvtColor(img_Color, img_Color, cv::COLOR_GRAY2RGB); // Convert to gray
	}


#if(Enb_PatternCalibration)
	//cv::imshow("img_gray_Before", img_gray);
	//cv::waitKey(0);
	//cv::destroyAllWindows();
#endif

	//threshold(img_gray, img_gray, 190, 255, cv::THRESH_BINARY_INV); // Threshold the gray
	threshold(img_gray, img_gray, 0, 255, cv::THRESH_OTSU | cv::THRESH_BINARY_INV); // Threshold the gray

#if(Enb_PatternCalibration)
	//cv::imshow("img_gray_After", img_gray);
	//cv::waitKey(0);
	//cv::destroyAllWindows();
#endif

	//crop image from image by the seRect box setting.
	binimg_locate_gray_01 = img_gray(
		cv::Rect(
			cv::Point(rect_01.left, rect_01.top),
			cv::Point(rect_01.right, rect_01.bottom)
		)
	);

	binimg_locate_gray_02 = img_gray(
		cv::Rect(
			cv::Point(rect_02.left, rect_02.top),
			cv::Point(rect_02.right, rect_02.bottom)
		)
	);


#if(Enb_PatternCalibration)
	//cv::imshow("img_gray", img_gray);
	//cv::imshow("binimg_locate_gray_01", binimg_locate_gray_01);
	//cv::imshow("binimg_locate_gray_02", binimg_locate_gray_02);
	//cv::waitKey(0);
	//cv::destroyAllWindows();
#endif


	cv::Point posCenter[2] = { cv::Point() };
	double dbRadius[2] = { 0.0 };

	for (int idx = 0; idx < 2; idx++) {

		cv::Mat matTmp; //shallow copy 
		if (idx == 0) {
			matTmp = binimg_locate_gray_01;
		}
		else{
			matTmp = binimg_locate_gray_02;
		}

		cv::Mat matTmpShow;
		cvtColor(matTmp, matTmpShow, cv::COLOR_GRAY2RGB); // Convert to gray



		vector< vector< cv::Point > >contours;
		findContours(matTmp, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

		vector< cv::Point2f >centers(contours.size());
		vector<float>radius(contours.size());

		for (int i = 0; i < contours.size(); i++) {

#if(Enb_PatternCalibration)
			//cv::drawContours(img_Color, contours, i, cv::Scalar(255, 0, 255), 2);
			cv::drawContours(matTmpShow, contours, i, cv::Scalar(0, 0, 255), 2);
#endif

			minEnclosingCircle(contours[i], centers[i], radius[i]);

#if(Enb_PatternCalibration)

			//circle(img_Color, centers[i], radius[i], cv::Scalar(255, 0, 255), 2);
			circle(matTmpShow, centers[i], radius[i], cv::Scalar(0, 0, 255), 2);
			cv::imshow("matTmpShow", matTmpShow);
			cv::waitKey(0);
			cv::destroyAllWindows();
#endif

			posCenter[idx] = centers[i];
			dbRadius[idx] = radius[i];

			break;
		}

	}

	int sX[2], sY[2];
	sX[0] = rect_01.right + posCenter[0].x;
	sY[0] = rect_01.top + posCenter[0].y;
	
	sX[1] = rect_02.right + posCenter[1].x;
	sY[1] = rect_02.top + posCenter[1].y;

	dbPixelCount_X = pow((sX[0] - sX[1]), 2);
	dbPixelCount_Y = pow((sY[0] - sY[1]), 2);;


	m_Circle_01.cX = sX[0];
	m_Circle_01.cY = sY[0];
	m_Circle_01.dbRadius = dbRadius[0];

	m_Circle_02.cX = sX[1];
	m_Circle_02.cY = sY[1];
	m_Circle_02.dbRadius = dbRadius[1];


#if(Enb_PatternCalibration)
	cv::imshow("img_Color", img_Color);
	cv::waitKey(0);
	cv::destroyAllWindows();
#endif
	return res;
}




int CMethod_Alignment::Algo_CalcuPixelCnt_FullImage(cv::Mat& img_Source, double& dbPixelCount_X, double& dbPixelCount_Y)
{
	int res = 0;


	if (img_Source.empty()) {
		return ER_ABORT;
	}

	cv::Mat img_gray(img_Source), img_Color(img_Source);

	if (1 != img_gray.channels()) {

		cvtColor(img_gray, img_gray, cv::COLOR_BGR2GRAY); // Convert to gray
	}


	if (3 != img_Color.channels()) {

		cvtColor(img_Color, img_Color, cv::COLOR_GRAY2RGB); // Convert to gray
	}


	//threshold(img_gray, img_gray, 190, 255, cv::THRESH_BINARY_INV); // Threshold the gray
	threshold(img_gray, img_gray, 0, 255, cv::THRESH_OTSU | cv::THRESH_BINARY_INV); // Threshold the gray


#if(Enb_PatternCalibration)
	//cv::Mat matTmpShow;
	//cvtColor(img_gray, matTmpShow, cv::COLOR_GRAY2RGB); // Convert to gray
#endif

	vector< vector< cv::Point > >contours;
	findContours(img_gray, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

	vector< cv::Point2f >centers(contours.size());
	vector<float>radius(contours.size());

	typedef std::pair< int, double > pairInfo;	//std::pair : { area, BoundingBox }
	std::vector<pairInfo> vec_Distance;
	double dbX = 0.0, dbY = 0.0;
	double dbCX = img_Source.cols * 0.5;
	double dbCY = img_Source.rows * 0.5;
	double dbval = 0.0;


	for (int i = 0; i < contours.size(); i++) {


#if(Enb_PatternCalibration)
		//		cv::drawContours(matTmpShow, contours, i, cv::Scalar(0, 0, 255), 2);
#endif


		minEnclosingCircle(contours[i], centers[i], radius[i]);


#if(Enb_PatternCalibration)
		//		cv::circle(matTmpShow, centers[i], radius[i], cv::Scalar(0, 0, 255), 2);
#endif

		dbX = pow((centers[i].x - dbCX), 2);
		dbY = pow((centers[i].y - dbCY), 2);
		dbval = sqrt(dbX + dbY);

		vec_Distance.push_back(std::make_pair(i, dbval));

	}

#if(Enb_PatternCalibration)
	//cv::imshow("matTmpShow", matTmpShow);
	//cv::waitKey(0);
	//cv::destroyAllWindows();
#endif

	//comparise operator
	auto mySort = [](std::pair<int, double>a, std::pair<int, double>b) {

		return a.second < b.second;
	};


	std::sort(vec_Distance.begin(), vec_Distance.end(), mySort);


	double dbMax_X = -0.0, dbMax_Y = -0.0;
	double dbTmp = 0;
	for (int i = 1; i < 3; i++) {

		int id0 = vec_Distance[i - 1].first;
		int id1 = vec_Distance[i].first;

		dbTmp = sqrt(pow((centers[id0].x - centers[id1].x), 2));
		if (dbTmp > dbMax_X) {
			dbMax_X = dbTmp;
		}

		dbTmp = sqrt(pow((centers[id0].y - centers[id1].y), 2));
		if (dbTmp > dbMax_Y) {
			dbMax_Y = dbTmp;
		}
	}


	dbPixelCount_X = dbMax_X;
	dbPixelCount_Y = dbMax_Y;


	int id = vec_Distance[0].first;
	m_Circle_01.cX = centers[id].x;
	m_Circle_01.cY = centers[id].y;
	m_Circle_01.dbRadius = radius[id];

	id = vec_Distance[1].first;
	m_Circle_02.cX = centers[id].x;
	m_Circle_02.cY = centers[id].y;
	m_Circle_02.dbRadius = radius[id];

	id = vec_Distance[2].first;
	m_Circle_03.cX = centers[id].x;
	m_Circle_03.cY = centers[id].y;
	m_Circle_03.dbRadius = radius[id];


//<<

#if(Enb_PatternCalibration)
	//for (int i = 0; i < 3; i++) {
	//	int id = vec_Distance[i].first;
	//	cv::circle(matTmpShow, centers[id], radius[id], cv::Scalar(255, 0, 255), 2);
	//}

	//cv::imshow("Final_matTmpShow", matTmpShow);
	//cv::waitKey(0);
	//cv::destroyAllWindows();
#endif


	return res;
}




int CMethod_Alignment::Align_Pattern_GetResult(LPBoundingBox pFMarkBox, double* dbScore) //return info of bounding box and score
{
	int res = 0;
	int iOffset_X = 0, i0ffset_Y = 0;
	seBoundingBox seRet;

	iOffset_X = m_rectSearch.left;
	i0ffset_Y = m_rectSearch.top;

	m_tpl_matchpattern.getResult(m_retTargetMatch);
	
	*dbScore = (double)(m_retTargetMatch.dMatchScore * 100.0);

	seRet.cX = m_retTargetMatch.ptCenter.x + iOffset_X;
	seRet.cY = m_retTargetMatch.ptCenter.y + i0ffset_Y;

	seRet.dbAngle = m_retTargetMatch.dMatchedAngle;
	
	seRet.rectBox.left = m_retTargetMatch.ptLT.x + iOffset_X;
	seRet.rectBox.top = m_retTargetMatch.ptLT.y + i0ffset_Y;
	seRet.rectBox.right = m_retTargetMatch.ptRB.x + iOffset_X;
	seRet.rectBox.bottom = m_retTargetMatch.ptRB.y + i0ffset_Y;

	m_FMarkBox = seRet;
	*pFMarkBox = seRet;

	//*dbScore = m_dbScore;
	//try {

	//	*pFMarkBox = m_FMarkBox;
	//	//memcpy(pFMarkBox, &m_FMarkBox, sizeof(seBoundingBox));
	//}
	//catch (std::exception& e) {

	//	std::cout << "exception : return the BoundingBox info." << e.what() << std::endl;
	//}

	return res;
}


// Alignment mthod_Find Profile
int CMethod_Alignment::Align_FindProfile_SetParameter(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iSeletLineNo, int iKSize)
{
	int res = 0;

	if (Circle_IsProperSubset(roiSearch, roiMask) < ER_OK) {
		return ER_ABORT;
	}

	Align_FindProfile_Clear();

	m_circSearch = roiSearch;
	m_circMask = roiMask;

	m_bDirection = bDirection;	//The Search direction is --> 0: from Outside to Inside; 1: from Inside to Outside.
	m_bPolarity = bPolarity;	//The Polarity curve is --> 0: Rising Edges (dark to bright); 1: Falling Edges. (Bright to dark).

	m_iSelectLineNo = iSeletLineNo;

	m_stepSize = stepSize;
	m_iKSize = (0 == iKSize % 2) ? ((iKSize + 1 <= 3) ? 5 : (iKSize + 1)) : iKSize;

	return res;
}


int CMethod_Alignment::Align_FindProfile_TestIt(const LPImageInfo pIn, int iSelLineNo)
{
	int res = 0;

	cv::Mat matImg_gray;
	if (Uint8ToCvMat_GrayScalar(pIn, matImg_gray) < ER_OK) {
		return ER_ABORT;
	}

	//assign parameter to Annnulus struct. 
	seAnnulus roiAnnuls;
	roiAnnuls.cX = m_circMask.cX;
	roiAnnuls.cY = m_circMask.cY;
	roiAnnuls.dbRadius_Inner = m_circMask.dbRadius;
	roiAnnuls.dbRadius_Outer = m_circSearch.dbRadius;

	double dbAngle_start(((roiAnnuls.dbStartAngle < 0) || (roiAnnuls.dbStartAngle > 360)) ? 0 : roiAnnuls.dbStartAngle);
	double dbAngle_end(((roiAnnuls.dbEndAngle < 0) || (roiAnnuls.dbEndAngle > 360)) ? 360 : roiAnnuls.dbEndAngle);

	//dbAngle_start = min(dbAngle_start, dbAngle_end);
	//dbAngle_end = max(dbAngle_start, dbAngle_end);
	if (dbAngle_start > dbAngle_end) {
		std::swap(dbAngle_start, dbAngle_end);
	}

	double dbDegrees_Total = (dbAngle_end - dbAngle_start);
	double dbDegrees_Divided = (m_stepSize > dbDegrees_Total) ? dbDegrees_Total : m_stepSize;
	double dbDegrees_Section = round(dbDegrees_Total / dbDegrees_Divided);

	m_iSelectLineNo = (iSelLineNo > dbDegrees_Section) ? dbDegrees_Section :((iSelLineNo < 0) ? 0 : iSelLineNo);

	std::vector<pairPos> vec_List_PairPos;
	Annulus_Degrees_RangeSetting(roiAnnuls, m_stepSize, m_bDirection, vec_List_PairPos);
	std::vector<std::vector<int>> vec2d_tmp;

	//remove noise
	cv::GaussianBlur(matImg_gray, matImg_gray, cv::Size(m_iKSize, m_iKSize), 0, 0);

	Annulus_Degrees_GetInfoByLine(matImg_gray, vec_List_PairPos, vec2d_tmp);
	Algo_EdgeDetection_Sobel_X(vec2d_tmp, m_vec2d_val_intensity);

	return res;
}

int CMethod_Alignment::Align_FindProfile_TestIt(const cv::Mat srcImg_gray, int iSelLineNo)
{
	int res = 0;

	if (srcImg_gray.empty()) {
		return ER_ABORT;
	}

	cv::Mat matImg_gray = srcImg_gray.clone();

	//assign parameter to Annnulus struct. 
	seAnnulus roiAnnuls;
	roiAnnuls.cX = m_circMask.cX;
	roiAnnuls.cY = m_circMask.cY;
	roiAnnuls.dbRadius_Inner = m_circMask.dbRadius;
	roiAnnuls.dbRadius_Outer = m_circSearch.dbRadius;

	double dbAngle_start(((roiAnnuls.dbStartAngle < 0) || (roiAnnuls.dbStartAngle > 360)) ? 0 : roiAnnuls.dbStartAngle);
	double dbAngle_end(((roiAnnuls.dbEndAngle < 0) || (roiAnnuls.dbEndAngle > 360)) ? 360 : roiAnnuls.dbEndAngle);

	//dbAngle_start = min(dbAngle_start, dbAngle_end);
	//dbAngle_end = max(dbAngle_start, dbAngle_end);
	if (dbAngle_start > dbAngle_end) {
		std::swap(dbAngle_start, dbAngle_end);
	}

	double dbDegrees_Total = (dbAngle_end - dbAngle_start);
	double dbDegrees_Divided = (m_stepSize > dbDegrees_Total) ? dbDegrees_Total : m_stepSize;
	double dbDegrees_Section = round(dbDegrees_Total / dbDegrees_Divided);

	m_iSelectLineNo = (iSelLineNo > dbDegrees_Section) ? dbDegrees_Section : ((iSelLineNo < 0) ? 0 : iSelLineNo);

	std::vector<pairPos> vec_List_PairPos;
	Annulus_Degrees_RangeSetting(roiAnnuls, m_stepSize, m_bDirection, vec_List_PairPos);
	std::vector<std::vector<int>> vec2d_tmp;

	//remove noise
	cv::GaussianBlur(matImg_gray, matImg_gray, cv::Size(m_iKSize, m_iKSize), 0, 0);

	Annulus_Degrees_GetInfoByLine(matImg_gray, vec_List_PairPos, vec2d_tmp);
	Algo_EdgeDetection_Sobel_X(vec2d_tmp, m_vec2d_val_intensity);

	return res;
}



int CMethod_Alignment::Align_FindProfile_GetResult(seDetectCirle_Results& myResultSet) //return info of bounding box and 1d array data.
{
	int res = 0;

	seDetectCirle_Results tmpResult;

	int iID = m_iSelectLineNo;
	int iCount = static_cast<int>(m_vec2d_val_intensity[iID].size());

	tmpResult.vec_CntArryOut.push_back(iCount);
	tmpResult.vec_1DArrayOut.assign(m_vec2d_val_intensity[iID].begin(), m_vec2d_val_intensity[iID].end());

	myResultSet = tmpResult;

	return res;
}

int CMethod_Alignment::Algo_EdgeDetection_Sobel_X(std::vector<std::vector<int>> vec2d_In, std::vector<std::vector<int>>& vec2d_Out)
{
	if (vec2d_In.empty())
		return ER_ABORT;

	int res = 0;

	int iSize = vec2d_In.size();
	std::vector<std::vector<int>> vec2d_Tmp(iSize);

	int iNorm_x = 0;
	int iKernel_X[3] = { -1, 0, 1 };	//sobel kernel is [1 x 3] array;
	//int iKernel_X[3] = { 1, -4, 1 };	//Laplace kernel is [1 x 3] array;

	for (int c = 0; c < vec2d_In.size(); c++) {

		for (int x = 1; x < vec2d_In[c].size()-1; x++) {

			int iVal_1 = vec2d_In[c].at(x - 1) * iKernel_X[0];
			int iVal_2 = vec2d_In[c].at(x - 0) * iKernel_X[1];
			int iVal_3 = vec2d_In[c].at(x + 1) * iKernel_X[2];

			iNorm_x = iVal_1 + iVal_2 + iVal_3;
			//iNorm_x = abs(iVal_1 + iVal_2 + iVal_3);
			//iNorm_x = (iNorm_x > 255) ? 255 : (iNorm_x < 0) ? 0 : iNorm_x;

			vec2d_Tmp[c].push_back(iNorm_x);
		}
	}

	vec2d_Out = vec2d_Tmp;

	return res;
}


int CMethod_Alignment::Algo_EdgeDetection_Sobel_X(int iKernelSize, std::vector<std::vector<int>> vec2d_In, std::vector<std::vector<int>>& vec2d_Out)
{
	if (vec2d_In.empty())
		return ER_ABORT;

	int res = 0;

	cv::Mat matKernel;
	if (iKernelSize <= 3) {
		iKernelSize = 3;
		matKernel = (cv::Mat_<int>(1, iKernelSize) << -1, 0, 1);
	}
	else if (iKernelSize >= 5) {
		iKernelSize = 5;
		matKernel = (cv::Mat_<int>(1, iKernelSize) << -2, -1, 0, 1, 2);
	}

	int iSize = vec2d_In.size();
	std::vector<std::vector<int>> vec2d_Tmp(iSize);
	
	for (int c = 0; c < vec2d_In.size(); c++) {

		if (vec2d_In[c].empty()) {
			continue;
		}

		std::vector<float>vecValue(vec2d_In[c].begin(), vec2d_In[c].end());
		cv::Mat matVec_Value = cv::Mat(vecValue);
		matVec_Value = matVec_Value.reshape(1, 1);

		cv::Mat matRes;
		cv::filter2D(matVec_Value, matRes, matVec_Value.depth(), matKernel);

		vec2d_Tmp[c].assign(matRes.begin<float>(), matRes.end<float>());
	}

	vec2d_Out = vec2d_Tmp;

	return res;
}


int CMethod_Alignment::Algo_EdgeDetection_Sobel_X_New(int iKernelSize, std::vector<std::vector<int>> vec2d_In, std::vector<std::vector<float>>& vec2d_Out)
{
	if (vec2d_In.empty())
		return ER_ABORT;

	int res = 0;
	bool bflg_Polarity = m_bPolarity;			//0: Rising Edges_�ѷt��G����; 1: Falling Edges_�ѻP�G��t����

	cv::Mat matKernel;
	if (iKernelSize <= 3) {
		iKernelSize = 3;
		//matKernel = (cv::Mat_<int>(1, iKernelSize) << -1, 0, 1);
		matKernel = (cv::Mat_<int>(1, iKernelSize) << -2, 0, 2);
	}
	else if (iKernelSize >= 5) {
		iKernelSize = 5;
		matKernel = (cv::Mat_<int>(1, iKernelSize) << -2, -1, 0, 1, 2);
	}

	int iSize = vec2d_In.size();
	std::vector<std::vector<float>> vec2d_Tmp(iSize);

	for (int c = 0; c < vec2d_In.size(); c++) {

		if (vec2d_In[c].empty()) {
			continue;
		}

		std::vector<float>vecValue(vec2d_In[c].begin(), vec2d_In[c].end());
		cv::Mat matVec_Value = cv::Mat(vecValue);// .reshape(0, 1);
		matVec_Value = matVec_Value.reshape(0, 1);

		cv::Mat grad_Img, grad_Img2;
		int ddepth = matVec_Value.depth();

		// # padded the Mat  
		cv::Mat padded_matVec_Value;
		cv::copyMakeBorder(matVec_Value, padded_matVec_Value, 0, 0, 1, 1, cv::BORDER_REPLICATE);

		//cv::filter2D(matVec_Value, grad_Img, ddepth, matKernel);
		cv::filter2D(padded_matVec_Value, grad_Img, ddepth, matKernel);


		// # Rex -- > New -- > 	
		// # �̾���trising or falling�S�ʡA�h���t��׭�
		if (bflg_Polarity != 0) {

			grad_Img.copyTo(grad_Img2);
			grad_Img *= -1.0;
		}

		grad_Img = cv::max(grad_Img, cv::Scalar(0));

		// # float to uint8 for image display and for calculate after.
		cv::Mat normalize_Img;
		if (iKernelSize == 3) {
			normalize_Img = (grad_Img / 600) * 255;
		}
		else if (iKernelSize == 5) {
			normalize_Img = (grad_Img / 8000) * 255;
		}
		else if (iKernelSize == 7) {
			normalize_Img = (grad_Img / 113000) * 255;
		}
		else if (iKernelSize == 9) {
			normalize_Img = (grad_Img / 1640000) * 255;
		}
		else {
			double minVal(0.0), maxVal(0.0);
			cv::minMaxLoc(grad_Img, &minVal, &maxVal);
			normalize_Img = (grad_Img / maxVal) * 255;
		}
		// # Rex -- > New -- > 


		// # remove padded data and copy real data to vector.
		//vec2d_Tmp[c].assign(normalize_Img.begin<float>(), normalize_Img.end<float>());

		std::vector<float> subdata(normalize_Img.begin<float>(), normalize_Img.end<float>());	// �N�Ϲ��ƾڦs�x��V�q��
		vec2d_Tmp[c].assign(subdata.begin() + 1, subdata.end() - 1);				// �q�V�q������:1��:-1�������ƾ�

	}

	vec2d_Out = vec2d_Tmp;


#ifdef _RexTY_DEBUG

	std::fstream fS;
	fS.open("C:/Users/USER/Downloads/soble_lineprofile.csv", std::ios::out | std::ios::ate);

	for (int c = 0; c < vec2d_In.size(); c++) {

		fS << "ID: " << c << ", ";
		for (int i = 0; i < vec2d_Tmp[c].size(); i++) {

			fS << vec2d_Tmp[c][i] << ", ";

		}
		fS << endl;
	}

	fS.close();

#endif

	return res;
}



// Alignment mthod_Find Circle
int CMethod_Alignment::Align_DetectCircle_SetParameter(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize)
{
	int res = 0;

	if ( Circle_IsProperSubset(roiSearch, roiMask) < ER_OK) {
		return ER_ABORT;
	}

	Align_DetectCircle_Clear();

	m_circSearch = roiSearch;
	m_circMask = roiMask;

	m_bDirection = bDirection;	//The Search direction is --> 0: from Outside to Inside; 1: from Inside to Outside.
	m_bPolarity = bPolarity;	//The Polarity curve is --> 0: Rising Edges (dark to bright); 1: Falling Edges. (Bright to dark).

	m_iMinEdgeStrength = iMinEdgeStrength;

	m_stepSize = (stepSize >= 1) ? 1: stepSize;
	m_iKSize = (0 == iKSize % 2) ? ((iKSize + 1 <= 3) ? 3 : (iKSize + 1)) : iKSize;

	return res;
}


#define _DC_Method (2)

int CMethod_Alignment::Align_DetectCircle_TestIt(const LPImageInfo pIn)
{
	int res = 0;

	cv::Mat matImg_gray;
	if (Uint8ToCvMat_GrayScalar(pIn, matImg_gray) < ER_OK) {
		return ER_ABORT;
	}

	seAnnulus roiAnnuls;
	//count the total pixel of the image.
	int iTotalPixels = matImg_gray.rows * matImg_gray.cols;
	int iMegapixel = static_cast<int>(iTotalPixels / 10000);
	if (iMegapixel > 200) {
	
		m_dbResizeScale = 0.25;

		int new_W = matImg_gray.cols * m_dbResizeScale;
		int new_H = matImg_gray.rows * m_dbResizeScale;;
		cv::resize(matImg_gray, matImg_gray, cv::Size(new_W, new_H));

		//assign parameter to Annnulus struct. 
		//seAnnulus roiAnnuls;
		roiAnnuls.cX = m_circMask.cX * m_dbResizeScale;
		roiAnnuls.cY = m_circMask.cY * m_dbResizeScale;
		roiAnnuls.dbRadius_Inner = m_circMask.dbRadius * m_dbResizeScale;
		roiAnnuls.dbRadius_Outer = m_circSearch.dbRadius * m_dbResizeScale;

	}
	else {

		m_dbResizeScale = 1.0;

		//assign parameter to Annnulus struct. 
		//seAnnulus roiAnnuls;
		roiAnnuls.cX = m_circMask.cX;
		roiAnnuls.cY = m_circMask.cY;
		roiAnnuls.dbRadius_Inner = m_circMask.dbRadius;
		roiAnnuls.dbRadius_Outer = m_circSearch.dbRadius;
	}


#if (1 == _DC_Method)	//Pattrn match circle detection		


	seAnnulus Mask_Annulus;
	Mask_Annulus.cX	= m_iMinEdgeStrength;
	Mask_Annulus.cY = m_iMinEdgeStrength;
	Mask_Annulus.dbRadius_Outer = m_iMinEdgeStrength;
	int Mask_Size = m_iMinEdgeStrength * 2;
	cv::Mat	Mask_Img = cv::Mat::zeros(cv::Size(Mask_Size, Mask_Size), CV_8UC1);
	CreateMaskImg(Mask_Annulus, Mask_Img);
	if (!m_bPolarity) {
		cv::threshold(Mask_Img, Mask_Img, 128, 255, cv::THRESH_BINARY_INV);
	}

	seRect rectSearch;
	rectSearch.left		= roiAnnuls.cX - m_circSearch.dbRadius;
	rectSearch.top		= roiAnnuls.cY - m_circSearch.dbRadius;
	rectSearch.right	= roiAnnuls.cX + m_circSearch.dbRadius;
	rectSearch.bottom	= roiAnnuls.cY + m_circSearch.dbRadius;

	//crop image from source image by the search box setting.
	cv::Mat ref_locate_gray = matImg_gray(
		cv::Rect(
			cv::Point(rectSearch.left, rectSearch.top),
			cv::Point(rectSearch.right, rectSearch.bottom)
		)
	);
	//cropped image need the shift position for tamplate match calculate.
	sePoint posShift;
	posShift.x = rectSearch.left;
	posShift.y = rectSearch.top;

	Algo_PatternMatch(ref_locate_gray, Mask_Img, posShift, m_BoundBox_Circle);


#elif( 2 == _DC_Method)	//Contours circle detection_Method2

	std::vector<pairPos> vec_List_PairPos;
	Annulus_Degrees_RangeSetting(roiAnnuls, m_stepSize, m_bDirection, vec_List_PairPos);

	std::vector<std::vector<int>> vec2d_tmp;

	//remove noise
	cv::GaussianBlur(matImg_gray, matImg_gray, cv::Size(m_iKSize, m_iKSize),0,0);

	Annulus_Degrees_GetInfoByLine(matImg_gray, vec_List_PairPos, vec2d_tmp, m_vec2d_val_position);
	//Algo_EdgeDetection_Sobel_X(vec2d_tmp, m_vec2d_val_intensity);
	Algo_EdgeDetection_Sobel_X(m_iKSize, vec2d_tmp, m_vec2d_val_intensity);

	seCircle resCircle;
	Circle_Shape_Detector(matImg_gray, m_vec2d_val_intensity, m_vec2d_val_position, m_bDirection, m_bPolarity, m_iMinEdgeStrength, m_iKSize, resCircle);

	if (m_dbResizeScale != 1.0) {
	
		resCircle.cX = resCircle.cX * (1/ m_dbResizeScale);
		resCircle.cY = resCircle.cY * (1/ m_dbResizeScale);
		resCircle.dbRadius = resCircle.dbRadius * (1 / m_dbResizeScale);
	}

	CalcuBoundingBox(resCircle, m_BoundBox_Circle);


#else	// Hough circle detection.

	//crop target roi from source image to detecting circle. 
	int iH = matImg_gray.rows;
	int iW = matImg_gray.cols;
	int posL = 0, posT = 0;
	int posR = 0, posB = 0;
	posL = ((roiAnnuls.cX - roiAnnuls.dbRadius_Outer) < 0) ? 0 : (roiAnnuls.cX - roiAnnuls.dbRadius_Outer);
	posT = ((roiAnnuls.cY - roiAnnuls.dbRadius_Outer) < 0) ? 0 : (roiAnnuls.cY - roiAnnuls.dbRadius_Outer);
	posR = ((roiAnnuls.cX + roiAnnuls.dbRadius_Outer) > iW) ? iW : (roiAnnuls.cX + roiAnnuls.dbRadius_Outer);
	posB = ((roiAnnuls.cY + roiAnnuls.dbRadius_Outer) > iH) ? iH : (roiAnnuls.cY + roiAnnuls.dbRadius_Outer);

	cv::Rect rectROI(cv::Point(posL, posT), cv::Point(posR, posB));
	cv::Mat tmp_image = matImg_gray( rectROI );

	Circle_Shape_Detector(tmp_image, roiAnnuls, m_bDirection, m_bPolarity, m_iMinEdgeStrength, m_iKSize, m_Outline_Circle);

	//offset back to original position
	m_Outline_Circle.cX += posL;
	m_Outline_Circle.cY += posT;
	CalcuBoundingBox(m_Outline_Circle, m_BoundBox_Circle);


#endif	//_DC_Method


	return res;
}

int CMethod_Alignment::Align_DetectCircle_TestIt(const cv::Mat srcImg_gray)
{
	int res = 0;

	if (srcImg_gray.empty()) {
		return ER_ABORT;
	}

	cv::Mat matImg_gray = srcImg_gray.clone();

	seAnnulus roiAnnuls;
	//count the total pixel of the image.
	int iTotalPixels = matImg_gray.rows * matImg_gray.cols;
	int iMegapixel = static_cast<int>(iTotalPixels / 10000);
	if (iMegapixel > 200) {

		m_dbResizeScale = 0.25;

		int new_W = matImg_gray.cols * m_dbResizeScale;
		int new_H = matImg_gray.rows * m_dbResizeScale;;
		cv::resize(matImg_gray, matImg_gray, cv::Size(new_W, new_H));

		//assign parameter to Annnulus struct. 
		//seAnnulus roiAnnuls;
		roiAnnuls.cX = m_circMask.cX * m_dbResizeScale;
		roiAnnuls.cY = m_circMask.cY * m_dbResizeScale;
		roiAnnuls.dbRadius_Inner = m_circMask.dbRadius * m_dbResizeScale;
		roiAnnuls.dbRadius_Outer = m_circSearch.dbRadius * m_dbResizeScale;

	}
	else {

		m_dbResizeScale = 1.0;

		//assign parameter to Annnulus struct. 
		//seAnnulus roiAnnuls;
		roiAnnuls.cX = m_circMask.cX;
		roiAnnuls.cY = m_circMask.cY;
		roiAnnuls.dbRadius_Inner = m_circMask.dbRadius;
		roiAnnuls.dbRadius_Outer = m_circSearch.dbRadius;
	}


#if (1 == _DC_Method)	//Pattrn match circle detection		


	seAnnulus Mask_Annulus;
	Mask_Annulus.cX = m_iMinEdgeStrength;
	Mask_Annulus.cY = m_iMinEdgeStrength;
	Mask_Annulus.dbRadius_Outer = m_iMinEdgeStrength;
	int Mask_Size = m_iMinEdgeStrength * 2;
	cv::Mat	Mask_Img = cv::Mat::zeros(cv::Size(Mask_Size, Mask_Size), CV_8UC1);
	CreateMaskImg(Mask_Annulus, Mask_Img);
	if (!m_bPolarity) {
		cv::threshold(Mask_Img, Mask_Img, 128, 255, cv::THRESH_BINARY_INV);
	}

	seRect rectSearch;
	rectSearch.left = roiAnnuls.cX - m_circSearch.dbRadius;
	rectSearch.top = roiAnnuls.cY - m_circSearch.dbRadius;
	rectSearch.right = roiAnnuls.cX + m_circSearch.dbRadius;
	rectSearch.bottom = roiAnnuls.cY + m_circSearch.dbRadius;

	//crop image from source image by the search box setting.
	cv::Mat ref_locate_gray = matImg_gray(
		cv::Rect(
			cv::Point(rectSearch.left, rectSearch.top),
			cv::Point(rectSearch.right, rectSearch.bottom)
		)
	);
	//cropped image need the shift position for tamplate match calculate.
	sePoint posShift;
	posShift.x = rectSearch.left;
	posShift.y = rectSearch.top;

	Algo_PatternMatch(ref_locate_gray, Mask_Img, posShift, m_BoundBox_Circle);


#elif( 2 == _DC_Method)	//Contours circle detection_Method2


	/////////////////////////////////////////////////////////////////////
	// New
	/////////////////////////////////////////////////////////////////////

	std::vector<pairPos> temp_vec_List_PairPos;
	std::vector<std::vector<int>> tmp_vec2d_val_intensity;		//templat vectoe_Dtype Int
	std::vector<std::vector<float>> tmp_vec2d_F_val_intensity;	//templat vectoe_Dtype Float

	int tmp_vec2d_MaxSize = 0;
	seCircle resCircle;


	//1. �p��oAnnulus��Ƶ��c�����~��A��g�u�s�u�W��Position��T�C
	//1. Calculating the Position information on the radial line connecting the inner and outer circles in the Annulus data structure
	Annulus_Degrees_RangeSetting(roiAnnuls, m_stepSize, m_bDirection, temp_vec_List_PairPos);


	//2. Reduce Noise.
	// Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
	cv::GaussianBlur(matImg_gray, matImg_gray, cv::Size(m_iKSize, m_iKSize), 0, 0);


	//3. ���oAnnulus��Ƶ��c�����~��A��g�u�s�u�W��Pixel value��T
	//3. Retrieve the pixel value information on the radial lines within the inner and outer circles in the annulus data structure."�C
	// # Old
	//Annulus_Degrees_GetInfoByLine(matImg_gray, temp_vec_List_PairPos, tmp_vec2d_val_intensity, m_vec2d_val_position, tmp_vec2d_MaxSize);
	// # New --> . -->
	Annulus_Degrees_GetInfoByLine_New(matImg_gray, m_bDirection, temp_vec_List_PairPos, tmp_vec2d_val_intensity, m_vec2d_val_position, tmp_vec2d_MaxSize);

	// 4. Annuls_Feature Extraction
	// # Old 
	//Algo_EdgeDetection_Sobel_X(m_iKSize, tmp_vec2d_val_intensity, m_vec2d_val_intensity);
	// # New --> . -->
	Algo_EdgeDetection_Sobel_X_New(m_iKSize, tmp_vec2d_val_intensity, tmp_vec2d_F_val_intensity);

	// 5. Annulus_Search Algorithm.
	// # Old 
	//Circle_Shape_Detector(matImg_gray, m_vec2d_val_intensity, m_vec2d_val_position, m_bDirection, m_bPolarity, m_iMinEdgeStrength, m_iKSize, resCircle, m_vecPos_Circle);
	// # New --> . -->
	Circle_Shape_Detector_New(matImg_gray, tmp_vec2d_MaxSize, tmp_vec2d_F_val_intensity, m_vec2d_val_position, m_bDirection, m_bPolarity, m_iMinEdgeStrength, m_iKSize, resCircle, m_vecPos_Circle);


	if (m_dbResizeScale != 1.0) {

		for (int c = 0; c < m_vecPos_Circle.size(); c++) {

			m_vecPos_Circle[c].x = m_vecPos_Circle[c].x * (1 / m_dbResizeScale);
			m_vecPos_Circle[c].y = m_vecPos_Circle[c].y * (1 / m_dbResizeScale);
		}

		resCircle.cX = resCircle.cX * (1 / m_dbResizeScale);
		resCircle.cY = resCircle.cY * (1 / m_dbResizeScale);
		resCircle.dbRadius = resCircle.dbRadius * (1 / m_dbResizeScale);
	}

	CalcuBoundingBox(resCircle, m_BoundBox_Circle);

	/////////////////////////////////////////////////////////////////////
	// Old 
	/////////////////////////////////////////////////////////////////////
	/* // ==>

	std::vector<pairPos> vec_List_PairPos;
	std::vector<std::vector<int>> vec2d_tmp;
	seCircle resCircle;

	//1. �p��oAnnulus��Ƶ��c�����~��A��g�u�s�u�W��Position��T�C
	Annulus_Degrees_RangeSetting(roiAnnuls, m_stepSize, m_bDirection, vec_List_PairPos);


	//2. Reduce Noise.
	// Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
	cv::GaussianBlur(matImg_gray, matImg_gray, cv::Size(m_iKSize, m_iKSize), 0, 0);


	//3. ���oAnnulus��Ƶ��c�����~��A��g�u�s�u�W��Pixel value��T�C
	Annulus_Degrees_GetInfoByLine(matImg_gray, vec_List_PairPos, vec2d_tmp, m_vec2d_val_position);


	// 4. Annuls��_�S�x�Ѩ�
	//Algo_EdgeDetection_Sobel_X(vec2d_tmp, m_vec2d_val_intensity);
	Algo_EdgeDetection_Sobel_X(m_iKSize, vec2d_tmp, m_vec2d_val_intensity);


	// 5. Annuls��_�M���k
	Circle_Shape_Detector(matImg_gray, m_vec2d_val_intensity, m_vec2d_val_position, m_bDirection, m_bPolarity, m_iMinEdgeStrength, m_iKSize, resCircle);

	if (m_dbResizeScale != 1.0) {

		resCircle.cX = resCircle.cX * (1 / m_dbResizeScale);
		resCircle.cY = resCircle.cY * (1 / m_dbResizeScale);
		resCircle.dbRadius = resCircle.dbRadius * (1 / m_dbResizeScale);
	}

	CalcuBoundingBox(resCircle, m_BoundBox_Circle);


	// */


#else	// Hough circle detection.

	//crop target roi from source image to detecting circle. 
	int iH = matImg_gray.rows;
	int iW = matImg_gray.cols;
	int posL = 0, posT = 0;
	int posR = 0, posB = 0;
	posL = ((roiAnnuls.cX - roiAnnuls.dbRadius_Outer) < 0) ? 0 : (roiAnnuls.cX - roiAnnuls.dbRadius_Outer);
	posT = ((roiAnnuls.cY - roiAnnuls.dbRadius_Outer) < 0) ? 0 : (roiAnnuls.cY - roiAnnuls.dbRadius_Outer);
	posR = ((roiAnnuls.cX + roiAnnuls.dbRadius_Outer) > iW) ? iW : (roiAnnuls.cX + roiAnnuls.dbRadius_Outer);
	posB = ((roiAnnuls.cY + roiAnnuls.dbRadius_Outer) > iH) ? iH : (roiAnnuls.cY + roiAnnuls.dbRadius_Outer);

	cv::Rect rectROI(cv::Point(posL, posT), cv::Point(posR, posB));
	cv::Mat tmp_image = matImg_gray(rectROI);

	Circle_Shape_Detector(tmp_image, roiAnnuls, m_bDirection, m_bPolarity, m_iMinEdgeStrength, m_iKSize, m_Outline_Circle);

	//offset back to original position
	m_Outline_Circle.cX += posL;
	m_Outline_Circle.cY += posT;
	CalcuBoundingBox(m_Outline_Circle, m_BoundBox_Circle);


#endif	//_DC_Method


	return res;
}


int CMethod_Alignment::Align_DetectCircle_GetResult(seDetectCirle_Results& myResultSet) 
{
	int res = 0;

	seDetectCirle_Results tmpResult;

	tmpResult.seBoundBox_Circle = m_BoundBox_Circle;

	myResultSet = tmpResult;

	return res;;
}



// InspecBoxSetup_Annulus
int CMethod_Alignment::InspectBox_SetParameter(seAnnulus roiAnnuls)
{
	int res = 0;

	InspectBox_Clear();

	m_emBoxShape = emShapeTypes::SHAPE_ANNULUS;
	m_Annulus = roiAnnuls;

	return res;
}

int CMethod_Alignment::InspectBox_SetParameter(seBoundingBox roiRect)
{
	int res = 0;

	InspectBox_Clear();

	m_emBoxShape = emShapeTypes::SHAPE_RECT;
	m_Rect = roiRect;

	return res;
}

int CMethod_Alignment::InspectBox_SetParameter(seCircle roiCircle)
{
	int res = 0;

	InspectBox_Clear();

	m_emBoxShape = emShapeTypes::SHAPE_CIRCLE;
	m_Circle = roiCircle;

	return res;
}


int CMethod_Alignment::InspectBox_TestIt(const LPImageInfo pIn)
{
	int res = 0;

	cv::Mat matImg_gray;
	if (Uint8ToCvMat_GrayScalar(pIn, matImg_gray) < ER_OK) {
		return ER_ABORT;
	}

	m_Src_img = matImg_gray.clone();

	switch (m_emBoxShape) {

		case emShapeTypes::SHAPE_RECT:
			CalcuBoundingBox(m_Rect, m_BoundBox);
			break;
		case emShapeTypes::SHAPE_CIRCLE:
			CalcuBoundingBox(m_Circle, m_BoundBox);
			break;
		case emShapeTypes::SHAPE_ANNULUS:
			CalcuBoundingBox(m_Annulus, m_BoundBox);
			break;
		default:
			return ER_ABORT;
			break;
	}


	return res;
}

int CMethod_Alignment::InspectBox_TestIt(const cv::Mat matImg_gray)
{
	int res = 0;

	if (matImg_gray.empty()) {
		return ER_ABORT;
	}

	 matImg_gray.copyTo(m_Src_img);

	switch (m_emBoxShape) {

	case emShapeTypes::SHAPE_RECT:
		CalcuBoundingBox(m_Rect, m_BoundBox);
		break;
	case emShapeTypes::SHAPE_CIRCLE:
		CalcuBoundingBox(m_Circle, m_BoundBox);
		break;
	case emShapeTypes::SHAPE_ANNULUS:
		CalcuBoundingBox(m_Annulus, m_BoundBox);
		break;
	default:
		return ER_ABORT;
		break;
	}


	return res;
}

//int CMethod_Alignment::InspectBox_GetResult(LPBoundingBox pInspBox, LPImageInfo pOut) //retur ROI Image
int CMethod_Alignment::InspectBox_GetResult(LPBoundingBox pInspBox) //retur ROI Image
{
	int res = 0;

	*pInspBox = m_BoundBox;

	//seRect tmpRect = pInspBox->rectBox;

	//cv::Mat tmpMat = m_Src_img(
	//	cv::Rect(
	//		cv::Point(tmpRect.left, tmpRect.top),
	//		cv::Point(tmpRect.right, tmpRect.bottom)
	//	)
	//);

	//if (CvMatToUint8(tmpMat, pOut) < ER_OK) {
	//	return ER_ABORT;
	//}

	return res;
}



// calculate the dependency coordinates 
int CMethod_Alignment::CoordBind_Calcu_SetParameter(seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn)
{
	int res = 0;
	CoordBind_Clear();

	m_CoordInfo_Out.FMark = seFMarkBox;
	m_CoordInfo_Out.InsptBox = seInspBox;

	if ((seCoorBindBoxIn.FMark.cX != 0) &&
		(seCoorBindBoxIn.FMark.cY != 0) &&
		(seCoorBindBoxIn.InsptBox.cX != 0) &&
		(seCoorBindBoxIn.InsptBox.cY != 0) ){

		bflg_DeployMode = 1;
		m_CoordInfo_In = seCoorBindBoxIn;
	}
	else {

		bflg_DeployMode = 0;
		m_CoordInfo_In = seCoordBindBox();
	}

	
	return res;
}

int CMethod_Alignment::CoordBind_Calcu_TestIt()
{
	int res = 0;

	if (bflg_DeployMode) {

		int iDiff_W(0), iDiff_H(0);
		double dbAngle(0.0);
		int iDiff_IBox_W(0), iDiff_IBox_H(0);

		dbAngle = m_CoordInfo_Out.FMark.dbAngle;
		m_CoordInfo_Out.CalibCoord.dbAngle = dbAngle;

		iDiff_IBox_W = m_CoordInfo_In.InsptBox.cX - m_CoordInfo_In.FMark.cX;
		m_CoordInfo_Out.CalibCoord.iDelta_InspectBox_W = iDiff_IBox_W;

		iDiff_IBox_H = m_CoordInfo_In.InsptBox.cY - m_CoordInfo_In.FMark.cY;
		m_CoordInfo_Out.CalibCoord.iDelta_InspectBox_H = iDiff_IBox_H;


		iDiff_W = m_CoordInfo_Out.FMark.cX - m_CoordInfo_In.FMark.cX;
		m_CoordInfo_Out.CalibCoord.iDelta_W = iDiff_W;

		iDiff_H = m_CoordInfo_Out.FMark.cY - m_CoordInfo_In.FMark.cY;
		m_CoordInfo_Out.CalibCoord.iDelta_H = iDiff_H;


	}

	return res;
}

int CMethod_Alignment::CoordBind_Calcu_GetResult(LPCoordBindBox pCoordBoxInfo)
{
	int res = 0;

	*pCoordBoxInfo = m_CoordInfo_Out;

	return res;
}

int CMethod_Alignment::CoordBind_CropROI_SetParameter(seCoordBindBox seCoordBindBox, seAnnulus roiAnnulus)
{
	int res = 0;

	CoordBind_CropROI_Clear();

	m_emCrop_BoxShape = emShapeTypes::SHAPE_ANNULUS;
	m_crop_CoordInfo = seCoordBindBox;
	m_crop_Annulus = roiAnnulus;

	return 0;
}

int CMethod_Alignment::CoordBind_CropROI_SetParameter(seCoordBindBox seCoordBindBox, seBoundingBox roiRect)
{
	int res = 0;
	
	CoordBind_CropROI_Clear();

	m_emCrop_BoxShape = emShapeTypes::SHAPE_RECT;
	m_crop_CoordInfo = seCoordBindBox;
	m_crop_Rect = roiRect;

	return 0;
}

int CMethod_Alignment::CoordBind_CropROI_SetParameter(seCoordBindBox seCoordBindBox, seCircle roiCircle)
{
	int res = 0;

	CoordBind_CropROI_Clear();

	m_emCrop_BoxShape = emShapeTypes::SHAPE_CIRCLE;
	m_crop_CoordInfo = seCoordBindBox;
	m_crop_Circle = roiCircle;

	return 0;
}

int CMethod_Alignment::CoordBind_CropROI_TestIt(const LPImageInfo pIn)
{
	cv::Mat matImg_Dest;
	if (Uint8ToCvMat(pIn, matImg_Dest) < ER_OK) {
		return ER_ABORT;
	}
		
	if ( !m_Crop_img.empty() ) { m_Crop_img.release(); }
	if ( !m_Crop_Mask.empty() ) { m_Crop_Mask.release(); }
		
	m_Crop_img = cv::Mat::zeros(matImg_Dest.size(), CV_8UC1);
	m_Crop_Mask = cv::Mat::zeros(matImg_Dest.size(), CV_8UC1);

	cv::Rect rectImage = cv::Rect(0, 0, m_Crop_img.cols, m_Crop_img.rows);

	switch (m_emCrop_BoxShape) {

	case emShapeTypes::SHAPE_RECT:
		CalcuCenterOffset(m_Crop_img.size(), m_crop_CoordInfo, m_crop_Rect, m_offsetCrop_Rect);
		CreateMaskImg(m_offsetCrop_Rect, m_Crop_Mask);
		break;
	case emShapeTypes::SHAPE_CIRCLE:
		CalcuCenterOffset(m_Crop_img.size(), m_crop_CoordInfo, m_crop_Circle, m_offsetCrop_Circle);
		CreateMaskImg(m_offsetCrop_Circle, m_Crop_Mask);
		break;
	case emShapeTypes::SHAPE_ANNULUS:
		CalcuCenterOffset(m_Crop_img.size(), m_crop_CoordInfo, m_crop_Annulus, m_offsetCrop_Annulus);
		CreateMaskImg(m_offsetCrop_Annulus, m_Crop_Mask);
		break;
	default:
		return ER_ABORT;
		break;
	}

	matImg_Dest.copyTo(m_Crop_img, m_Crop_Mask);
	//cv::bitwise_and(matImg_Dest, matImg_Dest, m_Crop_img, m_Crop_Mask);

	return 0;
}

int CMethod_Alignment::CoordBind_CropROI_TestIt(const cv::Mat matImg_Dest)
{
	if(matImg_Dest.empty()) {
		return ER_ABORT;
	}

	if (!m_Crop_img.empty()) { m_Crop_img.release(); }
	if (!m_Crop_Mask.empty()) { m_Crop_Mask.release(); }

	m_Crop_img = cv::Mat::zeros(matImg_Dest.size(), CV_8UC1);
	m_Crop_Mask = cv::Mat::zeros(matImg_Dest.size(), CV_8UC1);

	cv::Rect rectImage = cv::Rect(0, 0, m_Crop_img.cols, m_Crop_img.rows);

	switch (m_emCrop_BoxShape) {

	case emShapeTypes::SHAPE_RECT:
		CalcuCenterOffset(m_Crop_img.size(), m_crop_CoordInfo, m_crop_Rect, m_offsetCrop_Rect);
		CreateMaskImg(m_offsetCrop_Rect, m_Crop_Mask);
		break;
	case emShapeTypes::SHAPE_CIRCLE:
		CalcuCenterOffset(m_Crop_img.size(), m_crop_CoordInfo, m_crop_Circle, m_offsetCrop_Circle);
		CreateMaskImg(m_offsetCrop_Circle, m_Crop_Mask);
		break;
	case emShapeTypes::SHAPE_ANNULUS:
		CalcuCenterOffset(m_Crop_img.size(), m_crop_CoordInfo, m_crop_Annulus, m_offsetCrop_Annulus);
		CreateMaskImg(m_offsetCrop_Annulus, m_Crop_Mask);
		break;
	default:
		return ER_ABORT;
		break;
	}

	matImg_Dest.copyTo(m_Crop_img, m_Crop_Mask);
	//cv::bitwise_and(matImg_Dest, matImg_Dest, m_Crop_img, m_Crop_Mask);

	return 0;
}


int CMethod_Alignment::CoordBind_CropROI_GetResult(LPImageInfo pOut, seCropImage_Results& myResultSet)
{
	int res = 0;

	int iW = pOut->iWidth;
	int iH = pOut->iHeight;
	int iChannels = pOut->iChannels;

	if (pOut->pbImgBuf == nullptr ||
		iW != m_Crop_img.cols ||
		iH != m_Crop_img.rows ||
		iChannels != m_Crop_img.channels()) {

		return ER_ABORT;
	}

	if (CvMatToUint8(m_Crop_img, pOut) < ER_OK) {
		return ER_ABORT;
	}

	myResultSet.emShapeType = m_emCrop_BoxShape;
	switch (m_emCrop_BoxShape) {

	case emShapeTypes::SHAPE_RECT:
		myResultSet.seOffsetCrop_Rect = m_offsetCrop_Rect;
		break;
	case emShapeTypes::SHAPE_CIRCLE:
		myResultSet.seOffsetCrop_Circle = m_offsetCrop_Circle;
		break;
	case emShapeTypes::SHAPE_ANNULUS:
		myResultSet.seOffsetCrop_Annulus = m_offsetCrop_Annulus;
		break;
	default:
		return ER_ABORT;
		break;
	}

	return 0;
}


int CMethod_Alignment::CoordBind_CropROI_GetResult(cv::Mat& matImg_Crop, seCropImage_Results& myResultSet)
{
	int res = 0;

	m_Crop_img.copyTo(matImg_Crop);

	myResultSet.emShapeType = m_emCrop_BoxShape;
	switch (m_emCrop_BoxShape) {

	case emShapeTypes::SHAPE_RECT:
		myResultSet.seOffsetCrop_Rect = m_offsetCrop_Rect;
		break;
	case emShapeTypes::SHAPE_CIRCLE:
		myResultSet.seOffsetCrop_Circle = m_offsetCrop_Circle;
		break;
	case emShapeTypes::SHAPE_ANNULUS:
		myResultSet.seOffsetCrop_Annulus = m_offsetCrop_Annulus;
		break;
	default:
		return ER_ABORT;
		break;
	}

	return 0;
}
