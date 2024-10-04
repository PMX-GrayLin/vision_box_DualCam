#pragma once

#include <utility>      // std::pair, std::make_pair
#include <string>       // std::string
#include <vector>		// std::vector
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <opencv2/opencv.hpp>
#include "BaseDataStructureDef.h"

#define VERSION_PRINT(...) printf("\e[1;34m[%s]\e[m",VSB_VERSION);
#define COUT_MACRO(...) std::cout<<__VA_ARGS__
#define COLOR_RED(...) printf("\e[1;31m[%s][%4d] =>\e[m",__func__,__LINE__); 

#ifdef   ALL_MSG_DEBUG

    #define COLOR_PRINT(...) do{ \
							VERSION_PRINT(); \
							COLOR_RED(); \
							printf(__VA_ARGS__); \
						}while(0)

    #define COLOR_COUT(...) do{ \
							VERSION_PRINT(); \
							COLOR_RED(); \
							COUT_MACRO(__VA_ARGS__); \
						}while(0)
#else

    #define COLOR_PRINT(...)
    #define COLOR_COUT(...) 

#endif

#ifdef ALGO_Enable_CYCLE_TIME_DEBUG

    #define  printD(...) COLOR_PRINT("Algo Entrance dbg: " __VA_ARGS__)
    
#else 

    #define  printD(...) 
    
#endif


using namespace cv;


/// # //////////////////////////////////////////////////////////////////////////////
/// 
/// # //////////////////////////////////////////////////////////////////////////////


static std::string formatTimestamp(const std::chrono::system_clock::time_point &timestamp)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
    std::tm *gmt = std::localtime(&tt);

    char timestr[32];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", gmt);

    auto duration_since_epoch = timestamp.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration_since_epoch).count() % 1000;

    std::ostringstream oss;
    oss << timestr << "." << std::setfill('0') << std::setw(3) << millis;

    return oss.str();
}

static string getCurrentTime()
{
	string strTime;

	time_t curtime = time(0);
	tm* nowtime = localtime(&curtime);

	strTime += std::to_string(1900 + nowtime->tm_year);
	strTime += std::to_string(1 + nowtime->tm_mon);
	strTime += std::to_string(nowtime->tm_mday);
	strTime += "_";
	strTime += std::to_string(nowtime->tm_hour);
	strTime += std::to_string(nowtime->tm_min);
	strTime += std::to_string(nowtime->tm_sec);

	//printf("time = %s\n", strTime.c_str());

	return strTime;
}


/// # //////////////////////////////////////////////////////////////////////////////
/// 
/// # //////////////////////////////////////////////////////////////////////////////

static void my_memcpy(volatile void* dst, volatile void* src, int sz)
{
	if (sz & 63) {
		sz = (sz & -64) + 64;
	}
	asm volatile (
		"NEONCopyPLD: \n"
		"sub %[src], %[src], #32 \n"
		"sub %[dst], %[dst], #32 \n"
		"1: \n"
		"ldp q0, q1, [%[src], #32] \n"
		"ldp q2, q3, [%[src], #64]! \n"
		"subs %[sz], %[sz], #64 \n"
		"stp q0, q1, [%[dst], #32] \n"
		"stp q2, q3, [%[dst], #64]! \n"
		"b.gt 1b \n"
		: [dst] "+r"(dst), [src]"+r"(src), [sz]"+r"(sz) : : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
}


/// # //////////////////////////////////////////////////////////////////////////////
/// 
/// # //////////////////////////////////////////////////////////////////////////////


static int calcMultiplesofFour(int x) {

	if (x <= 0) 
		return 0;
	double val = x + round(x % 4);
	return val;
}


static int calcMultiplesofTwo(int x) {

	if (x <= 0)
		return 0;
	double val = x + round(x % 2);
	return val;
}


/// # //////////////////////////////////////////////////////////////////////////////
/// 
/// # //////////////////////////////////////////////////////////////////////////////


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}


static string send_char_arr(char* memblock, long size)
{
	CURL* curl;
	CURLcode res;

	string readBuffer;

	curl = curl_easy_init();

	if (curl) {

		curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8000/streaming");

		// disable Expect:
		struct curl_slist* chunk = nullptr;
		chunk = curl_slist_append(chunk, "Content-Type: opencv/image"); // use strange type to prevent server to parse content
		chunk = curl_slist_append(chunk, "Expect:");
		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

		// data
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, memblock);
		// data length
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size);

		// response handler
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	return readBuffer;
}


