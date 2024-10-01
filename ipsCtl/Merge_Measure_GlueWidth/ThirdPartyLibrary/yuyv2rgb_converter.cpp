#include <opencv2/opencv.hpp>
#include "yuyv2rgb_converter.h"


using namespace std;
using namespace cv;

int yuyv2rgb_converter::m_Pre_W = 0;
int yuyv2rgb_converter::m_Pre_H = 0;
int yuyv2rgb_converter::m_Pre_C = 0;

// static int yuyv2rgb_converter::m_Pre_W(0);
// static int yuyv2rgb_converter::m_Pre_H(0);
// static int yuyv2rgb_converter::m_Pre_C(0);

yuyv2rgb_converter::yuyv2rgb_converter()
    : m_width(0)
    , m_height(0)
    , m_yuyv_size(0)
    , m_rgb_size(0)
{}


yuyv2rgb_converter::~yuyv2rgb_converter()
{
    Release();
}


int yuyv2rgb_converter::Init(const int w, const int h)
{
    return 0;
}


int yuyv2rgb_converter::Run_CPU(const unsigned char* pYuyv, cv::Mat& matDst)
{
    // 將灰度圖轉換為彩色圖像
    cv::Mat tmp_mat;

    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration(0);
    std::string strtmp;

    if (pYuyv == nullptr) {
        std::cerr << " Error !!! > The yuv_data is empty." << std::endl;
        return -1;
    }

    //cycletime_start >>
    start = std::chrono::high_resolution_clock::now();

    // # func()
    unsigned char* img_data_ptr = pYuyv;
    tmp_mat = cv::Mat(m_height, m_width, CV_8UC2, img_data_ptr);


    cv::cvtColor(tmp_mat, matDst, cv::COLOR_YUV2BGR_YUYV); //<-- Good~~
    //cv::cvtColor(tmp_mat, matDst, cv::COLOR_YUV2RGBA_YUYV); //


    // # cycletime_end <<
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    strtmp = std::to_string(duration.count());

    std::cout << " # 02 _ YuYv2RGB _ CycleTime_( CPU ) : " << strtmp.c_str() << " (ms)" << endl;


    //if (!tmp_mat.empty()) {
    //    tmp_mat.release();
    //}

    return 0;

}

int yuyv2rgb_converter::RunCPU(const unsigned char* yuv_data, const char* szFilePath)
{
    cv::Mat matRet;
    string strPath(szFilePath);

    Run_CPU(yuv_data, matRet);

    // 儲存彩色圖像
    cv::imwrite(strPath.c_str(), matRet);

    return 0;
}



int yuyv2rgb_converter::RunCPU(const unsigned char* yuv_data, cv::Mat& matDst)
{

    Run_CPU(yuv_data, matDst);

    return 0;
}

int yuyv2rgb_converter::Release()
{
    return 0;
}