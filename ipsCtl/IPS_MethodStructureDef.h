#pragma once

#ifndef _IPS_METHOD_STRUCTURE_H_
#define	_IPS_METHOD_STRUCTURE_H_ 


/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include <string>

#include "GigECamLib_InterfaceClass.h"
#include "Measure_GlueWidth_InterfaceClass.h"
#include "Measure_GlueWidth_DLL.h"
#include "BaseDataStructureDef.h"

/* GLOBAL VARIABLE DECLARATIONS ------------------------------------------------------- */
extern const char* enum_Subscribe_CAMReg[];
extern const char* enum_Publish_CAMReg[];

/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef enum tEnumAlgoName {

	//VisonBox Method control.
	FLAGE_AUTO_RUNNING,
	FLAGE_TRIGGERMODETYPE,

	//GigE caomera control and streaming === >> 
	//GigE camera control. < static image >
	METHOD_GigeCam_Initialize,
	METHOD_GigeCam_Inquiry,
	METHOD_GigeCam_Config,
	METHOD_GigeCam_Capture,
	METHOD_GigeCam_Release,

	//GigE camera control. < streaming >
	METHOD_GigeCam_Streaming_Initialize,
	METHOD_GigeCam_Streaming_Inquiry,
	METHOD_GigeCam_Streaming_Start,
	METHOD_GigeCam_Streaming_Capture,
	METHOD_GigeCam_Streaming_Stop,
	METHOD_GigeCam_Streaming_Release,

	//GigE caomera control and streaming << === 

	//IPL Algorithm control.
	ALGO_ImageCalibration,

	ALGO_Crop_GoldenTemplate,
	ALGO_PatternMatch,
    
	ALGO_FindProfile,
	ALGO_DetectCircle,

	ALGO_IBOX_Annulus,
	ALGO_IBOX_Rect,
	ALGO_IBOX_Circle,

	ALGO_CalcCoord,

	ALGO_Crop_Annulus,
	ALGO_Crop_Rect,
	ALGO_Crop_Circle,

	ALGO_Hisg_Annulus,
	ALGO_Hisg_Rect,
	ALGO_Hisg_Circle,

	ALGO_IP_Threshold,

	ALGO_IP_Morphology,

	ALGO_IP_NoiseRemoval,

	ALGO_IP_DataAugmentation,


	ALGO_MeasGW_Annulus,
	ALGO_MeasGW_Rect,

	ENUM_ALGO_END	//end of enum 

} enum_IdReg;



typedef struct tagJsonInfo
{
	enum_IdReg	emAlgoId;

	char szCmd[200];
	
	char szJsonBuf[4096];

} seJsonInfo, * LPJsonInfo;



typedef struct tagAlgoParamRegister
{
	enum_IdReg	emAlgoId;
	const char* strCmd;

	//Parameter structure. 
	void* pParam;

	//Json text info.
	char* szJsonInfo;

	//MQTT command parses function.
	int (*MqttParsesFunc)(const char* pCmd, const void* pJson_Obj, void* pParam, void* Json_CmdInfo_Out);

} seAlgoParamReg, * LPAlgoParamReg;



typedef struct tagAlgoMethodRegister
{
	enum_IdReg	emAlgoId;
	const char* strCmd;	//MQTT command [Key]

	
	//Parameter structure pointer.
	void* pParm;

	
	//Results structure pointer.
	void* pRes;

	
	//Json info of "arg"
	char* pJsonBuf;

	//GigE or IPL Algorithm. 
	int(*AlgoMthd)(void* phdl, const LPImageInfo pImgIn, void* pParam, void* pResults, LPImageInfo pImgOut);


	//Json create
    int(*JsonGenerator)(const char* szKey, void* pResult, const char* JsonBuf, const bool bCamdID); // for Dual camera


	//Parameter convertor function pointrer.
	int(*(*ParamConverter)[3])(void* pParam, void* pAllParam);


} seAlgoMethodReg, * LPAlgoMethodReg;


typedef int (*(*ParamCvtMethod)[3])(void* pParam, void* pAllParam);

/* CLASS DECLARATIONS ---------------------------------------------------------------- */
class CAlgoMethodParametr
{
public:
	/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */
	~CAlgoMethodParametr() {};
	CAlgoMethodParametr()
		: pParm{'\0'}
		, strInputImgPath("")
		, strTemplateImgPath("")
		, strSaveImgPath("")
		, strResultImgPath("")
		, mAlgoMethod( { ENUM_ALGO_END, enum_Subscribe_CAMReg[ENUM_ALGO_END], nullptr, nullptr, nullptr, nullptr, nullptr, nullptr } )
		, strMode_Project("")
		, strMode_Name("")
		, strMode_Attr("")
	{};

public:
	/* EXPORTED VARIABLE DECLARATIONS ---------------------------------------------------- */
	char pParm[4096];

	std::string strInputImgPath;
	std::string strTemplateImgPath;
	std::string strSaveImgPath;
	std::string strResultImgPath;

	seAlgoMethodReg mAlgoMethod;

	std::string strMode_Project;	//ex: PROJECT
	std::string strMode_Name;		//ex: utils
	std::string strMode_Attr;		//ex: Engine

};


/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef struct tagExpansionMode {

	int flg_AutoRunning;
	int flg_Enb_TriggerMode;

	int flg_TriggerMode_Activat;

	tagExpansionMode():
		flg_AutoRunning(0),
		flg_Enb_TriggerMode(0),
		flg_TriggerMode_Activat(0)
	{}

} seExpansionMode, * LPseExpansionMode;


#endif		//_IPS_METHOD_STRUCTURE_H_