static string send_mat(cv::Mat mat)
{
	// clock_t start, end;
	// double elapsed = 0.0;
	string strtmp;
	// int nRet = 0;

	//cycletime_start
	// start = clock();

	string strRet = "";

	std::vector<int> param = vector<int>(2);
	/// # ////////////////////////////////////////////
	
	// # M-JPEG
	param[0] = cv::IMWRITE_JPEG_QUALITY;
	param[1] = 30;	// 95;//default(95) 0-100

	/// # ////////////////////////////////////////////

	std::vector<uchar> buf;
	cv::imencode(".jpg", mat, buf, param);

	char* memblock = reinterpret_cast<char*>(buf.data());
	long size = buf.size();

	strRet = send_char_arr(memblock, size);

	//// # cycletime_end
	//end = clock();
	//elapsed = double(end - start) / CLOCKS_PER_SEC;
	//strtmp = std::to_string(elapsed);
	//cout << "2. --.--> Send_char_arr() Cycle time : " << strtmp.c_str() << " (seconds)" << endl;

	return strRet;
}


/// # //////////////////////////////////////////////////////////////////////////////
/// 
/// # //////////////////////////////////////////////////////////////////////////////


template<typename _Tp>
std::vector<_Tp> cvMat2Vec_1D(const cv::Mat& mat)
{
	return (std::vector<_Tp>)(mat.reshape(1, 1));
}

template<typename _Tp>
cv::Mat Vec2cvMat_iD(std::vector<_Tp> v, int channels, int rows)
{
	cv::Mat mat = cv::Mat(v);
	cv::Mat dest = mat.reshape(channels, rows).clone();
	return dest;
}

template <typename _Tp>
cv::Mat_<_Tp> Vec2cvMat_2D(std::vector< std::vector<_Tp> >& inVec) 
{
	int rows = static_cast<int>(inVec.size());
	int cols = static_cast<int>(inVec[0].size());

	cv::Mat_<_Tp> resmat(rows, cols);
	for (int i = 0; i < rows; i++)
	{
		resmat.row(i) = cv::Mat(inVec[i]).t();
	}
	return resmat;
}


typedef std::pair< cv::Point, cv::Point > pairPos;


class CCVIPItem 
{

public:
	static int Rect_IsProperSubset(seRect roiMain, seRect roiSub);			// Sub Main
	static int Circle_IsProperSubset(seCircle roiMain, seCircle roiSub);	// Sub Main

	static int Uint8ToCvMat(const LPImageInfo pSrc, cv::Mat& mDest);
	static int Uint8ToCvMat_GrayScalar(const LPImageInfo pSrc, cv::Mat& mDest);
	static int Uint8ToCvMat_ColorScalar(const LPImageInfo pSrc, cv::Mat& mDest);

	static int CvMatToUint8(const cv::Mat mSrc, LPImageInfo pDest);

	static double Similarity_CalcuSSIM(cv::Mat i1, cv::Mat i2); //SSIM(�structural similarity)

	static int CalcHistogram(const cv::Mat src, std::vector< std::vector<int>>& vec2d_histogram);
	static int CalcHistogram(const cv::Mat src, const cv::Mat mask, std::vector< std::vector<int>>& vec2d_histogram);

	static int CalcThreshold(const cv::Mat src, Mat& matMerge, std::vector<double> vecThresh, std::vector<double> vecMaxVal, emThresholdTypes emTypes);

	static int CalcuBoundingBox(seBoundingBox	se_rect, seBoundingBox& se_out);
	static int CalcuBoundingBox(seCircle	se_circle, seBoundingBox& se_out);
	static int CalcuBoundingBox(seAnnulus	se_annulus, seBoundingBox& se_out);

	static int CreateMaskImg(seBoundingBox se_rect, cv::Mat& out, int iMasktype = 1);
	static int CreateMaskImg(seCircle se_circle,	cv::Mat& out);
	static int CreateMaskImg(seAnnulus se_annulus,	cv::Mat& out);

	static int CalcuCenterOffset(cv::Size szImage, seCoordBindBox se_coord, seBoundingBox se_rect, seBoundingBox& se_out);
	static int CalcuCenterOffset(cv::Size szImage, seCoordBindBox se_coord, seCircle se_circle, seCircle& se_out);
	static int CalcuCenterOffset(cv::Size szImage, seCoordBindBox se_coord, seAnnulus se_annulus, seAnnulus& se_out);

