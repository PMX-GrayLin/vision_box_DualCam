#pragma once

#ifdef _DEBUG
#define _RexTY_DEBUG
#endif

#include <string>
#include <cstdlib>
#include <cstring>
#include <memory>


enum {
	IPL_LOG_LEVEL_NONE = 0,	//No debug information is output.
	IPL_LOG_LEVEL_ERROR,		//Logs all fatal errors.
	IPL_LOG_LEVEL_WARNING,		//Logs all warnings.
	IPL_LOG_LEVEL_MAJOR,		//Logs all "major" messages.	<=== '3'
	IPL_LOG_LEVEL_INFO,			//Logs all informational messages.
	IPL_LOG_LEVEL_DEBUG,		//Logs all debug messages.
	IPL_LOG_LEVEL_LOG,			//Logs all log messages.     (Reserved)
	IPL_LOG_LEVEL_TRACE,		//Logs all trace messages.   (Reserved)
	IPL_LOG_LEVEL_VERBOSE,		//Logs all level messages.
};

extern int iplDebugLevel;

inline void Algo_Log_Level() {
	char* szLogLevel = getenv("IPL_LOG_LEVEL");
	if (szLogLevel) {
		iplDebugLevel = atoi(szLogLevel);
		printf("# iplDebugLevel = %d\n", iplDebugLevel);
	}
	else {
		iplDebugLevel = IPL_LOG_LEVEL_NONE;
		printf("# iplDebugLevel = %d\n", iplDebugLevel);
	}
}

/* define the tty color */
#define iplNONE "\033[m"
#define iplWHITE "\033[1;37m"
#define iplGREEN "\033[0;32;32m"
#define iplBLUE "\033[0;32;34m"
#define iplYELLOW "\033[1;33m"
#define iplRED "\033[0;32;31m"

// white
#define IPLV(format, b...) if ((iplDebugLevel) >= (IPL_LOG_LEVEL_VERBOSE))  printf("__ipl_# %s(): ln:%d :" iplWHITE format iplNONE, __FUNCTION__, __LINE__, ##b)
	//if ((iplDebugLevel) >= (IPL_LOG_LEVEL_VERBOSE))  printf("__ipl_# %s: %s(): ln:%d :" iplWHITE format iplNONE, __FILE__, __FUNCTION__, __LINE__, ##b)

// white
#define IPLD(format, b...) if ((iplDebugLevel) >= (IPL_LOG_LEVEL_DEBUG))  printf("__ipl_# %s(): ln:%d :" iplWHITE format iplNONE, __FUNCTION__, __LINE__, ##b)
	//if ((iplDebugLevel) >= (IPL_LOG_LEVEL_DEBUG))  printf("__ipl_# %s: %s(): ln:%d :" iplWHITE format iplNONE, __FILE__, __FUNCTION__, __LINE__, ##b)

// green
#define IPLI(format, b...) if ((iplDebugLevel) >= (IPL_LOG_LEVEL_INFO))  printf("__ipl_# %s(): ln:%d :" iplGREEN format iplNONE, __FUNCTION__, __LINE__, ##b)
	//if ((iplDebugLevel) >= (IPL_LOG_LEVEL_INFO))  printf("__ipl_# %s: %s(): ln:%d :" iplGREEN format iplNONE, __FILE__, __FUNCTION__, __LINE__, ##b)

// blue
#define IPLM(format, b...) if ((iplDebugLevel) >= (IPL_LOG_LEVEL_MAJOR))  printf("__ipl_# %s(): ln:%d :" iplBLUE format iplNONE, __FUNCTION__, __LINE__, ##b)
	//if ((iplDebugLevel) >= (IPL_LOG_LEVEL_MAJOR))  printf("__cam_# %s: %s(): ln:%d :" iplGREEN format iplNONE, __FILE__, __FUNCTION__, __LINE__, ##b)

// yellow
#define IPLW(format, b...) if ((iplDebugLevel) >= (IPL_LOG_LEVEL_WARNING))  printf("__ipl_# %s: %s(): ln:%d :" iplYELLOW format iplNONE, __FILE__, __FUNCTION__, __LINE__, ##b)
	//if ( (iplDebugLevel) >= (IPL_LOG_LEVEL_WARNING) )  printf("__ipl_# %s(): ln:%d :" iplYELLOW format iplNONE, __FUNCTION__, __LINE__, ##b) 

