#pragma once

#include "GigECamDataStructureDef.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <fcntl.h>
#include <condition_variable>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include "Handler_CamCtrl_Strategy.h"
#include "../ThirdPartyLibrary/yuyv2rgb_converter.h"
#include "../ThirdPartyLibrary/OverwriteRingBuffer.h"
#include "../ThirdPartyLibrary/RestfulClient.h"

//V4L2 >
// by joe #include <MediaBuffer.h>
#include <linux/fb.h>
// by joe #include <cam_device_api.hpp>
// by joe #include <json_helper.h>
// by joe #include "log.h"
// by joe #include "ioctl_cmds.h"
// by joe #include "viv_video_kevent.h"

//V4L2 <


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <dlfcn.h>
#include <filesystem>
#include <chrono>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
// #include "RuntimeAPI.h"
// #include "Types.h"


using namespace std;
using namespace cv;


static std::unordered_map<std::string, int> string2V4L2MediaFormat = {
		/* by joe {"YUYV", MEDIA_PIX_FMT_YUV422I},
		{"NV12", MEDIA_PIX_FMT_YUV420SP},
		{"NV16", MEDIA_PIX_FMT_YUV422SP},
		{"RAW8", MEDIA_PIX_FMT_RAW8},
		{"RAW10", MEDIA_PIX_FMT_RAW10},
		{"RAW12", MEDIA_PIX_FMT_RAW12},*/
};

static std::unordered_map<int, int> mediaUVCFormat2V4l2Format = {
		/*{MEDIA_PIX_FMT_YUV422I, V4L2_PIX_FMT_YUYV},
		{MEDIA_PIX_FMT_YUV420SP, V4L2_PIX_FMT_NV12},
		{MEDIA_PIX_FMT_YUV422SP, V4L2_PIX_FMT_NV16},*/
};

static std::unordered_map<std::string, int> ctrlIUVCDList = {
		{"sensor.mode", 0},
		{"sensor.resw", 0},
		{"sensor.resh", 0},
		/* To be added */
};



class CMethod_V4L2CamCtrl : public CHandler_CamCtrl
{
	int g_ctrl_id = -1;


	std::mutex u_mutex;
	std::mutex u_streaming_mutex;
	std::condition_variable cond_var;
	bool notified_IsDone = false;
	bool notified_IsRun = false;
	std::thread::id tid;
	bool bIsCreated = false;
	unsigned long iCnt_0 = 0;
	unsigned long iCnt_1 = 0;

	static int prev_W;
	static int prev_H;

public:
	CMethod_V4L2CamCtrl();
	~CMethod_V4L2CamCtrl();


public:
	int GigeCam_Init() override;
	int GigECam_SetConfig(const LpGigECamConfig pParamIn) override;	//nModeType 0:Frame, 1:ROI
	int GigECam_GetConfig(LpGigECamConfig pParamOut) override;
	int GigECam_AcquireImages(string strFilePath) override;
	int GigECam_AcquireImages(cv::Mat& matImg) override;
	int GigECam_Release() override;


	int GigECam_Strm_Prepare() override;
	int GigECam_Strm_Start() override;
    int GigECam_Strm_AcquireImages(string strFilePath) override;
	int GigECam_Strm_AcquireImages(cv::Mat& matImg) override;
	int GigECam_Strm_Stop() override;
	int GigECam_Strm_Close() override;
	//int GigECam_Strm_Inquiry(seStreamingInfo* pStrmInfo);



	int GigECam_DebugPrint() override;

private: 

// V4L2 >
/*	
	v4l2_capability		�]�Ư�O��T�A�]�A�X�ʦW�١B�]�ƦW�١B������ާ@��
	v4l2_fmtdesc		�]�Ƥ���������榡�y�z
	v4l2_frmsizeenum	�]�Ƥ��������v�T�|
	v4l2_format			�]�m�άd�ߵ��W�榡�A�]�A�����榡�B����v��
	v4l2_requestbuffers	�Ω�ШD�M�t�m�w�İϼƶq�A�]�A�w�İ������]�Ҧp�GV4L2_BUF_TYPE_VIDEO_CAPTURE�^�M�w�İϼƶq
	v4l2_buffer			�Ω�y�z�w�İϪ���T�A�Ҧp�w�İϯ��ޡB�w�İϪ��סB�w�İϦa�}��
	v4l2_framebuffer	�]�m�άd�ߵ��W�w�İϡ]framebuffer�^����T
	v4l2_cropcap		�d�ߵ�ũM�Y���O�A�Ҧp��ɡB�q�{��ůx�ε�
	v4l2_crop			�]�m�άd�ߵ�ůx��
	v4l2_streamparm		�]�m�άd�߬y�ѼơA�Ҧp�ɶ��W�����B����Ҧ���
	v4l2_ext_controls	�Ω�]�m�άd���X�i����]�Ҧp�G�G�סB���׵��^
*/

