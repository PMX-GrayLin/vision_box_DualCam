#include "Measure_GlueWidth_ConcreteClass.h"
#include <string>
#define CVUI_IMPLEMENTATION
// ?? gray : temp remove for no cvui support at yocto
#include "cvui.h"

using namespace std;


#define MAJOR 2
#define MINOR 7	
#define BUILD 20220328
std::string strVer = std::to_string(MAJOR) + '.' + std::to_string(MINOR) + "." + std::to_string(BUILD);


//int PTWDLL_C_GlueWidth::vbs_Align_ImageCalibration(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_Align_ImageCalibration(void* pSrcImg, 
	seRect roiRect_01, 
	seRect roiRect_02, 
	const char* strSavePath, 
	double* pPixelCount_X, 
	double* pPixelCount_Y)
{
	int res = 0;


	//if (strSrcImg == nullptr) {
	if (pSrcImg == nullptr) {

		return ER_ABORT;
	}


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}

#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Align_ImageCalibration.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}


	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}



	//Algorithm of PatternMatch
	//res = cvip_Align.Align_Calibration_SetParameter(&seIn, roiRect_01, roiRect_02);
	res = cvip_Align.Align_Calibration_SetParameter(m_SrcImg, roiRect_01, roiRect_02);

	res = cvip_Align.Align_Calibration_TestIt();

	seImageCalibration_Results myRet;

	res = cvip_Align.Align_Calibration_GetResult(&myRet);


	//Record the testing results
	*pPixelCount_X = myRet.dbPixelCount_X;
	*pPixelCount_Y = myRet.dbPixelCount_Y;



	//Drawing bandbox information on Image and save that.

	if (pIP->bDumpRetImg) {

		cv::Mat tmpRes = m_SrcImg.clone();
		cv::cvtColor(tmpRes, tmpRes, COLOR_GRAY2RGB);


		seCircle c01, c02, c03;
		c01 = myRet.circle_01;
		c02 = myRet.circle_02;
		c03 = myRet.circle_03;

		cv::Point centers[3];

		centers[0] = cv::Point(c01.cX, c01.cY);
		centers[1] = cv::Point(c02.cX, c02.cY);
		centers[2] = cv::Point(c03.cX, c03.cY);

		double dbRadius[3];
		dbRadius[0] = myRet.circle_01.dbRadius;
		dbRadius[1] = myRet.circle_02.dbRadius;
		dbRadius[2] = myRet.circle_03.dbRadius;
		if (dbRadius[0] <= 0) dbRadius[0] = 1;
		if (dbRadius[1] <= 0) dbRadius[1] = 1;
		if (dbRadius[2] <= 0) dbRadius[2] = 1;

		cv::circle(tmpRes, centers[0], dbRadius[0], cv::Scalar(0, 0, 255), 2);
		cv::circle(tmpRes, centers[1], dbRadius[1], cv::Scalar(0, 0, 255), 2);
		cv::circle(tmpRes, centers[2], dbRadius[2], cv::Scalar(0, 0, 255), 2);

		cv::imwrite(strSavePath, tmpRes);

	}


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	return res;

}





//int PTWDLL_C_GlueWidth::vbs_Align_CropTemplate(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_Align_CropTemplate(void* pSrcImg, 
	seRect roiSearch, 
	seExpandable* seExpandable)
{
	int res = 0;

	
	//	const char* strInputPath = seExpandable->strImage_Input;
	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();


	//if (strSrcImg == nullptr || strSavePath == nullptr || strResultPath == nullptr) {
	if (pSrcImg == nullptr || strSavePath == nullptr || strResultPath == nullptr) {

		return ER_ABORT;
	}


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Align_CropTemplate.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}


	//Drawing bandbox information on Image and save that.
	cv::Rect cropROI;
	cropROI.x		= roiSearch.left;
	cropROI.y		= roiSearch.top;
	cropROI.width	= roiSearch.width;
	cropROI.height	= roiSearch.height;


	// Fool-proof mechanism >>> === === === >>> 
	seRect rectTmp = seRect();
	rectTmp.width = m_SrcImg.cols;
	rectTmp.height = m_SrcImg.rows;

	res = cvip_Judgment.Judg_IsOverRange(rectTmp, roiSearch);
	if (res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === === <<<


	cv::Mat cropImg = m_SrcImg(cropROI);

	cv::imwrite(strSavePath, cropImg);

	if (pIP->bDumpRetImg) {

		cv::imwrite(strResultPath, cropImg);
	}

	return res;

}




//int PTWDLL_C_GlueWidth::vbs_Align_PatternMatch(const char* strSrcImg, 
int PTWDLL_C_GlueWidth::vbs_Align_PatternMatch(void* pSrcImg,
											const char* strTemplateImg, 
											seRect roiSearch, 
											const char* strSavePath,
											LPBoundingBox pFMarkBoxOut,
											double* pdbScoreOut)
{
	int res = 0;

	//if (strSrcImg == nullptr || strTemplateImg == nullptr || strSavePath == nullptr || pFMarkBoxOut == nullptr || pdbScoreOut == nullptr) {
	if (pSrcImg == nullptr || strTemplateImg == nullptr || strSavePath == nullptr || pFMarkBoxOut == nullptr || pdbScoreOut == nullptr) {
		return ER_ABORT;
	}



	//argument define
	//seBoundingBox m_Border_FMark;
	double dbScore = 0.0;


	//// Source Image and Template Image
	//cv::Mat m_SrcImg = cv::imread(strSrcImg, cv::IMREAD_GRAYSCALE);
	//if (m_SrcImg.empty()) {

	//	return ER_ABORT;
	//}

	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Align_PatternMatch.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}


	// Template Image
	cv::Mat m_TemplateImg = cv::imread(strTemplateImg, cv::IMREAD_GRAYSCALE);
	if (m_TemplateImg.empty()) {

		return ER_ABORT;
	}

	
	// Fool-proof mechanism === === === >>> 
	res = cvip_Judgment.Judg_IsOverRange(m_SrcImg, m_TemplateImg);
	if (res) {
		return ER_ABORT;
	}

	seRect rectTmp = seRect();
	rectTmp.width = m_SrcImg.cols;
	rectTmp.height = m_SrcImg.rows;

	res = cvip_Judgment.Judg_IsOverRange(rectTmp, roiSearch);
	if (res) {

		return ER_ABORT;
	}

	rectTmp = seRect();
	rectTmp.width = m_TemplateImg.cols;
	rectTmp.height = m_TemplateImg.rows;

	res = cvip_Judgment.Judg_IsOverRange(roiSearch, rectTmp);
	if(res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === ===



	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	seImageInfo seTemplat;
	seTemplat.iWidth = m_TemplateImg.cols;
	seTemplat.iHeight = m_TemplateImg.rows;
	seTemplat.iChannels = m_TemplateImg.channels();
	seTemplat.pbImgBuf = new unsigned char[seTemplat.iWidth * seTemplat.iHeight * seTemplat.iChannels];
	if (CCVIPItem::CvMatToUint8(m_TemplateImg, &seTemplat) < ER_OK) {
		return ER_ABORT;
	}

	//Setting default seRect information.
	//seRect searchROI;
	//searchROI.left = 558;
	//searchROI.top = 358;
	//searchROI.right = 778;
	//searchROI.bottom = 583;
	//searchROI.width = 220;
	//searchROI.height = 225;


	//Algorithm of PatternMatch
	res = cvip_Align.Align_Pattern_SetParameter(roiSearch, &seTemplat);

	//res = cvip_Align.Align_Pattern_TestIt(&seIn);
	res = cvip_Align.Align_Pattern_TestIt(m_SrcImg);

	res = cvip_Align.Align_Pattern_GetResult(&m_Border_FMark, &dbScore);


	//Record the testing results
	*pFMarkBoxOut = m_Border_FMark;
	*pdbScoreOut = dbScore;


	//Drawing bandbox information on Image and save that.

	if (pIP->bDumpRetImg) {

		cv::Mat tmpRes = m_SrcImg.clone();
		cv::cvtColor(tmpRes, tmpRes, COLOR_GRAY2RGB);

		cv::Point posResS(m_Border_FMark.rectBox.left, m_Border_FMark.rectBox.top);
		cv::Point posResE(m_Border_FMark.rectBox.right, m_Border_FMark.rectBox.bottom);
		cv::rectangle(tmpRes, posResS, posResE, cv::Scalar(255, 0, 0), 4);

		cv::imwrite(strSavePath, tmpRes);

	}


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	if (seTemplat.pbImgBuf) {

		delete[] seTemplat.pbImgBuf;
		seTemplat.pbImgBuf = nullptr;
	}


	return res;
}


