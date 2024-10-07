#include <opencv2/opencv.hpp>
#include "Method_Measure.h"


using namespace std;
using namespace cv;

CMethod_Measure::CMethod_Measure()
{}

CMethod_Measure::~CMethod_Measure()
{}

int CMethod_Measure::MeasGlueWidth_SetParameter(seAnnulus roiAnnuls, int stepSize /*degrees*/)
{
	int res = 0;

	MeasGlueWidth_Clear();

	m_emBoxShape = emShapeTypes::SHAPE_ANNULUS;
	m_roiAnnuls = roiAnnuls;
	m_StepSize = (stepSize<= 0) ? 1 : stepSize;

	return res;
}

int CMethod_Measure::MeasGlueWidth_SetParameter(seBoundingBox roiRect, int stepSize /*degrees*/)
{
	int res = 0;

	MeasGlueWidth_Clear();

	m_emBoxShape = emShapeTypes::SHAPE_RECT;
	m_roiRect = roiRect;
	m_StepSize = (stepSize <= 0) ? 1 : stepSize;

	return res;
}


int CMethod_Measure::MeasGlueWidth_TestIt(const LPImageInfo pIn)
{
	int res = 0;

#ifdef ALGO_Enable_MeasGlueWidth_ResultIMG_DEBUG
	if (Uint8ToCvMat_ColorScalar(pIn, matSrcImg) < ER_OK) {
		return ER_ABORT;
	}
#endif

	cv::Mat matImg_gray;
	if (Uint8ToCvMat_GrayScalar(pIn, matImg_gray) < ER_OK) {
		return ER_ABORT;
	}
	//Protection mechanism: when the insanity of pIn is not Binary Image, so will need to doing this.
	threshold(matImg_gray, matImg_gray, 123, 255, cv::THRESH_BINARY);

	cv::Mat matMask = cv::Mat::zeros(matImg_gray.size(), CV_8UC1);
	CreateMaskImg(m_roiAnnuls, matMask);

	switch (m_emBoxShape) {

		case emShapeTypes::SHAPE_RECT:
			Algo_GlueWidth_RangeSetting(m_roiRect, m_StepSize, m_vecPos_BoxShape_CenterToOuter);
			CreateMaskImg(m_roiRect, matMask);
			break;
		case emShapeTypes::SHAPE_ANNULUS:
			Algo_GlueWidth_RangeSetting(m_roiAnnuls, m_StepSize, m_vecPos_BoxShape_CenterToOuter);
			CreateMaskImg(m_roiAnnuls, matMask);
			break;
		case emShapeTypes::SHAPE_CIRCLE:
		default:
			return ER_ABORT;
			break;
	}

	//calculate the glue width of object from image.
	Algo_GlueWidth_MeasureLength_GetPosistion(matImg_gray, m_vecPos_BoxShape_CenterToOuter, m_vecPos_Annulus_Inner, m_vecPos_Annulus_Outer);

	Algo_GlueWidth_MeasureLength_Calculate(m_vecPos_Annulus_Inner, m_vecVal_GlueWidth_Inner, m_vecPos_GlueWidth_Inner);
	Algo_GlueWidth_MeasureLength_Calculate(m_vecPos_Annulus_Outer, m_vecVal_GlueWidth_Outer, m_vecPos_GlueWidth_Outer);

	//Claculate area of (obj / mask);
	double tmpArea_Obj = 0.0, tmpArea_Mask = 0.0;
	Algo_GlueWidth_MeasureArea(matMask, tmpArea_Mask);

	cv::Mat tmpROI = cv::Mat::zeros(matImg_gray.size(), matImg_gray.type());
	matImg_gray.copyTo(tmpROI, matMask);
	//cv::bitwise_and(matImg_gray, matImg_gray, tmp, matMask);

#ifdef _RexTY_DEBUG

	cv::imshow("matMask", matMask);
	cv::imshow("tmp_BoxShape_ROI", tmpROI);
	cv::waitKey(0);
	cv::destroyAllWindows();

#endif

	Algo_GlueWidth_MeasureArea(tmpROI, tmpArea_Obj);

	m_dbArea_GlueWidth = tmpArea_Obj;	//<=== 20220302_Shoan say only return pixel count of object in image.

	m_dbRatio_GlueWidth = (tmpArea_Obj / tmpArea_Mask);

	return res;
}