// red
#define IPLE(format, b...) if ((iplDebugLevel) >= (IPL_LOG_LEVEL_ERROR))  printf("__ipl_# %s: %s(): ln:%d :" iplRED format iplNONE, __FILE__, __FUNCTION__, __LINE__, ##b)
	//if ( (iplDebugLevel) >= (IPL_LOG_LEVEL_ERROR) )  printf("__ipl_# %s(): ln:%d :" iplRED format iplNONE, __FUNCTION__, __LINE__, ##b)





#define _Enb_DebugMode	0

//erroe code of DLL
#define ER_TRUE			( 1)	//TRUE
#define ER_OK			( 0)	//OK
#define ER_ABORT		(-1)	//Abort
#define ER_FAIL			(-2)	//Unknown Error




using namespace std;

//Data structure 
//typedef int IPL_Hnadle;

enum class emColorSpace {

	COLORSPACE_RGB = 0,
	COLORSPACE_HSL = 1,
};



typedef struct tagImageInfo
{
	unsigned char* pbImgBuf;	//Image buffer point
	//shared_ptr<unsigned char> spImgBuf;
	int		iWidth;				//Image Width
	int		iHeight;			//Image Height
	int		iChannels;			//Gray or Color
	int		iBPP;				//Bits per pixel
	int		dwRowBytes;			//Bytes pre Row
	bool	bBottomUp;			//The first line on top or bottom, 0->On top, 1-> On bottom
	bool	bBGR;				//The RGB order, 0->RGB, 1->BGR
	int		iColorSpace;		//ColorSpace define: 0->RGB, 1->HSL, 2->....

	tagImageInfo() :
		pbImgBuf(nullptr),
		iWidth(0),
		iHeight(0),
		iChannels(1),
		iBPP(8),
		dwRowBytes(0),
		bBottomUp(0),
		bBGR(1),
		iColorSpace(static_cast<int>(emColorSpace::COLORSPACE_RGB))
	{}

} seImageInfo, * LPImageInfo;



typedef struct tagStreamingInfo {

	bool IsEnbStreaming;
	std::string strWhatModeIs;

	tagStreamingInfo():
		IsEnbStreaming(0), 	//0 : disable, 1:enable
		strWhatModeIs("None")
	{}

} seStreamingInfo, *LPStreamingInfo;



typedef struct tagRectangle {

	int	left;
	int	top;
	int	right;
	int	bottom;
	int width;
	int height;
	int padding;

	tagRectangle():
		left(0), 
		top(0), 
		right(0), 
		bottom(0), 
		width(0), 
		height(0), 
		padding(0) 
	{}

} seRect, * LPRect;


typedef struct tagCircle {

	int	cX;
	int	cY;
	double dbRadius;
	double dbAngle;

	tagCircle()	:
		cX(0), 
		cY(0), 
		dbRadius(0.0), 
		dbAngle(0.0) 
	{}

} seCircle, * LPCircle;


typedef struct tagAnnulus {

	int cX;
	int cY;
	double dbRadius_Inner;
	double dbRadius_Outer;
	double dbStartAngle;
	double dbEndAngle;

	tagAnnulus(): 
		cX(0), 
		cY(0), 
		dbRadius_Inner(0.0), 
		dbRadius_Outer(0.0), 
		dbStartAngle(0.0), 
		dbEndAngle(360.0) 
	{}

} seAnnulus, * LPAnnulus;


typedef struct tagBoundingBox {

	int	cX;
	int	cY;
	double dbAngle;
	seRect rectBox;

	tagBoundingBox() :
		cX(0), 
		cY(0), 
		dbAngle(0.0), 
		rectBox(seRect()) 
	{}

} seBoundingBox, * LPBoundingBox;