//int PTWDLL_C_GlueWidth::vbs_Align_FindProfile(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_Align_FindProfile(void* pSrcImg,
												seCircle roiSearch,
												seCircle roiMask,
												bool bDirection, 
												bool bPolarity, 
												int iSelLineNo, 
												const char* strSavePath,
												int* pCntArryOut, 
												int* p1DArrayOut)
{
	int res = 0;

	//if (strSrcImg == nullptr || strSavePath == nullptr )
	if (pSrcImg == nullptr || strSavePath == nullptr)
		return ER_ABORT;


	//argument define
	seDetectCirle_Results myRes;


	//Source Image
	//cv::Mat m_SrcImg = cv::imread(strSrcImg, cv::IMREAD_GRAYSCALE);
	//if (m_SrcImg.empty()) {

	//	return ER_ABORT;
	//}

	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Align_FindProfile.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	//cv::cvtColor(m_SrcImg, m_SrcImg, COLOR_GRAY2BGR);


	// Fool-proof mechanism === === === >>> 

	seRect rectTmp1 = seRect(), rectTmp2 = seRect(), rectTmp3 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	cvip_Judgment.Cvt_Circle2Rect(roiSearch, rectTmp2);
	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp2);
	if (res) {

		return ER_ABORT;
	}

	cvip_Judgment.Cvt_Circle2Rect(roiMask, rectTmp3);
	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp3);
	if (res) {

		return ER_ABORT;
	}

	res = cvip_Judgment.Judg_IsOverRange(rectTmp2, rectTmp3);
	if (res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === ===


	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	//seImageInfo seOut;
	//seOut.iWidth = m_SrcImg.cols;
	//seOut.iHeight = m_SrcImg.rows;
	//seOut.iChannels = m_SrcImg.channels();
	//seOut.pbImgBuf = new unsigned char[seOut.iWidth * seOut.iHeight * seOut.iChannels];



	//Algorithm of PatternMatch
	int stepSize	= 10;
	int iKSize		= 3;
	//int iCnt		= abs(roiSearch.dbRadius - roiMask.dbRadius) + 50;	
	//int* p1DArrayOut = new int[iCnt];


	res = cvip_Align.Align_FindProfile_SetParameter(roiSearch, roiMask, bDirection, bPolarity, stepSize, iSelLineNo, iKSize);

	//res = cvip_Align.Align_FindProfile_TestIt(&seIn, iSelLineNo);
	res = cvip_Align.Align_FindProfile_TestIt(m_SrcImg, iSelLineNo);

	res = cvip_Align.Align_FindProfile_GetResult(myRes);



	//Record the testing results
	*pCntArryOut = myRes.vec_1DArrayOut.size();
	std::copy(myRes.vec_1DArrayOut.begin(), myRes.vec_1DArrayOut.end(), p1DArrayOut);


	
	//Drawing information on Image and save that.

	if (pIP->bDumpRetImg) {

		int iCntArryOut = myRes.vec_1DArrayOut.size();
		if (iCntArryOut > 0) {

			int iW = 0, iH = 0;
			iW = 1000;
			iH = 500;

			cv::Mat frame = cv::Mat(iH, iW, CV_8UC3);
			frame = cv::Scalar(49, 52, 49);

			cv::Rect aRect(10, 10, 800, 400);


			std::vector<double> values;
			for (std::vector<double>::size_type i = 0; i < iCntArryOut; i++) {

				values.push_back(p1DArrayOut[i]);
			}
			std::vector<double>::size_type aHowManyValues = values.size();

			// ?? gray : temp remove for no cvui support at yocto
			cvui::cvui_block_t theBlock;
			theBlock.where = cv::Mat(frame);

			double aMax, aMin;
			if (aHowManyValues >= 2) {
				cvui::internal::findMinMax(values, &aMin, &aMax);
				cvui::render::sparkline(theBlock, values, aRect, aMin, aMax, 0xff0000);
			}

			cv::imwrite(strSavePath, frame);

		}

	}


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	//if (seOut.pbImgBuf) {

	//	delete[] seOut.pbImgBuf;
	//	seOut.pbImgBuf = nullptr;
	//}


	return res;
}


//int PTWDLL_C_GlueWidth::vbs_Align_DetectCircle(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_Align_DetectCircle(void* pSrcImg,
	seCircle roiSearch,
	seCircle roiMask,
	bool bDirection,
	bool bPolarity,
	int iMinEdgeStrength,
	const char* strSavePath,
	LPBoundingBox pFMarkBox_Circle,
	int* pCntArryOut,
	sePoint* pPoinitArrayOut)
{
	int res = 0;

	//if (strSrcImg == nullptr || strSavePath == nullptr)
	if (pSrcImg == nullptr || strSavePath == nullptr)
		return ER_ABORT;


	//argument define
	seDetectCirle_Results myRes;


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Align_DetectCircle.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	//cv::cvtColor(m_SrcImg, m_SrcImg, COLOR_GRAY2BGR);



	// Fool-proof mechanism === === === >>> 

	seRect rectTmp1 = seRect(), rectTmp2 = seRect(), rectTmp3 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	cvip_Judgment.Cvt_Circle2Rect(roiSearch, rectTmp2);
	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp2);
	if (res) {

		return ER_ABORT;
	}

	cvip_Judgment.Cvt_Circle2Rect(roiMask, rectTmp3);
	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp3);
	if (res) {

		return ER_ABORT;
	}

	res = cvip_Judgment.Judg_IsOverRange(rectTmp2, rectTmp3);
	if (res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === ===



	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}



	//Algorithm of PatternMatch
	int stepSize = 10;
	int iKSize = 3;
	//int iCnt		= abs(roiSearch.dbRadius - roiMask.dbRadius) + 50;	
	//int* p1DArrayOut = new int[iCnt];


	res = cvip_Align.Align_DetectCircle_SetParameter(roiSearch, roiMask, bDirection, bPolarity, stepSize, iMinEdgeStrength, iKSize);

	//res = cvip_Align.Align_DetectCircle_TestIt(&seIn);
	res = cvip_Align.Align_DetectCircle_TestIt(m_SrcImg);

	res = cvip_Align.Align_DetectCircle_GetResult(myRes);


	//Record the testing results
	//*pCntArryOut = myRes.vec_1DArrayOut.size();
	//std::copy(myRes.vec_1DArrayOut.begin(), myRes.vec_1DArrayOut.end(), p1DArrayOut);
	*pFMarkBox_Circle = myRes.seBoundBox_Circle;



	//Drawing information on Image and save that.

	if (pIP->bDumpRetImg) {

		cv::cvtColor(m_SrcImg, m_SrcImg, COLOR_GRAY2BGR);
		cv::Mat tmpRes = m_SrcImg.clone();

		int pos_L(0), pos_T(0), pos_R(0), pos_B(0);
		cv::Point posS, posE;
		pos_L = myRes.seBoundBox_Circle.rectBox.left;
		pos_T = myRes.seBoundBox_Circle.rectBox.top;
		pos_R = myRes.seBoundBox_Circle.rectBox.right;
		pos_B = myRes.seBoundBox_Circle.rectBox.bottom;
		posS = cv::Point(pos_L, pos_T);
		posE = cv::Point(pos_R, pos_B);

		cv::Point posVal(myRes.seBoundBox_Circle.cX, myRes.seBoundBox_Circle.cY);
		cv::circle(tmpRes, posVal, 1, cv::Scalar(0, 255, 0), 2);
		cv::rectangle(tmpRes, posS, posE, cv::Scalar(255, 0, 255), 2);
		cv::imwrite(strSavePath, tmpRes);

	}

	
	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}



	return res;
}



//int PTWDLL_C_GlueWidth::vbs_InspectBox_Annulus(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_InspectBox_Annulus(void* pSrcImg, 
												seAnnulus roiAnnulus, 
												const char* strSavePath, 
												LPBoundingBox pBoundBoxOut)
{
	int res = 0;

	//if (strSrcImg == nullptr || strSavePath == nullptr || pBoundBoxOut == nullptr)
	if (pSrcImg == nullptr || strSavePath == nullptr || pBoundBoxOut == nullptr)
		return ER_ABORT;


	//argument define


	//Source Image
	//cv::Mat m_SrcImg = cv::imread(strSrcImg, cv::IMREAD_GRAYSCALE);
	//if (m_SrcImg.empty()) {

	//	return ER_ABORT;
	//}


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_InspectBox_Annulus.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}



	// Fool-proof mechanism === === === >>> 

	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	cvip_Judgment.Cvt_Annulus2Rect(roiAnnulus, rectTmp2);
	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp2);
	if (res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === ===



	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}


	//Algorithm
	m_InspectBox_Annulus = roiAnnulus;


	res = cvip_Align.InspectBox_SetParameter(m_InspectBox_Annulus);

	//res = cvip_Align.InspectBox_TestIt(&seIn);
	res = cvip_Align.InspectBox_TestIt(m_SrcImg);

	res = cvip_Align.InspectBox_GetResult(&m_Border_Annulus);


	//Record the testing results
	*pBoundBoxOut = m_Border_Annulus;


	//Drawing information on Image and save that.

	if (pIP->bDumpRetImg) {

		cv::Mat tmpRes = m_SrcImg.clone();
		cv::cvtColor(tmpRes, tmpRes, COLOR_GRAY2RGB);

		seBoundingBox tmpBB = m_Border_Annulus;
		cv::Point posStart(tmpBB.rectBox.left, tmpBB.rectBox.top);
		cv::Point posEnd(tmpBB.rectBox.right, tmpBB.rectBox.bottom);
		cv::rectangle(tmpRes, posStart, posEnd, cv::Scalar(0, 0, 255), 2);
		cv::imwrite(strSavePath, tmpRes);

	}


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}


	return res;

}


//int PTWDLL_C_GlueWidth::vbs_InspectBox_Rect(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_InspectBox_Rect(void* pSrcImg,
	seBoundingBox roiRect,
	const char* strSavePath,
	LPBoundingBox pBoundBoxOut)
{
	int res = 0;

	//if (strSrcImg == nullptr || strSavePath == nullptr || pBoundBoxOut == nullptr)
	if (pSrcImg == nullptr || strSavePath == nullptr || pBoundBoxOut == nullptr)
		return ER_ABORT;


	//argument define



	////Source Image
	//cv::Mat m_SrcImg = cv::imread(strSrcImg, cv::IMREAD_GRAYSCALE);
	//if (m_SrcImg.empty()) {

	//	return ER_ABORT;
	//}


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_InspectBox_Rect.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}



	// Fool-proof mechanism === === === >>> 


	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, roiRect.rectBox);
	if (res) {

		return ER_ABORT;
	}


	// Fool-proof mechanism <<< === === ===



	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}


	//Algorithm
	m_InspectBox_Rect = roiRect;

	res = cvip_Align.InspectBox_SetParameter(m_InspectBox_Rect);

	//res = cvip_Align.InspectBox_TestIt(&seIn);
	res = cvip_Align.InspectBox_TestIt(m_SrcImg);

	res = cvip_Align.InspectBox_GetResult(&m_Border_Rect);


	//Record the testing results
	*pBoundBoxOut = m_Border_Rect;
	seBoundingBox Border_Rect = m_Border_Rect;


	//Drawing information on Image and save that.

	if (pIP->bDumpRetImg) {

		cv::Mat tmpRes = m_SrcImg.clone();
		cv::cvtColor(tmpRes, tmpRes, COLOR_GRAY2RGB);

		const int iMaskType = 2;

		cv::Point posCenter(Border_Rect.cX, Border_Rect.cY);
		double dbAngle(Border_Rect.dbAngle);
		double dbScale(1.0);
		cv::Size szSize(Border_Rect.rectBox.width, Border_Rect.rectBox.height);

		// Create the rotated rectangle
		cv::RotatedRect rotatedRectangle(posCenter, szSize, dbAngle);

		// We take the edges that OpenCV calculated for us
		cv::Point2f vertices2f[4];
		rotatedRectangle.points(vertices2f);


		if (1 == iMaskType) {

			// Convert them so we can use them in a fillConvexPoly
			cv::Point vertices[4];
			for (int i = 0; i < 4; ++i) {
				vertices[i] = vertices2f[i];
			}
			// Now we can fill the rotated rectangle with our specified color
			cv::fillConvexPoly(tmpRes, vertices, 4, cv::Scalar(255, 0, 255));
		}
		else if (2 == iMaskType) {

			// Convert them so we can use them in a fillConvexPoly
			std::vector<cv::Point> vecVertices;
			for (int i = 0; i < 4; ++i) {
				vecVertices.push_back(vertices2f[i]);
			}
			// Now we can fill the rotated rectangle with our specified color
			cv::polylines(tmpRes, vecVertices, true, cv::Scalar(255, 0, 255), 2);
		}
		else {

			cv::Point posS(Border_Rect.rectBox.left, Border_Rect.rectBox.top);
			cv::Point posE(Border_Rect.rectBox.right, Border_Rect.rectBox.bottom);
			cv::rectangle(tmpRes, posS, posE, cv::Scalar(255, 0, 255), FILLED);
		}


		cv::Point posStart(Border_Rect.rectBox.left, Border_Rect.rectBox.top);
		cv::Point posEnd(Border_Rect.rectBox.right, Border_Rect.rectBox.bottom);

		cv::rectangle(tmpRes, posStart, posEnd, cv::Scalar(255, 255, 0), 3);

		cv::imwrite(strSavePath, tmpRes);

	}


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}


	return res;

}