	// flag of Device and SensorMode
	int m_DeviceId;
	std::string m_strDeviceName;

	int m_SensorMode;
	char m_szFormat[16];
	int m_isRawData;
	int m_Crop_enable;
	int m_Scale_enable;
	int m_DisplayType;

	int m_BUFFER_COUNT;

	int m_Prev_width;
	int m_Prev_height;
	unsigned int m_Prev_pixelFormat;


	// V4L2 structure 
	v4l2_capability			m_Caps;					//# v4l2_capability_�]�Ư�O��T�A�]�A�X�ʦW�١B�]�ƦW�١B������ާ@���C����VIDIOC_QUERYCAP�C
	v4l2_fmtdesc			m_FormatDescriptions;	//# v4l2_fmtdesc_�]�Ƥ���������榡�y�z
	v4l2_query_ext_ctrl		m_VivExtQctrl;			//# query for viv_ext_ctrl
	v4l2_requestbuffers		m_Bufrequest;			//# v4l2_requestbuffers	�Ω�ШD�M�t�m�w�İϼƶq�A�]�A�w�İ������]�Ҧp�GV4L2_BUF_TYPE_VIDEO_CAPTURE�^�M�w�İϼƶq
	v4l2_buffer				m_Buffer;				//# v4l2_buffer �Ω�y�z�w�İϪ���T�A�Ҧp�w�İϯ��ޡB�w�İϪ��סB�w�İϦa�}��

	std::vector<void*>		m_vecData;

	// Viv_capability structure
	// by joe viv_caps_supports m_Caps_supports;				//# viv_caps_supports_�۾��˸m�v���榡��T




	int OpenDevice(int id);
	void UpdateCtrlIDList(const int pCam);
	int SetFeature(const char* value);


	yuyv2rgb_converter yuyv2rgb_cvtr;
	CurlHandler restful_client;

// V4L2 <




private:

	int v4l2_xioctl(int fh, int request, void* arg);
	int v4l2_open_device(int iId);
	int v4l2_init_device(const int pCam);
	int v4l2_start_capturing(const int pCam);
	int v4l2_stop_capturing(const int pCam);
	int v4l2_uninit_device();
	int v4l2_close_device();


	int PrintDeviceInfo(const int pCam);

	int Configure_ImageFormat(const int pCam, const LpGigECamConfig pParamIn);
	int Configure_Binning(const int pCam, const LpGigECamConfig pParamIn);
	int Configure_Decimation(const int pCam, const LpGigECamConfig pParamIn);
	int Configure_PersistentIP(const int pCam, const LpGigECamConfig pParamIn);
	int Configure_ResetConfigureCustomImageSettings(const int pCam);
	int Configure_ResetExposure(const int pCam);
	int Configure_Get(const int pCam, LpGigECamConfig pParamOut, bool bDumpInf = 0);
	int Configure_TimestampFilter(const int pCam, const LpGigECamConfig pParamIn);


	int AcquireImages(const int pCam, string strFilePath);
	int AcquireImages(const int pCam, cv::Mat& matImg);
	int AcquireStreaming(const int pCam, cv::Mat& matImg);


	int AcquireStreaming_Prepare();
	int AcquireStreaming_StartorStop(bool bflgEnableThd);
	int AcquireStreaming_Capture(cv::Mat& matImg);
	int AcquireStreaming_Capture(cv::Mat& matImg, int iTimeVal);	// has Timestamp method
	int AcquireStreaming_Close();
	int AcquireStreaming_Inquiry(seStreamingInfo* pStrmInfo);


	//int Thread_Acquire(const int pCam, LpGigECamConfig pParamOut);

	int Streaming_Inquiry();
	int Streaming_Setting(const int iflg);

	void Streaming_CallbackEx(const std::string& strProj);


	int Thread_Acquire(const int pCam, LpGigECamConfig pParamOut, std::function<void(const std::string&)> callback);


	// by joe int viv_private_ioctl(const int pCam, const char* cmd, Json::Value& jsonRequest, Json::Value& jsonResponse);

	//int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);
	//int Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData, cv::Mat& mat_img);

private:
	string m_strFilePath;

	seGigECamConfig m_global_cfg_ParamInfo;

	double m_fLimit_ExposureTime_Min;
	double m_fLimit_ExposureTime_Max;
	double m_fLimit_Gain_Min;
	double m_fLimit_Gain_Max;

	cv::Rect m_rectCrop_Frame;
	cv::Rect m_rectCrop_ROI;

	bool m_IsCropped;
	int m_pCam;
	int m_Streamid;

	unsigned long m_iFrameRate_Cnt;

	int m_RetryCnt;

    cv::VideoCapture cap_uvc;
};

// gray
enum MDLACoreOptions {
    Single,
    Multi
};