typedef struct tagCalibrateCoord {

	int	iDelta_W;
	int	iDelta_H;
	double dbAngle;
	int iDelta_InspectBox_W;
	int iDelta_InspectBox_H;

	tagCalibrateCoord()	: 
		iDelta_W(0), 
		iDelta_H(0), 
		dbAngle(0.0), 
		iDelta_InspectBox_W(0), 
		iDelta_InspectBox_H(0) 
	{}

} seCalibrateCoord, * LPCalibrateCoord;


typedef struct tagPoint {

	int x;
	int y;

	tagPoint() : 
		x(0), 
		y(0) 
	{}

} sePoint, * LPPoint;



typedef struct tagCoordBindBox {

	seCalibrateCoord CalibCoord;

	seBoundingBox FMark;
	seBoundingBox InsptBox;

} seCoordBindBox, * LPCoordBindBox;


typedef struct tagPoint2D {

	sePoint posStart;
	sePoint posEnd;

} sePoint2D, * LPPoint2D;


typedef struct tagUnitBoxShape
{
	seBoundingBox	se_rect;
	seCircle		se_circle;
	seAnnulus		se_annulus;

} seUintBox_Shape, * LPUintBox_Shape;


typedef struct tagFileInfo {

	string strSaveImgPath;
	string strSaveCropImg;

	tagFileInfo() {

		strSaveImgPath = "";
		strSaveCropImg = "";
	}

} seFileInfo, * LPFileInfo;


typedef struct tagExpandable {

	std::string strImage_Input;
	std::string strImage_Crop;
	std::string strImage_Save;
	std::string strImage_Result;

	seImageInfo* poutImgInfo;

	tagExpandable() :
		strImage_Input(""), 
		strImage_Crop(""), 
		strImage_Save(""),
		strImage_Result(""),
		poutImgInfo(nullptr)
	{}

} seExpandable, LPExpandable;


typedef struct tagDataAugmentationInfo
{
	bool bEnb_Flip_Xasix;
	bool bEnb_Flip_Yasix;
	bool bEnb_Flip_XYasix;
	double dbRotateAngle;
	int iVal_Brightness[3];	//0:R, 1:G, 2:B
	std::string strFileName;

	// Default constructor
	tagDataAugmentationInfo() :
		bEnb_Flip_Xasix(false),
		bEnb_Flip_Yasix(false),
		bEnb_Flip_XYasix(false),
		dbRotateAngle(0.0),
		strFileName("")
	{
		// memset(iVal_Brightness, 0, (sizeof(iVal_Brightness) / sizeof(int)));
		memset(iVal_Brightness, 0, sizeof(iVal_Brightness));
	}

} seDataAugmentationInfo, * LPDataAugmentationInfo;



//typedef struct tagAiELIC_MaseGW_Color {
//
//	seAnnulus roiAnnuls;
//
//	std::string strTask;
//	std::string strUtils;
//	std::string strEngine;
//	std::string strTest;
//
//	tagAiELIC_MaseGW_Color() {
//	
//		roiAnnuls = seAnnulus();
//		strTask = "";
//		strUtils = "";
//		strEngine = "";
//		strTest = "";
//	}
//
//	tagAiELIC_MaseGW_Color(seAnnulus roiAnnuls, std::string strTask, std::string strUtils, std::string strEngine, std::string strTest)
//	:roiAnnuls(seAnnulus()), strTask(""), strUtils(""), strEngine(""), strTest("") {}
//
//} seAiELIC_MaseGW_Color_Param, * LPAiELIC_MaseGW_Color_Param;




//Enum
enum class emMorphShapes {

	MORPH_RECT = 0,
	MORPH_CROSS = 1,
	MORPH_ELLIPSE = 2
};

enum class emMorphOperation {

	MORPH_ERODE = 0,
	MORPH_DILATION = 1,
	MORPH_OPEN = 2,
	MORPH_CLOSE = 3,
};


enum class emThresholdTypes {

	THRSH_BINARY = 0,
	THRSH_OTSU = 1,
	THRSH_ADAPTIVE = 2,
};


typedef enum class emShapeTypes {

	SHAPE_NODEF = 0,
	SHAPE_RECT = 1,
	SHAPE_CIRCLE = 2,
	SHAPE_ANNULUS = 3,
} emBoxShape;