//int PTWDLL_C_GlueWidth::vbs_InspectBox_Circle(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_InspectBox_Circle(void* pSrcImg,
	seCircle roiCircle,
	const char* strSavePath,
	LPBoundingBox pBoundBoxOut)
{
	int res = 0;

	//if (strSrcImg == nullptr || strSavePath == nullptr || pBoundBoxOut == nullptr)
	if (pSrcImg == nullptr || strSavePath == nullptr || pBoundBoxOut == nullptr)
		return ER_ABORT;


	//argument define


	////Source Image
	//cv::Mat m_SrcImg = cv::imread(strSrcImg, cv::IMREAD_GRAYSCALE);
	//if (m_SrcImg.empty()) {

	//	return ER_ABORT;
	//}


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_InspectBox_Circle.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}



	// Fool-proof mechanism === === === >>> 


	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	cvip_Judgment.Cvt_Circle2Rect(roiCircle, rectTmp2);

	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp2);
	if (res) {

		return ER_ABORT;
	}


	// Fool-proof mechanism <<< === === ===



	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}


	//Algorithm
	m_InspectBox_Circle = roiCircle;

	res = cvip_Align.InspectBox_SetParameter(m_InspectBox_Circle);

	//res = cvip_Align.InspectBox_TestIt(&seIn);
	res = cvip_Align.InspectBox_TestIt(m_SrcImg);

	res = cvip_Align.InspectBox_GetResult(&m_Border_Circle);

	
	//Record the testing results
	*pBoundBoxOut = m_Border_Circle;
	seBoundingBox Border_Circle = m_Border_Circle;


	//Drawing information on Image and save that.

	if (pIP->bDumpRetImg) {

		cv::Mat tmpRes = m_SrcImg.clone();
		cv::cvtColor(tmpRes, tmpRes, COLOR_GRAY2RGB);

		const int iMaskType = 2;

		CCVIPItem::CreateMaskImg(Border_Circle, tmpRes, iMaskType);

		cv::imwrite(strSavePath, tmpRes);

	}


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}


	return res;

}



//int PTWDLL_C_GlueWidth::vbs_InspectBox_CoordCalculate(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_InspectBox_CoordCalculate(void* pSrcImg,
											seBoundingBox seFMarkBox,
											seBoundingBox seInspBox,
											seCoordBindBox seCoorBindBoxIn,
											LPCoordBindBox pCoorBindBoxOut) 
{
	int res = 0;


	res = cvip_Align.CoordBind_Calcu_SetParameter(seFMarkBox, seInspBox, seCoorBindBoxIn);

	res = cvip_Align.CoordBind_Calcu_TestIt();

	res = cvip_Align.CoordBind_Calcu_GetResult(&m_CoorCalcu_AnyShape);


	*pCoorBindBoxOut = m_CoorCalcu_AnyShape;


	return res;
}



//int PTWDLL_C_GlueWidth::vbs_InspectBox_CropImg_Annulus(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_InspectBox_CropImg_Annulus(void* pSrcImg,
	seCoordBindBox seCoordBox,
	seAnnulus roiAnnulus,
	seExpandable* seExpandable,
	LPAnnulus pRoiOffset_Annulus)
{

	int res = 0;

	//	const char* strInputPath = seExpandable->strImage_Input;
	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();


	//if (strSrcImg == nullptr || strSavePath == nullptr || pRoiOffset_Annulus == nullptr)
	if (pSrcImg == nullptr || strSavePath == nullptr || pRoiOffset_Annulus == nullptr)
		return ER_ABORT;


	//Argument define
	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg, m_DestImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_InspectBox_CropImg_Annulus.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	//cv::cvtColor(m_SrcImg, m_SrcImg, COLOR_GRAY2BGR);

	m_DestImg = cv::Mat::zeros(m_SrcImg.size(), m_SrcImg.type());


	// Fool-proof mechanism === === === >>> 

	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	cvip_Judgment.Cvt_Annulus2Rect(roiAnnulus, rectTmp2);

	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp2);
	if (res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === ===


	//Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	//seImageInfo seOut;
	//seOut.iWidth = m_SrcImg.cols;
	//seOut.iHeight = m_SrcImg.rows;
	//seOut.iChannels = m_SrcImg.channels();
	//seOut.pbImgBuf = new unsigned char[seOut.iWidth * seOut.iHeight * seOut.iChannels];


	//Algorithm
	seCropImage_Results myRes;

	res = cvip_Align.CoordBind_CropROI_SetParameter(seCoordBox, roiAnnulus);

	//res = cvip_Align.CoordBind_CropROI_TestIt(&seIn);
	res = cvip_Align.CoordBind_CropROI_TestIt(m_SrcImg);

	//res = cvip_Align.CoordBind_CropROI_GetResult(&seOut, myRes);
	res = cvip_Align.CoordBind_CropROI_GetResult(m_DestImg, myRes);


	//RexTYW, TBD <<==== <<====
	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			seExpandable->poutImgInfo->iWidth = m_DestImg.cols;
			seExpandable->poutImgInfo->iHeight = m_DestImg.rows;
			seExpandable->poutImgInfo->iChannels = m_DestImg.channels();

			CCVIPItem::CvMatToUint8(m_DestImg, seExpandable->poutImgInfo);

			//memcpy(seExpandable->poutImgInfo->pbImgBuf, seOut.pbImgBuf, seOut.iWidth * seOut.iHeight * seOut.iChannels*sizeof(char));
		}
	}



	//Record the testing results
	*pRoiOffset_Annulus = myRes.seOffsetCrop_Annulus;

	//Drawing information on Image and save that.
	//cv::Mat tmpRes;
	//CCVIPItem::Uint8ToCvMat(&seOut, tmpRes);
	//cv::imwrite(strSavePath, tmpRes);
	//cv::imwrite(strResultPath, tmpRes);

	if (pIP->bDumpRetImg) {

		cv::imwrite(strSavePath, m_DestImg);
		cv::imwrite(strResultPath, m_DestImg);

	}

	//Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	//if (seOut.pbImgBuf) {

	//	delete[] seOut.pbImgBuf;
	//	seOut.pbImgBuf = nullptr;
	//}


	return res;

}



//int PTWDLL_C_GlueWidth::vbs_InspectBox_CropImg_Rect(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_InspectBox_CropImg_Rect(void* pSrcImg,
	seCoordBindBox seCoordBox,
	seBoundingBox roiRect,
	seExpandable* seExpandable,
	LPBoundingBox pRoiOffset_Rect)
{

	int res = 0;


	//	const char* strInputPath = seExpandable->strImage_Input;
	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();


	//if (strSrcImg == nullptr || strSavePath == nullptr || pRoiOffset_Rect == nullptr)
	if (pSrcImg == nullptr || strSavePath == nullptr || pRoiOffset_Rect == nullptr)
		return ER_ABORT;


	//Argument define
	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg, m_DestImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_InspectBox_CropImg_Rect.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	m_DestImg = cv::Mat::zeros(m_SrcImg.size(), m_SrcImg.type());


	// Fool-proof mechanism === === === >>> 

	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	//cvip_Judgment.Cvt_Annulus2Rect(roiAnnulus, rectTmp2);

	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, roiRect.rectBox);
	if (res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === ===


	//Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	//seImageInfo seOut;
	//seOut.iWidth = m_SrcImg.cols;
	//seOut.iHeight = m_SrcImg.rows;
	//seOut.iChannels = m_SrcImg.channels();
	//seOut.pbImgBuf = new unsigned char[seOut.iWidth * seOut.iHeight * seOut.iChannels];


	//Algorithm
	seCropImage_Results myRes;

	res = cvip_Align.CoordBind_CropROI_SetParameter(seCoordBox, roiRect);

	//res = cvip_Align.CoordBind_CropROI_TestIt(&seIn);
	res = cvip_Align.CoordBind_CropROI_TestIt(m_SrcImg);

	//res = cvip_Align.CoordBind_CropROI_GetResult(&seOut, myRes);
	res = cvip_Align.CoordBind_CropROI_GetResult(m_DestImg, myRes);


	//RexTYW, TBD <<==== <<====
	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			seExpandable->poutImgInfo->iWidth = m_DestImg.cols;
			seExpandable->poutImgInfo->iHeight = m_DestImg.rows;
			seExpandable->poutImgInfo->iChannels = m_DestImg.channels();

			CCVIPItem::CvMatToUint8(m_DestImg, seExpandable->poutImgInfo);

			//memcpy(seExpandable->poutImgInfo->pbImgBuf, seOut.pbImgBuf, seOut.iWidth * seOut.iHeight * seOut.iChannels*sizeof(char));
		}
	}



	//Record the testing results
	*pRoiOffset_Rect = myRes.seOffsetCrop_Rect;


	//Drawing information on Image and save that.
	//cv::Mat tmpRes;
	//CCVIPItem::Uint8ToCvMat(&seOut, tmpRes);
	//cv::imwrite(strSavePath, tmpRes);
	//cv::imwrite(strResultPath, tmpRes);

	if (pIP->bDumpRetImg) {

		cv::imwrite(strSavePath, m_DestImg);
		cv::imwrite(strResultPath, m_DestImg);

	}

	//Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	//if (seOut.pbImgBuf) {

	//	delete[] seOut.pbImgBuf;
	//	seOut.pbImgBuf = nullptr;
	//}


	return res;

}