struct EnvOptions {
    int deviceKind;
    MDLACoreOptions MDLACoreOption;
    int CPUThreadNum;
    bool suppressInputConversion;
    bool suppressOutputConversion;
};

class DL_Model 
{
    protected:
        cv::Size2i   wh;
        string       path;
        void*        runtime;
        size_t       input_size; 
        size_t       output_size; 
        vector<char> input;
        vector<char> output;

        virtual void preprocess(cv::Mat& img, cv::Size2i wh) = 0;

        void init()
        {
            // set env
            EnvOptions envOptions = {
                .deviceKind= 0,
                .MDLACoreOption = Single,
                .CPUThreadNum = 1,
                .suppressInputConversion = false,
                .suppressOutputConversion = false,
            };

            // create model instance
            int err_code;
            err_code = NeuronRuntime_create(&envOptions, &this->runtime);
            if (err_code != NEURONRUNTIME_NO_ERROR) {
                cerr << "Failed to create Neuron runtime." << endl;
                exit(EXIT_FAILURE);
            }

            // load model
            err_code = NeuronRuntime_loadNetworkFromFile(this->runtime, this->path.c_str());
            if (err_code != NEURONRUNTIME_NO_ERROR) {
                cerr << "Failed to load model: " << this->path << endl;
                exit(EXIT_FAILURE);
            }

            // input size
            err_code = NeuronRuntime_getInputSize(this->runtime, 0, &this->input_size);
            cout << "input size: " << this->input_size << endl;
            if (err_code != NEURONRUNTIME_NO_ERROR) {
                cerr << "Failed to get input size." << endl;
                exit(EXIT_FAILURE);
            }

            // output size
            err_code = NeuronRuntime_getOutputSize(this->runtime, 0, &this->output_size);
            cout << "output size: " << this->output_size << endl;
            if (err_code != NEURONRUNTIME_NO_ERROR) {
                cerr << "Failed to get output size." << endl;
                exit(EXIT_FAILURE);
            }

            // set input buffer
            this->input.resize(this->input_size);

            // set output buffer
            this->output.resize(this->output_size);
        }

        void infer(const cv::Mat& img)
        {
            // set input content
            memcpy(this->input.data(), img.data, this->input_size);
            int err_code = NeuronRuntime_setInput(this->runtime, 0, this->input.data(), this->input_size, {-1});
            if (err_code != NEURONRUNTIME_NO_ERROR) {
                cerr << "Failed to set input." << endl;
                exit(EXIT_FAILURE);
            }

            // set output content
            err_code = NeuronRuntime_setOutput(this->runtime, 0, this->output.data(), this->output_size, {-1});
            if (err_code != NEURONRUNTIME_NO_ERROR) {
                cerr << "Failed to set output." << endl;
                exit(EXIT_FAILURE);
            }

            // do inference
            err_code = NeuronRuntime_inference(this->runtime);
            if (err_code != NEURONRUNTIME_NO_ERROR) {
                cerr << "Failed to do inference." << endl;
                exit(EXIT_FAILURE);
            }
        }

    public:
        virtual void operator()(cv::Mat& img, vector<float>& output) = 0;
        string  get_model_path() { return this->path; }
        size_t  get_input_size() { return this->input_size; }
        size_t  get_output_size() { return this->output_size; }
};


class Wide_ResNet : public DL_Model 
{
    private:        
        void preprocess(cv::Mat& img, cv::Size2i wh) override
        {
            auto t1 = chrono::high_resolution_clock::now();
            cv::resize(img, img, wh);
            auto t2 = chrono::high_resolution_clock::now();
            img.convertTo(img, CV_32FC3, 1/255., 0);
            auto t3 = chrono::high_resolution_clock::now();
            img = (img - cv::Scalar(0.485, 0.456, 0.406)) / cv::Scalar(0.229, 0.224, 0.225);
            auto t4 = chrono::high_resolution_clock::now();
            auto tt1 = chrono::duration_cast<chrono::microseconds>(t2 - t1).count() / 1000.;
            auto tt2 = chrono::duration_cast<chrono::microseconds>(t3 - t2).count() / 1000.;
            auto tt3 = chrono::duration_cast<chrono::microseconds>(t4 - t3).count() / 1000.;
            cout << "resize: " << tt1 << " ms, ";
            cout << "convertTo: " << tt2 << " ms, ";
            cout << "normalize: " << tt3 << " ms" << endl;
        }

    public:
        Wide_ResNet(const string path, cv::Size2i wh)
        {
            this->path = path;
            this->wh = wh;
            this->init();
        }

        void operator()(cv::Mat& img, vector<float>& output) override 
        {
            this->infer(img);
            memcpy(output.data(), this->output.data(), this->output_size);
        }

        ~Wide_ResNet() {
            NeuronRuntime_release(this->runtime);
        }
};

