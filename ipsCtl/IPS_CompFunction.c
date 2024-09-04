/**
 ******************************************************************************
 * @file    IPS_CompFunction.c
 * @brief   The utility tool about Define the content of the Parameter Register Table 
 *          and its related applications.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 Primax Technology Ltd.
 * All rights reserved.
 *
 ******************************************************************************
 */

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <mqueue.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <semaphore.h>
#include <json.h>
#include "../common.h"

#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include "IPS_CompFunction.h"
#include "IPS_CompAlgorithm.h"
#include "IPLAlgoDataStructureDef.h"

using namespace std;

bool was_empty = 1;
pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _cond = PTHREAD_COND_INITIALIZER;

/* EXPORTED SUBPROGRAM BODIES -------------------------------------------------------- */
////////////////////////////////////////////////////////////////////////////////////
// deque of Json information.
////////////////////////////////////////////////////////////////////////////////////

pthread_mutex_t _JsonQ_lock = PTHREAD_MUTEX_INITIALIZER;
std::deque<seJsonInfo> BufQueue_Json;
/***********************************************************
 *	Function 	: JsonQ_Init
 *	Description : To initiate _JsonQ_lock
 *	Param 		:
 *	Return		: 0, -1
 *************************************************************/
int JsonQ_Init()
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_JsonQ_lock, nullptr);
    return res;
}

/***********************************************************
 *	Function 	: JsonQ_EnQ
 *	Description : To push back the queue: BufQueue_Json
 *	Param 		: seJsonInfo seInfo
 *	Return		: error number
 *************************************************************/
int JsonQ_EnQ(seJsonInfo seInfo)
{
    int res = 0;
    pthread_mutex_lock(&_JsonQ_lock);

    BufQueue_Json.push_back(seInfo);

    pthread_mutex_unlock(&_JsonQ_lock);
    return res;
}

/***********************************************************
 *	Function 	: JsonQ_DeQ
 *	Description : To pop front of the queue: BufQueue_Json
 *	Param 		: return message: seJsonInfo *pInfo
 *	Return		: error number
 *************************************************************/
int JsonQ_DeQ(seJsonInfo *pInfo)
{
    int res = 0;
    pthread_mutex_lock(&_JsonQ_lock);

    if (!BufQueue_Json.empty())
    {

        *pInfo = *BufQueue_Json.begin(); // [0] ;

        BufQueue_Json.pop_front();
    }
    else
    {
        res = -1;
    }

    pthread_mutex_unlock(&_JsonQ_lock);
    return res;
}

void JsonQ_Destory()
{
    pthread_mutex_destroy(&_JsonQ_lock);
}


////////////////////////////////////////////////////////////////////////////////////
// deque of Json information.
////////////////////////////////////////////////////////////////////////////////////

pthread_mutex_t _JsonQ_Dual_lock[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};
std::vector<std::deque<seJsonInfo>> BufQueue_Dual_Json(2);
/***********************************************************
 *	Function 	: JsonQ_Init_Dual
 *	Description : To initiate _JsonQ_Dual_lock
 *	Param 		:
 *	Return		: 0, -1
 *************************************************************/
int JsonQ_Init_Dual(const int iID)
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_JsonQ_Dual_lock[iID], nullptr);
    return res;
}

/***********************************************************
 *	Function 	: JsonQ_EnQ_Dual
 *	Description : To push back the queue: BufQueue_Dual_Json
 *	Param 		: seJsonInfo seInfo
 *	Return		: error number
 *************************************************************/
int JsonQ_EnQ_Dual(seJsonInfo seInfo, const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_JsonQ_Dual_lock[iID]);

    BufQueue_Dual_Json[iID].push_back(seInfo);

    pthread_mutex_unlock(&_JsonQ_Dual_lock[iID]);
    return res;
}

/***********************************************************
 *	Function 	: JsonQ_DeQ_Dual
 *	Description : To pop front of the queue: _JsonQ_Dual_lock
 *	Param 		: return message: seJsonInfo *pInfo
 *	Return		: error number
 *************************************************************/
int JsonQ_DeQ_Dual(seJsonInfo *pInfo, const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_JsonQ_Dual_lock[iID]);

    if (!BufQueue_Dual_Json[iID].empty())
    {

        *pInfo = *BufQueue_Dual_Json[iID].begin(); // [0] ;

        BufQueue_Dual_Json[iID].pop_front();
    }
    else
    {
        res = -1;
    }

    pthread_mutex_unlock(&_JsonQ_Dual_lock[iID]);
    return res;
}

void JsonQ_Destory_Dual(const int iID)
{
    pthread_mutex_destroy(&_JsonQ_Dual_lock[iID]);
}


////////////////////////////////////////////////////////////////////////////////////
// Parameter List define.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// --> Vision Box Mode.

static seMode_AutoRunning gArg_Mode_AutoRunning;

static seMode_TriggerModeType gArg_TriggerModeType;

// Gige camera control. < static image >
static seGigECamConfig gArg_GigeCam_Initialize;
static seGigECamConfig gArg_GigeCam_Inquiry;
static seGigECamConfig gArg_GigeCam_Config;
static seGigECamConfig gArg_GigeCam_Parameter;
static seGigECamConfig gArg_GigeCam_Release;

// Gige camera control. < streaming >
static seGigECamConfig gArg_GigeCamStrm_Initialize;
static seGigECamConfig gArg_GigeCamStrm_Inquiry;
static seGigECamConfig gArg_GigeCamStrm_Start;
static seGigECamConfig gArg_GigeCamStrm_Capture;
static seGigECamConfig gArg_GigeCamStrm_Stop;
static seGigECamConfig gArg_GigeCamStrm_Release;

// IPL algorithm control.
static seCropROI_GTemplate gArg_Crop_GTemplate;

static seMth_ImageCalibration gArg_Image_Calibration;

static seMth_PatternMatch gArg_PatternMatch;
static seMth_FindProfile gArg_FindProfile;
static seMth_DetectCircle gArg_DetectCircle;

static seIBox_Annulus gArg_IBox_Annulus;
static seIBox_Rect gArg_IBox_Rect;
static seIBox_Circle gArg_IBox_Circle;

static seCalcCoord gArg_CalcCoord;

static seCropROI_Annulus gArg_Crop_Annulus;
static seCropROI_Rect gArg_Crop_Rect;
static seCropROI_Circle gArg_Crop_Circle;

static seHisg_Annulus gArg_Hisg_Annulus;
static seHisg_Rect gArg_Hisg_Rect;
static seHisg_Circle gArg_Hisg_Circle;

static seIP_Threshold gArg_Threshold;

static seMorphology gArg_Morphology;

static seNoiseRemoval gArg_NoiseRemoval;

static seDataAugmentation gArg_DataAugmentation;

static seMeasGW_Annulus gArg_MeasGW_Annulus;
static seMeasGW_Rect gArg_MeasGW_Rect;

static seAiELIC_MeasGlueWidth_Color gArg_AiELIC_MeasGW_Color;