	static int Annulus_Degrees_RangeSetting(seAnnulus roiAnnuls, double dbStepSize, bool bDirection, std::vector<pairPos>& vec_List_PairPos);
	static int Annulus_Degrees_GetInfoByLine(const cv::Mat mat_gray_Img, std::vector<pairPos> vec_List_PairPos, std::vector<std::vector<int>>& vec2d_Info_Data);
	static int Annulus_Degrees_GetInfoByLine(const cv::Mat mat_gray_Img, std::vector<pairPos> vec_List_PairPos, std::vector<std::vector<int>>& vec2d_Info_Data, std::vector<std::vector<cv::Point>>& vec2d_Pos_Data);
	static int Annulus_Degrees_GetInfoByLine_New(const cv::Mat mat_gray_Img, bool bDirection, std::vector<pairPos> vec_List_PairPos, std::vector<std::vector<int>>& vec2d_Info_Data, std::vector<std::vector<cv::Point>>& vec2d_Pos_Data, int& vec2d_MaxSize);


	// Hough circle detection.
	static int Circle_Shape_Detector(const cv::Mat mat_gry_Imeg, seAnnulus roiAnnuls, bool bDirection, bool bPolarity, int iMinEdgeStrength, int iKernelSize, seCircle& resCircle);
	// Edge contours circle detection
	static int Circle_Shape_Detector(const cv::Mat mat_gry_Imeg, std::vector<std::vector<int>> vec2d_Info_Data, std::vector<std::vector<cv::Point>> vec2d_Pos_Data, bool bDirection, bool bPolarity, int iMinEdgeStrength, int iKernelSize, seCircle& resCircle);
	static int Circle_Shape_Detector_New(const cv::Mat mat_gry_Imeg, int vec2d_MaxSize, std::vector<std::vector<float>> vec2d_Info_Data, std::vector<std::vector<cv::Point>> vec2d_Pos_Data, bool bDirection, bool bPolarity, int iMinEdgeStrength, int iKernelSize, seCircle& resCircle, std::vector<sePoint>& vecPos_Circle);


	static int RANSAC_CircleDetection(std::vector<cv::Point> edgePoints, std::vector<seCircle>& bestCircles, int iterations);
	static int LeastSquares_CircleDetection(std::vector<cv::Point> edgePoints, seCircle& bestCircles);


	static int Straight_Line_Detector(const cv::Mat mat_gry_Imeg, seBoundingBox roiRect, bool bDirection, bool bPolarity, int iMinEdgeStrength, int iKernelSize, seBoundingBox& resRect);
	static int RANSAC_FitLine(std::vector<cv::Point> edgePoints, int distance, double confidence, double correct_rate, int fit_use_num, cv::Vec4f& vec4LineRet);



	static int myGetAppPath(std::string& strAppPath);
	static int myCreateDirectory(const std::string folder);

	static int SaveTestImage(std::string strFunctionname, std::string strImageName, const LPImageInfo aryImageDataBuf, std::string& strFullPath);
	static int SaveTestImage(std::string strFunctionname, std::string strImageName, const cv::Mat matImageDataBuf, std::string& strFullPath);


	static int Auto_Canny(cv::Mat image, cv::Mat& output, int& thresh_lower, int& thresh_upper);

	static bool rect_contains(const cv::Rect& rMain, const cv::Rect& rSub) {
		return rMain.contains(rSub.tl())
			&& rMain.contains(rSub.br())
			&& rMain.contains(cv::Point(rSub.x + rSub.width, rSub.y))
			&& rMain.contains(cv::Point(rSub.x, rSub.y + rSub.height));
	}


	///////////////////////////////////////////////////////////////////////////
	// Geometric calculation
	///////////////////////////////////////////////////////////////////////////
	static int Trigonometry_Angle(cv::Point ptCenter, cv::Point pt1, cv::Point pt2, double& dbAngle);

	static double to_radians(double degrees);
	static cv::Point2f ptRotatePt2f(Point2f ptInput, Point2f ptOrg, double dAngle);
	static int RotateImage(const cv::Mat& src, cv::Mat& dst, const double angle, const int mode = 0);

public:
	CCVIPItem();
	virtual ~CCVIPItem();

private:
	static int Histogram_CtvInteger(const cv::Mat valHist, std::vector<int>& vec1d_Data);
	//static int Uint8ToCvMat_HSLtoGray(const LPImageInfo pSrc, cv::Mat& mDest);



};


class CGeneralFunc
{
public:

	static int format(char* dest, size_t destlen, const char* fmt, ...);



//#if defined(__GNUC__)
//
//	static int getdir(std::string dir, std::vector<std::string>& files);
//
//#endif


};