//int PTWDLL_C_GlueWidth::vbs_InspectBox_CropImg_Circle(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_InspectBox_CropImg_Circle(void* pSrcImg,
	seCoordBindBox seCoordBox,
	seCircle roiCircle,
	seExpandable* seExpandable,
	LPCircle pRoiOffset_Circle)
{

	int res = 0;


	//	const char* strInputPath = seExpandable->strImage_Input;
	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();

	//if (strSrcImg == nullptr || strSavePath == nullptr || pRoiOffset_Circle == nullptr)
	if (pSrcImg == nullptr || strSavePath == nullptr || pRoiOffset_Circle == nullptr)
		return ER_ABORT;


	//Argument define
	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg, m_DestImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_InspectBox_CropImg_Circle.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	m_DestImg = cv::Mat::zeros(m_SrcImg.size(), m_SrcImg.type());


	// Fool-proof mechanism === === === >>> 

	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	cvip_Judgment.Cvt_Circle2Rect(roiCircle, rectTmp2);

	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp2);
	if (res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === ===


	//Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	//seImageInfo seOut;
	//seOut.iWidth = m_SrcImg.cols;
	//seOut.iHeight = m_SrcImg.rows;
	//seOut.iChannels = m_SrcImg.channels();
	//seOut.pbImgBuf = new unsigned char[seOut.iWidth * seOut.iHeight * seOut.iChannels];


	//Algorithm
	seCropImage_Results myRes;

	res = cvip_Align.CoordBind_CropROI_SetParameter(seCoordBox, roiCircle);

	//res = cvip_Align.CoordBind_CropROI_TestIt(&seIn);
	res = cvip_Align.CoordBind_CropROI_TestIt(m_SrcImg);

	//res = cvip_Align.CoordBind_CropROI_GetResult(&seOut, myRes);
	res = cvip_Align.CoordBind_CropROI_GetResult(m_DestImg, myRes);


	//RexTYW, TBD <<==== <<====
	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			seExpandable->poutImgInfo->iWidth = m_DestImg.cols;
			seExpandable->poutImgInfo->iHeight = m_DestImg.rows;
			seExpandable->poutImgInfo->iChannels = m_DestImg.channels();

			CCVIPItem::CvMatToUint8(m_DestImg, seExpandable->poutImgInfo);

			//memcpy(seExpandable->poutImgInfo->pbImgBuf, seOut.pbImgBuf, seOut.iWidth * seOut.iHeight * seOut.iChannels*sizeof(char));
		}
	}



	//Record the testing results
	*pRoiOffset_Circle = myRes.seOffsetCrop_Circle;


	//Drawing information on Image and save that.
	//cv::Mat tmpRes;
	//CCVIPItem::Uint8ToCvMat(&seOut, tmpRes);
	//cv::imwrite(strSavePath, tmpRes);
	//cv::imwrite(strResultPath, tmpRes);

	if (pIP->bDumpRetImg) {

		cv::imwrite(strSavePath, m_DestImg);
		cv::imwrite(strResultPath, m_DestImg);

	}

	//Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	//if (seOut.pbImgBuf) {

	//	delete[] seOut.pbImgBuf;
	//	seOut.pbImgBuf = nullptr;
	//}


	return res;

}


//int PTWDLL_C_GlueWidth::vbs_Histogram_Annulus(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_Histogram_Annulus(void* pSrcImg, 
	seAnnulus roiAnnulus, 
	seExpandable* seExpandable,
	int* pDataCntOut, 
	double* p1DArrayOut)
{
	int res = 0;

	
	//	const char* strInputPath = seExpandable->strImage_Input;
	//const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();


	//if (strSrcImg == nullptr || strResultPath == nullptr || pDataCntOut == nullptr || p1DArrayOut == nullptr)
	if (pSrcImg == nullptr || strResultPath == nullptr || pDataCntOut == nullptr || p1DArrayOut == nullptr)
		return ER_ABORT;


	//Argument define
	double db1DArray[256 * 3];

	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Histogram_Annulus.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	// Fool-proof mechanism === === === >>> 

	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	cvip_Judgment.Cvt_Annulus2Rect(roiAnnulus, rectTmp2);

	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp2);
	if (res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === ===


	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}


	//Algorithm
	//res = cvip_IP.IpHistogram_SetParameter(&seIn, roiAnnulus);
	res = cvip_IP.IpHistogram_SetParameter(m_SrcImg, roiAnnulus);

	res = cvip_IP.IpHistogram_TestIt();

	res = cvip_IP.IpHistogram_GetResult(db1DArray);


	//Record the testing results
	int arrlen = m_SrcImg.channels() * 256;
	vector<double> valHist(db1DArray, db1DArray + arrlen);

	*pDataCntOut = valHist.size();
	//*p1DArrayOut = valHist.data();
	std::copy(valHist.begin(), valHist.end(), p1DArrayOut);


	//Drawing information on Image and save that.

	if (pIP->bDumpRetImg) {

		cv::Mat hist(valHist);

		double minValue(0.0);		//����Ϥ��̤p��bin����
		double maxValue(0.0);		//����Ϥ��̤j��bin����
		cv::minMaxLoc(hist, &minValue, &maxValue, 0, 0);	//minMaxLoc�i�H�p��̤j�ȳ̤p�ȥH�Ψ��������m �o�̨D�̤j��

		double histHeight = 256.0;	//�nø�s����Ϫ��̤j����
		int histSize = 256;			//����ϨC�@����bin�Ӽ�
		int scale = 3;				//����v�����e�j�p

		cv::Mat histImg = cv::Mat::zeros(cv::Size(histSize * scale, histSize), CV_8UC1);//�Ω���ܪ����		

		float* p = hist.ptr<float>(0);
		for (size_t i = 0; i < histSize; i++)//�i�檽��Ϫ�ø�s
		{
			float bin_val = p[i];
			int intensity = cvRound((bin_val * histHeight) / maxValue);  //�nø�s������ 

			//_fordebug
			for (size_t j = 0; j < scale; j++) //ø�s���u �o�̥ΨCscale�����u�N��@��bin
			{
				cv::line(histImg, cv::Point(i * scale + j, histHeight - intensity), cv::Point(i * scale + j, histHeight - 1), 255);
			}

		}

		cv::imwrite(strResultPath, histImg);

	}


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	return res;
}


//int PTWDLL_C_GlueWidth::vbs_Histogram_Rect(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_Histogram_Rect(void* pSrcImg,
	seBoundingBox roiRect,
	seExpandable* seExpandable,
	int* pDataCntOut,
	double* p1DArrayOut)
{
	int res = 0;


	//	const char* strInputPath = seExpandable->strImage_Input;
	//const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();


	//if (strSrcImg == nullptr || strResultPath == nullptr || pDataCntOut == nullptr || p1DArrayOut == nullptr)
	if (pSrcImg == nullptr || strResultPath == nullptr || pDataCntOut == nullptr || p1DArrayOut == nullptr)
		return ER_ABORT;


	//Argument define
	double db1DArray[256 * 3];

	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Histogram_Rect.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}


	// Fool-proof mechanism === === === >>> 

	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	//cvip_Judgment.Cvt_Annulus2Rect(roiAnnulus, rectTmp2);

	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, roiRect.rectBox);
	if (res) {

		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === ===


	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}


	//Algorithm
	//res = cvip_IP.IpHistogram_SetParameter(&seIn, roiRect);
	res = cvip_IP.IpHistogram_SetParameter(m_SrcImg, roiRect);

	res = cvip_IP.IpHistogram_TestIt();

	res = cvip_IP.IpHistogram_GetResult(db1DArray);


	//Record the testing results
	int arrlen = m_SrcImg.channels() * 256;
	vector<double> valHist(db1DArray, db1DArray + arrlen);

	*pDataCntOut = valHist.size();
	std::copy(valHist.begin(), valHist.end(), p1DArrayOut);


	//Drawing information on Image and save that.

	if (pIP->bDumpRetImg) {

		cv::Mat hist(valHist);

		double minValue(0.0);		//����Ϥ��̤p��bin����
		double maxValue(0.0);		//����Ϥ��̤j��bin����
		cv::minMaxLoc(hist, &minValue, &maxValue, 0, 0);	//minMaxLoc�i�H�p��̤j�ȳ̤p�ȥH�Ψ��������m �o�̨D�̤j��

		double histHeight = 256.0;	//�nø�s����Ϫ��̤j����
		int histSize = 256;			//����ϨC�@����bin�Ӽ�
		int scale = 3;				//����v�����e�j�p

		cv::Mat histImg = cv::Mat::zeros(cv::Size(histSize * scale, histSize), CV_8UC1);//�Ω���ܪ����		

		float* p = hist.ptr<float>(0);
		for (size_t i = 0; i < histSize; i++)//�i�檽��Ϫ�ø�s
		{
			float bin_val = p[i];
			int intensity = cvRound((bin_val * histHeight) / maxValue);  //�nø�s������ 

			//_fordebug
			for (size_t j = 0; j < scale; j++) //ø�s���u �o�̥ΨCscale�����u�N��@��bin
			{
				cv::line(histImg, cv::Point(i * scale + j, histHeight - intensity), cv::Point(i * scale + j, histHeight - 1), 255);
			}

		}

		cv::imwrite(strResultPath, histImg);

	}


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	return res;
}


