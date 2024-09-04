#pragma once


#include "BaseDataStructureDef.h"
#include "cvip.h"


class CMethod_ImgProcessing : public CCVIPItem
{
public:
	CMethod_ImgProcessing();
	~CMethod_ImgProcessing();

public:
	//int Initiation();
	//int Release();

	//Setp_02: 
	// Alignment 

	//Step_03
	// Image Processing_(Threshold)
	int IpThreshold_SetParameter(int iChannels, double* pThresh, double* pMaxVal, emThresholdTypes emTypes);
	int IpThreshold_TestIt(const LPImageInfo pIn);
	int IpThreshold_TestIt(const cv::Mat mat_img);
	int IpThreshold_GetResult(LPImageInfo pOut);
	int IpThreshold_GetResult(cv::Mat& mDest_Img);

	// Image Processing_( Histogram )
	int IpHistogram_SetParameter(const LPImageInfo pIn);
	int IpHistogram_SetParameter(const LPImageInfo pIn, seAnnulus roiAnnulus);
	int IpHistogram_SetParameter(const cv::Mat matImg, seAnnulus roiAnnulus);
	int IpHistogram_SetParameter(const LPImageInfo pIn, seBoundingBox roiRect);
	int IpHistogram_SetParameter(const cv::Mat matImg, seBoundingBox roiRect);
	int IpHistogram_SetParameter(const LPImageInfo pIn, seCircle roiCircle);
	int IpHistogram_SetParameter(const cv::Mat matImg, seCircle roiCircle);
	int IpHistogram_TestIt();
	int IpHistogram_GetResult(double* p1DArray);

	// Image Processing_( Morphology )
	int IpMorphology_SetParameter(emMorphShapes emShapes, int iKSize);	
	int IpMorphology_TestIt(const LPImageInfo pIn, emMorphOperation emOperation);
	int IpMorphology_TestIt(const cv::Mat matImg_gray, emMorphOperation emOperation);
	int IpMorphology_GetResult(LPImageInfo pOut);
	int IpMorphology_GetResult(cv::Mat& mDest_Img);

	// Image Processing_( Morpholoy )( Fill Holes )
	int IpNoiseRemoval_SetParameter(double dbLimit_min, double dbLimit_max);
	int IpNoiseRemoval_TestIt(const LPImageInfo pIn);
	int IpNoiseRemoval_TestIt(const cv::Mat matImg_gray);
	int IpNoiseRemoval_GetResult(LPImageInfo pOut);
	int IpNoiseRemoval_GetResult(cv::Mat& mDest_Img);
	//int IpFilter_FillHoles();

	//Image Processing_(Data Augmentation)
	int IpDataAugmeatation_SetParameter(seDataAugmentationInfo se_DA_Param);
	int IpDataAugmentation_TestIt(const cv::Mat matImg_gray);
	int IpDataAugmentation_GetResult();


	//Step_4
	// Measure Glue Width





private:
	int Algo_NoiseRemoval_Arae(const cv::Mat src_binImg, cv::Mat& dst_binImg, double dbLimit_min, double dbLimit_max);



private:
	//Threshold
	std::vector<double> m_vec_dbThresh;
	std::vector<double> m_vec_dbMaxVal;
	emThresholdTypes m_emTypes;
	cv::Mat m_thresh_img;


	//Histogram
	cv::Mat m_hist_Img;
	std::vector< std::vector<int>> m_vec2d_hist_Val;
	seAnnulus		m_hist_Annulus;
	seBoundingBox	m_hist_Rect;
	seCircle		m_hist_Circle;
	emBoxShape		m_emHist_BoxShape;


	//Morphology
	cv::Mat m_morph_element;
	int m_kernel_size;
	cv::Mat m_morph_destImg;


	//Noise Removal
	double m_dbLimit_min;
	double m_dbLimit_max;
	cv::Mat m_object_binImg;


	//Data Augmentation
	seDataAugmentationInfo m_DA_Param;



private:
	void IpThreshold_Clear()
	{
		m_vec_dbThresh.clear();
		m_vec_dbMaxVal.clear();
		m_emTypes = emThresholdTypes::THRSH_BINARY;
		if (!m_thresh_img.empty()) m_thresh_img.release();
	}
	void IpHistogram_Clear()
	{
		if (!m_hist_Img.empty()) m_hist_Img.release();
		std::for_each(m_vec2d_hist_Val.begin(), m_vec2d_hist_Val.end(), [](std::vector<int>& v)
			{
				std::fill(v.begin(), v.end(), 0);
			});

		m_hist_Annulus = seAnnulus();
		m_hist_Rect = seBoundingBox();
		m_hist_Circle = seCircle();
		m_emHist_BoxShape = emBoxShape::SHAPE_NODEF;

	}
	void IpMorphology_Clear()
	{
		if (!m_morph_element.empty()) m_morph_element.release();
		m_kernel_size = 3;
		if (!m_morph_destImg.empty()) m_morph_destImg.release();
	}
	void IpNoiseRemoval_Clear()
	{
		m_dbLimit_min = m_dbLimit_max = 0.0;
		if (!m_object_binImg.empty()) m_object_binImg.release();
	}
	void IpDataAugmentation_Clear()
	{
		//TBD
	}


};