////////////////////////////////////////////////////////////////////////////////////
// Function List define.
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: MqttParse_AutoRunningMode
 *	Description : Parse the parameters related to Auto Running Mode 
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_AutoRunningMode(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seMode_AutoRunning *pSE = (seMode_AutoRunning *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_param = (struct json_object *)json_object_object_get(j_args, "Enb_AutoRunning");
    if (nullptr == j_param)
    {
        pSE->bFlg_AutoRunning = 0;
    }
    else
    {
        pSE->bFlg_AutoRunning = json_object_get_int(j_param);
    }

    IPSLOG(0, "[__%s__] : Enb_AutoRunning = %d\n", __func__, json_object_get_int(j_param));
    IPSLOG(0, "[__%s__] : Enb_AutoRunning = %d\n", __func__, json_object_get_int(j_param));

    j_param = (struct json_object *)json_object_object_get(j_args, "Enb_TriggerMode");
    pSE->bFlg_Enable_TriggerMode = json_object_get_int(j_param);

    IPSLOG(0, "[__%s__] : Enb_TriggerMode = %d\n", __func__, json_object_get_int(j_param));
    IPSLOG(0, "[__%s__] : Enb_TriggerMode = %d\n", __func__, json_object_get_int(j_param));

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_TriggerModeType
 *	Description : Parse the parameters related to Trigger Mode Type 
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_TriggerModeType(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seMode_TriggerModeType *pSE = (seMode_TriggerModeType *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_param = (struct json_object *)json_object_object_get(j_args, "Enb_TriggerMode_Activate");

    IPSLOG(0, "[__%s__] : Enb_TriggerMode_Activate = %d\n", __func__, json_object_get_int(j_param));
    IPSLOG(0, "[__%s__] : Enb_TriggerMode_Activate = %d\n", __func__, json_object_get_int(j_param));
    IPSLOG(0, "[__%s__] : Enb_TriggerMode_Activate = %d\n", __func__, json_object_get_int(j_param));

    pSE->bFlg_TriggerMode_Activate = json_object_get_int(j_param);


    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

// ------- . -------- GigE camera control. < static image >.start
/***********************************************************
 *	Function 	: MqttParse_GigECameraInit
 *	Description : Parse the parameters related to Initial GigE Camera 
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraInit(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    (void)pSE;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));


    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_GigECameraInq
 *	Description : Parse the parameters related to GigE Camera Inqury
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraInq(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    (void)pSE;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_GigECameraCfg
 *	Description : Parse the parameters related to GigE Camera Config
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraCfg(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_param = (struct json_object *)json_object_object_get(j_args, "PixelFormat");
    IPSLOG(0, "[__%s__] : PixelFormat = %s\n", __func__, json_object_get_string(j_param));
    strTmp = json_object_get_string(j_param);
    if (strTmp == "Mono8")
    { // 0:Mono8, 1:BayerGR8
        pSE->bPixelFormat = 0;
    }
    else
    {
        pSE->bPixelFormat = 1;
    }
    IPSLOG(0, "[__%s__] : pSE->bPixelFormat = %d\n", __func__, pSE->bPixelFormat);

    j_param = (struct json_object *)json_object_object_get(j_args, "Width");
    IPSLOG(0, "[__%s__] : Width = %d\n", __func__, json_object_get_int(j_param));
    pSE->iWidth = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "Height");
    IPSLOG(0, "[__%s__] : Height = %d\n", __func__, json_object_get_int(j_param));
    pSE->iHeight = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "Offset_X");
    IPSLOG(0, "[__%s__] : Offset_X = %d\n", __func__, json_object_get_int(j_param));
    pSE->iOffset_X = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "Offset_Y");
    IPSLOG(0, "[__%s__] : Offset_Y = %d\n", __func__, json_object_get_int(j_param));
    pSE->iOffset_Y = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "BinningScale");
    IPSLOG(0, "[__%s__] : BinningScale = %d\n", __func__, json_object_get_int(j_param));
    pSE->iBinning_Scale = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ExposureMode");
    IPSLOG(0, "[__%s__] : ExposureAuto = %s\n", __func__, json_object_get_string(j_param));
    strTmp = json_object_get_string(j_param);
    if (strTmp == "Auto")
    {
        IPSLOG(0, "[__%s__] : ExposureAuto ==>> ===> Auto\n", __func__);
        pSE->bExposureAuto = 0; // 0: Auto
    }
    else
    {
        IPSLOG(0, "[__%s__] : ExposureAuto ==>> ===> Off\n", __func__);
        pSE->bExposureAuto = 1; // 1: Timed_(Off);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "ExposureTime");
    IPSLOG(0, "[__%s__] : ExposureTime = %5.3f\n", __func__, json_object_get_double(j_param));
    pSE->dbExposureTime = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "TriggerMode");
    IPSLOG(0, "[__%s__] : TriggerMode = %d\n", __func__, json_object_get_int(j_param));
    pSE->bIsEnbTriggerMode = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "TriggerActivation");
    IPSLOG(0, "[__%s__] : TriggerActivation = %d\n", __func__, json_object_get_int(j_param));
    pSE->iTriggerActivation = json_object_get_int(j_param);
    pSE->strSaveImgPath = ""; // json_object_get_string(j_param);


    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_GigECameraParam
 *	Description : Parse the parameters related to GigE Camera Parameters
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraParam(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */

    j_param = (struct json_object *)json_object_object_get(j_args, "IsEnbReadImageMode");
    if (nullptr == j_param)
    {
        IPSLOG(0, "[__%s__] : IsEnbReadImageMode is Empty\n", __func__);
        pSE->bIsEnbReadImageMode = 0;
    }
    else
    {
        IPSLOG(0, "[__%s__] : IsEnbReadImageMode = %d\n", __func__, json_object_get_int(j_param));
        pSE->bIsEnbReadImageMode = json_object_get_int(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    if (nullptr == j_param)
    {
        IPSLOG(0, "[__%s__] : InputImgPath is Empty()\n", __func__);
        pSE->strInputImgPath = "";
    }
    else
    {
        IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
        pSE->strInputImgPath = json_object_get_string(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
    if (nullptr == j_param)
    {
        IPSLOG(0, "[__%s__] : InputImgPath is Empty()\n", __func__);
        pSE->strSaveImgPath = "";
    }
    else
    {
        IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
        pSE->strSaveImgPath = json_object_get_string(j_param);
    }

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_GigECameraRelease
 *	Description : Parse the parameters related to GigE Camera Release
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraRelease(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    (void)pSE;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));


    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}
// ------- . -------- GigE camera control. < static image >.end


// ------- . -------- GigE camera control. < streaming >.start
/***********************************************************
 *	Function 	: MqttParse_GigECameraStrmInit
 *	Description : Parse the parameters related to GigE Camera Streaming Initiation
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraStrmInit(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    (void)pSE;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_GigECameraStrmInq
 *	Description : Parse the parameters related to GigE Camera Streaming Inqury
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraStrmInq(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    (void)pSE;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_GigECameraStrmStart
 *	Description : Parse the parameters related to GigE Camera Streaming Start
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraStrmStart(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    (void)pSE;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_GigECameraStrmCapture
 *	Description : Parse the parameters related to GigE Camera Streaming Capture
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraStrmCapture(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    (void)pSE;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    if (nullptr == j_param)
    {
        IPSLOG(0, "[__%s__] : strSaveImgPath is Empty()\n", __func__);
        pSE->strSaveImgPath = "";
    }
    else
    {
        IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
        pSE->strSaveImgPath = json_object_get_string(j_param);
    }

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_GigECameraStrmStop
 *	Description : Parse the parameters related to GigE Camera Streaming Stop
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraStrmStop(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    (void)pSE;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_GigECameraStrmRelease
 *	Description : Parse the parameters related to GigE Camera Streaming Release
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_GigECameraStrmRelease(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    IPSLOG(0, "[__%s__] : Json Handle Address = 0x%llx\n", __func__, pJson_Obj);

    j_subsystem = (struct json_object *)pJson_Obj;

    seGigECamConfig *pSE = (seGigECamConfig *)pParam;
    (void)pSE;
    IPSLOG(0, "[__%s__] : seGigECamConfig* pSE = 0x%llx\n", __func__, pSE);

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}
// ------- . -------- GigE camera control. < sreaming >.end


/***********************************************************
 *	Function 	: MqttParse_IMG_Calibration
 *	Description : Parse the parameters related to Image Calibration
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_IMG_Calibration(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    string strTmp;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seMth_ImageCalibration *pSE = (seMth_ImageCalibration *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    j_param = (struct json_object *)json_object_object_get(j_args, "E2E_Distance_mm");
    IPSLOG(0, "[__%s__] : E2E_Distance_mm = %s5.2f\n", __func__, json_object_get_double(j_param));
    pSE->dbE2E_Distance_mm = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_CropROI_GTemplate
 *	Description : Parse the parameters related to Crop ROI as a Golden Template
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_CropROI_GTemplate(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seCropROI_GTemplate *pSE = (seCropROI_GTemplate *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Rect");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.right = json_object_get_int(j_param);

        pSE->roiRect.width = abs(pSE->roiRect.right - pSE->roiRect.left);
        pSE->roiRect.height = abs(pSE->roiRect.bottom - pSE->roiRect.top);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strSaveImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_PatternMatch
 *	Description : Parse the parameters related to Pattern Match
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_PatternMatch(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seMth_PatternMatch *pSE = (seMth_PatternMatch *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_SearchRect");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiSearch.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiSearch.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiSearch.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiSearch.right = json_object_get_int(j_param);

        pSE->roiSearch.width = abs(pSE->roiSearch.right - pSE->roiSearch.left);
        pSE->roiSearch.height = abs(pSE->roiSearch.bottom - pSE->roiSearch.top);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "TemplateImgPath");
    IPSLOG(0, "[__%s__] : TemplateImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strTemplateImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_FindProfile
 *	Description : Parse the parameters related to Find Profile
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_FindProfile(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seMth_FindProfile *pSE = (seMth_FindProfile *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Annulus_Outer");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiSearch.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiSearch.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiSearch.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius");
        IPSLOG(0, "[__%s__] : Radius = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiSearch.dbRadius = json_object_get_double(j_param);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Annulus_Inner");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiMask.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiMask.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiMask.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius");
        IPSLOG(0, "[__%s__] : Radius = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiMask.dbRadius = json_object_get_double(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "SearchDirection");
    IPSLOG(0, "[__%s__] : SearchDirection = %d\n", __func__, json_object_get_int(j_param));
    pSE->bDirection = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "EdgePolarity");
    IPSLOG(0, "[__%s__] : EdgePolarity = %d\n", __func__, json_object_get_int(j_param));
    pSE->bPolarity = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "SelLineNo");
    IPSLOG(0, "[__%s__] : SelLineNo = %d\n", __func__, json_object_get_int(j_param));
    pSE->iSelLineNo = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_DetectCircle
 *	Description : Parse the parameters related to Detect Circle
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_DetectCircle(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seMth_DetectCircle *pSE = (seMth_DetectCircle *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Annulus_Outer");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiSearch.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiSearch.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiSearch.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius");
        IPSLOG(0, "[__%s__] : Radius = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiSearch.dbRadius = json_object_get_double(j_param);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Annulus_Inner");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiMask.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiMask.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiMask.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius");
        IPSLOG(0, "[__%s__] : Radius = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiMask.dbRadius = json_object_get_double(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "SearchDirection");
    IPSLOG(0, "[__%s__] : SearchDirection = %d\n", __func__, json_object_get_int(j_param));
    pSE->bDirection = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "EdgePolarity");
    IPSLOG(0, "[__%s__] : EdgePolarity = %d\n", __func__, json_object_get_int(j_param));
    pSE->bPolarity = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "iMinEdgeStrength");
    IPSLOG(0, "[__%s__] : iMinEdgeStrength = %d\n", __func__, json_object_get_int(j_param));
    pSE->iMinEdgeStrength = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_IBOX_Annulus
 *	Description : Parse the parameters related to Detection Box (Annulus)
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_IBOX_Annulus(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seIBox_Annulus *pSE = (seIBox_Annulus *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Annulus");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiAnnulus.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiAnnulus.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius_Inner");
        IPSLOG(0, "[__%s__] : Radius_Inner = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbRadius_Inner = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius_Outer");
        IPSLOG(0, "[__%s__] : Radius_Outer = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbRadius_Outer = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle_Start");
        IPSLOG(0, "[__%s__] : Angle_Start = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbStartAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle_End");
        IPSLOG(0, "[__%s__] : Angle_End = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbEndAngle = json_object_get_double(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_IBOX_Rect
 *	Description : Parse the parameters related to Detection Box (Rectangle)
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_IBOX_Rect(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seIBox_Rect *pSE = (seIBox_Rect *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROIBB_Rect");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiRect.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : Left = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : Bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : Right = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.right = json_object_get_int(j_param);

        pSE->roiRect.rectBox.width = abs(pSE->roiRect.rectBox.right - pSE->roiRect.rectBox.left);
        pSE->roiRect.rectBox.height = abs(pSE->roiRect.rectBox.bottom - pSE->roiRect.rectBox.top);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_IBOX_Circle
 *	Description : Parse the parameters related to Detection Box (Circle)
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_IBOX_Circle(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seIBox_Circle *pSE = (seIBox_Circle *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Circle");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiCircle.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiCircle.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiCircle.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius");
        IPSLOG(0, "[__%s__] : Radius = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiCircle.dbRadius = json_object_get_double(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_CalcCoord
 *	Description : Parse the parameters related to Calculate Coordinate
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_CalcCoord(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seCalcCoord *pSE = (seCalcCoord *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_C_FMark");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seFMarkBox.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seFMarkBox.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seFMarkBox.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seFMarkBox.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seFMarkBox.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seFMarkBox.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seFMarkBox.rectBox.right = json_object_get_int(j_param);

        pSE->seFMarkBox.rectBox.width = abs(pSE->seFMarkBox.rectBox.right - pSE->seFMarkBox.rectBox.left);
        pSE->seFMarkBox.rectBox.height = abs(pSE->seFMarkBox.rectBox.bottom - pSE->seFMarkBox.rectBox.top);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_C_IBox");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seInspBox.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seInspBox.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seInspBox.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seInspBox.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seInspBox.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seInspBox.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seInspBox.rectBox.right = json_object_get_int(j_param);

        pSE->seInspBox.rectBox.width = abs(pSE->seInspBox.rectBox.right - pSE->seInspBox.rectBox.left);
        pSE->seInspBox.rectBox.height = abs(pSE->seInspBox.rectBox.bottom - pSE->seInspBox.rectBox.top);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "CalibCoord_G");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Anlgle");
        IPSLOG(0, "[__%s__] : Anlgle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seCoorBindBoxIn.CalibCoord.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_FMark_W");
        IPSLOG(0, "[__%s__] : Delta_FMark_W = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.CalibCoord.iDelta_W = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_FMark_H");
        IPSLOG(0, "[__%s__] : Delta_FMark_H = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.CalibCoord.iDelta_H = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_IBox_W");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.CalibCoord.iDelta_InspectBox_W = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_IBox_H");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.CalibCoord.iDelta_InspectBox_H = json_object_get_int(j_param);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_G_FMaek");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.FMark.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.FMark.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seCoorBindBoxIn.FMark.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.FMark.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.FMark.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.FMark.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.FMark.rectBox.right = json_object_get_int(j_param);

        pSE->seCoorBindBoxIn.FMark.rectBox.width = abs(pSE->seCoorBindBoxIn.FMark.rectBox.right - pSE->seCoorBindBoxIn.FMark.rectBox.left);
        pSE->seCoorBindBoxIn.FMark.rectBox.height = abs(pSE->seCoorBindBoxIn.FMark.rectBox.bottom - pSE->seCoorBindBoxIn.FMark.rectBox.top);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_G_IBox");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.InsptBox.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.InsptBox.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seCoorBindBoxIn.InsptBox.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.InsptBox.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.InsptBox.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.InsptBox.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoorBindBoxIn.InsptBox.rectBox.right = json_object_get_int(j_param);

        pSE->seCoorBindBoxIn.InsptBox.rectBox.width = abs(pSE->seCoorBindBoxIn.InsptBox.rectBox.right - pSE->seCoorBindBoxIn.InsptBox.rectBox.left);
        pSE->seCoorBindBoxIn.InsptBox.rectBox.height = abs(pSE->seCoorBindBoxIn.InsptBox.rectBox.bottom - pSE->seCoorBindBoxIn.InsptBox.rectBox.top);
    }

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_Crop_Annulus
 *	Description : Parse the parameters related to Crop Annulus
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_Crop_Annulus(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seCropROI_Annulus *pSE = (seCropROI_Annulus *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "CalibCoord");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {
        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Anlgle");
        IPSLOG(0, "[__%s__] : Anlgle = %5.2f\n", __func__, json_object_get_double(j_param));
        pSE->seCoordBox.CalibCoord.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_FMark_W");
        IPSLOG(0, "[__%s__] : Delta_FMark_W = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_W = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_FMark_H");
        IPSLOG(0, "[__%s__] : Delta_FMark_H = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_H = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_IBox_W");
        IPSLOG(0, "[__%s__] : Delta_IBox_W = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_InspectBox_W = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_IBox_H");
        IPSLOG(0, "[__%s__] : Delta_IBox_H = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_InspectBox_H = json_object_get_int(j_param);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_FMark");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seCoordBox.FMark.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.right = json_object_get_int(j_param);

        pSE->seCoordBox.FMark.rectBox.width = abs(pSE->seCoordBox.FMark.rectBox.right - pSE->seCoordBox.FMark.rectBox.left);
        pSE->seCoordBox.FMark.rectBox.height = abs(pSE->seCoordBox.FMark.rectBox.bottom - pSE->seCoordBox.FMark.rectBox.top);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_IBox");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seCoordBox.InsptBox.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.right = json_object_get_int(j_param);

        pSE->seCoordBox.InsptBox.rectBox.width = abs(pSE->seCoordBox.InsptBox.rectBox.right - pSE->seCoordBox.InsptBox.rectBox.left);
        pSE->seCoordBox.InsptBox.rectBox.height = abs(pSE->seCoordBox.InsptBox.rectBox.bottom - pSE->seCoordBox.InsptBox.rectBox.top);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Annulus");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiAnnulus.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiAnnulus.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius_Inner");
        IPSLOG(0, "[__%s__] : Radius_Inner = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbRadius_Inner = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius_Outer");
        IPSLOG(0, "[__%s__] : Radius_Outer = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbRadius_Outer = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle_Start");
        IPSLOG(0, "[__%s__] : Angle_Start = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbStartAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle_End");
        IPSLOG(0, "[__%s__] : Angle_End = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbEndAngle = json_object_get_double(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strSaveImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_Crop_Rect
 *	Description : Parse the parameters related to Crop Rectangle
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_Crop_Rect(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seCropROI_Rect *pSE = (seCropROI_Rect *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "CalibCoord");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {
        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Anlgle");
        IPSLOG(0, "[__%s__] : Anlgle = %5.2f\n", __func__, json_object_get_double(j_param));
        pSE->seCoordBox.CalibCoord.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_FMark_W");
        IPSLOG(0, "[__%s__] : Delta_FMark_W = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_W = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_FMark_H");
        IPSLOG(0, "[__%s__] : Delta_FMark_H = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_H = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_IBox_W");
        IPSLOG(0, "[__%s__] : Delta_IBox_W = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_InspectBox_W = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_IBox_H");
        IPSLOG(0, "[__%s__] : Delta_IBox_H = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_InspectBox_H = json_object_get_int(j_param);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_FMark");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seCoordBox.FMark.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.right = json_object_get_int(j_param);

        pSE->seCoordBox.FMark.rectBox.width = abs(pSE->seCoordBox.FMark.rectBox.right - pSE->seCoordBox.FMark.rectBox.left);
        pSE->seCoordBox.FMark.rectBox.height = abs(pSE->seCoordBox.FMark.rectBox.bottom - pSE->seCoordBox.FMark.rectBox.top);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_IBox");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seCoordBox.InsptBox.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.right = json_object_get_int(j_param);

        pSE->seCoordBox.InsptBox.rectBox.width = abs(pSE->seCoordBox.InsptBox.rectBox.right - pSE->seCoordBox.InsptBox.rectBox.left);
        pSE->seCoordBox.InsptBox.rectBox.height = abs(pSE->seCoordBox.InsptBox.rectBox.bottom - pSE->seCoordBox.InsptBox.rectBox.top);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROIBB_Rect");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiRect.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : Left = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : Bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : Right = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.right = json_object_get_int(j_param);

        pSE->roiRect.rectBox.width = abs(pSE->roiRect.rectBox.right - pSE->roiRect.rectBox.left);
        pSE->roiRect.rectBox.height = abs(pSE->roiRect.rectBox.bottom - pSE->roiRect.rectBox.top);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strSaveImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_Crop_Circle
 *	Description : Parse the parameters related to Crop Circle
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_Crop_Circle(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seCropROI_Circle *pSE = (seCropROI_Circle *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "CalibCoord");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {
        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Anlgle");
        IPSLOG(0, "[__%s__] : Anlgle = %5.2f\n", __func__, json_object_get_double(j_param));
        pSE->seCoordBox.CalibCoord.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_FMark_W");
        IPSLOG(0, "[__%s__] : Delta_FMark_W = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_W = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_FMark_H");
        IPSLOG(0, "[__%s__] : Delta_FMark_H = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_H = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_IBox_W");
        IPSLOG(0, "[__%s__] : Delta_IBox_W = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_InspectBox_W = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Delta_IBox_H");
        IPSLOG(0, "[__%s__] : Delta_IBox_H = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.CalibCoord.iDelta_InspectBox_H = json_object_get_int(j_param);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_FMark");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seCoordBox.FMark.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.FMark.rectBox.right = json_object_get_int(j_param);

        pSE->seCoordBox.FMark.rectBox.width = abs(pSE->seCoordBox.FMark.rectBox.right - pSE->seCoordBox.FMark.rectBox.left);
        pSE->seCoordBox.FMark.rectBox.height = abs(pSE->seCoordBox.FMark.rectBox.bottom - pSE->seCoordBox.FMark.rectBox.top);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "BoundingBox_IBox");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->seCoordBox.InsptBox.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : left = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : right = %d\n", __func__, json_object_get_int(j_param));
        pSE->seCoordBox.InsptBox.rectBox.right = json_object_get_int(j_param);

        pSE->seCoordBox.InsptBox.rectBox.width = abs(pSE->seCoordBox.InsptBox.rectBox.right - pSE->seCoordBox.InsptBox.rectBox.left);
        pSE->seCoordBox.InsptBox.rectBox.height = abs(pSE->seCoordBox.InsptBox.rectBox.bottom - pSE->seCoordBox.InsptBox.rectBox.top);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Circle");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiCircle.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiCircle.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiCircle.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius");
        IPSLOG(0, "[__%s__] : Radius = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiCircle.dbRadius = json_object_get_double(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strSaveImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_Crop_Circle
 *	Description : Parse the parameters related to Histogram Annulus
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_Hisg_Annulus(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seHisg_Annulus *pSE = (seHisg_Annulus *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Annulus");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiAnnulus.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiAnnulus.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius_Inner");
        IPSLOG(0, "[__%s__] : Radius_Inner = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbRadius_Inner = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius_Outer");
        IPSLOG(0, "[__%s__] : Radius_Outer = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbRadius_Outer = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle_Start");
        IPSLOG(0, "[__%s__] : Angle_Start = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbStartAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle_End");
        IPSLOG(0, "[__%s__] : Angle_End = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnulus.dbEndAngle = json_object_get_double(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_Hisg_Rect
 *	Description : Parse the parameters related to Histogram Rectangle
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_Hisg_Rect(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seHisg_Rect *pSE = (seHisg_Rect *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROIBB_Rect");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiRect.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : Left = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : Bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : Right = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.right = json_object_get_int(j_param);

        pSE->roiRect.rectBox.width = abs(pSE->roiRect.rectBox.right - pSE->roiRect.rectBox.left);
        pSE->roiRect.rectBox.height = abs(pSE->roiRect.rectBox.bottom - pSE->roiRect.rectBox.top);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_Hisg_Circle
 *	Description : Parse the parameters related to Histogram Circle
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_Hisg_Circle(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seHisg_Circle *pSE = (seHisg_Circle *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Circle");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiCircle.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiCircle.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiCircle.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius");
        IPSLOG(0, "[__%s__] : Radius = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiCircle.dbRadius = json_object_get_double(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_IP_Threshold
 *	Description : Parse the parameters related to IP Threshold
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_IP_Threshold(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seIP_Threshold *pSE = (seIP_Threshold *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_param = (struct json_object *)json_object_object_get(j_args, "pThresh_Min");
    IPSLOG(0, "[__%s__] : pThresh_Min = %5.2f\n", __func__, json_object_get_double(j_param));
    pSE->dbThresh = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "pThresh_Max");
    IPSLOG(0, "[__%s__] : pThresh_Max = %5.2f\n", __func__, json_object_get_double(j_param));
    pSE->dbMaxVal = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "emThresholdTypes");
    IPSLOG(0, "[__%s__] : emThresholdTypes = %d\n", __func__, json_object_get_int(j_param));
    pSE->emTypes = static_cast<emThresholdTypes>(json_object_get_int(j_param));

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strSaveImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_IP_Morphology
 *	Description : Parse the parameters related to IP Morphology
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_IP_Morphology(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seMorphology *pSE = (seMorphology *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_param = (struct json_object *)json_object_object_get(j_args, "emMorphShapes");
    IPSLOG(0, "[__%s__] : emMorphShapes = %d\n", __func__, json_object_get_int(j_param));
    pSE->emShapes = static_cast<emMorphShapes>(json_object_get_int(j_param));

    j_param = (struct json_object *)json_object_object_get(j_args, "iKSize");
    IPSLOG(0, "[__%s__] : iKSize = %d\n", __func__, json_object_get_int(j_param));
    pSE->iKSize = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "emMorphOperation");
    IPSLOG(0, "[__%s__] : emMorphOperation = %d\n", __func__, json_object_get_int(j_param));
    pSE->emOperation = static_cast<emMorphOperation>(json_object_get_int(j_param));

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strSaveImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_IP_Morphology
 *	Description : Parse the parameters related to IP Noise Removal
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_IP_NoiseRemoval(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seNoiseRemoval *pSE = (seNoiseRemoval *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_param = (struct json_object *)json_object_object_get(j_args, "dbLimit_min");
    IPSLOG(0, "[__%s__] : dbLimit_min = %5.2f\n", __func__, json_object_get_double(j_param));
    pSE->dbLimit_min = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "dbLimit_max");
    IPSLOG(0, "[__%s__] : dbLimit_max = %5.2f\n", __func__, json_object_get_double(j_param));
    pSE->dbLimit_max = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strSaveImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_IP_Morphology
 *	Description : Parse the parameters related to IP Data Augmentation
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_IP_DataAugmentation(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seDataAugmentation *pSE = (seDataAugmentation *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_param = (struct json_object *)json_object_object_get(j_args, "bEnb_Flip_Xasix");
    IPSLOG(0, "[__%s__] : bEnb_Flip_Xasix = %d\n", __func__, json_object_get_int(j_param));
    pSE->seDA_Param.bEnb_Flip_Xasix = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "bEnb_Flip_Yasix");
    IPSLOG(0, "[__%s__] : bEnb_Flip_Yasix = %d\n", __func__, json_object_get_int(j_param));
    pSE->seDA_Param.bEnb_Flip_Yasix = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "bEnb_Flip_XYasix");
    IPSLOG(0, "[__%s__] : bEnb_Flip_XYasix = %d\n", __func__, json_object_get_int(j_param));
    pSE->seDA_Param.bEnb_Flip_XYasix = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "dbRotateAngle");
    IPSLOG(0, "[__%s__] : dbRotateAngle = %5.2f\n", __func__, json_object_get_double(j_param));
    pSE->seDA_Param.dbRotateAngle = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "iVal_Brightness_R");
    IPSLOG(0, "[__%s__] : iVal_Brightness_R = %d\n", __func__, json_object_get_int(j_param));
    pSE->seDA_Param.iVal_Brightness[0] = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "iVal_Brightness_G");
    IPSLOG(0, "[__%s__] : iVal_Brightness_G = %d\n", __func__, json_object_get_int(j_param));
    pSE->seDA_Param.iVal_Brightness[1] = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "iVal_Brightness_B");
    IPSLOG(0, "[__%s__] : iVal_Brightness_B = %d\n", __func__, json_object_get_int(j_param));
    pSE->seDA_Param.iVal_Brightness[2] = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "SaveImgPath");
    IPSLOG(0, "[__%s__] : SaveImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strSaveImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_MeasGW_Annulus
 *	Description : Parse the parameters related to Measurement Glue Width (Annulus)
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_MeasGW_Annulus(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seMeasGW_Annulus *pSE = (seMeasGW_Annulus *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROI_Annulus");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiAnnuls.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiAnnuls.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius_Inner");
        IPSLOG(0, "[__%s__] : Radius_Inner = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnuls.dbRadius_Inner = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Radius_Outer");
        IPSLOG(0, "[__%s__] : Radius_Outer = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnuls.dbRadius_Outer = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle_Start");
        IPSLOG(0, "[__%s__] : Angle_Start = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnuls.dbStartAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle_End");
        IPSLOG(0, "[__%s__] : Angle_End = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiAnnuls.dbEndAngle = json_object_get_double(j_param);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "StepSize");
    IPSLOG(0, "[__%s__] : StepSize = %d\n", __func__, json_object_get_int(j_param));
    pSE->stepSize = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "mm_per_pixel");
    IPSLOG(0, "[__%s__] : mm_per_pixel = %5.2f\n", __func__, json_object_get_double(j_param));
    pSE->dbmm_per_pixel = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

/***********************************************************
 *	Function 	: MqttParse_ALGO_MeasGW_Rect
 *	Description : Parse the parameters related to Measurement Glue Width (Rectangle)
 *                in MQTT messages and store the parsing results 
 *                in the specified structure.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int MqttParse_ALGO_MeasGW_Rect(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    struct json_object *j_subsystem;
    struct json_object *j_args, *j_param, *j_parm0;

    IPSLOG(0, "[__%s__] : ===> \n", __func__);

    j_subsystem = (struct json_object *)pJson_Obj;

    seMeasGW_Rect *pSE = (seMeasGW_Rect *)pParam;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(j_subsystem, "args");
    IPSLOG(0, "[__%s__] : args=%s\n", __func__, json_object_get_string(j_args));

    // Copy all json of "arg" information.
    strcpy((char*)Json_CmdInfo_Out, json_object_get_string(j_args));

    ///////////////////////////////////////////////////////////////////////////////////////
    /* paring PARAMETER_0 */
    j_parm0 = (struct json_object *)json_object_object_get(j_args, "ROIBB_Rect");
    IPSLOG(0, "[__%s__] : ROI_Type = %s\n", __func__, json_object_get_string(j_parm0));
    {

        /* paring PARAMETER */
        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_X");
        IPSLOG(0, "[__%s__] : Center_X = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.cX = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Center_Y");
        IPSLOG(0, "[__%s__] : Center_Y = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.cY = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Angle");
        IPSLOG(0, "[__%s__] : Angle = %5.3f\n", __func__, json_object_get_double(j_param));
        pSE->roiRect.dbAngle = json_object_get_double(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Top");
        IPSLOG(0, "[__%s__] : Top = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.top = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Left");
        IPSLOG(0, "[__%s__] : Left = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.left = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Bottom");
        IPSLOG(0, "[__%s__] : Bottom = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.bottom = json_object_get_int(j_param);

        j_param = (struct json_object *)json_object_object_get(j_parm0, "Right");
        IPSLOG(0, "[__%s__] : Right = %d\n", __func__, json_object_get_int(j_param));
        pSE->roiRect.rectBox.right = json_object_get_int(j_param);

        pSE->roiRect.rectBox.width = abs(pSE->roiRect.rectBox.right - pSE->roiRect.rectBox.left);
        pSE->roiRect.rectBox.height = abs(pSE->roiRect.rectBox.bottom - pSE->roiRect.rectBox.top);
    }

    j_param = (struct json_object *)json_object_object_get(j_args, "StepSize");
    IPSLOG(0, "[__%s__] : StepSize = %d\n", __func__, json_object_get_int(j_param));
    pSE->stepSize = json_object_get_int(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "mm_per_pixel");
    IPSLOG(0, "[__%s__] : mm_per_pixel = %5.2f\n", __func__, json_object_get_double(j_param));
    pSE->dbmm_per_pixel = json_object_get_double(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "InputImgPath");
    IPSLOG(0, "[__%s__] : InputImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strInputImgPath = json_object_get_string(j_param);

    j_param = (struct json_object *)json_object_object_get(j_args, "ResultImgPath");
    IPSLOG(0, "[__%s__] : ResultImgPath = %s\n", __func__, json_object_get_string(j_param));
    pSE->strResultImgPath = json_object_get_string(j_param);

    IPSLOG(0, "[__%s__] : <=== \n", __func__);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
// Json text information by the MQTT subscribe
////////////////////////////////////////////////////////////////////////////////////
static const int gJson_MAXIMUM_SIZE = 4096;

// Vision Box Mode
static char szJson_AutoRunningMode[gJson_MAXIMUM_SIZE];

static char szJson_TriggerModeType[gJson_MAXIMUM_SIZE];

// --> Gige camera
static char szJson_GigeCamInit[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamInq[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamCfg[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamParam[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamRelease[gJson_MAXIMUM_SIZE];

// --> Gige camera streaming
static char szJson_GigeCamStrmInit[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamStrmInq[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamStrmStart[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamStrmCapture[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamStrmStop[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamStrmRelease[gJson_MAXIMUM_SIZE];

// IPL algorithm
static char szJson_Image_Calibration[gJson_MAXIMUM_SIZE];

static char szJson_Crop_GTemplate[gJson_MAXIMUM_SIZE];

static char szJson_PatternMatch[gJson_MAXIMUM_SIZE];
static char szJson_FindProfile[gJson_MAXIMUM_SIZE];
static char szJson_DetectCircle[gJson_MAXIMUM_SIZE];

static char szJson_IBox_Annulus[gJson_MAXIMUM_SIZE];
static char szJson_IBox_Rect[gJson_MAXIMUM_SIZE];
static char szJson_IBox_Circle[gJson_MAXIMUM_SIZE];

static char szJson_CalcCoord[gJson_MAXIMUM_SIZE];

static char szJson_Crop_Annulus[gJson_MAXIMUM_SIZE];
static char szJson_Crop_Rect[gJson_MAXIMUM_SIZE];
static char szJson_Crop_Circle[gJson_MAXIMUM_SIZE];

static char szJson_Hisg_Annulus[gJson_MAXIMUM_SIZE];
static char szJson_Hisg_Rect[gJson_MAXIMUM_SIZE];
static char szJson_Hisg_Circle[gJson_MAXIMUM_SIZE];

static char szJson_Threshold[gJson_MAXIMUM_SIZE];

static char szJson_Morphology[gJson_MAXIMUM_SIZE];

static char szJson_NoiseRemoval[gJson_MAXIMUM_SIZE];

static char szJson_DataAugmentation[gJson_MAXIMUM_SIZE];

static char szJson_MeasGW_Annulus[gJson_MAXIMUM_SIZE];
static char szJson_MeasGW_Rect[gJson_MAXIMUM_SIZE];


////////////////////////////////////////////////////////////////////////////////////
// Parameter structure register
////////////////////////////////////////////////////////////////////////////////////
seAlgoParamReg gAlgoParamReg[] = {

    // VisonBox Mode
    {FLAGE_AUTO_RUNNING,
     enum_Subscribe_CAMReg[FLAGE_AUTO_RUNNING], &gArg_Mode_AutoRunning, szJson_AutoRunningMode, MqttParse_AutoRunningMode},

    // Hardware trigger
    {FLAGE_TRIGGERMODETYPE,
     enum_Subscribe_CAMReg[FLAGE_TRIGGERMODETYPE], &gArg_TriggerModeType, szJson_TriggerModeType, MqttParse_TriggerModeType},

    // Gige camera control.< static image >
    {METHOD_GigeCam_Initialize,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Initialize], &gArg_GigeCam_Initialize, szJson_GigeCamInit, MqttParse_GigECameraInit},
    {METHOD_GigeCam_Inquiry,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Inquiry], &gArg_GigeCam_Inquiry, szJson_GigeCamInq, MqttParse_GigECameraInq},
    {METHOD_GigeCam_Config,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Config], &gArg_GigeCam_Config, szJson_GigeCamCfg, MqttParse_GigECameraCfg},
    {METHOD_GigeCam_Capture,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Capture], &gArg_GigeCam_Parameter, szJson_GigeCamParam, MqttParse_GigECameraParam},
    {METHOD_GigeCam_Release,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Release], &gArg_GigeCam_Release, szJson_GigeCamRelease, MqttParse_GigECameraRelease},

    // Gige camera control.< streaming >
    {METHOD_GigeCam_Streaming_Initialize,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Initialize], &gArg_GigeCamStrm_Initialize, szJson_GigeCamStrmInit, MqttParse_GigECameraStrmInit},
    {METHOD_GigeCam_Streaming_Inquiry,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Inquiry], &gArg_GigeCamStrm_Inquiry, szJson_GigeCamStrmInq, MqttParse_GigECameraStrmInq},
    {METHOD_GigeCam_Streaming_Start,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Start], &gArg_GigeCamStrm_Start, szJson_GigeCamStrmStart, MqttParse_GigECameraStrmStart},
    {METHOD_GigeCam_Streaming_Capture,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Capture], &gArg_GigeCamStrm_Capture, szJson_GigeCamStrmCapture, MqttParse_GigECameraStrmCapture},
    {METHOD_GigeCam_Streaming_Stop,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Stop], &gArg_GigeCamStrm_Stop, szJson_GigeCamStrmStop, MqttParse_GigECameraStrmStop},
    {METHOD_GigeCam_Streaming_Release,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Release], &gArg_GigeCamStrm_Release, szJson_GigeCamStrmRelease, MqttParse_GigECameraStrmRelease},

    // IPL algorithm control.
    {ALGO_ImageCalibration,
     enum_Subscribe_CAMReg[ALGO_ImageCalibration], &gArg_Image_Calibration, szJson_Image_Calibration, MqttParse_IMG_Calibration},

    {ALGO_Crop_GoldenTemplate,
     enum_Subscribe_CAMReg[ALGO_Crop_GoldenTemplate], &gArg_Crop_GTemplate, szJson_Crop_GTemplate, MqttParse_CropROI_GTemplate},

    {ALGO_PatternMatch,
     enum_Subscribe_CAMReg[ALGO_PatternMatch], &gArg_PatternMatch, szJson_PatternMatch, MqttParse_ALGO_PatternMatch},
    {ALGO_FindProfile,
     enum_Subscribe_CAMReg[ALGO_FindProfile], &gArg_FindProfile, szJson_FindProfile, MqttParse_ALGO_FindProfile},
    {ALGO_DetectCircle,
     enum_Subscribe_CAMReg[ALGO_DetectCircle], &gArg_DetectCircle, szJson_DetectCircle, MqttParse_ALGO_DetectCircle},

    {ALGO_IBOX_Annulus,
     enum_Subscribe_CAMReg[ALGO_IBOX_Annulus], &gArg_IBox_Annulus, szJson_IBox_Annulus, MqttParse_ALGO_IBOX_Annulus},
    {ALGO_IBOX_Rect,
     enum_Subscribe_CAMReg[ALGO_IBOX_Rect], &gArg_IBox_Rect, szJson_IBox_Rect, MqttParse_ALGO_IBOX_Rect},
    {ALGO_IBOX_Circle,
     enum_Subscribe_CAMReg[ALGO_IBOX_Circle], &gArg_IBox_Circle, szJson_IBox_Circle, MqttParse_ALGO_IBOX_Circle},

    {ALGO_CalcCoord,
     enum_Subscribe_CAMReg[ALGO_CalcCoord], &gArg_CalcCoord, szJson_CalcCoord, MqttParse_ALGO_CalcCoord},

    {ALGO_Crop_Annulus,
     enum_Subscribe_CAMReg[ALGO_Crop_Annulus], &gArg_Crop_Annulus, szJson_Crop_Annulus, MqttParse_ALGO_Crop_Annulus},
    {ALGO_Crop_Rect,
     enum_Subscribe_CAMReg[ALGO_Crop_Rect], &gArg_Crop_Rect, szJson_Crop_Rect, MqttParse_ALGO_Crop_Rect},
    {ALGO_Crop_Circle,
     enum_Subscribe_CAMReg[ALGO_Crop_Circle], &gArg_Crop_Circle, szJson_Crop_Circle, MqttParse_ALGO_Crop_Circle},

    {ALGO_Hisg_Annulus,
     enum_Subscribe_CAMReg[ALGO_Hisg_Annulus], &gArg_Hisg_Annulus, szJson_Hisg_Annulus, MqttParse_ALGO_Hisg_Annulus},
    {ALGO_Hisg_Rect,
     enum_Subscribe_CAMReg[ALGO_Hisg_Rect], &gArg_Hisg_Rect, szJson_Hisg_Rect, MqttParse_ALGO_Hisg_Rect},
    {ALGO_Hisg_Circle,
     enum_Subscribe_CAMReg[ALGO_Hisg_Circle], &gArg_Hisg_Circle, szJson_Hisg_Circle, MqttParse_ALGO_Hisg_Circle},

    {ALGO_IP_Threshold,
     enum_Subscribe_CAMReg[ALGO_IP_Threshold], &gArg_Threshold, szJson_Threshold, MqttParse_ALGO_IP_Threshold},
    {ALGO_IP_Morphology,
     enum_Subscribe_CAMReg[ALGO_IP_Morphology], &gArg_Morphology, szJson_Morphology, MqttParse_ALGO_IP_Morphology},
    {ALGO_IP_NoiseRemoval,
     enum_Subscribe_CAMReg[ALGO_IP_NoiseRemoval], &gArg_NoiseRemoval, szJson_NoiseRemoval, MqttParse_ALGO_IP_NoiseRemoval},

    {ALGO_IP_DataAugmentation,
     enum_Subscribe_CAMReg[ALGO_IP_DataAugmentation], &gArg_DataAugmentation, szJson_DataAugmentation, MqttParse_ALGO_IP_DataAugmentation},

    {ALGO_MeasGW_Annulus,
     enum_Subscribe_CAMReg[ALGO_MeasGW_Annulus], &gArg_MeasGW_Annulus, szJson_MeasGW_Annulus, MqttParse_ALGO_MeasGW_Annulus},
    {ALGO_MeasGW_Rect,
     enum_Subscribe_CAMReg[ALGO_MeasGW_Rect], &gArg_MeasGW_Rect, szJson_MeasGW_Rect, MqttParse_ALGO_MeasGW_Rect},

    // The End of AlgoParam register
    {ENUM_ALGO_END, enum_Subscribe_CAMReg[ENUM_ALGO_END], nullptr, nullptr, nullptr}

};

////////////////////////////////////////////////////////////////////////////////////
// Parameter Tasks assign function
////////////////////////////////////////////////////////////////////////////////////

static std::unordered_map<std::string, int> gHashMap_Param;

const int AlgoParam_TblCnt = (sizeof(gAlgoParamReg) / sizeof(gAlgoParamReg[0]));

/***********************************************************
 *	Function 	: createHashMap_Param
 *	Description : Create Hash Map for Parameter Register Table
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int createHashMap_Param()
{

    if (!gHashMap_Param.empty())
    {

        IPSLOG(0, " ## Param__The gHashMap_Param is already create.\n");
        return 0;
    }

    for (int i = 0; i < AlgoParam_TblCnt; i++)
    {

        std::string strTmp = gAlgoParamReg[i].strCmd;

        // IPSLOG(0, " ## gHashMap_Param[ %s ] = %d\n", strTmp.c_str(), i);

        gHashMap_Param[strTmp] = i;
    }

    return 0;
}

/***********************************************************
 *	Function 	: createHashMap_Param
 *	Description : Compare Hash Map for Parameter Register Table
 *	Param 		: std::string strKey
 *	Return		: Parameter ID
 *************************************************************/
int compareHashMap_Param(std::string strKey)
{
    auto it = gHashMap_Param.find(strKey);

    if (it != gHashMap_Param.end())
    {

        IPSLOG(0, " ## Param__Element: %s, has ID: %d\n", strKey.c_str(), it->second);
        printf(" ## Param__Element: %s, has ID: %d\n", strKey.c_str(), it->second);
        return it->second;
    }
    else
    {

        IPSLOG(0, " ## Param__Element: %s, not found in the map.\n", strKey.c_str());
        printf(" ## Param__Element: %s, not found in the map.\n", strKey.c_str());
        return -1;
    }
}

seExpansionMode iEnbExMode;
/***********************************************************
 *	Function 	: setAlgo_ParamAssign
 *	Description : Lookup the corresponding parameter settings 
 *                based on the given key, parse and set 
 *                the algorithm parameters, and apply these settings 
 *                to the relevant queues and methods.
 *	Param 		: 
 *	Return		: Parameter ID
 *************************************************************/
int setAlgo_ParamAssign(const char *szKey, struct json_object *j_subsystem, seJsonInfo *pInfo)
{
    IPSLOG(0, "\n\n[__%s__] : ===> \n", __func__);

    int ret = 0;

    seAlgoParamReg *pAlgoParam = nullptr;

    // for (int i = 0; i < AlgoParam_TblCnt; i++)
    {

        int res = 0;

        int i = compareHashMap_Param(szKey);
        IPSLOG(0, "%s()%d: ## ---------------> compareHashMap_Param = %d \n\n", __FUNCTION__, __LINE__, i);
        printf("%s()%d: ## ---------------> compareHashMap_Param = %d \n\n", __FUNCTION__, __LINE__, i);

        if (-1 != i)
        {
            pAlgoParam = &gAlgoParamReg[i];
            res = pAlgoParam->MqttParsesFunc(pAlgoParam->strCmd, j_subsystem, pAlgoParam->pParam, pAlgoParam->szJsonInfo);

            if (!strcmp(pAlgoParam->strCmd, enum_Subscribe_CAMReg[FLAGE_AUTO_RUNNING]))
            {

                seMode_AutoRunning *pParamSet = (seMode_AutoRunning *)pAlgoParam->pParam;

                iEnbExMode.flg_AutoRunning = pParamSet->bFlg_AutoRunning;
                iEnbExMode.flg_Enb_TriggerMode = pParamSet->bFlg_Enable_TriggerMode;
            }
            if (!strcmp(pAlgoParam->strCmd, enum_Subscribe_CAMReg[FLAGE_TRIGGERMODETYPE]))
            {

                seMode_TriggerModeType *pParamSet = (seMode_TriggerModeType *)pAlgoParam->pParam;

                iEnbExMode.flg_TriggerMode_Activat = pParamSet->bFlg_TriggerMode_Activate;
            }

            ExModeQ_EnQ(iEnbExMode);

            setAlgo_MethodAssign(pAlgoParam->strCmd, pAlgoParam->szJsonInfo);

            if (res == 0)
            {

                pInfo->emAlgoId = pAlgoParam->emAlgoId;

                // Copy the MQTT "Key" ID of Algorithm.
                strcpy(pInfo->szCmd, pAlgoParam->strCmd);

                // Copy the Json information by the MQTT subscribe function.
                strcpy(pInfo->szJsonBuf, pAlgoParam->szJsonInfo);
            }
            else
            {
                ret = res;
            }
        }
    }

    IPSLOG(0, "[__%s__] : <=== \n\n", __func__);

    return ret;
}

/***********************************************************
 *	Function 	: setAlgo_ParamAssign_Dual
 *	Description : Lookup the corresponding parameter settings 
 *                based on the given key, parse and set 
 *                the algorithm parameters, and apply these settings 
 *                to the relevant queues and methods.
 *	Param 		: 
 *	Return		: Parameter ID
 *************************************************************/
int setAlgo_ParamAssign_Dual(const char *szKey, struct json_object *j_subsystem, seJsonInfo *pInfo, const int iID)
{
    IPSLOG(0, "\n\n[__%s__] : ===> \n", __func__);

    int ret = 0;
    int res = 0;    

    seAlgoParamReg *pAlgoParam = nullptr;

    int i = compareHashMap_Param(szKey);
    IPSLOG(0, " ## ---------------> compareHashMap_Param = %d \n\n", i);
    printf(" ## ---------------> compareHashMap_Param = %d \n\n", i);

    if (-1 != i)
    {
        pAlgoParam = &gAlgoParamReg[i];
        res = pAlgoParam->MqttParsesFunc(pAlgoParam->strCmd, j_subsystem, pAlgoParam->pParam, pAlgoParam->szJsonInfo);

        if (!strcmp(pAlgoParam->strCmd, enum_Subscribe_CAMReg[FLAGE_AUTO_RUNNING]))
        {
            seMode_AutoRunning *pParamSet = (seMode_AutoRunning *)pAlgoParam->pParam;

            iEnbExMode.flg_AutoRunning = pParamSet->bFlg_AutoRunning;
            iEnbExMode.flg_Enb_TriggerMode = pParamSet->bFlg_Enable_TriggerMode;
        }
        if (!strcmp(pAlgoParam->strCmd, enum_Subscribe_CAMReg[FLAGE_TRIGGERMODETYPE]))
        {
            seMode_TriggerModeType *pParamSet = (seMode_TriggerModeType *)pAlgoParam->pParam;

            iEnbExMode.flg_TriggerMode_Activat = pParamSet->bFlg_TriggerMode_Activate;
        }

        ExModeQ_EnQ_Dual(iEnbExMode, iID);

        setAlgo_MethodAssign_Dual(pAlgoParam->strCmd, pAlgoParam->szJsonInfo, iID);

        if (res == 0)
        {
            pInfo->emAlgoId = pAlgoParam->emAlgoId;

            // Copy the MQTT "Key" ID of Algorithm.
            strcpy(pInfo->szCmd, pAlgoParam->strCmd);
            // Copy the Json information by the MQTT subscribe function.
            strcpy(pInfo->szJsonBuf, pAlgoParam->szJsonInfo);
        }
        else
        {
            ret = res;
        }
    }    

    IPSLOG(0, "[__%s__] : <=== \n\n", __func__);

    return ret;
}