//int PTWDLL_C_GlueWidth::vbs_Histogram_Circle(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_Histogram_Circle(void* pSrcImg, 
	seCircle roiCircle, 
	seExpandable* seExpandable,
	int* pDataCntOut, 
	double* p1DArrayOut)
{
	int res = 0;


	//	const char* strInputPath = seExpandable->strImage_Input;
	//const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();



	//if (strSrcImg == nullptr || strResultPath == nullptr || pDataCntOut == nullptr || p1DArrayOut == nullptr)
	if (pSrcImg == nullptr || strResultPath == nullptr || pDataCntOut == nullptr || p1DArrayOut == nullptr)
		return ER_ABORT;


	//Argument define
	double db1DArray[256 * 3];

	////Source Image
	//cv::Mat m_SrcImg = cv::imread(strSrcImg, cv::IMREAD_GRAYSCALE);
	//if (m_SrcImg.empty()) {

	//	return ER_ABORT;
	//}
	////cv::cvtColor(m_SrcImg, m_SrcImg, COLOR_GRAY2BGR);


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Histogram_Circle.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}



	// Fool-proof mechanism === === === >>> 


	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	cvip_Judgment.Cvt_Circle2Rect(roiCircle, rectTmp2);

	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp2);
	if (res) {

		return ER_ABORT;
	}


	// Fool-proof mechanism <<< === === ===


	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}


	//Algorithm
	//res = cvip_IP.IpHistogram_SetParameter(&seIn, roiCircle);
	res = cvip_IP.IpHistogram_SetParameter(m_SrcImg, roiCircle);

	res = cvip_IP.IpHistogram_TestIt();

	res = cvip_IP.IpHistogram_GetResult(db1DArray);


	//Record the testing results
	int arrlen = m_SrcImg.channels() * 256;
	vector<double> valHist(db1DArray, db1DArray + arrlen);

	*pDataCntOut = valHist.size();
	std::copy(valHist.begin(), valHist.end(), p1DArrayOut);


	//Drawing information on Image and save that.

	if (pIP->bDumpRetImg) {

		cv::Mat hist(valHist);

		double minValue(0.0);		//����Ϥ��̤p��bin����
		double maxValue(0.0);		//����Ϥ��̤j��bin����
		cv::minMaxLoc(hist, &minValue, &maxValue, 0, 0);	//minMaxLoc�i�H�p��̤j�ȳ̤p�ȥH�Ψ��������m �o�̨D�̤j��

		double histHeight = 256.0;	//�nø�s����Ϫ��̤j����
		int histSize = 256;			//����ϨC�@����bin�Ӽ�
		int scale = 3;				//����v�����e�j�p

		cv::Mat histImg = cv::Mat::zeros(cv::Size(histSize * scale, histSize), CV_8UC1);//�Ω���ܪ����		

		float* p = hist.ptr<float>(0);
		for (size_t i = 0; i < histSize; i++)//�i�檽��Ϫ�ø�s
		{
			float bin_val = p[i];
			int intensity = cvRound((bin_val * histHeight) / maxValue);  //�nø�s������ 

			//_fordebug
			for (size_t j = 0; j < scale; j++) //ø�s���u �o�̥ΨCscale�����u�N��@��bin
			{
				cv::line(histImg, cv::Point(i * scale + j, histHeight - intensity), cv::Point(i * scale + j, histHeight - 1), 255);
			}

		}

		cv::imwrite(strResultPath, histImg);

	}


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	return res;
}



//int PTWDLL_C_GlueWidth::vbs_Threshold(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_Threshold(void* pSrcImg, 
	double* pThresh, 
	double* pMaxVal, 
	emThresholdTypes emTypes, 
	seExpandable* seExpandable)
{
	int res = 0;


	//	const char* strInputPath = seExpandable->strImage_Input;
	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();



	//if (strSrcImg == nullptr || pThresh == nullptr || pMaxVal == nullptr)
	if (pSrcImg == nullptr || pThresh == nullptr || pMaxVal == nullptr )
		return ER_ABORT;


	//Argument define


	////Source Image
	//cv::Mat m_SrcImg = cv::imread(strSrcImg, cv::IMREAD_GRAYSCALE);
	//if (m_SrcImg.empty()) {

	//	return ER_ABORT;
	//}
	////cv::cvtColor(m_SrcImg, m_SrcImg, COLOR_GRAY2BGR);


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg, m_DestImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Threshold.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	m_DestImg = cv::Mat::zeros(m_SrcImg.size(), m_SrcImg.type());

	//Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	//seImageInfo seOut;
	//seOut.iWidth = m_SrcImg.cols;
	//seOut.iHeight = m_SrcImg.rows;
	//seOut.iChannels = m_SrcImg.channels();
	//seOut.pbImgBuf = new unsigned char[seOut.iWidth * seOut.iHeight * seOut.iChannels];


	//Algorithm
	//res = cvip_IP.IpThreshold_SetParameter(seIn.iChannels, pThresh, pMaxVal, emTypes);
	res = cvip_IP.IpThreshold_SetParameter(m_SrcImg.channels(), pThresh, pMaxVal, emTypes);

	//res = cvip_IP.IpThreshold_TestIt(&seIn);
	res = cvip_IP.IpThreshold_TestIt(m_SrcImg);

	//res = cvip_IP.IpThreshold_GetResult(&seOut);
	res = cvip_IP.IpThreshold_GetResult(m_DestImg);
	


	//RexTYW, TBD <<==== <<====
	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			seExpandable->poutImgInfo->iWidth = m_DestImg.cols;
			seExpandable->poutImgInfo->iHeight = m_DestImg.rows;
			seExpandable->poutImgInfo->iChannels = m_DestImg.channels();

			CCVIPItem::CvMatToUint8(m_DestImg, seExpandable->poutImgInfo);

			//memcpy(seExpandable->poutImgInfo->pbImgBuf, seOut.pbImgBuf, seOut.iWidth * seOut.iHeight * seOut.iChannels * sizeof(char));
		}
	}



	//Record the testing results



	////Drawing information on Image and save that.
	//cv::Mat tmpRes;
	//CCVIPItem::Uint8ToCvMat(&seOut, tmpRes);
	//cv::imwrite(strSavePath, tmpRes);
	//cv::imwrite(strResultPath, tmpRes);

	if (pIP->bDumpRetImg) {

		cv::imwrite(strSavePath, m_DestImg);
		cv::imwrite(strResultPath, m_DestImg);

	}

	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	//if (seOut.pbImgBuf) {

	//	delete[] seOut.pbImgBuf;
	//	seOut.pbImgBuf = nullptr;
	//}


	return 0;
}



//int PTWDLL_C_GlueWidth::vbs_Morphology(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_Morphology(void* pSrcImg, 
	emMorphShapes emShapes, 
	int iKSize, 
	emMorphOperation emOperation, 
	seExpandable* seExpandable)
{
	int res = 0;


	//	const char* strInputPath = seExpandable->strImage_Input;
	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();


	//if (strSrcImg == nullptr)
	if (pSrcImg == nullptr) {
		return ER_ABORT;
	}


	//Argument define

	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg, m_DestImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_Morphology.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	m_DestImg = cv::Mat::zeros(m_SrcImg.size(), m_SrcImg.type());


	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	//seImageInfo seOut;
	//seOut.iWidth = m_SrcImg.cols;
	//seOut.iHeight = m_SrcImg.rows;
	//seOut.iChannels = m_SrcImg.channels();
	//seOut.pbImgBuf = new unsigned char[seOut.iWidth * seOut.iHeight * seOut.iChannels];


	//Algorithm
	res = cvip_IP.IpMorphology_SetParameter(emShapes, iKSize);

	//res = cvip_IP.IpMorphology_TestIt(&seIn, emOperation);
	res = cvip_IP.IpMorphology_TestIt(m_SrcImg, emOperation);

	//res = cvip_IP.IpMorphology_GetResult(&seOut);
	res = cvip_IP.IpMorphology_GetResult(m_DestImg);

	//RexTYW, TBD <<==== <<====
	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			seExpandable->poutImgInfo->iWidth = m_SrcImg.cols;
			seExpandable->poutImgInfo->iHeight = m_SrcImg.rows;
			seExpandable->poutImgInfo->iChannels = m_SrcImg.channels();

			CCVIPItem::CvMatToUint8(m_DestImg, seExpandable->poutImgInfo);

			//memcpy(seExpandable->poutImgInfo->pbImgBuf, seOut.pbImgBuf, seOut.iWidth * seOut.iHeight * seOut.iChannels * sizeof(char));
		}
	}



	//Record the testing results



	////Drawing information on Image and save that.
	//cv::Mat tmpRes;
	//CCVIPItem::Uint8ToCvMat(&seOut, tmpRes);
	//cv::imwrite(strSavePath, tmpRes);
	//cv::imwrite(strResultPath, tmpRes);

	if (pIP->bDumpRetImg) {

		cv::imwrite(strSavePath, m_DestImg);
		cv::imwrite(strResultPath, m_DestImg);

	}

	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	//if (seOut.pbImgBuf) {

	//	delete[] seOut.pbImgBuf;
	//	seOut.pbImgBuf = nullptr;
	//}


	return 0;
}



//int PTWDLL_C_GlueWidth::vbs_NoiseRemoval(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_NoiseRemoval(void* pSrcImg, 
	double dbLimit_min, 
	double dbLimit_max, 
	seExpandable* seExpandable)
{
	int res = 0;


	//	const char* strInputPath = seExpandable->strImage_Input;
	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();


	//if (strSrcImg == nullptr)
	if (pSrcImg == nullptr) {
		return ER_ABORT;
	}


	//Argument define

	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg, m_DestImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_NoiseRemoval.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	m_DestImg = cv::Mat::zeros(m_SrcImg.size(), m_SrcImg.type());


	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	//seImageInfo seOut;
	//seOut.iWidth = m_SrcImg.cols;
	//seOut.iHeight = m_SrcImg.rows;
	//seOut.iChannels = m_SrcImg.channels();
	//seOut.pbImgBuf = new unsigned char[seOut.iWidth * seOut.iHeight * seOut.iChannels];


	//Algorithm
	res = cvip_IP.IpNoiseRemoval_SetParameter(dbLimit_min, dbLimit_max);

	//res = cvip_IP.IpNoiseRemoval_TestIt(&seIn);
	res = cvip_IP.IpNoiseRemoval_TestIt(m_SrcImg);

	//res = cvip_IP.IpNoiseRemoval_GetResult(&seOut);
	res = cvip_IP.IpNoiseRemoval_GetResult(m_DestImg);


	//RexTYW, TBD <<==== <<====
	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			seExpandable->poutImgInfo->iWidth = m_SrcImg.cols;
			seExpandable->poutImgInfo->iHeight = m_SrcImg.rows;
			seExpandable->poutImgInfo->iChannels = m_SrcImg.channels();

			CCVIPItem::CvMatToUint8(m_DestImg, seExpandable->poutImgInfo);

			//memcpy(seExpandable->poutImgInfo->pbImgBuf, seOut.pbImgBuf, seOut.iWidth * seOut.iHeight * seOut.iChannels * sizeof(char));
		}
	}


	//Record the testing resultss

	////Drawing information on Image and save that.
	//cv::Mat tmpRes;
	//CCVIPItem::Uint8ToCvMat(&seOut, tmpRes);
	//cv::imwrite(strSavePath, tmpRes);
	//cv::imwrite(strResultPath, tmpRes);

	if (pIP->bDumpRetImg) {

		cv::imwrite(strSavePath, m_DestImg);
		cv::imwrite(strResultPath, m_DestImg);

	}

	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	//if (seOut.pbImgBuf) {

	//	delete[] seOut.pbImgBuf;
	//	seOut.pbImgBuf = nullptr;
	//}


	return 0;
}



