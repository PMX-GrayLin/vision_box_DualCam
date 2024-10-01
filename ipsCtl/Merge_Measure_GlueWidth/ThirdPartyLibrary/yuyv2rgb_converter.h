#pragma once

#ifndef __YUYV_2_RGB_CONVERTER_H__
#define __YUYV_2_RGB_CONVERTER_H__


#include <iostream>
#include <vector>
#include <chrono>


class yuyv2rgb_converter
{
const char* m_yuv422_to_rgb_kernel_source = R"(
__kernel void yuv422_to_rgb(__global uchar* yuv, __global uchar* rgb, int width, int height) {
        int x = get_global_id(0);
        int y = get_global_id(1);

        if (x >= width || y >= height) {
            return;
        }

        int yuv_index = y * width * 2 + x * 2;
        int rgb_index = y * width * 3 + x * 3;

        uchar y1 = yuv[yuv_index];
        uchar u = x % 2 == 0 ? yuv[yuv_index + 1] : yuv[yuv_index - 1];
        uchar v = x % 2 == 0 ? yuv[yuv_index + 3] : yuv[yuv_index + 1];

        int c = y1 - 16;
        int d = u - 128;
        int e = v - 128;

        int r = clamp((298 * c + 409 * e + 128) >> 8, 0, 255);
        int g = clamp((298 * c - 100 * d - 208 * e + 128) >> 8, 0, 255);
        int b = clamp((298 * c + 516 * d + 128) >> 8, 0, 255);

        rgb[rgb_index] = (uchar)r;
        rgb[rgb_index + 1] = (uchar)g;
        rgb[rgb_index + 2] = (uchar)b;
}
)";
const char* m_yuv422_to_bgr_kernel_source = R"(
__kernel void yuv422_to_bgr(__global uchar* yuv, __global uchar* bgr, int width, int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= width || y >= height) {
        return;
    }

    int yuv_index = y * width * 2 + x * 2;
    int bgr_index = y * width * 3 + x * 3;

    uchar y1 = yuv[yuv_index];
    uchar u = x % 2 == 0 ? yuv[yuv_index + 1] : yuv[yuv_index - 1];
    uchar v = x % 2 == 0 ? yuv[yuv_index + 3] : yuv[yuv_index + 1];

    int c = y1 - 16;
    int d = u - 128;
    int e = v - 128;

    int r = clamp((298 * c + 409 * e + 128) >> 8, 0, 255);
    int g = clamp((298 * c - 100 * d - 208 * e + 128) >> 8, 0, 255);
    int b = clamp((298 * c + 516 * d + 128) >> 8, 0, 255);

    bgr[bgr_index] = (uchar)b;
    bgr[bgr_index + 1] = (uchar)g;
    bgr[bgr_index + 2] = (uchar)r;
}
)";

private:
    int m_width;
    int m_height;
    int m_yuyv_size;
    int m_rgb_size;

    std::vector<unsigned char> m_yuyv_data;
    std::vector<unsigned char> m_rgb_data;

public:

    static int m_Pre_W;
    static int m_Pre_H;
    static int m_Pre_C;

    yuyv2rgb_converter();
    ~yuyv2rgb_converter();
    int Init(const int w, const int h);

    int RunCPU(const unsigned char* yuv_data, const char* szFilePath);
    int RunCPU(const unsigned char* yuv_data, cv::Mat &matDst);

    int Release();

private:
    int Run_CPU(const unsigned char* pYuyv, cv::Mat& matDst);

};


#endif      //__YUYV_2_RGB_CONVERTER_H__