int CMethod_Measure::MeasGlueWidth_TestIt(const cv::Mat srcImg_gray)
{
	int res = 0;

	if (srcImg_gray.empty()) {
		return ER_ABORT;
	}

	cv::Mat matImg_gray = srcImg_gray.clone();

#ifdef ALGO_Enable_MeasGlueWidth_ResultIMG_DEBUG

	cv::cvtColor(matImg_gray, matSrcImg, COLOR_GRAY2BGR);

#endif

	//Protection mechanism: when the insanity of pIn is not Binary Image, so will need to doing this.
	cv::threshold(matImg_gray, matImg_gray, 123, 255, cv::THRESH_BINARY);

	cv::Mat matMask = cv::Mat::zeros(matImg_gray.size(), CV_8UC1);

	switch (m_emBoxShape) {

	case emShapeTypes::SHAPE_RECT:
		Algo_GlueWidth_RangeSetting(m_roiRect, m_StepSize, m_vecPos_BoxShape_CenterToOuter);
		CreateMaskImg(m_roiRect, matMask);
		break;
	case emShapeTypes::SHAPE_ANNULUS:
		Algo_GlueWidth_RangeSetting(m_roiAnnuls, m_StepSize, m_vecPos_BoxShape_CenterToOuter);
		CreateMaskImg(m_roiAnnuls, matMask);
		break;
	case emShapeTypes::SHAPE_CIRCLE:
	default:
		return ER_ABORT;
		break;
	}

	//calculate the glue width of object from image.
	Algo_GlueWidth_MeasureLength_GetPosistion(matImg_gray, m_vecPos_BoxShape_CenterToOuter, m_vecPos_Annulus_Inner, m_vecPos_Annulus_Outer);

	Algo_GlueWidth_MeasureLength_Calculate(m_vecPos_Annulus_Inner, m_vecVal_GlueWidth_Inner, m_vecPos_GlueWidth_Inner);
	Algo_GlueWidth_MeasureLength_Calculate(m_vecPos_Annulus_Outer, m_vecVal_GlueWidth_Outer, m_vecPos_GlueWidth_Outer);

	//Claculate area of (obj / mask);
	double tmpArea_Obj = 0.0, tmpArea_Mask = 0.0;
	Algo_GlueWidth_MeasureArea(matMask, tmpArea_Mask);

	cv::Mat tmpROI = cv::Mat::zeros(matImg_gray.size(), matImg_gray.type());
	matImg_gray.copyTo(tmpROI, matMask);
	//cv::bitwise_and(matImg_gray, matImg_gray, tmp, matMask);

#ifdef _RexTY_DEBUG

	cv::imshow("matMask", matMask);
	cv::imshow("tmp_BoxShape_ROI", tmpROI);
	cv::waitKey(0);
	cv::destroyAllWindows();

#endif

	Algo_GlueWidth_MeasureArea(tmpROI, tmpArea_Obj);

	m_dbArea_GlueWidth = tmpArea_Obj;	//<=== 20220302_Shoan say only return pixel count of object in image.

	m_dbRatio_GlueWidth = (tmpArea_Obj / tmpArea_Mask);

	return res;
}



int CMethod_Measure::MeasGlueWidth_GetResult(seGlueWidth_Results& myResultSet)
{
	int res = 0;

	myResultSet.Length_Outer.assign(m_vecVal_GlueWidth_Outer.begin(), m_vecVal_GlueWidth_Outer.end());
	myResultSet.Length_Inner.assign(m_vecVal_GlueWidth_Inner.begin(), m_vecVal_GlueWidth_Inner.end());

	myResultSet.Position_Outer.assign(m_vecPos_GlueWidth_Outer.begin(), m_vecPos_GlueWidth_Outer.end());
	myResultSet.Position_Inner.assign(m_vecPos_GlueWidth_Inner.begin(), m_vecPos_GlueWidth_Inner.end());

	myResultSet.GlueArea = m_dbArea_GlueWidth;
	myResultSet.GlueRatio = m_dbRatio_GlueWidth;

	return res;	
}