int PTWDLL_C_GlueWidth::vbs_DataAugmentation(void* pSrcImg,
	seDataAugmentationInfo seDA_Param,
	seExpandable* seExpandable)
{
	int res = 0;

	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();

	seDA_Param.strFileName = seExpandable->strImage_Save;

	if (pSrcImg == nullptr || seDA_Param.strFileName.empty()) {
		return ER_ABORT;
	}

	//Argument define
	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
				return ER_ABORT;
			}

#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_DataAugmentation.png", m_SrcImg);
			}
#endif
		}
		else {

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		return ER_ABORT;
	}

	//Algorithm
	res = cvip_IP.IpDataAugmeatation_SetParameter(seDA_Param);

	res = cvip_IP.IpDataAugmentation_TestIt(m_SrcImg);

	res = cvip_IP.IpDataAugmentation_GetResult();


	////RexTYW, TBD <<==== <<====
	//if (pIP->bSelInputSrc) {

	//	if (pIP->ptrImgInfo) {

	//		seExpandable->poutImgInfo->iWidth = m_SrcImg.cols;
	//		seExpandable->poutImgInfo->iHeight = m_SrcImg.rows;
	//		seExpandable->poutImgInfo->iChannels = m_SrcImg.channels();

	//		CCVIPItem::CvMatToUint8(m_DestImg, seExpandable->poutImgInfo);

	//	}
	//}

	////Record the testing resultss
	//if (pIP->bDumpRetImg) {

	//	cv::imwrite(strSavePath, m_DestImg);
	//	cv::imwrite(strResultPath, m_DestImg);

	//}

	return 0;
}



//int PTWDLL_C_GlueWidth::vbs_GlueWidth_Measure_Annulus(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_GlueWidth_Measure_Annulus(void* pSrcImg,
	seAnnulus roiAnnuls,
	int stepSize,
	seExpandable* seExpandable,
	int* pDataCntOut,
	double* pLength_InnerOut,
	double* pLength_OuterOut,
	sePoint* pPosition_InnerOut,
	sePoint* pPosition_OuterOut,
	double* pGlueAreaOut)
{
	int res = 0;


	//	const char* strInputPath = seExpandable->strImage_Input;
	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();


	if (pSrcImg == nullptr) {

		IPLE(" Error !!!, the pSrcImg is nullptr.\n");
		return ER_ABORT;
	}


	//Argument define
	seGlueWidth_Results myRes;


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {

				IPLE(" Error !!!, Can not convert the ptrImgInfo to cv::Mat of OpenCV.\n");
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_GlueWidth_Measure_Annulus.png", m_SrcImg);
			}
#endif
		}
		else {

			IPLE(" Error !!!, the ptrImgInfo is nullptr.\n");

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		IPLE(" Error !!!, the Source Image is empty.\n");
		return ER_ABORT;
	}


	// Fool-proof mechanism === === === >>> 

	seRect rectTmp1 = seRect(), rectTmp2 = seRect();

	rectTmp1.width = m_SrcImg.cols;
	rectTmp1.height = m_SrcImg.rows;

	cvip_Judgment.Cvt_Annulus2Rect(roiAnnuls, rectTmp2);

	res = cvip_Judgment.Judg_IsOverRange(rectTmp1, rectTmp2);
	if (res) {

		IPLE(" Error !!!, the ROI range out of Source Image size.\n");
		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === === <<<



	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	//seImageInfo seOut;
	//seOut.iWidth = m_SrcImg.cols;
	//seOut.iHeight = m_SrcImg.rows;
	//seOut.iChannels = m_SrcImg.channels();
	//seOut.pbImgBuf = new unsigned char[seOut.iWidth * seOut.iHeight * seOut.iChannels];


	//Algorithm
	res = cvip_Measure.MeasGlueWidth_SetParameter(roiAnnuls, stepSize);

	//res = cvip_Measure.MeasGlueWidth_TestIt(&seIn);
	res = cvip_Measure.MeasGlueWidth_TestIt(m_SrcImg);

	res = cvip_Measure.MeasGlueWidth_GetResult(myRes);

	res = cvip_Measure.MeasGlueWidth_GetRetImg(strResultPath);


	//Record the testing results
	*pDataCntOut = myRes.Length_Outer.size();

	std::copy(myRes.Length_Inner.begin(), myRes.Length_Inner.end(), pLength_InnerOut);
	std::copy(myRes.Length_Outer.begin(), myRes.Length_Outer.end(), pLength_OuterOut);

	std::copy(myRes.Position_Inner.begin(), myRes.Position_Inner.end(), pPosition_InnerOut);
	std::copy(myRes.Position_Outer.begin(), myRes.Position_Outer.end(), pPosition_OuterOut);

	*pGlueAreaOut = myRes.GlueArea;


	//Drawing information on Image and save that.
	//cv::Mat tmpRes;
	//CCVIPItem::Uint8ToCvMat(&seOut, tmpRes);
	//cv::imwrite(strSavePath, tmpRes);


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	//if (seOut.pbImgBuf) {

	//	delete[] seOut.pbImgBuf;
	//	seOut.pbImgBuf = nullptr;
	//}


	return 0;


}



//int PTWDLL_C_GlueWidth::vbs_GlueWidth_Measure_Rect(const char* strSrcImg,
int PTWDLL_C_GlueWidth::vbs_GlueWidth_Measure_Rect(void* pSrcImg,
	seBoundingBox roiRect,
	int stepSize,
	seExpandable* seExpandable,
	int* pDataCntOut,
	double* pLength_InnerOut,
	double* pLength_OuterOut,
	sePoint* pPosition_InnerOut,
	sePoint* pPosition_OuterOut,
	double* pGlueAreaOut)
{
	int res = 0;


	//	const char* strInputPath = seExpandable->strImage_Input;
	const char* strSavePath = seExpandable->strImage_Save.c_str();
	const char* strResultPath = seExpandable->strImage_Result.c_str();


	//if (strSrcImg == nullptr)
	if (pSrcImg == nullptr) {

		IPLE(" Error !!!, the pSrcImg is nullptr.\n");
		return ER_ABORT;
	}


	//Argument define
	seGlueWidth_Results myRes;


	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;

	cv::Mat m_SrcImg;

	if (pIP->bSelInputSrc) {

		if (pIP->ptrImgInfo) {

			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {

				IPLE(" Error !!!, Can not convert the ptrImgInfo to cv::Mat of OpenCV.\n");
				return ER_ABORT;
			}
#ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
			if (!m_SrcImg.empty()) {
				cv::imwrite("/home/user/primax/vsb/grap/vbs_GlueWidth_Measure_Rect.png", m_SrcImg);
			}
#endif
		}
		else {

			IPLE(" Error !!!, the ptrImgInfo is nullptr.\n");

			return ER_ABORT;
		}
	}
	else {

		// Source Image and Template Image
		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

	}

	if (m_SrcImg.empty()) {

		IPLE(" Error !!!, the Source Image is empty.\n");
		return ER_ABORT;
	}

	// Fool-proof mechanism >>> === === === >>> 
	seRect rectTmp = seRect();
	rectTmp.width = m_SrcImg.cols;
	rectTmp.height = m_SrcImg.rows;

	res = cvip_Judgment.Judg_IsOverRange(rectTmp, roiRect.rectBox);
	if (res) {

		IPLE(" Error !!!, the ROI range out of Source Image size.\n");
		return ER_ABORT;
	}

	// Fool-proof mechanism <<< === === === <<<



	////Convert to structure of seImageInfo.
	//seImageInfo seIn;
	//seIn.iWidth = m_SrcImg.cols;
	//seIn.iHeight = m_SrcImg.rows;
	//seIn.iChannels = m_SrcImg.channels();
	//seIn.pbImgBuf = new unsigned char[seIn.iWidth * seIn.iHeight * seIn.iChannels];
	//if (CCVIPItem::CvMatToUint8(m_SrcImg, &seIn) < ER_OK) {
	//	return ER_ABORT;
	//}

	//seImageInfo seOut;
	//seOut.iWidth = m_SrcImg.cols;
	//seOut.iHeight = m_SrcImg.rows;
	//seOut.iChannels = m_SrcImg.channels();
	//seOut.pbImgBuf = new unsigned char[seOut.iWidth * seOut.iHeight * seOut.iChannels];


	//Algorithm
	res = cvip_Measure.MeasGlueWidth_SetParameter(roiRect, stepSize);

	//res = cvip_Measure.MeasGlueWidth_TestIt(&seIn);
	res = cvip_Measure.MeasGlueWidth_TestIt(m_SrcImg);

	res = cvip_Measure.MeasGlueWidth_GetResult(myRes);

	res = cvip_Measure.MeasGlueWidth_GetRetImg(strResultPath);


	//Record the testing results
	*pDataCntOut = myRes.Length_Outer.size();

	std::copy(myRes.Length_Inner.begin(), myRes.Length_Inner.end(), pLength_InnerOut);
	std::copy(myRes.Length_Outer.begin(), myRes.Length_Outer.end(), pLength_OuterOut);

	std::copy(myRes.Position_Inner.begin(), myRes.Position_Inner.end(), pPosition_InnerOut);
	std::copy(myRes.Position_Outer.begin(), myRes.Position_Outer.end(), pPosition_OuterOut);

	*pGlueAreaOut = myRes.GlueArea;


	//Drawing information on Image and save that.
	//cv::Mat tmpRes;
	//CCVIPItem::Uint8ToCvMat(&seOut, tmpRes);
	//cv::imwrite(strSavePath, tmpRes);


	////Release buffer
	//if (seIn.pbImgBuf) {

	//	delete[] seIn.pbImgBuf;
	//	seIn.pbImgBuf = nullptr;
	//}

	//if (seOut.pbImgBuf) {

	//	delete[] seOut.pbImgBuf;
	//	seOut.pbImgBuf = nullptr;
	//}


	return 0;


}


// int PTWDLL_C_GlueWidth::vsb_AiELIC_Initialize(seAI_FUNC_Init seParam)
// {
// 	int res = 0;

// 	const char* pMode_Project = seParam.strMode_Project.c_str();
// 	const char* pMode_Name = seParam.strMode_Name.c_str();
// 	const char* pMode_Attr = seParam.strMode_Attr.c_str();

// 	//Algorithm
// 	res = cvai_elic.ELIC_Init(pMode_Project, pMode_Name, pMode_Attr);
// 	if (res) {
// 		return res;
// 	}

// 	return res;
// }



// int PTWDLL_C_GlueWidth::vbs_AiELIC_GlueWidth_Measure_Color(void* pSrcImg,
// 	seExpandable* seExpandable,
// 	seAiELIC_MeasGlueWidth_Color seAiELIC_Param, 
// 	char* szPredClassm, 
// 	double* dbTime)
// {
// 	int res = 0;

// 	//fonly for debug.start
// 	const char* strInputPath = seExpandable->strImage_Input.c_str();
// 	//fonly for debug.end

// 	const char* strSavePath = seExpandable->strImage_Save.c_str();
// 	const char* strResultPath = seExpandable->strImage_Result.c_str();


// 	if (pSrcImg == nullptr)
// 		return ER_ABORT;


// 	//Argument define
// 	seImgSrcTab* pIP = (seImgSrcTab*)pSrcImg;


// 	cv::Mat m_SrcImg;

// 	if (pIP->bSelInputSrc) {	//0: Path of SrcImg, 1:pointer of LPImageInfo

// 		if (pIP->ptrImgInfo) {

// 			if (CCVIPItem::Uint8ToCvMat(const_cast<LPImageInfo>(pIP->ptrImgInfo), m_SrcImg) < ER_OK) {
// 				return ER_ABORT;
// 			}
// #ifdef ALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
// 			if (!m_SrcImg.empty()) {
// 				cv::imwrite("/home/user/primax/vsb/grap/vbs_AiELIC_GlueWidth_Measure_Color.png", m_SrcImg);
// 			}
// #endif
// 		}
// 		else {

// 			return ER_ABORT;
// 		}
// 	}
// 	else {

// 		// Source Image and Template Image
// 		m_SrcImg = cv::imread(pIP->strSrcImg, cv::IMREAD_GRAYSCALE);

// 	}

// 	if (m_SrcImg.empty()) {

// 		return ER_ABORT;
// 	}

// 	//m_DestImg = cv::Mat::zeros(m_SrcImg.size(), m_SrcImg.type());


// 	//Algorithm
// 	seAiELIC_GlueWidth_Color_Results myRes;

// 	//const char* pMode_Project = seAiELIC_Param.strMode_Project.c_str();
// 	//const char* pMode_Name = seAiELIC_Param.strMode_Name.c_str();
// 	//const char* pMode_Attr = seAiELIC_Param.strMode_Attr.c_str();

// 	//res = cvai_elic.ELIC_Init(pMode_Project, pMode_Name, pMode_Attr);
// 	//if (res) {
// 	//	return res;
// 	//}

// 	//res = cvai_elic.ELIC_Inference(strInputPath);	//use image path
// 	res = cvai_elic.ELIC_Inference(m_SrcImg);		//usiing image buffer
// 	if (res) {
// 		return res;
// 	}

// 	res = cvai_elic.ELIC_GetResults(myRes);
// 	if (res) {
// 		return res;
// 	}

// 	//Record the testing results
// 	*szPredClassm = myRes.strPredict_Class.c_str();
// 	*dbTime = myRes.dbTime;

// 	return 0;

// }








//int template()
//{
// 
// //int res =0
// 
//	//argument define
//
//	//Source Image and Template Image
//
//	//Convert to structure of seImageInfo.
//
//	//Algorithm
//
//	//Drawing information on Image and save that.
//
//	//Record the testing results
//
//	//Release buffer
//
//	//return res;
//}















PTWDLL_C_GlueWidth::PTWDLL_C_GlueWidth()
:strpath_LOG("d:\\ptwdll_log.txt")
{ 
	//EnableLogger(true, SPDLOG_LEVEL_INFO, strpath_LOG);
}
PTWDLL_C_GlueWidth::~PTWDLL_C_GlueWidth()
{ }


int PTWDLL_C_GlueWidth::printHelloWorld(unsigned char* pBuf)
{
	if (pBuf == nullptr) { return ER_ABORT; }

	std::string str = "HelloWorld_DLL";
	memcpy(pBuf, str.data(), sizeof(unsigned char) * str.size());

	//LogMsg(str);

	return 0;

}


void PTWDLL_C_GlueWidth::EnableLogger(unsigned char enable, unsigned short level, const char* filepath)
{
	std::string  strFilePath(filepath);
	EnableLogger(enable, level, strFilePath);
}

void PTWDLL_C_GlueWidth::LogMsg(const char* msg, unsigned short level)
{
	std::string  strMSG(msg);
	LogMsg(strMSG, level);
}


bool PTWDLL_C_GlueWidth::IsFileExists(const std::string& path) {

	struct stat info;

	if (stat(path.c_str(), &info) == 0) {
		return true;
	}
	return false;
}

void PTWDLL_C_GlueWidth::EnableLogger(unsigned char bOn, unsigned short level, std::string filePath)
{

#if defined(_MSC_VER)

	m_bLogger = bOn ? true : false;
	if (m_bLogger)
	{
		if (filePath.empty())
			filePath = "d:\\ptwdll_log.txt";

		m_hLogger = spdlog::daily_logger_mt("PTWDLL_logger", filePath, 0, 0);
		//m_hLogger->flush_on((spdlog::level::level_enum)level);
		m_hLogger->set_level((spdlog::level::level_enum)level);
		//m_hLogger->set_pattern("%Y-%m-%d %H:%M:%S.%f [%l] %v");
		m_hLogger->set_pattern("%Y-%m-%d %H:%M:%S.%f [%l] [thread %t] %v");
		//m_hLogger->set_pattern("%Y-%m-%d %H:%M:%S [%l] [thread %t] - <%s>|<%#>|<%!>,%v");

		std::string msg = fmt::format("Logger enabled. PTWDLL ver. {}", strVer); //"Logger enabled. PxDaq ver. " + strVer;
		LogMsg(msg);
	}
	else {
		spdlog::drop("PTWDLL_logger");
	}


#endif //#if defined(_MSC_VER)

}

void PTWDLL_C_GlueWidth::LogMsg(std::string msg, unsigned short level)
{
#if defined(_MSC_VER)


	if (m_bLogger)
	{
		switch (level)
		{
		case SPDLOG_LEVEL_TRACE:
			m_hLogger->trace(msg);
			break;
		case SPDLOG_LEVEL_DEBUG:
			m_hLogger->debug(msg);
			break;
		case SPDLOG_LEVEL_INFO:
			m_hLogger->info(msg);
			break;
		case SPDLOG_LEVEL_WARN:
			m_hLogger->warn(msg);
			break;
		case SPDLOG_LEVEL_ERROR:
			m_hLogger->error(msg);
			break;
		case SPDLOG_LEVEL_CRITICAL:
			m_hLogger->critical(msg);
			break;
		}
	}


#endif	//#if defined(_MSC_VER)
}


int PTWDLL_C_GlueWidth::Align_Pattern_SetParameter(seRect roiSearch, LPImageInfo pTemplatIn)
{
	int res = 0;

	res = cvip_Align.Align_Pattern_SetParameter(roiSearch, pTemplatIn);

	return res;
}

int PTWDLL_C_GlueWidth::Align_Pattern_TestIt(LPImageInfo pIn)
{
	int res = 0;

	res = cvip_Align.Align_Pattern_TestIt(pIn);

	return res;
}

int PTWDLL_C_GlueWidth::Align_Pattern_GetResult(LPBoundingBox pFMarkBox, double* dbScore)	//return info of bounding box and score.
{
	int res = 0;

	res = cvip_Align.Align_Pattern_GetResult(pFMarkBox, dbScore);

	return res;
}


int PTWDLL_C_GlueWidth::Align_FindProfile_SetPattern(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iSelectLineID, int iKSize)
{
	int res = 0;

	res = cvip_Align.Align_FindProfile_SetParameter(roiSearch, roiMask, bDirection, bPolarity, stepSize, iSelectLineID, iKSize);

	return res;
}

int PTWDLL_C_GlueWidth::Align_FindProfile_TestIt(LPImageInfo pIn, int iSelLineNo)
{
	int res = 0;

	res = cvip_Align.Align_FindProfile_TestIt(pIn, iSelLineNo);

	return res;
}

int PTWDLL_C_GlueWidth::Align_FindProfile_GetResult(int* CntArryOut, int* p1DArrayOut) //return info of bounding box and 1d array data.
{
	int res = 0;

	seDetectCirle_Results myRes;

	res = cvip_Align.Align_FindProfile_GetResult(myRes);

	std::copy(myRes.vec_CntArryOut.begin(), myRes.vec_CntArryOut.end(), CntArryOut);
	std::copy(myRes.vec_1DArrayOut.begin(), myRes.vec_1DArrayOut.end(), p1DArrayOut);

	return res;
}



// Alignment mthod_Find Circle
int PTWDLL_C_GlueWidth::Align_DetectCircle_SetPattern(seCircle roiSearch, seCircle roiMask, bool bDirection, bool bPolarity, int stepSize, int iMinEdgeStrength, int iKSize)
{
	int res = 0;

	res = cvip_Align.Align_DetectCircle_SetParameter(roiSearch, roiMask, bDirection, bPolarity, stepSize, iMinEdgeStrength, iKSize);

	return res;
}

int PTWDLL_C_GlueWidth::Align_DetectCircle_TestIt(LPImageInfo pIn)
{
	int res = 0;

	res = cvip_Align.Align_DetectCircle_TestIt(pIn);

	return res;
}

int PTWDLL_C_GlueWidth::Align_DetectCircle_GetResult(LPBoundingBox pFMarkBox)	//return info of bounding box and 1d array data.
{
	int res = 0;

	seDetectCirle_Results myRes;

	res = cvip_Align.Align_DetectCircle_GetResult(myRes);

	*pFMarkBox = myRes.seBoundBox_Circle;

	return res;
}