int CMethod_Measure::MeasGlueWidth_GetRetImg(const char* strSavePath)
{
	int res = 0;

#ifdef	ALGO_Enable_MeasGlueWidth_ResultIMG_DEBUG

	if (matSrcImg.empty()) {
		return ER_ABORT;
	}

	cv::imwrite(strSavePath, matSrcImg);
	
	if (!matSrcImg.empty()) {
		matSrcImg.release();
	}

#endif

	return res;
}



int CMethod_Measure::MeasGlueWidth_SetInfo4Dump(std::string strInfo4Dump)
{
	int res = 0;

	if (!strInfo4Dump.empty()) {
		m_Measure_vecInof_forDump.push_back(strInfo4Dump);
	}

	return res;
}


int CMethod_Measure::MeasGlueWidth_GetInfo4Dump(std::vector< std::string>& vecInof_forDump)
{
	int res = 0;

	vecInof_forDump = m_Measure_vecInof_forDump;

	return res;
}


int CMethod_Measure::Algo_GlueWidth_RangeSetting(seAnnulus roiAnnuls, double dbStepSize, std::vector<pairPos>& vec_List_PairPos)
{
	int res = 0;

	if ((0 == roiAnnuls.dbRadius_Inner && 0 == roiAnnuls.dbRadius_Outer) ||
		(roiAnnuls.dbRadius_Inner > roiAnnuls.dbRadius_Outer)) {

		return ER_ABORT;
	}

	cv::Point2f posCenter(roiAnnuls.cX, roiAnnuls.cY);
	double fRadius[2]{ roiAnnuls.dbRadius_Inner, roiAnnuls.dbRadius_Outer };

	double dbAngle_start(((roiAnnuls.dbStartAngle <= 0) || (roiAnnuls.dbStartAngle >= 360)) ? 0 : roiAnnuls.dbStartAngle);
	double dbAngle_end(((roiAnnuls.dbEndAngle <= 0) || (roiAnnuls.dbEndAngle >= 360)) ? 360 : roiAnnuls.dbEndAngle);
	//dbAngle_start = min(dbAngle_start, dbAngle_end);
	//dbAngle_end = max(dbAngle_start, dbAngle_end);
	if (dbAngle_start > dbAngle_end) {
		std::swap(dbAngle_start, dbAngle_end);
	}

	double dbDegrees_Total = (dbAngle_end - dbAngle_start);
	double dbDegrees_Divided = dbStepSize;
	double dbDegrees_Section = round(dbDegrees_Total / dbDegrees_Divided);

	int iRange = static_cast<int>(dbDegrees_Section);
	std::vector<pairPos> vecCnt_GlueWidth(iRange);


	cv::Point second_begin, second_end; //刻度的起點，终點 
	double scale_long(0.0);				//刻度的長度 
	int scale_width(2);					//刻度的寬度 


//#ifdef _RexTY_DEBUG

	//Circle
	//Scalar color = Scalar(0, 255, 255); //BGR
	Scalar color = Scalar(0, 255, 0); //BGR
	//Center
	Scalar color_Center = Scalar(0, 0, 255);
	//Scale
	Scalar color_Scale = Scalar(0, 0, 255);
	//Glue
	Scalar color_Glue = Scalar(255, 0, 0);


	cv::Point center(roiAnnuls.cX, roiAnnuls.cY);

#ifdef ALGO_Enable_MeasGlueWidth_ResultIMG_DEBUG

	cv::circle(matSrcImg, center, 1, color_Center, 2);

	int radius[2] = { roiAnnuls.dbRadius_Inner, roiAnnuls.dbRadius_Outer };
	cv::circle(matSrcImg, center, radius[0], color, 2);
	cv::circle(matSrcImg, center, radius[1], color, 2);

#endif

//#endif


	for (int second_Scale = static_cast<int>(dbAngle_start); second_Scale < dbDegrees_Section; second_Scale++) {

		//Radius_Inner
		//second_begin.x = posCenter.x + fRadius[0] * std::cos(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的起點x坐標賦值
		//second_begin.y = posCenter.y + fRadius[0] * std::sin(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的起點y坐標賦值 
		second_begin.x = posCenter.x;	// +0 * std::cos(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的起點x坐標賦值
		second_begin.y = posCenter.y;	// +0 * std::sin(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的起點y坐標賦值 

		//Radius_Oute
		second_end.x = posCenter.x + (fRadius[1] - scale_long) * std::cos(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的終點x坐標賦
		second_end.y = posCenter.y + (fRadius[1] - scale_long) * std::sin(dbDegrees_Divided * second_Scale * CV_PI / 180);	//刻度的終點y坐標賦

		vecCnt_GlueWidth.at(second_Scale) = std::make_pair(second_begin, second_end);

#ifdef ALGO_Enable_MeasGlueWidth_ResultIMG_DEBUG

		arrowedLine(matSrcImg, second_begin, second_end, color_Scale, scale_width);
		arrowedLine(matSrcImg, second_begin, second_end, color_Scale, scale_width);   //連接起短終點 

#endif


	}


	//if (!matRetImg.empty()) {
	//	matRetImg.release();
	//}
	//matRetImg = matSrcImg.clone();


#ifdef _RexTY_DEBUG

	cv::imshow("matSrcImg", matSrcImg);
	cv::waitKey(0);
	cv::destroyAllWindows();

#endif


#ifdef NDEBUG

	string strFullPath;
	SaveTestImage("IPL_Measure", "Results_GlueWidth_RangeSetting", matSrcImg, strFullPath);

	MeasGlueWidth_SetInfo4Dump("RangeSetting_ImagePath = " + strFullPath);

#endif	//#ifdef NDEBUG


	vec_List_PairPos.assign(vecCnt_GlueWidth.begin(), vecCnt_GlueWidth.end());

	return res;
}


int CMethod_Measure::Algo_GlueWidth_RangeSetting(seBoundingBox roiRect, double dbStepSize, std::vector<pairPos>& vec_List_PairPos)
{
	int res = 0;

	if (0 == roiRect.rectBox.width || 0 == roiRect.rectBox.height) {

		return ER_ABORT;
	}


	int cX = roiRect.cX;
	int cY = roiRect.cY;

	double dbDegree = roiRect.dbAngle * CV_PI / 180.0;
	double dbCos = cos(dbDegree);
	double dbSin = sin(dbDegree);

	cv::Mat matRotate = (cv::Mat_<double>(2, 2) << dbCos, -1.0 * dbSin, dbSin, dbCos);
	cv::Mat matPos_Center = (cv::Mat_<double>(2, 1) << cX, cY);

	int  iL{ 0 }, iT{ 0 }, iR{ 0 }, iB{ 0 }, iW{ 0 }, iH{ 0 };

	iL = roiRect.rectBox.left;
	iT = roiRect.rectBox.top;
	iR = roiRect.rectBox.right;
	iB = roiRect.rectBox.bottom;

	iW = roiRect.rectBox.width;
	iH = roiRect.rectBox.height;

	// ## < ## < ## < ## < ## < ## < ## < ## < ## <

	cv::Point posCenter(roiRect.cX, roiRect.cY);
	double dbAngle(roiRect.dbAngle);
	double dbScale(1.0);
	cv::Size szSize(roiRect.rectBox.width, roiRect.rectBox.height);

	// Create the rotated rectangle
	cv::RotatedRect rotatedRectangle(posCenter, szSize, dbAngle);

	// We take the edges that OpenCV calculated for us
	cv::Point2f vertices2f[4];
	rotatedRectangle.points(vertices2f);

	// Convert them so we can use them in a fillConvexPoly
	std::vector<cv::Point> vecVertices;
	for (int i = 0; i < 4; ++i) {
		vecVertices.push_back(vertices2f[i]);
	}

	// ## > ## > ## > ## > ## > ## > ## > ## > ## >

	double dbSample_Total = iH;
	double dbSample_Divided = (dbStepSize <= 0) ? 1 : dbStepSize;
	int iSample_Section = round(dbSample_Total / dbSample_Divided);

	//int iRange = static_cast<int>(iSample_Section);
	std::vector<pairPos> vecCnt_GlueWidth;// (iRange);

	int iPosX_1(0), iPosY_1(0), iPosX_2(0), iPosY_2(0);
	cv::Mat matPos_Inspect;
	cv::Mat matValue;
	double dbVal_01(0), dbVal_02(0);
	cv::Point posLeft{0,0}, posRight{ 0,0 };
	for (int i = 0; i <= iSample_Section; i++) {

		//Left side point
		iPosX_1 = iL;
		iPosY_1 = iT + i * dbSample_Divided;
		iPosY_1 = (iPosY_1 > (iT + iH)) ? (iT + iH) : iPosY_1;

		matPos_Inspect = (cv::Mat_<double>(2, 1) << iPosX_1, iPosY_1);
		matValue = matRotate * (matPos_Inspect - matPos_Center) + matPos_Center;

		posLeft = cv::Point(round(matValue.at<double>(0)), round(matValue.at<double>(1)));

		//Right side point
		iPosX_2 = iR;
		iPosY_2 = iT + i * dbSample_Divided;
		iPosY_2 = (iPosY_2 > (iT + iH)) ? (iT + iH) : iPosY_2;

		matPos_Inspect = (cv::Mat_<double>(2, 1) << iPosX_2, iPosY_2);
		matValue = matRotate * (matPos_Inspect - matPos_Center) + matPos_Center;

		posRight = cv::Point(round(matValue.at<double>(0)), round(matValue.at<double>(1)));

		if (i == 0 || i == iSample_Section) {

			dbVal_01 = cv::pointPolygonTest(vecVertices, posLeft, false);
			dbVal_02 = cv::pointPolygonTest(vecVertices, posRight, false);

			if (dbVal_01 < 0 || dbVal_02 < 0) {
				continue;
			}
			else {

				vecCnt_GlueWidth.push_back(std::make_pair(posLeft, posRight));
			}

		}
		else {

			//Storge the pair of cv::Point.
			//vecCnt_GlueWidth.at(i) = std::make_pair(posLeft, posRight);
			vecCnt_GlueWidth.push_back(std::make_pair(posLeft, posRight));
		}

	}


	//Circle
	//Scalar color = Scalar(0, 255, 255); //BGR
	Scalar color = Scalar(0, 255, 0); //BGR
	//Center
	Scalar color_Center = Scalar(0, 0, 255);
	//Scale
	Scalar color_Scale = Scalar(0, 0, 255);
	//Glue
	Scalar color_Glue = Scalar(255, 0, 0);


	//drawing center
	cv::Point center(roiRect.cX, roiRect.cY);

#ifdef ALGO_Enable_MeasGlueWidth_ResultIMG_DEBUG

	cv::circle(matSrcImg, center, 1, color_Center, 2);

//#endif

	//drawing bbox
	//cv::rectangle(matSrcImg, cv::Rect(cv::Point(roiRect.rectBox.left, roiRect.rectBox.top), cv::Point(roiRect.rectBox.right, roiRect.rectBox.bottom)), color_Glue, 2);

	//// ###

	//cv::Point posCenter(roiRect.cX, roiRect.cY);
	//double dbAngle(roiRect.dbAngle);
	//double dbScale(1.0);
	//cv::Size szSize(roiRect.rectBox.width, roiRect.rectBox.height);

	//// Create the rotated rectangle
	//cv::RotatedRect rotatedRectangle(posCenter, szSize, dbAngle);

	//// We take the edges that OpenCV calculated for us
	//cv::Point2f vertices2f[4];
	//rotatedRectangle.points(vertices2f);

	//// Convert them so we can use them in a fillConvexPoly
	//std::vector<cv::Point> vecVertices;
	//for (int i = 0; i < 4; ++i) {
	//	vecVertices.push_back(vertices2f[i]);
	//}

	//// ###


//#ifdef ALGO_Enable_MeasGlueWidth_ResultIMG_DEBUG
// 
	// Now we can drawing the rotated rectangle with our specified color
	cv::polylines(matSrcImg, vecVertices, true, cv::Scalar(0, 255, 0), 4);

	//drawing ROI 
	for (int i = 0; i < vecCnt_GlueWidth.size(); i++) {

		cv::Point posBegin, posEnd; //刻度的起點，终點 

		posBegin = vecCnt_GlueWidth.at(i).first;
		posEnd = vecCnt_GlueWidth.at(i).second;

		arrowedLine(matSrcImg, posBegin, posEnd, color_Scale, 2);

	}

#endif



	//if (!matRetImg.empty()) {
	//	matRetImg.release();
	//}
	//matRetImg = matSrcImg.clone();


#ifdef _RexTY_DEBUG

	if (!matSrcImg.empty()) {

	cv::imshow("matSrcImg", matSrcImg);
	cv::waitKey(0);
	cv::destroyAllWindows();
	}

#endif


#ifdef NDEBUG

	string strFullPath;
	SaveTestImage("IPL_Measure", "Results_GlueWidth_Rect_RangeSetting", matSrcImg, strFullPath);

	MeasGlueWidth_SetInfo4Dump("RangeSetting_ImagePath = " + strFullPath);

#endif	//#ifdef NDEBUG


	vec_List_PairPos.assign(vecCnt_GlueWidth.begin(), vecCnt_GlueWidth.end());

	return res;
}


int CMethod_Measure::Algo_GlueWidth_MeasureLength_GetPosistion(const cv::Mat& mat_objImg, std::vector<pairPos> vec_List_PairPos, std::vector< pairPos>& vec_Pos_GlueWidth_Inner, std::vector< pairPos>& vec_Pos_GlueWidth_Outer)
{
	int res = 0;

	if (mat_objImg.empty() || mat_objImg.channels() != 1 || vec_List_PairPos.empty()) {
		return ER_ABORT;
	}

	int iRangeSize = static_cast<int>(vec_List_PairPos.size());

	std::vector< std::vector<int>> vecVal_AllPixels(iRangeSize);
	std::vector< std::vector< pairPos>> vecPos_AllPixels(iRangeSize);
	std::vector< pairPos> vecCenterToInner(iRangeSize);
	std::vector< pairPos> vecCenterToOuter(iRangeSize);

	for (int x = 0; x < iRangeSize; x++) {

		//calculate the Binary image
		LineIterator itor(mat_objImg, vec_List_PairPos[x].first, vec_List_PairPos[x].second);
		LineIterator itrForward = itor;

		for (int i = 0; i < itrForward.count; i++, ++itrForward) {

			uchar* ptr = *itrForward;
			vecVal_AllPixels[x].push_back(ptr[0]);

			cv::Point posCenter = vec_List_PairPos[x].first;
			cv::Point posCurrent = itrForward.pos();
			vecPos_AllPixels[x].push_back(std::make_pair(posCenter, posCurrent));
		}
	}

	int iSel1 = 0, iSel2 = 0;
	int ifilter_LH[3] = { 000, 255, 255 };
	int ifilter_HL[3] = { 255, 255, 000 };
	for (size_t x = 0; x < vecVal_AllPixels.size(); x++) {

		iSel1 = iSel2 = 0;

		for (size_t i = 0; i < vecVal_AllPixels[x].size() - 3; i++) {

			if ((vecVal_AllPixels[x].at(i + 0) == ifilter_LH[0]) &&
				(vecVal_AllPixels[x].at(i + 1) > ifilter_LH[0]) &&
				(vecVal_AllPixels[x].at(i + 2) > ifilter_LH[0]) &&
				iSel1 == 0) {

				iSel1 = i+1;
			}
		}

		for (size_t i = vecVal_AllPixels[x].size()-3; i > 0; i--) {

			if ((vecVal_AllPixels[x].at(i + 0) > ifilter_HL[2]) &&
				(vecVal_AllPixels[x].at(i + 1) > ifilter_HL[2]) &&
				(vecVal_AllPixels[x].at(i + 2) == ifilter_HL[2]) &&
				iSel2 == 0) {

				iSel2 = i+1;
			}

		}
		if (iSel1 && iSel2) {

			vecCenterToInner.at(x) = vecPos_AllPixels[x].at(iSel1);
			vecCenterToOuter.at(x) = vecPos_AllPixels[x].at(iSel2);

		}

		}

	vec_Pos_GlueWidth_Inner.assign(vecCenterToInner.begin(), vecCenterToInner.end());
	vec_Pos_GlueWidth_Outer.assign(vecCenterToOuter.begin(), vecCenterToOuter.end());

	return res;
}


int CMethod_Measure::Algo_GlueWidth_MeasureLength_Calculate(std::vector< pairPos> vec_Pos_GlueWidth, std::vector<double>& vec_ResValue_GlueWidth, std::vector<sePoint>& vec_ResPos_GlueWidth)
{
	if (vec_Pos_GlueWidth.empty()) {

		return ER_ABORT;
	}

	int res(0);
	int iNorm_X(0), iNorm_Y(0);
	int iPosX_1(0), iPosX_2(0);
	int iPosY_1(0), iPosY_2(0);
	double dbRes(0.0);
	sePoint sePos = sePoint();

	for (int i = 0; i < vec_Pos_GlueWidth.size(); i++) {

		iPosX_1 = vec_Pos_GlueWidth[i].second.x;
		iPosX_2 = vec_Pos_GlueWidth[i].first.x;
		iNorm_X = (iPosX_1 - iPosX_2) * (iPosX_1 - iPosX_2);

		iPosY_1 = vec_Pos_GlueWidth[i].second.y;
		iPosY_2 = vec_Pos_GlueWidth[i].first.y;
		iNorm_Y = (iPosY_1 - iPosY_2) * (iPosY_1 - iPosY_2);

		dbRes = sqrt( iNorm_X + iNorm_Y );

		vec_ResValue_GlueWidth.push_back(dbRes);


		cv::Point posPoint = vec_Pos_GlueWidth[i].second;
		sePos.x = posPoint.x;
		sePos.y = posPoint.y;
		vec_ResPos_GlueWidth.push_back(sePos);

	}

	return res;
}


int CMethod_Measure::Algo_GlueWidth_MeasureLength(const cv::Mat& mat_objImg, std::vector<pairPos> vec_List_PairPos, std::vector<int>& vec_Res_GlueWidth)
{
	int res = 0;

	if (mat_objImg.empty() || mat_objImg .channels() != 1 || vec_List_PairPos.empty())
		return ER_ABORT;

	int iRangeSize = static_cast<int>(vec_List_PairPos.size());

	std::vector<int> vecCnt_GlueWidth(iRangeSize);

	for (int x = 0; x < iRangeSize; x++) {

		LineIterator itor(mat_objImg, vec_List_PairPos[x].first, vec_List_PairPos[x].second, 8);
		LineIterator it2 = itor;

		//calculate the count of maxval(255) from the Binary image.
		int iValidCnt_InLine= 0;
		for (int i = 0; i < it2.count; i++, ++it2) {

			uchar* ptr = *it2;
			iValidCnt_InLine += (static_cast<int>(ptr[0]) != 0) ? 1 : 0;
		}

		vecCnt_GlueWidth.at(x) = iValidCnt_InLine;

	}

	vec_Res_GlueWidth.assign(vecCnt_GlueWidth.begin(), vecCnt_GlueWidth.end());

	return res;
}

int CMethod_Measure::Algo_GlueWidth_MeasureArea(const cv::Mat& mat_objImg, double& dbArea_GlueWidth)
{
	int res = 0;

	if (mat_objImg.empty() ||
		mat_objImg.channels() != 1) {

		return ER_ABORT;
	}

	int iW = mat_objImg.cols;
	int iH = mat_objImg.rows;
	int iChannels = mat_objImg.channels();
	unsigned int ui_PixelCnt = 0;

	const unsigned char* pter_Mat = mat_objImg.ptr<unsigned char>(0);

	for (int y = 0; y < iH; y++) {

		pter_Mat = mat_objImg.ptr<unsigned char>(y);

		for (int x = 0; x < iW; x++) {

			ui_PixelCnt += (pter_Mat[x]) ? 1 : 0;
		}

	}

	dbArea_GlueWidth = static_cast<double>(ui_PixelCnt);


	//vector<vector<Point>> contours;
	//vector<Vec4i> hierarchy;

	//cv::findContours(mat_objImg, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

	//double area = 0.0;
	//double arclength = 0.0;
	//double circularity = 0.0;

	//for (int i = 0; i < contours.size(); i++)
	//{
	//	area = contourArea(cv::Mat(contours[i]));

	//	// Circularity 計算
	//	arclength = cv::arcLength(contours[i], true);
	//	circularity = (4.0 * CV_PI * area / (arclength * arclength));
	//}

	//dbArea_GlueWidth = area;

	return 0;
}