// InspecBoxSetup_Annulus
int PTWDLL_C_GlueWidth::InspectBox_Annulus_SetParameter(seAnnulus roiAnnuls)
{
	int res = 0;

	res = cvip_Align.InspectBox_SetParameter(roiAnnuls);

	return res;
}

int PTWDLL_C_GlueWidth::InspectBox_Rectangle_SetParameter(seBoundingBox roiRect)
{
	int res = 0;

	res = cvip_Align.InspectBox_SetParameter(roiRect);

	return res;
}

int PTWDLL_C_GlueWidth::InspectBox_Circle_SetParameter(seCircle roiCircle)
{
	int res = 0;

	res = cvip_Align.InspectBox_SetParameter(roiCircle);

	return res;
}

int PTWDLL_C_GlueWidth::InspectBox_TestIt(LPImageInfo pIn)
{
	int res = 0;

	res = cvip_Align.InspectBox_TestIt(pIn);

	return res;
}

//int PTWDLL_C_GlueWidth::InspectBox_GetResult(LPBoundingBox pInspBox, LPImageInfo pOut)	//retur ROI Image
int PTWDLL_C_GlueWidth::InspectBox_GetResult(LPBoundingBox pInspBox)
{
	int res = 0;

	//res = cvip_Align.InspectBox_GetResult(pInspBox, pOut);
	res = cvip_Align.InspectBox_GetResult(pInspBox);

	return res;
}


// calculate the dependency coordinates 
int PTWDLL_C_GlueWidth::CoordBind_Calcu_SetParameter(seBoundingBox seFMarkBox, seBoundingBox seInspBox, seCoordBindBox seCoorBindBoxIn)
{
	int res = 0;

	res = cvip_Align.CoordBind_Calcu_SetParameter(seFMarkBox, seInspBox, seCoorBindBoxIn);

	return res;
}

int PTWDLL_C_GlueWidth::CoordBind_Calcu_TestIt()
{
	int res = 0;

	res = cvip_Align.CoordBind_Calcu_TestIt();

	return res;
}

int PTWDLL_C_GlueWidth::CoordBind_Calcu_GetResult(LPCoordBindBox pCoordBoxInfo)
{
	int res = 0;

	res = cvip_Align.CoordBind_Calcu_GetResult(pCoordBoxInfo);

	return res;
}

int PTWDLL_C_GlueWidth::CoordBind_CropROI_Annulus_SetParameter(seCoordBindBox pCoordBox, seAnnulus roiAnnuls)
{
	int res = 0;

	res = cvip_Align.CoordBind_CropROI_SetParameter(pCoordBox, roiAnnuls);

	return res;
}

int PTWDLL_C_GlueWidth::CoordBind_CropROI_Rectangle_SetParameter(seCoordBindBox pCoordBox, seBoundingBox roiRect)
{
	int res = 0;

	res = cvip_Align.CoordBind_CropROI_SetParameter(pCoordBox, roiRect);

	return res;
}

int PTWDLL_C_GlueWidth::CoordBind_CropROI_Circle_SetParameter(seCoordBindBox pCoordBox, seCircle roiCircle)
{
	int res = 0;

	res = cvip_Align.CoordBind_CropROI_SetParameter(pCoordBox, roiCircle);

	return res;
}

int PTWDLL_C_GlueWidth::CoordBind_CropROI_TestIt(LPImageInfo pIn)
{
	int res = 0;

	res = cvip_Align.CoordBind_CropROI_TestIt(pIn);

	return res;
}

int PTWDLL_C_GlueWidth::CoordBind_CropROI_Annulus_GetResult(LPImageInfo pOut, LPAnnulus pRoiOffset_Annuls)
{
	int res = 0;

	seCropImage_Results myRes;

	res = cvip_Align.CoordBind_CropROI_GetResult(pOut, myRes);

	*pRoiOffset_Annuls = myRes.seOffsetCrop_Annulus;

	return res;
}

int PTWDLL_C_GlueWidth::CoordBind_CropROI_Rectangle_GetResult(LPImageInfo pOut, LPBoundingBox pRoiOffset_Rect)
{
	int res = 0;

	seCropImage_Results myRes;

	res = cvip_Align.CoordBind_CropROI_GetResult(pOut, myRes);

	*pRoiOffset_Rect = myRes.seOffsetCrop_Rect;

	return res;
}

int PTWDLL_C_GlueWidth::CoordBind_CropROI_Circle_GetResult(LPImageInfo pOut, LPCircle pRoiOffset_Circle)
{
	int res = 0;

	seCropImage_Results myRes;

	res = cvip_Align.CoordBind_CropROI_GetResult(pOut, myRes);

	*pRoiOffset_Circle = myRes.seOffsetCrop_Circle;

	return res;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Step_03
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Image Processing_(Threshold)
int PTWDLL_C_GlueWidth::IpThreshold_SetParameter(int iChannels, double* pThresh, double* pMaxVal, emThresholdTypes emTypes) 
{
	int res = 0;

	res = cvip_IP.IpThreshold_SetParameter(iChannels, pThresh, pMaxVal, emTypes);

	return res;
}

int PTWDLL_C_GlueWidth::IpThreshold_TestIt(LPImageInfo pIn)
{
	int res = 0;

	res = cvip_IP.IpThreshold_TestIt(pIn);

	return res;
}

int PTWDLL_C_GlueWidth::IpThreshold_GetResult(LPImageInfo pOut)
{
	int res = 0;

	res = cvip_IP.IpThreshold_GetResult(pOut);

	return res;
}


// Image Processing_( Histogram )
int PTWDLL_C_GlueWidth::IpHistogram_SetParameter(LPImageInfo pIn)
{
	int res = 0;

	res = cvip_IP.IpHistogram_SetParameter(pIn);

	return res;
}

int PTWDLL_C_GlueWidth::IpHistogram_Annulus_SetParameter(LPImageInfo pIn, seAnnulus roiAnnuls)
{
	int res = 0;

	res = cvip_IP.IpHistogram_SetParameter(pIn, roiAnnuls);

	return res;
}

int PTWDLL_C_GlueWidth::IpHistogram_Rectangle_SetParameter(LPImageInfo pIn, seBoundingBox roiRect)
{
	int res = 0;

	res = cvip_IP.IpHistogram_SetParameter(pIn, roiRect);

	return res;
}

int PTWDLL_C_GlueWidth::IpHistogram_Circle_SetParameter(LPImageInfo pIn, seCircle roiCircle)
{
	int res = 0;

	res = cvip_IP.IpHistogram_SetParameter(pIn, roiCircle);

	return res;
}

int PTWDLL_C_GlueWidth::IpHistogram_TestIt()
{
	int res = 0;

	res = cvip_IP.IpHistogram_TestIt();

	return res;
}

int PTWDLL_C_GlueWidth::IpHistogram_GetResult(double* p1DArray)
{
	int res = 0;

	res = cvip_IP.IpHistogram_GetResult(p1DArray);

	return res;
}


// Image Processing_( Morphology )
int PTWDLL_C_GlueWidth::IpMorphology_SetParameter(emMorphShapes emShapes, int iKSize)
{
	int res = 0;

	res = cvip_IP.IpMorphology_SetParameter(emShapes, iKSize);

	return res;
}

int PTWDLL_C_GlueWidth::IpMorphology_TestIt(LPImageInfo pIn, emMorphOperation emOperation)
{
	int res = 0;

	res = cvip_IP.IpMorphology_TestIt(pIn, emOperation);

	return res;
}

int PTWDLL_C_GlueWidth::IpMorphology_GetResult(LPImageInfo pOut)
{
	int res = 0;

	res = cvip_IP.IpMorphology_GetResult(pOut);

	return res;
}


// Image Processing_( Morpholoy )( Fill Holes )
int PTWDLL_C_GlueWidth::IpNoiseRemoval_SetParameter(double dbLimit_min, double dbLimit_max)
{
	int res = 0;

	res = cvip_IP.IpNoiseRemoval_SetParameter(dbLimit_min, dbLimit_max);

	return res;
}

int PTWDLL_C_GlueWidth::IpNoiseRemoval_TestIt(LPImageInfo pIn)
{
	int res = 0;

	res = cvip_IP.IpNoiseRemoval_TestIt(pIn);

	return res;
}

int PTWDLL_C_GlueWidth::IpNoiseRemoval_GetResult(LPImageInfo pOut)
{
	int res = 0;

	res = cvip_IP.IpNoiseRemoval_GetResult(pOut);

	return res;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Step_04
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// Measure 
int PTWDLL_C_GlueWidth::MeasGlueWidth_Annulus_SetParameter(seAnnulus roiAnnuls, int stepSize /*degrees*/)
{
	int res = 0;

	res = cvip_Measure.MeasGlueWidth_SetParameter(roiAnnuls, stepSize);

	return res;
}

int PTWDLL_C_GlueWidth::MeasGlueWidth_Rectangle_SetParameter(seBoundingBox roiRect, int stepSize /*degrees*/)
{
	int res = 0;

	res = cvip_Measure.MeasGlueWidth_SetParameter(roiRect, stepSize);

	return res;
}

int PTWDLL_C_GlueWidth::MeasGlueWidth_TestIt(LPImageInfo pIn)
{
	int res = 0;

	res = cvip_Measure.MeasGlueWidth_TestIt(pIn);

	return res;
}

int PTWDLL_C_GlueWidth::MeasGlueWidth_GetResult(double* pLength_Inner, double* pLength_Outer, double* pGlueArea)
{
	int res = 0;

	if( nullptr == pLength_Inner || 
		nullptr == pLength_Outer ||
		nullptr == pGlueArea) {

		res = ER_ABORT;
	}
	else {

		seGlueWidth_Results myRes;

		res = cvip_Measure.MeasGlueWidth_GetResult(myRes);

		std::copy(myRes.Length_Inner.begin(), myRes.Length_Inner.end(), pLength_Inner);
		std::copy(myRes.Length_Outer.begin(), myRes.Length_Outer.end(), pLength_Outer);
		*pGlueArea = myRes.GlueArea;

	}

	return res;
}


int PTWDLL_C_GlueWidth::MeasGlueWidth_GetInfo4Dump(std::vector<std::string>& vec_strInfo)
{
	int res = 0;

	res = cvip_Measure.MeasGlueWidth_GetInfo4Dump(vec_strInfo);

	return res;
}
