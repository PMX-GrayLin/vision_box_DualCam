/**
 ******************************************************************************
 * @file    IPS_CompAlgorithm.c
 * @brief   Defines and declares relevant algorithms and camera control functions 
 *          for use in an Image Processing System (IPS).
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
#include <time.h>
#include "common.hpp"
#include "ext_mqtt_client.hpp"

#include <list>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include "IPS_CompFunction.h"
#include "IPS_CompAlgorithm.h"
#include "IPLAlgoDataStructureDef.h"

using namespace std;

/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */
void *pter_hdl_GigE = nullptr;
void *pter_hdl_GigE_Dual[2] = {nullptr, nullptr};
void *pter_hdl_IPL = nullptr;

////////////////////////////////////////////////////////////////////////////////////
// deque of Algorithm Tasks information.
////////////////////////////////////////////////////////////////////////////////////

pthread_mutex_t _TasksQ_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t _ExModeQ_lock = PTHREAD_MUTEX_INITIALIZER;

std::deque<CAlgoMethodParametr> deqMthParmTasksRespo;
std::deque<seExpansionMode> deqFlagExMode;


/* EXPORTED SUBPROGRAM BODIES -------------------------------------------------------- */
/***********************************************************
 *	Function 	: TasksQ_Init
 *	Description : To initiate _TasksQ_lock
 *	Param 		:
 *	Return		: 0, -1
 *************************************************************/
int TasksQ_Init()
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_TasksQ_lock, nullptr);

    return res;
}

/***********************************************************
 *	Function 	: TasksQ_EnQ
 *	Description : To push back the queue: deqMthParmTasksRespo
 *	Param 		: CAlgoMethodParametr seMthdInfo
 *	Return		: error number
 *************************************************************/
int TasksQ_EnQ(CAlgoMethodParametr seMthdInfo)
{
    int res = 0;
    pthread_mutex_lock(&_TasksQ_lock);

    deqMthParmTasksRespo.push_back(seMthdInfo);

    pthread_mutex_unlock(&_TasksQ_lock);
    return res;
}

/***********************************************************
 *	Function 	: TasksQ_DeQ
 *	Description : To pop front of the queue: deqMthParmTasksRespo
 *	Param 		: return message: CAlgoMethodParametr *pMthdInfo
 *	Return		: error number
 *************************************************************/
int TasksQ_DeQ(CAlgoMethodParametr *pMthdInfo)
{
    int res = 0;
    pthread_mutex_lock(&_TasksQ_lock);

    if (!deqMthParmTasksRespo.empty())
    {

        *pMthdInfo = *deqMthParmTasksRespo.begin(); // [0] ;
        deqMthParmTasksRespo.pop_front();
    }
    else
    {
        res = -1;
    }

    pthread_mutex_unlock(&_TasksQ_lock);
    return res;
}

/***********************************************************
 *	Function 	: TasksQ_GetSzie
 *	Description : To get queue: deqMthParmTasksRespo size
 *	Param 		: NONE
 *	Return		: deqMthParmTasksRespo size
 *************************************************************/
int TasksQ_GetSzie()
{
    int res = 0;
    pthread_mutex_lock(&_TasksQ_lock);
    res = deqMthParmTasksRespo.size();
    pthread_mutex_unlock(&_TasksQ_lock);
    return res;
}

/***********************************************************
 *	Function 	: TasksQ_IsEmpty
 *	Description : To check deqMthParmTasksRespo is empty or not
 *	Param 		: NONE
 *	Return		: true, false
 *************************************************************/
int TasksQ_IsEmpty()
{
    int res = 0;
    pthread_mutex_lock(&_TasksQ_lock);
    res = deqMthParmTasksRespo.empty();
    pthread_mutex_unlock(&_TasksQ_lock);
    return res;
}

/***********************************************************
 *	Function 	: TasksQ_Destory
 *	Description : To release the lock: _TasksQ_lock
 *	Param 		: NONE
 *	Return		: NONE
 *************************************************************/
void TasksQ_Destory()
{
    pthread_mutex_destroy(&_TasksQ_lock);
}

/***********************************************************
 *	Function 	: ExModeQ_Init
 *	Description : To initiate _ExModeQ_lock
 *	Param 		: NONE
 *	Return		: error number
 *************************************************************/
int ExModeQ_Init()
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_ExModeQ_lock, nullptr);

    return res;
}

/***********************************************************
 *	Function 	: ExModeQ_EnQ
 *	Description : To push back the queue: deqFlagExMode
 *	Param 		: seExpansionMode iEnbExMode
 *	Return		: error number
 *************************************************************/
int ExModeQ_EnQ(seExpansionMode iEnbExMode)
{
    int res = 0;
    pthread_mutex_lock(&_ExModeQ_lock);

    deqFlagExMode.push_back(iEnbExMode);

    pthread_mutex_unlock(&_ExModeQ_lock);
    return res;
}

/***********************************************************
 *	Function 	: ExModeQ_DeQ
 *	Description : To pop front of the queue: deqFlagExMode
 *	Param 		: return message: seExpansionMode *pEnbExMode
 *	Return		: error number
 *************************************************************/
int ExModeQ_DeQ(seExpansionMode *pEnbExMode)
{
    int res = 0;
    pthread_mutex_lock(&_ExModeQ_lock);

    if (!deqFlagExMode.empty())
    {

        *pEnbExMode = *deqFlagExMode.begin();
        deqFlagExMode.pop_front();
    }
    else
    {
        res = -1;
    }

    pthread_mutex_unlock(&_ExModeQ_lock);
    return res;
}

/***********************************************************
 *	Function 	: ExModeQ_GetSzie
 *	Description : To get queue: deqFlagExMode size
 *	Param 		: NONE
 *	Return		: deqFlagExMode size
 *************************************************************/
int ExModeQ_GetSzie()
{
    int res = 0;
    pthread_mutex_lock(&_ExModeQ_lock);
    res = deqFlagExMode.size();
    pthread_mutex_unlock(&_ExModeQ_lock);
    return res;
}

/***********************************************************
 *	Function 	: ExModeQ_Destory
 *	Description : To release the lock: _ExModeQ_lock
 *	Param 		: NONE
 *	Return		: NONE
 *************************************************************/
void ExModeQ_Destory()
{
    pthread_mutex_destroy(&_ExModeQ_lock);
}


////////////////////////////////////////////////////////////////////////////////////
// Dual deque of Algorithm Tasks information.
////////////////////////////////////////////////////////////////////////////////////

pthread_mutex_t _TasksQ_lock_Dual[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t _ExModeQ_lock_Dual[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

std::vector<std::deque<CAlgoMethodParametr>> deqMthParmTasksRespo_Dual(2);
std::vector<std::deque<seExpansionMode>> deqFlagExMode_Dual(2);

/***********************************************************
 *	Function 	: TasksQ_Init_Dual
 *	Description : To initiate _TasksQ_lock_Dual
 *	Param 		: int iID
 *	Return		: 0, -1
 *************************************************************/
int TasksQ_Init_Dual(const int iID)
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_TasksQ_lock_Dual[iID], nullptr);

    return res;
}

/***********************************************************
 *	Function 	: TasksQ_EnQ_Dual
 *	Description : To push back the queue: deqMthParmTasksRespo_Dual
 *	Param 		: CAlgoMethodParametr seMthdInfo
 *                int iID
 *	Return		: error number
 *************************************************************/
int TasksQ_EnQ_Dual(CAlgoMethodParametr seMthdInfo, const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_TasksQ_lock_Dual[iID]);

    deqMthParmTasksRespo_Dual[iID].push_back(seMthdInfo);

    pthread_mutex_unlock(&_TasksQ_lock_Dual[iID]);
    return res;
}

/***********************************************************
 *	Function 	: TasksQ_DeQ_Dual
 *	Description : To pop front of the queue: deqMthParmTasksRespo_Dual
 *	Param 		: return message: CAlgoMethodParametr *pMthdInfo
 *                int iID
 *	Return		: error number
 *************************************************************/
int TasksQ_DeQ_Dual(CAlgoMethodParametr *pMthdInfo, const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_TasksQ_lock_Dual[iID]);

    if (!deqMthParmTasksRespo_Dual[iID].empty())
    {

        *pMthdInfo = *deqMthParmTasksRespo_Dual[iID].begin(); // [0] ;
        deqMthParmTasksRespo_Dual[iID].pop_front();
    }
    else
    {
        res = -1;
    }

    pthread_mutex_unlock(&_TasksQ_lock_Dual[iID]);
    return res;
}

/***********************************************************
 *	Function 	: TasksQ_GetSzie_Dual
 *	Description : To get queue: deqMthParmTasksRespo_Dual size
 *	Param 		: int iID
 *	Return		: deqMthParmTasksRespo_Dual size
 *************************************************************/
int TasksQ_GetSzie_Dual(const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_TasksQ_lock_Dual[iID]);
    res = deqMthParmTasksRespo_Dual[iID].size();
    pthread_mutex_unlock(&_TasksQ_lock_Dual[iID]);
    return res;
}

/***********************************************************
 *	Function 	: TasksQ_IsEmpty_Dual
 *	Description : To check deqMthParmTasksRespo_Dual is empty or not
 *	Param 		: int iID
 *	Return		: true, false
 *************************************************************/
int TasksQ_IsEmpty_Dual(const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_TasksQ_lock_Dual[iID]);
    res = deqMthParmTasksRespo_Dual[iID].empty();
    pthread_mutex_unlock(&_TasksQ_lock_Dual[iID]);
    return res;
}

/***********************************************************
 *	Function 	: TasksQ_Destory_Dual
 *	Description : To release the lock: _TasksQ_lock_Dual
 *	Param 		: int iID
 *	Return		: NONE
 *************************************************************/
void TasksQ_Destory_Dual(const int iID)
{
    pthread_mutex_destroy(&_TasksQ_lock_Dual[iID]);
}

/***********************************************************
 *	Function 	: ExModeQ_Init_Dual
 *	Description : To initiate _ExModeQ_lock_Dual
 *	Param 		: int iID
 *	Return		: error number
 *************************************************************/
int ExModeQ_Init_Dual(const int iID)
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_ExModeQ_lock_Dual[iID], nullptr);

    return res;
}

/***********************************************************
 *	Function 	: ExModeQ_EnQ_Dual
 *	Description : To push back the queue: deqFlagExMode_Dual
 *	Param 		: seExpansionMode iEnbExMode
 *                int iID
 *	Return		: error number
 *************************************************************/
int ExModeQ_EnQ_Dual(seExpansionMode iEnbExMode, const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_ExModeQ_lock_Dual[iID]);

    deqFlagExMode_Dual[iID].push_back(iEnbExMode);

    pthread_mutex_unlock(&_ExModeQ_lock_Dual[iID]);
    return res;
}

/***********************************************************
 *	Function 	: ExModeQ_DeQ_Dual
 *	Description : To pop front of the queue: deqFlagExMode_Dual
 *	Param 		: return message: seExpansionMode *pEnbExMode
 *                int iID
 *	Return		: error number
 *************************************************************/
int ExModeQ_DeQ_Dual(seExpansionMode *pEnbExMode, const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_ExModeQ_lock_Dual[iID]);

    if (!deqFlagExMode_Dual[iID].empty())
    {
        *pEnbExMode = *deqFlagExMode_Dual[iID].begin();
        deqFlagExMode_Dual[iID].pop_front();
    }
    else
    {
        res = -1;
    }

    pthread_mutex_unlock(&_ExModeQ_lock_Dual[iID]);
    return res;
}

/***********************************************************
 *	Function 	: ExModeQ_GetSzie_Dual
 *	Description : To get queue: deqFlagExMode_Dual size
 *	Param 		: NONE
 *	Return		: deqFlagExMode_Dual size
 *************************************************************/
int ExModeQ_GetSzie_Dual(const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_ExModeQ_lock_Dual[iID]);
    res = deqFlagExMode_Dual[iID].size();
    pthread_mutex_unlock(&_ExModeQ_lock_Dual[iID]);
    return res;
}

void ExModeQ_Destory_Dual(const int iID)
{
    pthread_mutex_destroy(&_ExModeQ_lock_Dual[iID]);
}


////////////////////////////////////////////////////////////////////////////////////
// IPL handle pointer and function pointer define.
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: ipsComp_IPL_Init
 *	Description : Create IPL handle pointer
 *	Param 		: NONE
 *	Return		: error number
 *************************************************************/
int ipsComp_IPL_Init()
{
    int res = 0;
return 0;
    if (pter_hdl_IPL == nullptr)
    {
        pter_hdl_IPL = static_cast<void *>(CreateObject_Labview());
        IPSLOG(1, " ## > pter_hdl_IPL Handle Address( 0x%llx )\n", pter_hdl_IPL);

        if (pter_hdl_IPL == nullptr)
        {
            MAINLOG(0, " ## > Error!! pter_hdl_IPL = static_cast<void*>(CreateObject_Labview())");
            return res;
        }
    }

    return 0;
}

/***********************************************************
 *	Function 	: ipsComp_IPL_Release
 *	Description : Release IPL handle pointer
 *	Param 		: NONE
 *	Return		: error number
 *************************************************************/
int ipsComp_IPL_Release()
{
    int res = 0;

    if (pter_hdl_IPL)
    {
        IPSLOG(1, " ## > Destory the Handle of pter_hdl_IPL( 0x%llx )\n", pter_hdl_IPL);
        DestoryObject_Labview(reinterpret_cast<void**>(&pter_hdl_IPL));

    }

    return res;
}


////////////////////////////////////////////////////////////////////////////////////
// Single camera GigE handle pointer and function pointer define.
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: ipsComp_Camera_Init
 *	Description : Create Single camera GigE handle pointer
 *                and initiate GigE camera
 *	Param 		: NONE
 *	Return		: error number
 *************************************************************/
int ipsComp_Camera_Init()
{
    int res = 0;

    if (pter_hdl_GigE == nullptr)
    {
        pter_hdl_GigE = static_cast<void *>(CreateObject_GigECam());
        IPSLOG(1, " ## > pter_hdl_GigE Handle Address( 0x%llx )\n", pter_hdl_GigE);

        if (pter_hdl_GigE == nullptr)
        {
            MAINLOG(0, " ## > Error!! pter_hdl_GigE = static_cast<void*>(CreateObject_GigECam())");
            return -1;
        }
    }

    res = static_cast<GigECam_I_Library *>(pter_hdl_GigE)->Init();
    if (res)
    {
        IPSLOG(1, " ## > Error!!! static_cast<GigECam_I_Library *>(pter_hdl_GigE)->Init()");
        ipsComp_Camera_Release();
        return -1;
    }

    return 0;
}

/***********************************************************
 *	Function 	: ipsComp_Camera_Release
 *	Description : Release Single camera GigE handle pointer
 *	Param 		: NONE
 *	Return		: error number
 *************************************************************/
int ipsComp_Camera_Release()
{
    int res = 0;

    if (pter_hdl_GigE)
    {

        res = static_cast<GigECam_I_Library *>(pter_hdl_GigE)->Release();
        IPSLOG(1, "[IPS_RexTW] : pter_hdl_GigE Handle Address( 0x%llx )\n", pter_hdl_GigE);
        if (res)
        {
            MAINLOG(0, " Error!!! Release()\n");
            pter_hdl_GigE = nullptr;            
            return -1;
        }

        pter_hdl_GigE = nullptr;
    }

    return res;
}


////////////////////////////////////////////////////////////////////////////////////
// Dual camera GigE handle pointer and function pointer define.
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: ipsComp_Camera_Init_Dual
 *	Description : Create Single camera GigE handle pointer
 *                and initiate GigE camera
 *	Param 		: int iID
 *	Return		: error number
 *************************************************************/
int ipsComp_Camera_Init_Dual(const int iID)
{
    int res = 0;

    if (pter_hdl_GigE_Dual[iID] == nullptr)
    {
        pter_hdl_GigE_Dual[iID] = static_cast<void *>(CreateObject_GigECam());
        IPSLOG(1, " ## > pter_hdl_GigE_Dual[%d] Handle Address( 0x%llx )\n", iID, pter_hdl_GigE_Dual[iID]);

        if (pter_hdl_GigE_Dual[iID] == nullptr)
        {
            MAINLOG(0, " ## > Error!! pter_hdl_GigE_Dual[%d] = static_cast<void*>(CreateObject_GigECam())", iID);
            return -1;
        }
    }

    res = static_cast<GigECam_I_Library *>(pter_hdl_GigE_Dual[iID])->Init();
    if (res)
    {
        IPSLOG(1, " ## > Error!!! static_cast<GigECam_I_Library *>(pter_hdl_GigE_Dual[%d])->Init()", iID);
        ipsComp_Camera_Release_Dual(iID);
        return -1;
    }

    return 0;
}

/***********************************************************
 *	Function 	: ipsComp_Camera_Release_Dual
 *	Description : Release Single camera GigE handle pointer
 *	Param 		: int iID
 *	Return		: error number
 *************************************************************/
int ipsComp_Camera_Release_Dual(const int iID)
{
    int res = 0;

    if (pter_hdl_GigE_Dual[iID])
    {

        res = static_cast<GigECam_I_Library *>(pter_hdl_GigE_Dual[iID])->Release();
        IPSLOG(1, "[IPS_RexTW] : pter_hdl_GigE_Dual[%d] Handle Address( 0x%llx )\n", iID, pter_hdl_GigE_Dual[iID]);
        if (res)
        {
            MAINLOG(0, " Error!!! Release()\n");
            pter_hdl_GigE_Dual[iID] = nullptr;
            return -1;
        }

        pter_hdl_GigE_Dual[iID] = nullptr;
    }

    return res;
}




////////////////////////////////////////////////////////////////////////////////////
// Parameter List define.
////////////////////////////////////////////////////////////////////////////////////

// VisonBox mode
static seMode_AutoRunning gParm_AutoRunning;
static seMode_AutoRunning_Ret gResult_AutoRunning;

static seMode_TriggerModeType gParm_TriggerModeType;
static seMode_TriggerModeType_Ret gResult_TriggerModeType;

// Gige camera control.
static seGigECamConfig gInitialize_GigeCam;
static seGigECamConfig_Ret gInitialize_GigeCam_Ret;

static seGigECamConfig gInquiry_GigeCam;
static seGigECamConfig_Ret gInquiry_GigeCam_Ret;

static seGigECamConfig gConfig_GigeCam;
static seGigECamConfig_Ret gConfig_GigeCam_Ret;

static seGigECamCapture gCapture_GigeCam;
static seGigECamCapture_Ret gCapture_GigeCam_Ret;

static seGigECamCapture gRelease_GigeCam;
static seGigECamCapture_Ret gRelease_GigeCam_Ret;

// Gige camera control.
static seGigECamConfig gInitialize_GigeCamStrm;
static seGigECamConfig_Ret gInitialize_GigeCamStrm_Ret;

static seGigECamConfig gInquiry_GigeCamStrm;
static seGigECamConfig_Ret gInquiry_GigeCamStrm_Ret;

static seGigECamConfig gStart_GigeCamStrm;
static seGigECamConfig_Ret gStart_GigeCamStrm_Ret;

static seGigECamConfig gCapture_GigeCamStrm;
static seGigECamConfig_Ret gCapture_GigeCamStrm_Ret;

static seGigECamCapture gStop_GigeCamStrm;
static seGigECamCapture_Ret gStop_GigeCamStrm_Ret;

static seGigECamCapture gRelease_GigeCamStrm;
static seGigECamCapture_Ret gRelease_GigeCamStrm_Ret;

// IPL algorithm control.
static seMth_ImageCalibration gParm_Image_Calibration;
static seMth_ImageCalibration_Ret gResult_Image_Calibration;

static seCropROI_GTemplate gParm_Crop_GTemplate;
static seCropROI_GTemplate_Ret gParm_Crop_GTemplate_Ret;

static seMth_PatternMatch gParm_PatternMatch;       // Parameter
static seMth_PatternMatch_Ret gResult_PatternMatch; // Testing Results.

static seMth_FindProfile gParm_FindProfile;
static seMth_FindProfile_Ret gResult_FindProfile;

static seMth_DetectCircle gParm_DetectCircle;
static seMth_DetectCircle_Ret gResult_DetectCircle;

static seIBox_Annulus gParm_IBox_Annulus;
static seIBox_Annulus_Ret gResult_IBox_Annulus;

static seIBox_Rect gParm_IBox_Rect;
static seIBox_Rect_Ret gResult_IBox_Rect;

static seIBox_Circle gParm_IBox_Circle;
static seIBox_Circle_Ret gResult_IBox_Circle;

static seCalcCoord gParm_CalcCoord;
static seCalcCoord_Ret gResult_CalcCoord;

static seCropROI_Annulus gParm_Crop_Annulus;
static seCropROI_Annulus_Ret gResult_Crop_Annulus;

static seCropROI_Rect gParm_Crop_Rect;
static seCropROI_Rect_Ret gResult_Crop_Rect;

static seCropROI_Circle gParm_Crop_Circle;
static seCropROI_Circle_Ret gResult_Crop_Circle;

static seHisg_Annulus gParm_Hisg_Annulus;
static seHisg_Annulus_Ret gResult_Hisg_Annulus;

static seHisg_Rect gParm_Hisg_Rect;
static seHisg_Rect_Ret gResult_Hisg_Rect;

static seHisg_Circle gParm_Hisg_Circle;
static seHisg_Circle_Ret gResult_Hisg_Circle;

static seIP_Threshold gParm_Threshold;
static seIP_Threshold_Ret gResult_Threshold;

static seMorphology gParm_Morphology;
static seMorphology_Ret gResult_Morphology;

static seNoiseRemoval gParm_NoiseRemoval;
static seNoiseRemoval_Ret gResult_NoiseRemoval;

static seDataAugmentation gParm_DataAugmentation;
static seDataAugmentation_Ret gResult_DataAugmentation;

static seMeasGW_Annulus gParm_MeasGW_Annulus;
static seMeasGW_Annulus_Ret gResult_MeasGW_Annulus;

static seMeasGW_Rect gParm_MeasGW_Rect;
static seMeasGW_Rect_Ret gResult_MeasGW_Rect;

////////////////////////////////////////////////////////////////////////////////////
// Json text information by the MQTT subscribe
////////////////////////////////////////////////////////////////////////////////////
static const int gJson_MAXIMUM_SIZE = 4096;

// Vision Box Mode
static char szJson_AutoRunningMode[gJson_MAXIMUM_SIZE];

static char szJson_TriggerModeType[gJson_MAXIMUM_SIZE];

// --> Gige camera control. < static image >
static char szJson_GigeCamInit[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamInq[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamCfg[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamParam[gJson_MAXIMUM_SIZE];
static char szJson_GigeCamRelease[gJson_MAXIMUM_SIZE];

// --> Gige camera control. < streaming >
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
// Alagorithm Function List define.
////////////////////////////////////////////////////////////////////////////////////

/***********************************************************
 *	Function 	: Method_AutoRunning_Mode
 *	Description : To set flag: Auto Running(Enable/Disable)
 *                             Trigger Mode(Enable/Disable)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_AutoRunning_Mode(void *phdl, const LPImageInfo pImgInIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seMode_AutoRunning *pParamSet = (seMode_AutoRunning *)pCP->pParm;

    // cycletime_start
    start = clock();

    seMode_AutoRunning_Ret seRet;

    seRet.retState = 0;

    seRet.bRet_AutoRunning = pParamSet->bFlg_AutoRunning;
    seRet.bRet_Enable_TriggerMode = pParamSet->bFlg_Enable_TriggerMode;

    memcpy(pResults, &seRet, sizeof(seMode_AutoRunning_Ret));

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_TriggerMode_Type
 *	Description : To set flag: Trigger Mode Activate(Enable/Disable)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_TriggerMode_Type(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seMode_TriggerModeType *pParamSet = (seMode_TriggerModeType *)pCP->pParm;

    // cycletime_start
    start = clock();

    seMode_TriggerModeType_Ret seRet;

    seRet.retState = 0;

    seRet.bRet_TriggerMode_Activate = pParamSet->bFlg_TriggerMode_Activate;

    memcpy(pResults, &seRet, sizeof(seMode_TriggerModeType_Ret));

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

// Single camera
/***********************************************************
 *	Function 	: Method_GigECameraCtl_Initialize
 *	Description : To initiate IPL(Image Processing Library)
 *                and GigE camera(Single)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraCtl_Initialize(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    ipsComp_IPL_Init();
    ipsComp_Camera_Init();  //Single camera

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamConfig_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig.bIsConnected = 1;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    usleep(500000); /* delay 500  ms */

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

// Dual camera
/***********************************************************
 *	Function 	: Method_GigECameraCtl_Initialize_Dual
 *	Description : To initiate IPL(Image Processing Library)
 *                and GigE camera(Dual)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraCtl_Initialize_Dual(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seGigECamConfig *pGigeCfg = (seGigECamConfig *)pCP->pParm;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    ipsComp_IPL_Init();
    res = ipsComp_Camera_Init_Dual(pGigeCfg->iCamId);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamConfig_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig.bIsConnected = 1;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    usleep(500000); /* delay 500  ms */

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_GigECameraCtl_Inquiry
 *	Description : Query/Retrieve GigE camera configuration settings information
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraCtl_Inquiry(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seGigECamConfig *pGigeCfg = (seGigECamConfig *)pCP->pParm;

    printf(">>> Default information of Image format setting === === >>>\n");
    printf("    default.bIsConnected---> %d\n", pGigeCfg->bIsConnected);
    printf("    default.bIsEnbAcquisitionFrameRate---> %s\n", (pGigeCfg->bIsEnbAcquisitionFrameRate) ? "True" : "False");
    printf("    default.dbAcquisitionFrameRate---> %5.3f\n", pGigeCfg->dbAcquisitionFrameRate);
    printf("    default.bExposureAuto---> %s\n", (!pGigeCfg->bExposureAuto) ? "Auto" : "Off");
    printf("    default.bPixelFormat---> %s\n", (pGigeCfg->bPixelFormat) ? "RGB8" : "Mono8");
    printf("    default.iOffset_X ---> %d\n", pGigeCfg->iOffset_X);
    printf("    default.iOffset_Y---> %d\n", pGigeCfg->iOffset_Y);
    printf("    default.iWidth ---> %d\n", pGigeCfg->iWidth);
    printf("    default.iHeight---> %d\n", pGigeCfg->iHeight);
    printf("    default.iSensor_Width ---> %d\n", pGigeCfg->iSensor_Width);
    printf("    default.iSensor_Height---> %d\n", pGigeCfg->iSensor_Height);
    printf("    default.iBinning_Scale---> %d\n", pGigeCfg->iBinning_Scale);
    printf("<<< Default information of Image format setting <<< === ===\n\n");

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    IPSLOG(0, "[__%s__] : <GigECam_I_Library*>(phdl)->GetConfig\n", __func__);
    res = static_cast<GigECam_I_Library *>(phdl)->GetConfig(pGigeCfg);
    if (res)
    {
        perror("Error!!! GigECam_Config_Inquiry()\n");
    }

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    printf(">>> New Features information of Image format setting === >>> === >>>\n");
    printf("    @ GigeCfg.bIsConnected---> %d\n", pGigeCfg->bIsConnected);
    printf("    @ GigeCfg.bIsEnbAcquisitionFrameRate---> %s\n", (pGigeCfg->bIsEnbAcquisitionFrameRate) ? "True" : "False");
    printf("    @ GigeCfg.dbAcquisitionFrameRate---> %5.3f\n", pGigeCfg->dbAcquisitionFrameRate);
    printf("    @ GigeCfg.bExposureAuto---> %s\n", (!pGigeCfg->bExposureAuto) ? "Auto" : "Off");
    printf("    @ GigeCfg.bPixelFormat---> %s\n", (pGigeCfg->bPixelFormat) ? "RGB8" : "Mono8");
    printf("    @ GigeCfg.iOffset_X ---> %d\n", pGigeCfg->iOffset_X);
    printf("    @ GigeCfg.iOffset_Y---> %d\n", pGigeCfg->iOffset_Y);
    printf("    @ GigeCfg.iWidth ---> %d\n", pGigeCfg->iWidth);
    printf("    @ GigeCfg.iHeight---> %d\n", pGigeCfg->iHeight);
    printf("    @ GigeCfg.iSensor_Width ---> %d\n", pGigeCfg->iSensor_Width);
    printf("    @ GigeCfg.iSensor_Height---> %d\n", pGigeCfg->iSensor_Height);
    printf("    @ GigeCfg.iBinning_Scale---> %d\n", pGigeCfg->iBinning_Scale);
    printf("<<< New Features information of Image format setting <<< === ===\n\n");

    seGigECamConfig_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig = *pGigeCfg;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_GigECameraCtl_Config
 *	Description : To set GigE camera configuration
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraCtl_Config(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seGigECamConfig *pGigeCfg = (seGigECamConfig *)pCP->pParm;

    printf("@ param.bPixelFormat---> %d\n", pGigeCfg->bPixelFormat);
    printf("@ param.iOffset_X ---> %d\n", pGigeCfg->iOffset_X);
    printf("@ param.iOffset_Y---> %d\n", pGigeCfg->iOffset_Y);
    printf("@ param.iWidth ---> %d\n", pGigeCfg->iWidth);
    printf("@ param.iHeight---> %d\n", pGigeCfg->iHeight);
    printf("@ param.iBinning_Scale---> %d\n", pGigeCfg->iBinning_Scale);
    printf("@ param.bExposureAuto---> %s\n", (!pGigeCfg->bExposureAuto) ? "Auto" : "Off");
    printf("@ param.dbExposureTime---> %6.4f\n", pGigeCfg->dbExposureTime);
    printf("@ param.bIsEnbTriggerMode---> %s\n", ((pGigeCfg->bIsEnbTriggerMode) ? "true" : "false"));
    printf("@ param.iTriggerActivation---> %d\n", pGigeCfg->iTriggerActivation);

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    IPSLOG(0, "[__%s__] : <GigECam_I_Library*>(phdl)->SetConfig\n", __func__);
    res = static_cast<GigECam_I_Library *>(phdl)->SetConfig(pGigeCfg);
    if (res)
    {
        perror("Error!!! GigECam_Config_Set()\n");
    }

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamConfig_Ret seRet;

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_GigECameraCtl_Capture
 *	Description : To trigger GigE camera to capture image.
 *                bIsEnbReadImgMode: used to determine whether to save or read files.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraCtl_Capture(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seGigECamConfig *pGigeCfg = (seGigECamConfig *)pCP->pParm;

    MAINLOG(0, "[__%s__], @ param.strSaveImgPath ---> %s\n", __func__, pCP->strSaveImgPath.c_str());

    bool bIsEnbReadImgMode = pGigeCfg->bIsEnbReadImageMode;
    IPSLOG(0, "[__%s__] : bIsEnbReadImgMode == %d\n", __func__, bIsEnbReadImgMode);

    string strInputPath = pCP->strInputImgPath;
    IPSLOG(0, "[__%s__] : strInputPath == %s\n", __func__, strInputPath.c_str());

    string strSavePath = pCP->strSaveImgPath;
    IPSLOG(0, "[__%s__] : strSavePath == %s\n", __func__, strSavePath.c_str());

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    if (pImgOut)
    {

        IPSLOG(0, "call function of AcquireImages( pImgOut, strSavePath )\n ");

        if (bIsEnbReadImgMode)
        {

            res = static_cast<GigECam_I_Library *>(phdl)->AcquireImages(pImgOut, strInputPath, bIsEnbReadImgMode);
            if (res)
            {
                IPSLOG(0, "[__%s__] : Error!!! ->AcquireImages(pImgOut, strSavePath)\n", __func__);
                goto Err;
            }
        }
        else
        {

            res = static_cast<GigECam_I_Library *>(phdl)->AcquireImages(pImgOut, strSavePath, bIsEnbReadImgMode);
            if (res)
            {
                IPSLOG(0, "[__%s__] : Error!!! ->AcquireImages(pImgOut, strSavePath)\n", __func__);
                goto Err;
            }
        }
    }
    else
    {

        IPSLOG(0, "call function of AcquireImages( strSavePath )\n ");

        res = static_cast<GigECam_I_Library *>(phdl)->AcquireImages(strSavePath);
        if (res)
        {
            IPSLOG(0, "[__%s__] : Error!!! ->AcquireImages(pImgOut)\n", __func__);
            goto Err;
        }
    }

Err:
    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamCapture_Ret seRet;

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seGigECamCapture_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_GigECameraCtl_Release
 *	Description : To release IPL(Image Processing Library)
 *                and GigE camera(Dual)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraCtl_Release(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);


    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    ipsComp_IPL_Release();
    ipsComp_Camera_Release();


    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamCapture_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig.bIsConnected = 0;

    memcpy(pResults, &seRet, sizeof(seGigECamCapture_Ret));

    usleep(500000); /* delay 500  ms */

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

// -------- . -------- GigE camera < Streaming >.start
/***********************************************************
 *	Function 	: Method_GigECameraStrm_Initialize
 *	Description : To initiate GigE camera streaming
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraStrm_Initialize(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    IPSLOG(0, "[__%s__] : <GigECam_I_Library*>(phdl)->   Streaming_Prepare()  \n", __func__);
    res = static_cast<GigECam_I_Library *>(phdl)->Streaming_Prepare();
    if (res)
    {
        perror("Error!!! GigECam_Streaming_Prepare()\n");
    }


    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamConfig_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig.bIsConnected = 1;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    usleep(500000); /* delay 500  ms */

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_GigECameraStrm_Inquiry
 *	Description : Query/Retrieve GigE camera streaming connection status.
 *                This function does not actually determine the streaming status; 
 *                it simply returns the flag "bIsConnected = 1". To truly add 
 *                the Inquiry functionality, please refer to the method used by the Vision sensor.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraStrm_Inquiry(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();


    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamConfig_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig.bIsConnected = 1;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    usleep(500000); /* delay 500  ms */

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_GigECameraStrm_Start
 *	Description : To start GigE camera streaming
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraStrm_Start(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr) {
        IPSLOG(0, "Error!!! nullptr\n", __func__);
        return -1;
    }

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    IPSLOG(0, "[__%s__] : <GigECam_I_Library*>(phdl)->   Streaming_Start()  \n", __func__);
    res = static_cast<GigECam_I_Library *>(phdl)->Streaming_Start();
    if (res)
    {
        perror("Error!!! GigECam_Streaming_Start()\n");
    }


    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamConfig_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig.bIsConnected = 1;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    usleep(500000); /* delay 500  ms */

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_GigECameraStrm_Capture
 *	Description : To start GigE camera streaming
 *                Retrieve image information under the condition 
 *                that GigE camera streaming is activated.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraStrm_Capture(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    string strSavePath = pCP->strSaveImgPath;
    IPSLOG(0, "[__%s__] : strSavePath == %s\n", __func__, strSavePath.c_str());

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    if (pImgOut)
    {
        IPSLOG(0, "[__%s__] : <GigECam_I_Library*>(phdl)->   Streaming_AcquireImages(pImgOut, strSavePath)  \n", __func__);

        res = static_cast<GigECam_I_Library *>(phdl)->Streaming_AcquireImages(pImgOut, strSavePath);
        if (res)
        {
            IPSLOG(0, "[__%s__] : Error!!! ->Streaming_AcquireImages(pImgOut, strSavePath)\n", __func__);
            goto Err;
        }
    }
    else
    {

        IPSLOG(0, "[__%s__] : <GigECam_I_Library*>(phdl)->   Streaming_AcquireImages(strSavePath)  \n", __func__);

        res = static_cast<GigECam_I_Library *>(phdl)->Streaming_AcquireImages(strSavePath);
        if (res)
        {
            IPSLOG(0, "[__%s__] : Error!!! ->Streaming_AcquireImages(strSavePath)\n", __func__);
            goto Err;
        }
    }

Err:
    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamConfig_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig.bIsConnected = 1;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    usleep(5000); /* delay 5  ms */

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_GigECameraStrm_Stop
 *	Description : To stop GigE camera streaming
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraStrm_Stop(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    IPSLOG(0, "[__%s__] : <GigECam_I_Library*>(phdl)->   Streaming_Stop()  \n", __func__);
    res = static_cast<GigECam_I_Library *>(phdl)->Streaming_Stop();
    if (res)
    {
        perror("Error!!! GigECam_Streaming_Stop()\n");
    }

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamConfig_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig.bIsConnected = 1;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    usleep(500000); /* delay 500  ms */

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_GigECameraStrm_Release
 *	Description : To release GigE camera streaming
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GigECameraStrm_Release(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr || pResults == nullptr)
        return -1;

    IPSLOG(0, "[__Method_GigECameraCtl__] : <--- end \n", __func__);

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    IPSLOG(0, "[__%s__] : <GigECam_I_Library*>(phdl)->   Streaming_Colse()  \n", __func__);
    res = static_cast<GigECam_I_Library *>(phdl)->Streaming_Colse();
    if (res)
    {
        perror("Error!!! GigECam_Streaming_Colse()\n");
    }

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seGigECamConfig_Ret seRet;

    seRet.retState = res;
    seRet.seCamConfig.bIsConnected = 1;

    memcpy(pResults, &seRet, sizeof(seGigECamConfig_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test <--- end
    /////////////////////////////////////////////////////////////////////////

    usleep(500000); /* delay 500  ms */

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}
// -------- . -------- GigE camera < Streaming >.end


// -------- . -------- IPL(Image Processing Library).start
/***********************************************************
 *	Function 	: Method_ImageCalibration
 *	Description : Alignment: the calculation of the ratio(mm/pixel) 
 *                between pixel size and actual physical size 
 *                is performed using a circular dot pattern.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_ImageCalibration(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seMth_ImageCalibration *pParamSet = (seMth_ImageCalibration *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    string strResultPath = pCP->strResultImgPath;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1; // 1.: use seImageInfo structure.
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0; // 0 : use image path.
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seMth_ImageCalibration_Ret seRet;

    seRect seROI_1, seROI_2;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Align_ImageCalibration(&mImgSrcTab,
                                                                              seROI_1,
                                                                              seROI_2,
                                                                              strResultPath.c_str(),
                                                                              &(seRet.dbPixelCount_X),
                                                                              &(seRet.dbPixelCount_Y));

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seRet.retState = res;

    if (seRet.dbPixelCount_X <= 0)
        seRet.dbPixelCount_X = 0.01;
    seRet.dbmm_per_pixel = pParamSet->dbE2E_Distance_mm / seRet.dbPixelCount_X;

    memcpy(pResults, &seRet, sizeof(seMth_ImageCalibration_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_Crop_GoldenTemplate
 *	Description : Alignment: it is necessary to obtain an image 
 *                of a Golden sample for calculating and setting 
 *                the position of the alignment box.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_Crop_GoldenTemplate(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seCropROI_GTemplate *pParamSet = (seCropROI_GTemplate *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pCP->strSaveImgPath;
    seEx.strImage_Result = pCP->strResultImgPath;

    seRect roiRECT = pParamSet->roiRect;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Align_CropTemplate(
        &mImgSrcTab,
        roiRECT,
        &seEx);

    if (res)
    {
        perror("Error!!! GigECam_AcquireImages()\n");
    }

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seCropROI_GTemplate_Ret seRet;

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seCropROI_GTemplate_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_PatternMatch
 *	Description : Alignment: Execute the PatternMatch for the alignment box.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_PatternMatch(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seMth_PatternMatch *pParamSet = (seMth_PatternMatch *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    string strTemplateIn = pCP->strTemplateImgPath;

    string strResultPath = pCP->strResultImgPath;

    seRect roiSearch = pParamSet->roiSearch;

    if (strSrcIn.empty() || strTemplateIn.empty() || strResultPath.empty())
    {
        return -1;
    }

    seBoundingBox seFMarkBoxOut;
    double dbScoreOut = 0.0;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Align_PatternMatch(&mImgSrcTab,
                                                                          strTemplateIn.c_str(),
                                                                          roiSearch,
                                                                          strResultPath.c_str(),
                                                                          &seFMarkBoxOut,
                                                                          &dbScoreOut);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seMth_PatternMatch_Ret seRet;

    seRet.retState = res;

    seRet.seFMarkBoxOut = seFMarkBoxOut;
    seRet.dbScoreOut = dbScoreOut;

    memcpy(pResults, &seRet, sizeof(seMth_PatternMatch_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_FindProfile
 *	Description : Aligment: Search for alignment circles and 
 *                analyze the profile variations of the image.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_FindProfile(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seMth_FindProfile *pParamSet = (seMth_FindProfile *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    string strResultPath = pCP->strResultImgPath;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seMth_FindProfile_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Align_FindProfile(&mImgSrcTab,
                                                                         pParamSet->roiSearch,
                                                                         pParamSet->roiMask,
                                                                         pParamSet->bDirection,
                                                                         pParamSet->bPolarity,
                                                                         pParamSet->iSelLineNo,
                                                                         strResultPath.c_str(),
                                                                         &(seRet.iDataCnt),
                                                                         seRet.i1DArrayOut);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seMth_FindProfile_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_DetectCircle
 *	Description : Aligment: Detect Circle.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_DetectCircle(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seMth_DetectCircle *pParamSet = (seMth_DetectCircle *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    string strResultPath = pCP->strResultImgPath;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seMth_DetectCircle_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Align_DetectCircle(&mImgSrcTab,
                                                                          pParamSet->roiSearch,
                                                                          pParamSet->roiMask,
                                                                          pParamSet->bDirection,
                                                                          pParamSet->bPolarity,
                                                                          pParamSet->iMinEdgeStrength,
                                                                          strResultPath.c_str(),
                                                                          &(seRet.seFMarkBoxOut),
                                                                          &(seRet.iDataCnt),
                                                                          seRet.sePointArrayOut);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seMth_DetectCircle_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_IBOX_Annulus
 *	Description : ImageProcessing: Detection Box Settings (Annulus).
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_IBOX_Annulus(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seIBox_Annulus *pParamSet = (seIBox_Annulus *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    string strResultPath = pCP->strResultImgPath;

    seAnnulus roiAnnulus = pParamSet->roiAnnulus;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seBoundingBox seIBox_AnnulusOut;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_InspectBox_Annulus(&mImgSrcTab,
                                                                          roiAnnulus,
                                                                          strResultPath.c_str(),
                                                                          &seIBox_AnnulusOut);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seIBox_Annulus_Ret seRet;

    seRet.retState = res;

    seRet.seBoundBoxOut = seIBox_AnnulusOut;

    memcpy(pResults, &seRet, sizeof(seIBox_Annulus_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_IBOX_Rect
 *	Description : ImageProcessing: Detection Box Settings (Rectangle).
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_IBOX_Rect(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seIBox_Rect *pParamSet = (seIBox_Rect *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    string strResultPath = pCP->strResultImgPath;

    seBoundingBox roiBRect = pParamSet->roiRect;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seBoundingBox seIBox_RectOut;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_InspectBox_Rect(&mImgSrcTab,
                                                                       roiBRect,
                                                                       strResultPath.c_str(),
                                                                       &seIBox_RectOut);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seIBox_Rect_Ret seRet;

    seRet.retState = res;

    seRet.seBoundBoxOut = seIBox_RectOut;

    memcpy(pResults, &seRet, sizeof(seIBox_Rect_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_IBOX_Circle
 *	Description : ImageProcessing: Detection Box Settings (Circle).
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_IBOX_Circle(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seIBox_Circle *pParamSet = (seIBox_Circle *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    string strResultPath = pCP->strResultImgPath;

    seCircle roiCircle = pParamSet->roiCircle;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seBoundingBox seIBox_CircleOut;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_InspectBox_Circle(&mImgSrcTab,
                                                                         roiCircle,
                                                                         strResultPath.c_str(),
                                                                         &seIBox_CircleOut);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seIBox_Circle_Ret seRet;

    seRet.retState = res;

    seRet.seBoundBoxOut = seIBox_CircleOut;

    memcpy(pResults, &seRet, sizeof(seIBox_Circle_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_CalcCoord
 *	Description : ImageProcessing: Calculate the coordinate 
 *                displacement between the detection box and 
 *                the positioning box.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_CalcCoord(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seCalcCoord *pParamSet = (seCalcCoord *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seBoundingBox seFBox = pParamSet->seFMarkBox;
    seBoundingBox seIBox = pParamSet->seInspBox;
    seCoordBindBox seCoorBBoxIn = pParamSet->seCoorBindBoxIn;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    // cycletime_start
    start = clock();

    seCoordBindBox seCoorBBoxOut;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_InspectBox_CoordCalculate(nullptr,
                                                                                 seFBox,
                                                                                 seIBox,
                                                                                 seCoorBBoxIn,
                                                                                 &seCoorBBoxOut);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seCalcCoord_Ret seRet;

    seRet.retState = res;

    seRet.seCoorBindBoxOut = seCoorBBoxOut;

    memcpy(pResults, &seRet, sizeof(seCalcCoord_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> end
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_CropImg_Annulus
 *	Description : ImageProcessing: Using the information 
 *                obtained from coordinate displacement calculation, 
 *                crop the Annulus image within the detection box.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_CropImg_Annulus(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seCropROI_Annulus *pParamSet = (seCropROI_Annulus *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pCP->strSaveImgPath;
    seEx.strImage_Result = pCP->strResultImgPath;
    seEx.poutImgInfo = pImgOut;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seCropROI_Annulus_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_InspectBox_CropImg_Annulus(&mImgSrcTab,
                                                                                  pParamSet->seCoordBox,
                                                                                  pParamSet->roiAnnulus,
                                                                                  &seEx,
                                                                                  &(seRet.seRoiOffset_Annulus));

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seCropROI_Annulus_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);
    return res;
}

/***********************************************************
 *	Function 	: Method_CropImg_Rect
 *	Description : ImageProcessing: Using the information 
 *                obtained from coordinate displacement calculation, 
 *                crop the Rectangle image within the detection box.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_CropImg_Rect(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seCropROI_Rect *pParamSet = (seCropROI_Rect *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pCP->strSaveImgPath;
    seEx.strImage_Result = pCP->strResultImgPath;
    seEx.poutImgInfo = pImgOut;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seCropROI_Rect_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_InspectBox_CropImg_Rect(&mImgSrcTab,
                                                                               pParamSet->seCoordBox,
                                                                               pParamSet->roiRect,
                                                                               &seEx,
                                                                               &(seRet.seRoiOffset_Rect));

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seCropROI_Rect_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_CropImg_Circle
 *	Description : ImageProcessing: Using the information 
 *                obtained from coordinate displacement calculation, 
 *                crop the Circle image within the detection box.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_CropImg_Circle(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seCropROI_Circle *pParamSet = (seCropROI_Circle *)pCP->pParm;

    string strSrcIn = pParamSet->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pParamSet->strSaveImgPath;
    seEx.strImage_Result = pParamSet->strResultImgPath;
    seEx.poutImgInfo = pImgOut;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seCropROI_Circle_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_InspectBox_CropImg_Circle(&mImgSrcTab,
                                                                                 pParamSet->seCoordBox,
                                                                                 pParamSet->roiCircle,
                                                                                 &seEx,
                                                                                 &(seRet.seRoiOffset_Circle));

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seCropROI_Circle_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_Histogram_Annulus
 *	Description : ImageProcessing: Calculate the histogram of the 
 *                image within the detection box (Annulus)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_Histogram_Annulus(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seHisg_Annulus *pParamSet = (seHisg_Annulus *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Result = pCP->strResultImgPath;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seHisg_Annulus_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Histogram_Annulus(&mImgSrcTab,
                                                                         pParamSet->roiAnnulus,
                                                                         &seEx,
                                                                         &(seRet.iDataCnt),
                                                                         seRet.db1DArray);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seHisg_Annulus_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_Histogram_Rect
 *	Description : ImageProcessing: Calculate the histogram of the 
 *                image within the detection box (Rectangle)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_Histogram_Rect(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seHisg_Rect *pParamSet = (seHisg_Rect *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Result = pCP->strResultImgPath;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seHisg_Rect_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Histogram_Rect(&mImgSrcTab,
                                                                      pParamSet->roiRect,
                                                                      &seEx,
                                                                      &(seRet.iDataCnt),
                                                                      seRet.db1DArray);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seHisg_Rect_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_Histogram_Circle
 *	Description : ImageProcessing: Calculate the histogram of the 
 *                image within the detection box (Circle)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_Histogram_Circle(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seHisg_Circle *pParamSet = (seHisg_Circle *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Result = pCP->strResultImgPath;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seHisg_Circle_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Histogram_Circle(&mImgSrcTab,
                                                                        pParamSet->roiCircle,
                                                                        &seEx,
                                                                        &(seRet.iDataCnt),
                                                                        seRet.db1DArray);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seHisg_Circle_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_Threshold
 *	Description : ImageProcessing: Image Processing - Binary 
 *                thresholding within detection box.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_Threshold(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seIP_Threshold *pParamSet = (seIP_Threshold *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pCP->strSaveImgPath;
    seEx.strImage_Result = pCP->strResultImgPath;
    seEx.poutImgInfo = pImgOut;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Threshold(&mImgSrcTab,
                                                                 &(pParamSet->dbThresh),
                                                                 &(pParamSet->dbMaxVal),
                                                                 pParamSet->emTypes,
                                                                 &seEx);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seIP_Threshold_Ret seRet;

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seIP_Threshold_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_Morphology
 *	Description : ImageProcessing: Binary image morphology processing.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_Morphology(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seMorphology *pParamSet = (seMorphology *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pCP->strSaveImgPath;
    seEx.strImage_Result = pCP->strResultImgPath;
    seEx.poutImgInfo = pImgOut;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_Morphology(&mImgSrcTab,
                                                                  pParamSet->emShapes,
                                                                  pParamSet->iKSize,
                                                                  pParamSet->emOperation,
                                                                  &seEx);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seMorphology_Ret seRet;

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seMorphology_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_NoiseRemoval
 *	Description : ImageProcessing: After performing morphological 
 *                processing, remove noise 
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_NoiseRemoval(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seNoiseRemoval *pParamSet = (seNoiseRemoval *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pCP->strSaveImgPath;
    seEx.strImage_Result = pCP->strResultImgPath;
    seEx.poutImgInfo = pImgOut;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_NoiseRemoval(&mImgSrcTab,
                                                                    pParamSet->dbLimit_min,
                                                                    pParamSet->dbLimit_max,
                                                                    &seEx);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seNoiseRemoval_Ret seRet;

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seNoiseRemoval_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_DataAugmentation
 *	Description : ImageProcessing: data augmentation processing
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_DataAugmentation(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seDataAugmentation *pParamSet = (seDataAugmentation *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pCP->strSaveImgPath;
    seEx.strImage_Result = pCP->strResultImgPath;
    seEx.poutImgInfo = pImgOut;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1; // 0: Path of SrcImg, 1:pointer of LPImageInfo
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0; // 0: Path of SrcImg, 1:pointer of LPImageInfo
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_DataAugmentation(&mImgSrcTab,
                                                                        pParamSet->seDA_Param,
                                                                        &seEx);

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    seNoiseRemoval_Ret seRet;

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seNoiseRemoval_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_GlueWidthMeasure_Annulus
 *	Description : ImageProcessing: Glue Width detection (Annulus)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GlueWidthMeasure_Annulus(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seMeasGW_Annulus *pParamSet = (seMeasGW_Annulus *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pCP->strSaveImgPath;
    seEx.strImage_Result = pCP->strResultImgPath;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seMeasGW_Annulus_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_GlueWidth_Measure_Annulus(&mImgSrcTab,
                                                                                 pParamSet->roiAnnuls,
                                                                                 pParamSet->stepSize,
                                                                                 &seEx,
                                                                                 &(seRet.iDataCnt),
                                                                                 seRet.dbLength_InnerOut,
                                                                                 seRet.dbLength_OuterOut,
                                                                                 seRet.pos_InnerOut,
                                                                                 seRet.pos_OuterOut,
                                                                                 &(seRet.dbGlueAreaOut));

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    if (pParamSet->dbmm_per_pixel <= 0.0)
    {

        pParamSet->dbmm_per_pixel = 1.0;
    }

    int iCnt = seRet.iDataCnt;

    for (int i = 0; i < iCnt; i++)
    {

        seRet.dbLength_InnerOut[i] = seRet.dbLength_InnerOut[i] * pParamSet->dbmm_per_pixel;
        seRet.dbLength_OuterOut[i] = seRet.dbLength_OuterOut[i] * pParamSet->dbmm_per_pixel;
    }

    seRet.dbGlueAreaOut = seRet.dbGlueAreaOut * pParamSet->dbmm_per_pixel * pParamSet->dbmm_per_pixel;

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seMeasGW_Annulus_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

/***********************************************************
 *	Function 	: Method_GlueWidthMeasure_Rect
 *	Description : ImageProcessing: Glue Width detection (Rectabgle)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int Method_GlueWidthMeasure_Rect(void *phdl, const LPImageInfo pImgIn, void *pParam, void *pResults, LPImageInfo pImgOut)
{
    int res = 0;
    clock_t start, end;

    if (phdl == nullptr || pParam == nullptr)
        return -1;

    IPSLOG(0, "[__%s__] : ---> start \n", __func__);

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)pParam;

    seMeasGW_Rect *pParamSet = (seMeasGW_Rect *)pCP->pParm;

    string strSrcIn = pCP->strInputImgPath;

    seExpandable seEx;
    seEx.strImage_Save = pCP->strSaveImgPath;
    seEx.strImage_Result = pCP->strResultImgPath;

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    seImgSrcTab mImgSrcTab;

    if (pImgIn)
    {

        mImgSrcTab.bSelInputSrc = 1;
        mImgSrcTab.bDumpRetImg = 0;
        mImgSrcTab.ptrImgInfo = pImgIn;
    }
    else
    {

        mImgSrcTab.bSelInputSrc = 0;
        mImgSrcTab.bDumpRetImg = 1;
        mImgSrcTab.strSrcImg = strSrcIn.c_str();
    }

    // cycletime_start
    start = clock();

    seMeasGW_Rect_Ret seRet;

    res = static_cast<PTWDLL_I_GlueWidth *>(phdl)->vbs_GlueWidth_Measure_Rect(&mImgSrcTab,
                                                                              pParamSet->roiRect,
                                                                              pParamSet->stepSize,
                                                                              &seEx,
                                                                              &(seRet.iDataCnt),
                                                                              seRet.dbLength_InnerOut,
                                                                              seRet.dbLength_OuterOut,
                                                                              seRet.pos_InnerOut,
                                                                              seRet.pos_OuterOut,
                                                                              &(seRet.dbGlueAreaOut));

    // cycletime_end
    end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    (void)elapsed;
    IPSLOG(0, "[__%s__] : Cycle time = %5.2f (seconds) \n", __func__, elapsed);

    if (pParamSet->dbmm_per_pixel <= 0.0)
    {

        pParamSet->dbmm_per_pixel = 1.0;
    }

    int iCnt = seRet.iDataCnt;

    for (int i = 0; i < iCnt; i++)
    {

        seRet.dbLength_InnerOut[i] = seRet.dbLength_InnerOut[i] * pParamSet->dbmm_per_pixel;
        seRet.dbLength_OuterOut[i] = seRet.dbLength_OuterOut[i] * pParamSet->dbmm_per_pixel;
    }

    seRet.dbGlueAreaOut = seRet.dbGlueAreaOut * pParamSet->dbmm_per_pixel * pParamSet->dbmm_per_pixel;

    seRet.retState = res;

    memcpy(pResults, &seRet, sizeof(seMeasGW_Rect_Ret));

    /////////////////////////////////////////////////////////////////////////
    /// Unit Test ---> start
    /////////////////////////////////////////////////////////////////////////

    IPSLOG(0, "[__%s__] : <--- end \n", __func__);

    return res;
}

////////////////////////////////////////////////////////////////////////////////////
// Json JsonGenerator List define.
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: JsonEditor_AutoRunning_Mode
 *	Description : Generate a JSON formatted string for AutoRunning  
 *                Mode based on the passed in parameters and publish 
 *                it through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_AutoRunning_Mode(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seMode_AutoRunning_Ret seResult;
    memcpy(&seResult, (seMode_AutoRunning_Ret *)pResult, sizeof(seMode_AutoRunning_Ret));

    struct json_object *root, *obj_2, *obj_3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);

    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);
    

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_TriggerMode_Type
 *	Description : Generate a JSON formatted string for Trigger Mode 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_TriggerMode_Type(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seMode_TriggerModeType_Ret seResult;
    memcpy(&seResult, (seMode_TriggerModeType_Ret *)pResult, sizeof(seMode_TriggerModeType_Ret));

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);

    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "pset", obj_4);
    json_object_object_add(obj_4, "Enb_TriggerMode_Activate", json_object_new_int(1));

    const char *buf_cst = json_object_to_json_string(root);
    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECam_Initialize
 *	Description : Generate a JSON formatted string for GigECam Initiated 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECam_Initialize(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "bIsConnected", json_object_new_int(seResult.seCamConfig.bIsConnected));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECam_Inquiry
 *	Description : Generate a JSON formatted string for GigECam Inquiry 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECam_Inquiry(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "IsConnected", json_object_new_int(seResult.seCamConfig.bIsConnected));

    json_object_object_add(obj_4, "PixelFormat", json_object_new_int(seResult.seCamConfig.bPixelFormat));

    json_object_object_add(obj_4, "Offset_X", json_object_new_int(seResult.seCamConfig.iOffset_X));

    json_object_object_add(obj_4, "Offset_Y", json_object_new_int(seResult.seCamConfig.iOffset_Y));

    json_object_object_add(obj_4, "Width", json_object_new_int(seResult.seCamConfig.iWidth));

    json_object_object_add(obj_4, "Height", json_object_new_int(seResult.seCamConfig.iHeight));

    json_object_object_add(obj_4, "ExposureMode", json_object_new_int(seResult.seCamConfig.bExposureAuto));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECam_Config
 *	Description : Generate a JSON formatted string for GigECam Config 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECam_Config(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECam_Parameter
 *	Description : Generate a JSON formatted string for GigECam Parameter 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECam_Parameter(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamCapture_Ret seResult = *(seGigECamCapture_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECam_Release
 *	Description : Generate a JSON formatted string for GigECam Release 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECam_Release(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "bIsConnected", json_object_new_int(seResult.seCamConfig.bIsConnected));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}


// -------- . -------- GigE camera < Streaming >.Start
/***********************************************************
 *	Function 	: JsonEditor_GigECamStrm_Initialize
 *	Description : Generate a JSON formatted string for GigECam Streaming Initiated 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECamStrm_Initialize(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "bIsConnected", json_object_new_int(seResult.seCamConfig.bIsConnected));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECamStrm_Inquiry
 *	Description : Generate a JSON formatted string for GigECam Streaming Inquiry 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECamStrm_Inquiry(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "bIsConnected", json_object_new_int(seResult.seCamConfig.bIsConnected));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECamStrm_Start
 *	Description : Generate a JSON formatted string for Starting GigECam Streaming 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECamStrm_Start(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "bIsConnected", json_object_new_int(seResult.seCamConfig.bIsConnected));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECamStrm_Capture
 *	Description : Generate a JSON formatted string for GigECam Streaming Capture 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECamStrm_Capture(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "bIsConnected", json_object_new_int(seResult.seCamConfig.bIsConnected));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECamStrm_Stop
 *	Description : Generate a JSON formatted string for Stoping GigECam Streaming 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECamStrm_Stop(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "bIsConnected", json_object_new_int(seResult.seCamConfig.bIsConnected));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GigECamStrm_Release
 *	Description : Generate a JSON formatted string for GigECam Streaming Release 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GigECamStrm_Release(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seGigECamConfig_Ret seResult = *(seGigECamConfig_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "bIsConnected", json_object_new_int(seResult.seCamConfig.bIsConnected));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_ImageCalibrations
 *	Description : Generate a JSON formatted string for Image Calibration
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_ImageCalibrations(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr || pResult == nullptr)
        return -1;

    seMth_ImageCalibration_Ret seResult = *(seMth_ImageCalibration_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    json_object_object_add(obj_4, "PixelCount_X", json_object_new_double(seResult.dbPixelCount_X));

    json_object_object_add(obj_4, "PixelCount_Y", json_object_new_double(seResult.dbPixelCount_Y));

    json_object_object_add(obj_4, "mm_per_pixel", json_object_new_double(seResult.dbmm_per_pixel));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);
 
    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_Crop_GoldenTemplate
 *	Description : Generate a JSON formatted string for Crop Golden Template 
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_Crop_GoldenTemplate(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr)
        return -1;

    seCropROI_GTemplate_Ret seResult = *(seCropROI_GTemplate_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_PatternMatch
 *	Description : Generate a JSON formatted string for Pattern Match
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_PatternMatch(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr || pResult == nullptr)
        return -1;

    seMth_PatternMatch_Ret seResult = *(seMth_PatternMatch_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);
    json_object_object_add(obj_4, "SimilarityScore", json_object_new_double(seResult.dbScoreOut));

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "BoundingBox_FMark", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(seResult.seFMarkBoxOut.cX));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(seResult.seFMarkBoxOut.cY));
    json_object_object_add(obj_5, "Angle", json_object_new_double(seResult.seFMarkBoxOut.dbAngle));
    json_object_object_add(obj_5, "Top", json_object_new_int(seResult.seFMarkBoxOut.rectBox.top));
    json_object_object_add(obj_5, "Left", json_object_new_int(seResult.seFMarkBoxOut.rectBox.left));
    json_object_object_add(obj_5, "Bottom", json_object_new_int(seResult.seFMarkBoxOut.rectBox.bottom));
    json_object_object_add(obj_5, "Right", json_object_new_int(seResult.seFMarkBoxOut.rectBox.right));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_FindProfile
 *	Description : Generate a JSON formatted string for Find Profile
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_FindProfile(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seMth_FindProfile_Ret seResult = *(seMth_FindProfile_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;
    struct json_object *array0;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "1DArrayOut", obj_5);

    array0 = (struct json_object *)json_object_new_array();
    if (!array0)
    {
        return -1;
    }
    int size1 = seResult.iDataCnt;
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array0, json_object_new_double(seResult.i1DArrayOut[i]));
    }
    json_object_object_add(obj_5, "id_1", array0);

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_DetectCircle
 *	Description : Generate a JSON formatted string for Detect Circle
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_DetectCircle(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seMth_DetectCircle_Ret seResult = *(seMth_DetectCircle_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "BoundingBox_Circle", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(seResult.seFMarkBoxOut.cX));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(seResult.seFMarkBoxOut.cY));
    json_object_object_add(obj_5, "Angle", json_object_new_double(seResult.seFMarkBoxOut.dbAngle));
    json_object_object_add(obj_5, "Top", json_object_new_int(seResult.seFMarkBoxOut.rectBox.top));
    json_object_object_add(obj_5, "Left", json_object_new_int(seResult.seFMarkBoxOut.rectBox.left));
    json_object_object_add(obj_5, "Bottom", json_object_new_int(seResult.seFMarkBoxOut.rectBox.bottom));
    json_object_object_add(obj_5, "Right", json_object_new_int(seResult.seFMarkBoxOut.rectBox.right));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_IBOX_Annulus
 *	Description : Generate a JSON formatted string for Detection Box Settings (Annulus)
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_IBOX_Annulus(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr || pResult == nullptr)
        return -1;

    seIBox_Annulus_Ret seResult = *(seIBox_Annulus_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "BoundingBox_Out", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(seResult.seBoundBoxOut.cX));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(seResult.seBoundBoxOut.cY));
    json_object_object_add(obj_5, "Angle", json_object_new_double(seResult.seBoundBoxOut.dbAngle));
    json_object_object_add(obj_5, "Top", json_object_new_int(seResult.seBoundBoxOut.rectBox.top));
    json_object_object_add(obj_5, "Left", json_object_new_int(seResult.seBoundBoxOut.rectBox.left));
    json_object_object_add(obj_5, "Bottom", json_object_new_int(seResult.seBoundBoxOut.rectBox.bottom));
    json_object_object_add(obj_5, "Right", json_object_new_int(seResult.seBoundBoxOut.rectBox.right));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_IBOX_Rect
 *	Description : Generate a JSON formatted string for Detection Box Settings (Rectangle)
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_IBOX_Rect(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr || pResult == nullptr)
        return -1;

    seIBox_Rect_Ret seResult = *(seIBox_Rect_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "BoundingBox_Out", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(seResult.seBoundBoxOut.cX));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(seResult.seBoundBoxOut.cY));
    json_object_object_add(obj_5, "Angle", json_object_new_double(seResult.seBoundBoxOut.dbAngle));
    json_object_object_add(obj_5, "Top", json_object_new_int(seResult.seBoundBoxOut.rectBox.top));
    json_object_object_add(obj_5, "Left", json_object_new_int(seResult.seBoundBoxOut.rectBox.left));
    json_object_object_add(obj_5, "Bottom", json_object_new_int(seResult.seBoundBoxOut.rectBox.bottom));
    json_object_object_add(obj_5, "Right", json_object_new_int(seResult.seBoundBoxOut.rectBox.right));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_IBOX_Circle
 *	Description : Generate a JSON formatted string for Detection Box Settings (Circle)
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_IBOX_Circle(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr || pResult == nullptr)
        return -1;

    seIBox_Circle_Ret seResult = *(seIBox_Circle_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "BoundingBox_Out", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(seResult.seBoundBoxOut.cX));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(seResult.seBoundBoxOut.cY));
    json_object_object_add(obj_5, "Angle", json_object_new_double(seResult.seBoundBoxOut.dbAngle));
    json_object_object_add(obj_5, "Top", json_object_new_int(seResult.seBoundBoxOut.rectBox.top));
    json_object_object_add(obj_5, "Left", json_object_new_int(seResult.seBoundBoxOut.rectBox.left));
    json_object_object_add(obj_5, "Bottom", json_object_new_int(seResult.seBoundBoxOut.rectBox.bottom));
    json_object_object_add(obj_5, "Right", json_object_new_int(seResult.seBoundBoxOut.rectBox.right));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_CalcCoord
 *	Description : Generate a JSON formatted string for Calculate the coordinate
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_CalcCoord(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr || pResult == nullptr)
        return -1;

    seCalcCoord_Ret seResult = *(seCalcCoord_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "CalibCoord", obj_5);
    json_object_object_add(obj_5, "Anlgle", json_object_new_double(seResult.seCoorBindBoxOut.CalibCoord.dbAngle));
    json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(seResult.seCoorBindBoxOut.CalibCoord.iDelta_W));
    json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(seResult.seCoorBindBoxOut.CalibCoord.iDelta_H));
    json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(seResult.seCoorBindBoxOut.CalibCoord.iDelta_InspectBox_W));
    json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(seResult.seCoorBindBoxOut.CalibCoord.iDelta_InspectBox_H));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "BoundingBox_FMark", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(seResult.seCoorBindBoxOut.FMark.cX));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(seResult.seCoorBindBoxOut.FMark.cY));
    json_object_object_add(obj_6, "Angle", json_object_new_double(seResult.seCoorBindBoxOut.FMark.dbAngle));
    json_object_object_add(obj_6, "Top", json_object_new_int(seResult.seCoorBindBoxOut.FMark.rectBox.top));
    json_object_object_add(obj_6, "Left", json_object_new_int(seResult.seCoorBindBoxOut.FMark.rectBox.left));
    json_object_object_add(obj_6, "Bottom", json_object_new_int(seResult.seCoorBindBoxOut.FMark.rectBox.bottom));
    json_object_object_add(obj_6, "Right", json_object_new_int(seResult.seCoorBindBoxOut.FMark.rectBox.right));

    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "BoundingBox_IBox", obj_7);
    json_object_object_add(obj_7, "Center_X", json_object_new_int(seResult.seCoorBindBoxOut.InsptBox.cX));
    json_object_object_add(obj_7, "Center_Y", json_object_new_int(seResult.seCoorBindBoxOut.InsptBox.cY));
    json_object_object_add(obj_7, "Angle", json_object_new_double(seResult.seCoorBindBoxOut.InsptBox.dbAngle));
    json_object_object_add(obj_7, "Top", json_object_new_int(seResult.seCoorBindBoxOut.InsptBox.rectBox.top));
    json_object_object_add(obj_7, "Left", json_object_new_int(seResult.seCoorBindBoxOut.InsptBox.rectBox.left));
    json_object_object_add(obj_7, "Bottom", json_object_new_int(seResult.seCoorBindBoxOut.InsptBox.rectBox.bottom));
    json_object_object_add(obj_7, "Right", json_object_new_int(seResult.seCoorBindBoxOut.InsptBox.rectBox.right));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_CropImg_Annulus
 *	Description : Generate a JSON formatted string for Crop The Annulus Image
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_CropImg_Annulus(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr || pResult == nullptr)
        return -1;

    seCropROI_Annulus_Ret seResult = *(seCropROI_Annulus_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "ROI_Annulus_Out", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(seResult.seRoiOffset_Annulus.cX));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(seResult.seRoiOffset_Annulus.cY));
    json_object_object_add(obj_5, "Radius_Inner", json_object_new_double(seResult.seRoiOffset_Annulus.dbRadius_Inner));
    json_object_object_add(obj_5, "Radius_Outer", json_object_new_double(seResult.seRoiOffset_Annulus.dbRadius_Outer));
    json_object_object_add(obj_5, "Angle_Start", json_object_new_double(seResult.seRoiOffset_Annulus.dbStartAngle));
    json_object_object_add(obj_5, "Angle_End", json_object_new_double(seResult.seRoiOffset_Annulus.dbEndAngle));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_CropImg_Rect
 *	Description : Generate a JSON formatted string for Crop The Rectangle Image
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_CropImg_Rect(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr || pResult == nullptr)
        return -1;

    seCropROI_Rect_Ret seResult = *(seCropROI_Rect_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "ROIBB_Rect_Out", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(seResult.seRoiOffset_Rect.cX));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(seResult.seRoiOffset_Rect.cY));
    json_object_object_add(obj_5, "Angle", json_object_new_double(seResult.seRoiOffset_Rect.dbAngle));
    json_object_object_add(obj_5, "Top", json_object_new_int(seResult.seRoiOffset_Rect.rectBox.top));
    json_object_object_add(obj_5, "Left", json_object_new_int(seResult.seRoiOffset_Rect.rectBox.left));
    json_object_object_add(obj_5, "Bottom", json_object_new_int(seResult.seRoiOffset_Rect.rectBox.bottom));
    json_object_object_add(obj_5, "Right", json_object_new_int(seResult.seRoiOffset_Rect.rectBox.right));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_CropImg_Circle
 *	Description : Generate a JSON formatted string for Crop The Circle Image
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_CropImg_Circle(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    if (JsonBuf == nullptr || pResult == nullptr)
        return -1;

    seCropROI_Circle_Ret seResult = *(seCropROI_Circle_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "ROI_Circle_Out", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(seResult.seRoiOffset_Circle.cX));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(seResult.seRoiOffset_Circle.cY));
    json_object_object_add(obj_5, "Angle", json_object_new_double(seResult.seRoiOffset_Circle.dbAngle));
    json_object_object_add(obj_5, "Radius", json_object_new_double(seResult.seRoiOffset_Circle.dbRadius));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_Histogram_Annulus
 *	Description : Generate a JSON formatted string for histogram of the Image (Annulus)
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_Histogram_Annulus(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seHisg_Annulus_Ret seResult = *(seHisg_Annulus_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "p1DArray", obj_5);

    struct json_object *array0 = json_object_new_array();
    if (!array0)
    {
        return -1;
    }
    int size1 = seResult.iDataCnt;
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array0, json_object_new_double(seResult.db1DArray[i]));
    }
    json_object_object_add(obj_5, "1", array0);

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_Histogram_Rect
 *	Description : Generate a JSON formatted string for histogram of the Image (Rectangle)
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_Histogram_Rect(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seHisg_Rect_Ret seResult = *(seHisg_Rect_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "p1DArray", obj_5);

    struct json_object *array0 = json_object_new_array();
    if (!array0)
    {
        return -1;
    }
    int size1 = seResult.iDataCnt;
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array0, json_object_new_double(seResult.db1DArray[i]));
    }
    json_object_object_add(obj_5, "1", array0);

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_Histogram_Circle
 *	Description : Generate a JSON formatted string for histogram of the Image (Circle)
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_Histogram_Circle(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seHisg_Circle_Ret seResult = *(seHisg_Circle_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "p1DArray", obj_5);

    struct json_object *array0 = json_object_new_array();
    if (!array0)
    {
        return -1;
    }
    int size1 = seResult.iDataCnt;
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array0, json_object_new_double(seResult.db1DArray[i]));
    }
    json_object_object_add(obj_5, "1", array0);

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_Threshold
 *	Description : Generate a JSON formatted string for Binary thresholding
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_Threshold(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seIP_Threshold_Ret seResult = *(seIP_Threshold_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_Threshold
 *	Description : Generate a JSON formatted string for Morphology
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_Morphology(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seMorphology_Ret seResult = *(seMorphology_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_NoiseRemoval
 *	Description : Generate a JSON formatted string for Noise Removal
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_NoiseRemoval(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seNoiseRemoval_Ret seResult = *(seNoiseRemoval_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_DataAugmentation
 *	Description : Generate a JSON formatted string for Data Augmentation
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_DataAugmentation(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seNoiseRemoval_Ret seResult = *(seNoiseRemoval_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    const char *buf_cst = json_object_to_json_string(root);

    std::string strJob_tmp(buf_cst);

    json_object_put(root);

    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GlueWidthMeasure_Annulus
 *	Description : Generate a JSON formatted string for Glue Width detection (Annulus)
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GlueWidthMeasure_Annulus(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seMeasGW_Annulus_Ret seResult = *(seMeasGW_Annulus_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7, *obj_8;
    struct json_object *array0, *array1, *array2, *array3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    int size1 = seResult.iDataCnt;

    //
    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "plength_InnerOut", obj_5);

    array0 = (struct json_object *)json_object_new_array();
    if (!array0)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array0, json_object_new_double(seResult.dbLength_InnerOut[i]));
    }
    json_object_object_add(obj_5, "id_1", array0);

    //
    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "plength_OuterOut", obj_6);

    array1 = (struct json_object *)json_object_new_array();
    if (!array1)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array1, json_object_new_double(seResult.dbLength_OuterOut[i]));
    }
    json_object_object_add(obj_6, "id_1", array1);

    // ================= //
    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "pPosition_InnerOut_X", obj_7);

    array2 = (struct json_object *)json_object_new_array();
    if (!array2)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array2, json_object_new_double(seResult.pos_InnerOut[i].x));
    }
    json_object_object_add(obj_7, "id_1", array2);

    // ================= //
    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "pPosition_InnerOut_Y", obj_7);

    array2 = (struct json_object *)json_object_new_array();
    if (!array2)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array2, json_object_new_double(seResult.pos_InnerOut[i].y));
    }
    json_object_object_add(obj_7, "id_1", array2);

    // ================= //
    obj_8 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "pPosition_OuterOut_X", obj_8);

    array3 = (struct json_object *)json_object_new_array();
    if (!array3)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array3, json_object_new_double(seResult.pos_OuterOut[i].x));
    }
    json_object_object_add(obj_8, "id_1", array3);

    // ================= //
    obj_8 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "pPosition_OuterOut_Y", obj_8);

    array3 = (struct json_object *)json_object_new_array();
    if (!array3)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array3, json_object_new_double(seResult.pos_OuterOut[i].y));
    }
    json_object_object_add(obj_8, "id_1", array3);

    json_object_object_add(obj_4, "glueAreaOut", json_object_new_double(seResult.dbGlueAreaOut));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    
    const char *buf_cst = json_object_to_json_string(root);
    

    std::string strJob_tmp(buf_cst);

    
    json_object_put(root);

    
    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

/***********************************************************
 *	Function 	: JsonEditor_GlueWidthMeasure_Rect
 *	Description : Generate a JSON formatted string for Glue Width detection (Rectangle)
 *                based on the passed in parameters and publish it 
 *                through MQTT
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int JsonEditor_GlueWidthMeasure_Rect(const char *szKey, void *pResult, const char *JsonBuf, const bool bCamdID)
{
    int res = 0;

    seMeasGW_Rect_Ret seResult = *(seMeasGW_Rect_Ret*)pResult;

    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7, *obj_8;
    struct json_object *array0, *array1, *array2, *array3;

    MAINLOG(0, "%s json : %s\n", __func__, "create json format ---> start");

    // create json format
    root = json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(szKey));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    if (seResult.retState)
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Error!!!"));
    }
    else
    {
        json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    }

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);

    int size1 = seResult.iDataCnt;

    //
    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "plength_InnerOut", obj_5);

    array0 = (struct json_object *)json_object_new_array();
    if (!array0)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array0, json_object_new_double(seResult.dbLength_InnerOut[i]));
    }
    json_object_object_add(obj_5, "id_1", array0);

    //
    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "plength_OuterOut", obj_6);

    array1 = (struct json_object *)json_object_new_array();
    if (!array1)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array1, json_object_new_double(seResult.dbLength_OuterOut[i]));
    }
    json_object_object_add(obj_6, "id_1", array1);

    // ================= //
    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "pPosition_InnerOut_X", obj_7);

    array2 = (struct json_object *)json_object_new_array();
    if (!array2)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array2, json_object_new_double(seResult.pos_InnerOut[i].x));
    }
    json_object_object_add(obj_7, "id_1", array2);

    // ================= //
    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "pPosition_InnerOut_Y", obj_7);

    array2 = (struct json_object *)json_object_new_array();
    if (!array2)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array2, json_object_new_double(seResult.pos_InnerOut[i].y));
    }
    json_object_object_add(obj_7, "id_1", array2);

    // ================= //
    obj_8 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "pPosition_OuterOut_X", obj_8);

    array3 = (struct json_object *)json_object_new_array();
    if (!array3)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array3, json_object_new_double(seResult.pos_OuterOut[i].x));
    }
    json_object_object_add(obj_8, "id_1", array3);

    // ================= //
    obj_8 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "pPosition_OuterOut_Y", obj_8);

    array3 = (struct json_object *)json_object_new_array();
    if (!array3)
    {
        return -1;
    }
    for (int i = 0; i < size1; ++i)
    {
        json_object_array_add(array3, json_object_new_double(seResult.pos_OuterOut[i].y));
    }
    json_object_object_add(obj_8, "id_1", array3);

    json_object_object_add(obj_4, "glueAreaOut", json_object_new_double(seResult.dbGlueAreaOut));

    json_object_object_add(obj_3, "pset", json_object_new_string(JsonBuf));

    
    const char *buf_cst = json_object_to_json_string(root);
    

    std::string strJob_tmp(buf_cst);

    
    json_object_put(root);

    
    ext_mqtt_publisher_Dual((char*)strJob_tmp.c_str(), bCamdID);

    return res;
}

////////////////////////////////////////////////////////////////////////////////////
// Parameter converter to All Parameter
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_AUTO_RUNNING
 *	Description : Convert Annulus parameters to All parameters
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_AUTO_RUNNING(void *ptrParam, void *pAllParam)
{
    int res = 0;

    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMode_AutoRunning *pP = (seMode_AutoRunning *)pCP->pParm;

    pAP->ret_Mode_AutoRuning.bFlg_AutoRunning = pP->bFlg_AutoRunning;

    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_AUTO_RUNNING
 *	Description : Convert Rectangle parameters to All parameters
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_AUTO_RUNNING(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_AUTO_RUNNING
 *	Description : Convert Json parameters to All parameters
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_AUTO_RUNNING(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seMode_AutoRunning *pJP = (seMode_AutoRunning *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMode_AutoRunning *pP = (seMode_AutoRunning *)pCP->pParm;

    seMode_AutoRunning tmpP;

    tmpP.bFlg_AutoRunning = pJP->bFlg_AutoRunning;
    tmpP.bFlg_Enable_TriggerMode = pJP->bFlg_Enable_TriggerMode;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seMode_AutoRunning));

    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_TriggerMode_Type
 *	Description : Convert Annulus parameters to All parameters
 *                for Trigger Mode Type
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_TriggerMode_Type(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_TriggerMode_Type
 *	Description : Convert Rectangle parameters to All parameters
 *                for Trigger Mode Type
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_TriggerMode_Type(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_TriggerMode_Type
 *	Description : Convert Json parameters to All parameters
 *                for Trigger Mode Type
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_TriggerMode_Type(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seMode_TriggerModeType *pJP = (seMode_TriggerModeType *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMode_TriggerModeType *pP = (seMode_TriggerModeType *)pCP->pParm;

    seMode_TriggerModeType tmpP;
    tmpP.bFlg_TriggerMode_Activate = pJP->bFlg_TriggerMode_Activate;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seMode_TriggerModeType));

    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECam_Initialize
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Initialize
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECam_Initialize(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECam_Initialize
 *	Description : Convert Rectangle parameters to All parameters
 *                for GigECam Initialize
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECam_Initialize(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECam_Initialize
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Initialize
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECam_Initialize(void *pJsonParam, void *ptrParam)
{
    int res = 0;
    //
    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECam_Inquiry
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Inquiry
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECam_Inquiry(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECam_Inquiry
 *	Description : Convert Rectangle parameters to All parameters
 *                for GigECam Inquiry
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECam_Inquiry(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECam_Inquiry
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Inquiry
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECam_Inquiry(void *pJsonParam, void *ptrParam)
{
    int res = 0;
    //
    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECam_Config
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Config
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECam_Config(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtAP2P_GigECam_Config
 *	Description : Convert Rectange parameters to All parameters
 *                for GigECam Config
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECam_Config(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECam_Config
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Config
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECam_Config(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seGigECamConfig *pJP = (seGigECamConfig *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seGigECamConfig *pP = (seGigECamConfig *)pCP->pParm;

    seGigECamConfig tmpP;

    tmpP.bPixelFormat = pJP->bPixelFormat;
    tmpP.iOffset_X = pJP->iOffset_X;
    tmpP.iOffset_Y = pJP->iOffset_Y;
    tmpP.iWidth = pJP->iWidth;
    tmpP.iHeight = pJP->iHeight;

    tmpP.iBinning_Scale = pJP->iBinning_Scale;

    tmpP.dbExposureTime = pJP->dbExposureTime;
    tmpP.bExposureAuto = pJP->bExposureAuto;

    tmpP.bIsEnbTriggerMode = pJP->bIsEnbTriggerMode;
    tmpP.iTriggerActivation = pJP->iTriggerActivation;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seGigECamConfig));

    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECam_Capture
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Capture
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECam_Capture(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECam_Capture
 *	Description : Convert Rectangle parameters to All parameters
 *                for GigECam Capture
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECam_Capture(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECam_Capture
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Capture
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECam_Capture(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seGigECamConfig *pJP = (seGigECamConfig *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seGigECamConfig *pP = (seGigECamConfig *)pCP->pParm;

    seGigECamConfig tmpP;

    tmpP.bIsEnbReadImageMode = pJP->bIsEnbReadImageMode;

    tmpP.bPixelFormat = pJP->bPixelFormat;
    tmpP.iOffset_X = pJP->iOffset_X;
    tmpP.iOffset_Y = pJP->iOffset_Y;
    tmpP.iWidth = pJP->iWidth;
    tmpP.iHeight = pJP->iHeight;

    tmpP.iBinning_Scale = pJP->iBinning_Scale;

    tmpP.dbExposureTime = pJP->dbExposureTime;
    tmpP.bExposureAuto = pJP->bExposureAuto;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seGigECamConfig));

    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECam_Release
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Release
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECam_Release(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECam_Release
 *	Description : Convert Rectangle parameters to All parameters
 *                for GigECam Release
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECam_Release(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECam_Release
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Release
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECam_Release(void *pJsonParam, void *ptrParam)
{
    int res = 0;
    //
    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECamStrm_Initialize
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Streaming Initialize
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECamStrm_Initialize(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECamStrm_Initialize
 *	Description : Convert Rectangle parameters to All parameters
 *                for GigECam Streaming Initialize
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECamStrm_Initialize(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECamStrm_Initialize
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Streaming Initialize
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECamStrm_Initialize(void *pJsonParam, void *ptrParam)
{
    int res = 0;
    //
    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECamStrm_Inquiry
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Streaming Inquiry
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECamStrm_Inquiry(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECamStrm_Inquiry
 *	Description : Convert Rectangle parameters to All parameters
 *                for GigECam Streaming Inquiry
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECamStrm_Inquiry(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECamStrm_Inquiry
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Streaming Inquiry
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECamStrm_Inquiry(void *pJsonParam, void *ptrParam)
{
    int res = 0;
    //
    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECamStrm_Start
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Streaming Start
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECamStrm_Start(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECamStrm_Start
 *	Description : Convert Rectange parameters to All parameters
 *                for GigECam Streaming Start
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECamStrm_Start(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECamStrm_Start
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Streaming Start
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECamStrm_Start(void *pJsonParam, void *ptrParam)
{
    int res = 0;
    //
    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECamStrm_Capture
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Streaming Capture
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECamStrm_Capture(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECamStrm_Capture
 *	Description : Convert Rectange parameters to All parameters
 *                for GigECam Streaming Capture
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECamStrm_Capture(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECamStrm_Capture
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Streaming Capture
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECamStrm_Capture(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seGigECamConfig *pJP = (seGigECamConfig *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seGigECamConfig *pP = (seGigECamConfig *)pCP->pParm;

    seGigECamConfig tmpP;

    pCP->strSaveImgPath = pJP->strSaveImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seGigECamConfig));

    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECamStrm_Stop
 *	Description : Convert Annulus parameters to All parameters
 *                for GigECam Streaming Stop
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECamStrm_Stop(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECamStrm_Stop
 *	Description : Convert Rectange parameters to All parameters
 *                for GigECam Streaming Stop
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECamStrm_Stop(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECamStrm_Stop
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Streaming Stop
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECamStrm_Stop(void *pJsonParam, void *ptrParam)
{
    int res = 0;
    //
    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_GigECamStrm_Release
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Streaming Release
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_GigECamStrm_Release(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_GigECamStrm_Release
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Streaming Release
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_GigECamStrm_Release(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_GigECamStrm_Release
 *	Description : Convert Json parameters to All parameters
 *                for GigECam Streaming Release
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_GigECamStrm_Release(void *pJsonParam, void *ptrParam)
{
    int res = 0;
    //
    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Image_Calibration
 *	Description : Convert Annulus parameters to All parameters
 *                for Image Calibration
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Image_Calibration(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Image_Calibration
 *	Description : Convert Rectange parameters to All parameters
 *                for Image Calibration
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Image_Calibration(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Image_Calibration
 *	Description : Convert Json parameters to All parameters
 *                for Image Calibration
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Image_Calibration(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seMth_ImageCalibration *pJP = (seMth_ImageCalibration *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMth_ImageCalibration *pP = (seMth_ImageCalibration *)pCP->pParm;

    seMth_ImageCalibration tmpP;

    tmpP.dbE2E_Distance_mm = pJP->dbE2E_Distance_mm;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seMth_ImageCalibration));

    return res;
}

////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Crop_GoldenTemplate
 *	Description : Convert Annulus parameters to All parameters
 *                for Golden Template
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Crop_GoldenTemplate(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Crop_GoldenTemplate
 *	Description : Convert Rectange parameters to All parameters
 *                for Golden Template
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Crop_GoldenTemplate(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Crop_GoldenTemplate
 *	Description : Convert Json parameters to All parameters
 *                for Golden Template
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Crop_GoldenTemplate(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seCropROI_GTemplate *pJP = (seCropROI_GTemplate *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seCropROI_GTemplate *pP = (seCropROI_GTemplate *)pCP->pParm;

    seCropROI_GTemplate tmpP;

    tmpP.roiRect = pJP->roiRect;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seCropROI_GTemplate));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_PatternMatch
 *	Description : Convert Annulus parameters to All parameters
 *                for Pattern Match
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_PatternMatch(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtAP2P_PatternMatch
 *	Description : Convert Rectange parameters to All parameters
 *                for Pattern Match
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_PatternMatch(void *pResult, void *pAllParam)
{
    int res = 0;

    seMth_PatternMatch_Ret *pRet = (seMth_PatternMatch_Ret *)pResult;
    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    pAP->ret_FMark_PatternMatch.seFMarkBoxOut = pRet->seFMarkBoxOut;

    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_PatternMatch
 *	Description : Convert Json parameters to All parameters
 *                for Pattern Match
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_PatternMatch(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seMth_PatternMatch *pJP = (seMth_PatternMatch *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMth_PatternMatch *pP = (seMth_PatternMatch *)pCP->pParm;
    //
    seMth_PatternMatch tmpP;

    tmpP.roiSearch = pJP->roiSearch;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strTemplateImgPath = pJP->strTemplateImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seMth_PatternMatch));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_FindProfile
 *	Description : Convert Annulus parameters to All parameters
 *                for Find Profile
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_FindProfile(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_FindProfile
 *	Description : Convert Rectange parameters to All parameters
 *                for Find Profile
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_FindProfile(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_FindProfile
 *	Description : Convert Json parameters to All parameters
 *                for Find Profile
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_FindProfile(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seMth_FindProfile *pJP = (seMth_FindProfile *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMth_FindProfile *pP = (seMth_FindProfile *)pCP->pParm;

    seMth_FindProfile tmpP;

    tmpP.roiSearch = pJP->roiSearch;
    tmpP.roiMask = pJP->roiMask;
    tmpP.bDirection = pJP->bDirection;
    tmpP.bPolarity = pJP->bPolarity;
    tmpP.stepSize = pJP->stepSize;
    tmpP.iKSize = pJP->iKSize;
    tmpP.iSelLineNo = pJP->iSelLineNo;

    pCP->strInputImgPath = pJP->strInputImgPath;
    // pCP->strTemplateImgPath = pJP->strTemplateImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seMth_FindProfile));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_DetectCircle
 *	Description : Convert Annulus parameters to All parameters
 *                for Detect Circle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_DetectCircle(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_DetectCircle
 *	Description : Convert Rectange parameters to All parameters
 *                for Detect Circle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_DetectCircle(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_DetectCircle
 *	Description : Convert Json parameters to All parameters
 *                for Detect Circle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_DetectCircle(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seMth_DetectCircle *pJP = (seMth_DetectCircle *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMth_DetectCircle *pP = (seMth_DetectCircle *)pCP->pParm;

    seMth_DetectCircle tmpP;

    tmpP.roiSearch = pJP->roiSearch;
    tmpP.roiMask = pJP->roiMask;
    tmpP.bDirection = pJP->bDirection;
    tmpP.bPolarity = pJP->bPolarity;
    tmpP.stepSize = pJP->stepSize;
    tmpP.iMinEdgeStrength = pJP->iMinEdgeStrength;
    tmpP.iKSize = pJP->iKSize;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seMth_DetectCircle));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_IBOX_Annulus
 *	Description : Convert Annulus parameters to All parameters
 *                for Detect Box (Annulus)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_IBOX_Annulus(void *ptrParam, void *pAllParam)
{
    int res = 0;

    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seIBox_Annulus *pP = (seIBox_Annulus *)pCP->pParm;

    pAP->set_IBox_Annulus.roiAnnulus = pP->roiAnnulus;

    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_IBOX_Annulus
 *	Description : Convert Rectange parameters to All parameters
 *                for Detect Box (Annulus)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_IBOX_Annulus(void *pResult, void *pAllParam)
{
    int res = 0;

    seIBox_Annulus_Ret *pRet = (seIBox_Annulus_Ret *)pResult;
    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    pAP->ret_BBox_Annulus.seBoundBoxOut = pRet->seBoundBoxOut;

    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_IBOX_Annulus
 *	Description : Convert Json parameters to All parameters
 *                for Detect Box (Annulus)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_IBOX_Annulus(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seIBox_Annulus *pJP = (seIBox_Annulus *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seIBox_Annulus *pP = (seIBox_Annulus *)pCP->pParm;

    seIBox_Annulus tmpP;

    tmpP.roiAnnulus = pJP->roiAnnulus;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seIBox_Annulus));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_IBOX_Rect
 *	Description : Convert Annulus parameters to All parameters
 *                for Detect Box (Rectangle)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_IBOX_Rect(void *ptrParam, void *pAllParam)
{
    int res = 0;

    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seIBox_Rect *pP = (seIBox_Rect *)pCP->pParm;

    pAP->set_IBox_Rect.roiRect = pP->roiRect;

    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_IBOX_Rect
 *	Description : Convert Rectange parameters to All parameters
 *                for Detect Box (Rectangle)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_IBOX_Rect(void *pResult, void *pAllParam)
{
    int res = 0;

    seIBox_Rect_Ret *pRet = (seIBox_Rect_Ret *)pResult;
    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    pAP->ret_BBox_Rect.seBoundBoxOut = pRet->seBoundBoxOut;

    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_IBOX_Rect
 *	Description : Convert Json parameters to All parameters
 *                for Detect Box (Rectangle)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_IBOX_Rect(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seIBox_Rect *pJP = (seIBox_Rect *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seIBox_Rect *pP = (seIBox_Rect *)pCP->pParm;

    seIBox_Rect tmpP;

    tmpP.roiRect = pJP->roiRect;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seIBox_Rect));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_IBOX_Circle
 *	Description : Convert Annulus parameters to All parameters
 *                for Detect Box (Circle)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_IBOX_Circle(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_IBOX_Circle
 *	Description : Convert Rectange parameters to All parameters
 *                for Detect Box (Circle)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_IBOX_Circle(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtAP2P_IBOX_Circle
 *	Description : Convert Json parameters to All parameters
 *                for Detect Box (Circle)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_IBOX_Circle(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seIBox_Circle *pJP = (seIBox_Circle *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seIBox_Circle *pP = (seIBox_Circle *)pCP->pParm;

    seIBox_Circle tmpP;

    tmpP.roiCircle = pJP->roiCircle;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seIBox_Circle));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_CalcCoord
 *	Description : Convert Annulus parameters to All parameters
 *                for Calculate the coordinate
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_CalcCoord(void *ptrParam, void *pAllParam)
{
    int res = 0;

    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seCalcCoord *pP = (seCalcCoord *)pCP->pParm;

    pP->seFMarkBox = pAP->ret_FMark_PatternMatch.seFMarkBoxOut;
    pP->seInspBox = pAP->ret_BBox_Annulus.seBoundBoxOut;

    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_CalcCoord
 *	Description : Convert Rectange parameters to All parameters
 *                for Calculate the coordinate
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_CalcCoord(void *pResult, void *pAllParam)
{
    int res = 0;

    seCalcCoord_Ret *pRet = (seCalcCoord_Ret *)pResult;
    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    pAP->ret_CoBBox_CalcCoord.seCoorBindBoxOut = pRet->seCoorBindBoxOut;

    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_CalcCoord
 *	Description : Convert Json parameters to All parameters
 *                for Calculate the coordinate
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_CalcCoord(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seCalcCoord *pJP = (seCalcCoord *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seCalcCoord *pP = (seCalcCoord *)pCP->pParm;

    seCalcCoord tmpP;
    tmpP.seFMarkBox = pJP->seFMarkBox;
    tmpP.seInspBox = pJP->seInspBox;
    tmpP.seCoorBindBoxIn = pJP->seCoorBindBoxIn;

    pCP->strInputImgPath = pJP->strInputImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seCalcCoord));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Crop_Annulus
 *	Description : Convert Annulus parameters to All parameters
 *                for Crop Annulus
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Crop_Annulus(void *ptrParam, void *pAllParam)
{
    int res = 0;

    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seCropROI_Annulus *pP = (seCropROI_Annulus *)pCP->pParm;

    pP->seCoordBox = pAP->ret_CoBBox_CalcCoord.seCoorBindBoxOut;
    pP->roiAnnulus = pAP->set_IBox_Annulus.roiAnnulus;

    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Crop_Annulus
 *	Description : Convert Rectange parameters to All parameters
 *                for Crop Annulus
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Crop_Annulus(void *pResult, void *pAllParam)
{
    int res = 0;

    seCropROI_Annulus_Ret *pRet = (seCropROI_Annulus_Ret *)pResult;
    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    pAP->ret_CropImg_Annulu.seRoiOffset_Annulus = pRet->seRoiOffset_Annulus;

    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Crop_Annulus
 *	Description : Convert Json parameters to All parameters
 *                for Crop Annulus
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Crop_Annulus(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seCropROI_Annulus *pJP = (seCropROI_Annulus *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seCropROI_Annulus *pP = (seCropROI_Annulus *)pCP->pParm;

    seCropROI_Annulus tmpP;

    tmpP.seCoordBox = pJP->seCoordBox;
    tmpP.roiAnnulus = pJP->roiAnnulus;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seCropROI_Annulus));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Crop_Rect
 *	Description : Convert Annulus parameters to All parameters
 *                for Crop Rectangle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Crop_Rect(void *ptrParam, void *pAllParam)
{
    int res = 0;

    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seCropROI_Rect *pP = (seCropROI_Rect *)pCP->pParm;

    pP->seCoordBox = pAP->ret_CoBBox_CalcCoord.seCoorBindBoxOut;
    pP->roiRect = pAP->set_IBox_Rect.roiRect;

    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Crop_Rect
 *	Description : Convert Rectangle parameters to All parameters
 *                for Crop Rectangle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Crop_Rect(void *pResult, void *pAllParam)
{
    int res = 0;

    seCropROI_Rect_Ret *pRet = (seCropROI_Rect_Ret *)pResult;
    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    pAP->ret_CropImg_Rect.seRoiOffset_Rect = pRet->seRoiOffset_Rect;

    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Crop_Rect
 *	Description : Convert Json parameters to All parameters
 *                for Crop Rectangle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Crop_Rect(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seCropROI_Rect *pJP = (seCropROI_Rect *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seCropROI_Rect *pP = (seCropROI_Rect *)pCP->pParm;

    seCropROI_Rect tmpP;

    tmpP.seCoordBox = pJP->seCoordBox;
    tmpP.roiRect = pJP->roiRect;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seCropROI_Rect));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Crop_Circle
 *	Description : Convert Annulus parameters to All parameters
 *                for Crop Circle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Crop_Circle(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Crop_Circle
 *	Description : Convert Rectange parameters to All parameters
 *                for Crop Circle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Crop_Circle(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Crop_Circle
 *	Description : Convert Json parameters to All parameters
 *                for Crop Circle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Crop_Circle(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seCropROI_Circle *pJP = (seCropROI_Circle *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seCropROI_Circle *pP = (seCropROI_Circle *)pCP->pParm;

    seCropROI_Circle tmpP;

    tmpP.seCoordBox = pJP->seCoordBox;
    tmpP.roiCircle = pJP->roiCircle;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seCropROI_Circle));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Histogram_Annulus
 *	Description : Convert Annulus parameters to All parameters
 *                for Histogram Annulus
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Histogram_Annulus(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Histogram_Annulus
 *	Description : Convert Rectange parameters to All parameters
 *                for Histogram Annulus
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Histogram_Annulus(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Histogram_Annulus
 *	Description : Convert Json parameters to All parameters
 *                for Histogram Annulus
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Histogram_Annulus(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seHisg_Annulus *pJP = (seHisg_Annulus *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seHisg_Annulus *pP = (seHisg_Annulus *)pCP->pParm;

    seHisg_Annulus tmpP;

    tmpP.roiAnnulus = pJP->roiAnnulus;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seHisg_Annulus));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Histogram_Rect
 *	Description : Convert Annulus parameters to All parameters
 *                for Histogram Rectange
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Histogram_Rect(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Histogram_Rect
 *	Description : Convert Rectange parameters to All parameters
 *                for Histogram Rectange
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Histogram_Rect(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Histogram_Rect
 *	Description : Convert Json parameters to All parameters
 *                for Histogram Rectange
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Histogram_Rect(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seHisg_Rect *pJP = (seHisg_Rect *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seHisg_Rect *pP = (seHisg_Rect *)pCP->pParm;

    seHisg_Rect tmpP;

    tmpP.roiRect = pJP->roiRect;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seHisg_Rect));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Histogram_Circle
 *	Description : Convert Annulus parameters to All parameters
 *                for Histogram Circle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Histogram_Circle(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Histogram_Circle
 *	Description : Convert Rectange parameters to All parameters
 *                for Histogram Circle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Histogram_Circle(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Histogram_Circle
 *	Description : Convert Json parameters to All parameters
 *                for Histogram Circle
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Histogram_Circle(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seHisg_Circle *pJP = (seHisg_Circle *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seHisg_Circle *pP = (seHisg_Circle *)pCP->pParm;

    seHisg_Circle tmpP;

    tmpP.roiCircle = pJP->roiCircle;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seHisg_Circle));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Threshold
 *	Description : Convert Annulus parameters to All parameters
 *                for Binary Thresholding
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Threshold(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Threshold
 *	Description : Convert Rectange parameters to All parameters
 *                for Binary Thresholding
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Threshold(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Threshold
 *	Description : Convert Json parameters to All parameters
 *                for Binary Thresholding
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Threshold(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seIP_Threshold *pJP = (seIP_Threshold *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seIP_Threshold *pP = (seIP_Threshold *)pCP->pParm;

    seIP_Threshold tmpP;

    tmpP.dbThresh = pJP->dbThresh;
    tmpP.dbMaxVal = pJP->dbMaxVal;
    tmpP.emTypes = pJP->emTypes;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seIP_Threshold));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_Mrophology
 *	Description : Convert Annulus parameters to All parameters
 *                for Mrophology
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_Mrophology(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_Mrophology
 *	Description : Convert Rectange parameters to All parameters
 *                for Mrophology
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_Mrophology(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_Mrophology
 *	Description : Convert Json parameters to All parameters
 *                for Mrophology
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_Mrophology(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seMorphology *pJP = (seMorphology *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMorphology *pP = (seMorphology *)pCP->pParm;

    seMorphology tmpP;

    tmpP.emShapes = pJP->emShapes;
    tmpP.iKSize = pJP->iKSize;
    tmpP.emOperation = pJP->emOperation;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seMorphology));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_NoiseRemoval
 *	Description : Convert Annulus parameters to All parameters
 *                for Noise Removal
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_NoiseRemoval(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_NoiseRemoval
 *	Description : Convert Rectange parameters to All parameters
 *                for Noise Removal
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_NoiseRemoval(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_NoiseRemoval
 *	Description : Convert Json parameters to All parameters
 *                for Noise Removal
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_NoiseRemoval(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seNoiseRemoval *pJP = (seNoiseRemoval *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seNoiseRemoval *pP = (seNoiseRemoval *)pCP->pParm;

    seNoiseRemoval tmpP;

    tmpP.dbLimit_min = pJP->dbLimit_min;
    tmpP.dbLimit_max = pJP->dbLimit_max;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seNoiseRemoval));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_DataAugmentation
 *	Description : Convert Annulus parameters to All parameters
 *                for Data Augmentation
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_DataAugmentation(void *ptrParam, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_DataAugmentation
 *	Description : Convert Rectange parameters to All parameters
 *                for Data Augmentation
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_DataAugmentation(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_DataAugmentation
 *	Description : Convert Json parameters to All parameters
 *                for Data Augmentation
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_DataAugmentation(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seDataAugmentation *pJP = (seDataAugmentation *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seDataAugmentation *pP = (seDataAugmentation *)pCP->pParm;

    seDataAugmentation tmpP;

    tmpP.seDA_Param = pJP->seDA_Param;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seDataAugmentation));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_MeasGW_Annulus
 *	Description : Convert Annulus parameters to All parameters
 *                for Measure Glue Width (Annulus)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_MeasGW_Annulus(void *ptrParam, void *pAllParam)
{
    int res = 0;

    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMeasGW_Annulus *pP = (seMeasGW_Annulus *)pCP->pParm;

    pP->roiAnnuls = pAP->ret_CropImg_Annulu.seRoiOffset_Annulus;

    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_MeasGW_Annulus
 *	Description : Convert Rectange parameters to All parameters
 *                for Measure Glue Width (Annulus)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_MeasGW_Annulus(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_MeasGW_Annulus
 *	Description : Convert Json parameters to All parameters
 *                for Measure Glue Width (Annulus)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_MeasGW_Annulus(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seMeasGW_Annulus *pJP = (seMeasGW_Annulus *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMeasGW_Annulus *pP = (seMeasGW_Annulus *)pCP->pParm;

    seMeasGW_Annulus tmpP;

    tmpP.roiAnnuls = pJP->roiAnnuls;
    tmpP.stepSize = pJP->stepSize;
    tmpP.dbmm_per_pixel = pJP->dbmm_per_pixel;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seMeasGW_Annulus));

    return res;
}

////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: cvtAP2P_MeasGW_Rect
 *	Description : Convert Annulus parameters to All parameters
 *                for Measure Glue Width (Rectange)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtAP2P_MeasGW_Rect(void *ptrParam, void *pAllParam)
{
    int res = 0;

    seAllParamTable_MeasGW_Annulus *pAP = (seAllParamTable_MeasGW_Annulus *)pAllParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMeasGW_Rect *pP = (seMeasGW_Rect *)pCP->pParm;

    pP->roiRect = pAP->ret_CropImg_Rect.seRoiOffset_Rect;

    return res;
}

/***********************************************************
 *	Function 	: cvtRet2AP_MeasGW_Rect
 *	Description : Convert Rectange parameters to All parameters
 *                for Measure Glue Width (Rectange)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtRet2AP_MeasGW_Rect(void *pResult, void *pAllParam)
{
    int res = 0;
    //
    return res;
}

/***********************************************************
 *	Function 	: cvtJP2P_MeasGW_Rect
 *	Description : Convert Json parameters to All parameters
 *                for Measure Glue Width (Rectange)
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int cvtJP2P_MeasGW_Rect(void *pJsonParam, void *ptrParam)
{
    int res = 0;

    seMeasGW_Rect *pJP = (seMeasGW_Rect *)pJsonParam;

    CAlgoMethodParametr *pCP = (CAlgoMethodParametr *)ptrParam;

    seMeasGW_Rect *pP = (seMeasGW_Rect *)pCP->pParm;

    seMeasGW_Rect tmpP;

    tmpP.roiRect = pJP->roiRect;
    tmpP.stepSize = pJP->stepSize;
    tmpP.dbmm_per_pixel = pJP->dbmm_per_pixel;

    pCP->strInputImgPath = pJP->strInputImgPath;
    pCP->strSaveImgPath = pJP->strSaveImgPath;
    pCP->strResultImgPath = pJP->strResultImgPath;

    //*pP = tmpP;
    memcpy(pP, &tmpP, sizeof(seMeasGW_Rect));

    return res;
}

////////////////////////////////////////////////////////////////////////////////////
// Parameter convert function define
////////////////////////////////////////////////////////////////////////////////////

int (*ParamCvt_AUTO_RUNNING[3])(void *pParam, void *pAllParam) = {cvtAP2P_AUTO_RUNNING, cvtRet2AP_AUTO_RUNNING, cvtJP2P_AUTO_RUNNING};

int (*ParamCvt_TriggerMode_Type[3])(void *pParam, void *pAllParam) = {cvtAP2P_TriggerMode_Type, cvtRet2AP_TriggerMode_Type, cvtJP2P_TriggerMode_Type};

int (*ParamCvt_GigECam_Initialize[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECam_Initialize, cvtRet2AP_GigECam_Initialize, cvtJP2P_GigECam_Initialize};
int (*ParamCvt_GigECam_Inquiry[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECam_Inquiry, cvtRet2AP_GigECam_Inquiry, cvtJP2P_GigECam_Inquiry};
int (*ParamCvt_GigECam_Config[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECam_Config, cvtRet2AP_GigECam_Config, cvtJP2P_GigECam_Config};
int (*ParamCvt_GigECam_Capture[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECam_Capture, cvtRet2AP_GigECam_Capture, cvtJP2P_GigECam_Capture};
int (*ParamCvt_GigECam_Release[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECam_Release, cvtRet2AP_GigECam_Release, cvtJP2P_GigECam_Release};

int (*ParamCvt_GigECamStrm_Initialize[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECamStrm_Initialize, cvtRet2AP_GigECamStrm_Initialize, cvtJP2P_GigECamStrm_Initialize};
int (*ParamCvt_GigECamStrm_Inquiry[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECamStrm_Inquiry, cvtRet2AP_GigECamStrm_Inquiry, cvtJP2P_GigECamStrm_Inquiry};
int (*ParamCvt_GigECamStrm_Start[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECamStrm_Start, cvtRet2AP_GigECamStrm_Start, cvtJP2P_GigECamStrm_Start};
int (*ParamCvt_GigECamStrm_Capture[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECamStrm_Capture, cvtRet2AP_GigECamStrm_Capture, cvtJP2P_GigECamStrm_Capture};
int (*ParamCvt_GigECamStrm_Stop[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECamStrm_Stop, cvtRet2AP_GigECamStrm_Stop, cvtJP2P_GigECamStrm_Stop};
int (*ParamCvt_GigECamStrm_Release[3])(void *pParam, void *pAllParam) = {cvtAP2P_GigECamStrm_Release, cvtRet2AP_GigECamStrm_Release, cvtJP2P_GigECamStrm_Release};

int (*ParamCvt_Image_Calibration[3])(void *pParam, void *pAllParam) = {cvtAP2P_Image_Calibration, cvtRet2AP_Image_Calibration, cvtJP2P_Image_Calibration};

int (*ParamCvt_Crop_GoldenTemplate[3])(void *pParam, void *pAllParam) = {cvtAP2P_Crop_GoldenTemplate, cvtRet2AP_Crop_GoldenTemplate, cvtJP2P_Crop_GoldenTemplate};

int (*ParamCvt_PatternMatch[3])(void *pParam, void *pAllParam) = {cvtAP2P_PatternMatch, cvtRet2AP_PatternMatch, cvtJP2P_PatternMatch};
int (*ParamCvt_FindProfile[3])(void *pParam, void *pAllParam) = {cvtAP2P_FindProfile, cvtRet2AP_FindProfile, cvtJP2P_FindProfile};
int (*ParamCvt_DetectCircle[3])(void *pParam, void *pAllParam) = {cvtAP2P_DetectCircle, cvtRet2AP_DetectCircle, cvtJP2P_DetectCircle};

int (*ParamCvt_IBOX_Annulus[3])(void *pParam, void *pAllParam) = {cvtAP2P_IBOX_Annulus, cvtRet2AP_IBOX_Annulus, cvtJP2P_IBOX_Annulus};
int (*ParamCvt_IBOX_Rect[3])(void *pParam, void *pAllParam) = {cvtAP2P_IBOX_Rect, cvtRet2AP_IBOX_Rect, cvtJP2P_IBOX_Rect};
int (*ParamCvt_IBOX_Circle[3])(void *pParam, void *pAllParam) = {cvtAP2P_IBOX_Circle, cvtRet2AP_IBOX_Circle, cvtJP2P_IBOX_Circle};

int (*ParamCvt_CalcCoord[3])(void *pParam, void *pAllParam) = {cvtAP2P_CalcCoord, cvtRet2AP_CalcCoord, cvtJP2P_CalcCoord};

int (*ParamCvt_Crop_Annulus[3])(void *pParam, void *pAllParam) = {cvtAP2P_Crop_Annulus, cvtRet2AP_Crop_Annulus, cvtJP2P_Crop_Annulus};
int (*ParamCvt_Crop_Rect[3])(void *pParam, void *pAllParam) = {cvtAP2P_Crop_Rect, cvtRet2AP_Crop_Rect, cvtJP2P_Crop_Rect};
int (*ParamCvt_Crop_Circle[3])(void *pParam, void *pAllParam) = {cvtAP2P_Crop_Circle, cvtRet2AP_Crop_Circle, cvtJP2P_Crop_Circle};

int (*ParamCvt_Histogram_Annulus[3])(void *pParam, void *pAllParam) = {cvtAP2P_Histogram_Annulus, cvtRet2AP_Histogram_Annulus, cvtJP2P_Histogram_Annulus};
int (*ParamCvt_Histogram_Rect[3])(void *pParam, void *pAllParam) = {cvtAP2P_Histogram_Rect, cvtRet2AP_Histogram_Rect, cvtJP2P_Histogram_Rect};
int (*ParamCvt_Histogram_Circle[3])(void *pParam, void *pAllParam) = {cvtAP2P_Histogram_Circle, cvtRet2AP_Histogram_Circle, cvtJP2P_Histogram_Circle};

int (*ParamCvt_Threshold[3])(void *pParam, void *pAllParam) = {cvtAP2P_Threshold, cvtRet2AP_Threshold, cvtJP2P_Threshold};
int (*ParamCvt_Mrophology[3])(void *pParam, void *pAllParam) = {cvtAP2P_Mrophology, cvtRet2AP_Mrophology, cvtJP2P_Mrophology};
int (*ParamCvt_NoiseRemoval[3])(void *pParam, void *pAllParam) = {cvtAP2P_NoiseRemoval, cvtRet2AP_NoiseRemoval, cvtJP2P_NoiseRemoval};

int (*ParamCvt_DataAugmentation[3])(void *pParam, void *pAllParam) = {cvtAP2P_DataAugmentation, cvtRet2AP_DataAugmentation, cvtJP2P_DataAugmentation};

int (*ParamCvt_MeasGW_Annulus[3])(void *pParam, void *pAllParam) = {cvtAP2P_MeasGW_Annulus, cvtRet2AP_MeasGW_Annulus, cvtJP2P_MeasGW_Annulus};
int (*ParamCvt_MeasGW_Rect[3])(void *pParam, void *pAllParam) = {cvtAP2P_MeasGW_Rect, cvtRet2AP_MeasGW_Rect, cvtJP2P_MeasGW_Rect};

////////////////////////////////////////////////////////////////////////////////////
// Algorithm Method Registor table
////////////////////////////////////////////////////////////////////////////////////

seAlgoMethodReg gAlgoMethodReg[] = {

    // VisionBox Mode
    {FLAGE_AUTO_RUNNING,
     enum_Subscribe_CAMReg[FLAGE_AUTO_RUNNING], nullptr, &gResult_AutoRunning, szJson_AutoRunningMode, Method_AutoRunning_Mode, JsonEditor_AutoRunning_Mode, &ParamCvt_AUTO_RUNNING},

    // TriggerMode Activate
    {FLAGE_TRIGGERMODETYPE,
     enum_Subscribe_CAMReg[FLAGE_TRIGGERMODETYPE], &gParm_TriggerModeType, &gResult_TriggerModeType, szJson_TriggerModeType, Method_TriggerMode_Type, JsonEditor_TriggerMode_Type, &ParamCvt_TriggerMode_Type},

    // GigE camera control. < Static Image >
    {METHOD_GigeCam_Initialize,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Initialize], &gInitialize_GigeCam, &gInitialize_GigeCam_Ret, szJson_GigeCamInit, Method_GigECameraCtl_Initialize, JsonEditor_GigECam_Initialize, &ParamCvt_GigECam_Initialize},
    {METHOD_GigeCam_Inquiry,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Inquiry], &gInquiry_GigeCam, &gInquiry_GigeCam_Ret, szJson_GigeCamInq, Method_GigECameraCtl_Inquiry, JsonEditor_GigECam_Inquiry, &ParamCvt_GigECam_Inquiry},
    {METHOD_GigeCam_Config,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Config], &gConfig_GigeCam, &gConfig_GigeCam_Ret, szJson_GigeCamCfg, Method_GigECameraCtl_Config, JsonEditor_GigECam_Config, &ParamCvt_GigECam_Config},
    {METHOD_GigeCam_Capture,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Capture], &gCapture_GigeCam, &gCapture_GigeCam_Ret, szJson_GigeCamParam, Method_GigECameraCtl_Capture, JsonEditor_GigECam_Parameter, &ParamCvt_GigECam_Capture},
    {METHOD_GigeCam_Release,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Release], &gRelease_GigeCam, &gRelease_GigeCam_Ret, szJson_GigeCamRelease, Method_GigECameraCtl_Release, JsonEditor_GigECam_Release, &ParamCvt_GigECam_Release},

    // GigE camera control. < Streaming >
    {METHOD_GigeCam_Streaming_Initialize,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Initialize], &gInitialize_GigeCamStrm, &gInitialize_GigeCamStrm_Ret, szJson_GigeCamStrmInit, Method_GigECameraStrm_Initialize, JsonEditor_GigECamStrm_Initialize, &ParamCvt_GigECamStrm_Initialize},
    {METHOD_GigeCam_Streaming_Inquiry,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Inquiry], &gInquiry_GigeCamStrm, &gInquiry_GigeCamStrm_Ret, szJson_GigeCamStrmInq, Method_GigECameraStrm_Inquiry, JsonEditor_GigECamStrm_Inquiry, &ParamCvt_GigECamStrm_Inquiry},
    {METHOD_GigeCam_Streaming_Start,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Start], &gStart_GigeCamStrm, &gStart_GigeCamStrm_Ret, szJson_GigeCamStrmStart, Method_GigECameraStrm_Start, JsonEditor_GigECamStrm_Start, &ParamCvt_GigECamStrm_Start},
    {METHOD_GigeCam_Streaming_Capture,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Capture], &gCapture_GigeCamStrm, &szJson_GigeCamStrmCapture, szJson_GigeCamStrmCapture, Method_GigECameraStrm_Capture, JsonEditor_GigECamStrm_Capture, &ParamCvt_GigECamStrm_Capture},
    {METHOD_GigeCam_Streaming_Stop,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Stop], &gStop_GigeCamStrm, &gStop_GigeCamStrm_Ret, szJson_GigeCamStrmStop, Method_GigECameraStrm_Stop, JsonEditor_GigECamStrm_Stop, &ParamCvt_GigECamStrm_Stop},
    {METHOD_GigeCam_Streaming_Release,
     enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Release], &gRelease_GigeCamStrm, &gRelease_GigeCamStrm_Ret, szJson_GigeCamStrmRelease, Method_GigECameraStrm_Release, JsonEditor_GigECamStrm_Release, &ParamCvt_GigECamStrm_Release},

    // IPL Algorithm control.
    //
    // Resolution <Step by Step function.>
    {ALGO_ImageCalibration,
     enum_Subscribe_CAMReg[ALGO_ImageCalibration], &gParm_Image_Calibration, &gResult_Image_Calibration, szJson_Image_Calibration, Method_ImageCalibration, JsonEditor_ImageCalibrations, &ParamCvt_Image_Calibration},

    // Template image <Step by Step function.>
    {ALGO_Crop_GoldenTemplate,
     enum_Subscribe_CAMReg[ALGO_Crop_GoldenTemplate], &gParm_Crop_GTemplate, &gParm_Crop_GTemplate_Ret, szJson_Crop_GTemplate, Method_Crop_GoldenTemplate, JsonEditor_Crop_GoldenTemplate, &ParamCvt_Crop_GoldenTemplate},

    // Alignment
    {ALGO_PatternMatch,
     enum_Subscribe_CAMReg[ALGO_PatternMatch], &gParm_PatternMatch, &gResult_PatternMatch, szJson_PatternMatch, Method_PatternMatch, JsonEditor_PatternMatch, &ParamCvt_PatternMatch},

    {ALGO_FindProfile,
     enum_Subscribe_CAMReg[ALGO_FindProfile], &gParm_FindProfile, &gResult_FindProfile, szJson_FindProfile, Method_FindProfile, JsonEditor_FindProfile, &ParamCvt_FindProfile},

    {ALGO_DetectCircle,
     enum_Subscribe_CAMReg[ALGO_DetectCircle], &gParm_DetectCircle, &gResult_DetectCircle, szJson_DetectCircle, Method_DetectCircle, JsonEditor_DetectCircle, &ParamCvt_DetectCircle},

    {ALGO_IBOX_Annulus,
     enum_Subscribe_CAMReg[ALGO_IBOX_Annulus], &gParm_IBox_Annulus, &gResult_IBox_Annulus, szJson_IBox_Annulus, Method_IBOX_Annulus, JsonEditor_IBOX_Annulus, &ParamCvt_IBOX_Annulus},

    {ALGO_IBOX_Rect,
     enum_Subscribe_CAMReg[ALGO_IBOX_Rect], &gParm_IBox_Rect, &gResult_IBox_Rect, szJson_IBox_Rect, Method_IBOX_Rect, JsonEditor_IBOX_Rect, &ParamCvt_IBOX_Rect},

    {ALGO_IBOX_Circle,
     enum_Subscribe_CAMReg[ALGO_IBOX_Circle], &gParm_IBox_Circle, &gResult_IBox_Circle, szJson_IBox_Circle, Method_IBOX_Circle, JsonEditor_IBOX_Circle, &ParamCvt_IBOX_Circle},

    {ALGO_CalcCoord,
     enum_Subscribe_CAMReg[ALGO_CalcCoord], &gParm_CalcCoord, &gResult_CalcCoord, szJson_CalcCoord, Method_CalcCoord, JsonEditor_CalcCoord, &ParamCvt_CalcCoord},

    {ALGO_Crop_Annulus,
     enum_Subscribe_CAMReg[ALGO_Crop_Annulus], &gParm_Crop_Annulus, &gResult_Crop_Annulus, szJson_Crop_Annulus, Method_CropImg_Annulus, JsonEditor_CropImg_Annulus, &ParamCvt_Crop_Annulus},

    {ALGO_Crop_Rect,
     enum_Subscribe_CAMReg[ALGO_Crop_Rect], &gParm_Crop_Rect, &gResult_Crop_Rect, szJson_Crop_Rect, Method_CropImg_Rect, JsonEditor_CropImg_Rect, &ParamCvt_Crop_Rect},

    {ALGO_Crop_Circle,
     enum_Subscribe_CAMReg[ALGO_Crop_Circle], &gParm_Crop_Circle, &gResult_Crop_Circle, szJson_Crop_Circle, Method_CropImg_Circle, JsonEditor_CropImg_Circle, &ParamCvt_Crop_Circle},

    {ALGO_Hisg_Annulus,
     enum_Subscribe_CAMReg[ALGO_Hisg_Annulus], &gParm_Hisg_Annulus, &gResult_Hisg_Annulus, szJson_Hisg_Annulus, Method_Histogram_Annulus, JsonEditor_Histogram_Annulus, &ParamCvt_Histogram_Annulus},

    {ALGO_Hisg_Rect,
     enum_Subscribe_CAMReg[ALGO_Hisg_Rect], &gParm_Hisg_Rect, &gResult_Hisg_Rect, szJson_Hisg_Rect, Method_Histogram_Rect, JsonEditor_Histogram_Rect, &ParamCvt_Histogram_Rect},

    {ALGO_Hisg_Circle,
     enum_Subscribe_CAMReg[ALGO_Hisg_Circle], &gParm_Hisg_Circle, &gResult_Hisg_Circle, szJson_Hisg_Circle, Method_Histogram_Circle, JsonEditor_Histogram_Circle, &ParamCvt_Histogram_Circle},

    {ALGO_IP_Threshold,
     enum_Subscribe_CAMReg[ALGO_IP_Threshold], &gParm_Threshold, &gResult_Threshold, szJson_Threshold, Method_Threshold, JsonEditor_Threshold, &ParamCvt_Threshold},

    {ALGO_IP_Morphology,
     enum_Subscribe_CAMReg[ALGO_IP_Morphology], &gParm_Morphology, &gResult_Morphology, szJson_Morphology, Method_Morphology, JsonEditor_Morphology, &ParamCvt_Mrophology},

    {ALGO_IP_NoiseRemoval,
     enum_Subscribe_CAMReg[ALGO_IP_NoiseRemoval], &gParm_NoiseRemoval, &gResult_NoiseRemoval, szJson_NoiseRemoval, Method_NoiseRemoval, JsonEditor_NoiseRemoval, &ParamCvt_NoiseRemoval},

    {ALGO_IP_DataAugmentation,
     enum_Subscribe_CAMReg[ALGO_IP_DataAugmentation], &gParm_DataAugmentation, &gResult_DataAugmentation, szJson_DataAugmentation, Method_DataAugmentation, JsonEditor_DataAugmentation, &ParamCvt_DataAugmentation},

    {ALGO_MeasGW_Annulus,
     enum_Subscribe_CAMReg[ALGO_MeasGW_Annulus], &gParm_MeasGW_Annulus, &gResult_MeasGW_Annulus, szJson_MeasGW_Annulus, Method_GlueWidthMeasure_Annulus, JsonEditor_GlueWidthMeasure_Annulus, &ParamCvt_MeasGW_Annulus},

    {ALGO_MeasGW_Rect,
     enum_Subscribe_CAMReg[ALGO_MeasGW_Rect], &gParm_MeasGW_Rect, &gResult_MeasGW_Rect, szJson_MeasGW_Rect, Method_GlueWidthMeasure_Rect, JsonEditor_GlueWidthMeasure_Rect, &ParamCvt_MeasGW_Rect},

    // The End of AlgoParam register
    {ENUM_ALGO_END, enum_Subscribe_CAMReg[ENUM_ALGO_END], nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}

};

////////////////////////////////////////////////////////////////////////////////////
// Algorithm Tasks assign function
////////////////////////////////////////////////////////////////////////////////////

static std::unordered_map<std::string, int> gHashMap_Method;

const int AlgoMthd_TblCnt = (sizeof(gAlgoMethodReg) / sizeof(gAlgoMethodReg[0]));

/***********************************************************
 *	Function 	: createHashMap_Method
 *	Description : Create Hash Map
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int createHashMap_Method()
{

    if (!gHashMap_Method.empty())
    {

        IPSLOG(0, " ## Method__The gHashMap_Method is already create.\n");
        return 0;
    }

    for (int i = 0; i < AlgoMthd_TblCnt; i++)
    {

        std::string strTmp = gAlgoMethodReg[i].strCmd;
// IPSLOG(0, "%s()%d: [%s].\n", __FUNCTION__, __LINE__, strTmp.c_str());
        gHashMap_Method[strTmp] = i;
    }

    return 0;
}

/***********************************************************
 *	Function 	: compareHashMap_Method
 *	Description : Compare Hash Map
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int compareHashMap_Method(std::string strKey)
{
    auto it = gHashMap_Method.find(strKey);

    if (it != gHashMap_Method.end())
    {

        IPSLOG(0, "%s()%d: ## Method__Element: %s, has ID: %d\n", __FUNCTION__, __LINE__, strKey.c_str(), it->second);
        return it->second;
    }
    else
    {

        IPSLOG(0, "%s()%d: ## Method__Element: %s, not found in the map.\n", __FUNCTION__, __LINE__, strKey.c_str());
        printf("%s()%d: ## Method__Element: %s, not found in the map.\n", __FUNCTION__, __LINE__, strKey.c_str());
        return -1;
    }
}

/***********************************************************
 *	Function 	: setAlgo_MethodAssign
 *	Description : Add the parameters of the algorithm method to the task queue
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int setAlgo_MethodAssign(const char *szKey, const char *szJsonArg) // , std::vector<seAlgoMethodReg>& vecTasks)
{
    IPSLOG(0, "\n\n[__%s__] : ===> \n", __func__);

    int ret = 0;

    seAlgoParamReg *pAlgoParam = nullptr;
    ParamCvtMethod ptrParamCvt;


    int i = compareHashMap_Method(szKey);
    IPSLOG(0, "%s()%d: ## ---------------> compareHashMap_Method = %d \n\n", __FUNCTION__, __LINE__, i);
    if (-1 != i)
    {
        IPSLOG(0, "[__%s__] : AlgoMethod_TblCnt ===> %02d\n", __func__, i);
        IPSLOG(0, "[__%s__] : AlgoMethod_strCmd ===> %s\n", __func__, gAlgoMethodReg[i].strCmd);
        IPSLOG(0, "[__%s__] : szJsonArg ===>\n %s\n", __func__, szJsonArg);

        pAlgoParam = &gAlgoParamReg[i];

        CAlgoMethodParametr cAlgoMthdParam;
        cAlgoMthdParam.mAlgoMethod = gAlgoMethodReg[i];

        strcpy(cAlgoMthdParam.mAlgoMethod.pJsonBuf, szJsonArg);

        // Copy the pParm of seAlgoParamReg to the seAlgoMethodReg;
        ptrParamCvt = cAlgoMthdParam.mAlgoMethod.ParamConverter;
        if (ptrParamCvt != nullptr)
        {
            (*(*ptrParamCvt)[2])(pAlgoParam->pParam, &cAlgoMthdParam);
        }

        TasksQ_EnQ(cAlgoMthdParam);
    }

    IPSLOG(0, "[__%s__] : <=== \n\n", __func__);

    return ret;
}

/***********************************************************
 *	Function 	: setAlgo_MethodAssign
 *	Description : Add the parameters of the algorithm method to the Dual Cameras task queue
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int setAlgo_MethodAssign_Dual(const char *szKey, const char *szJsonArg, const int iID)
{
    IPSLOG(0, "\n\n[__%s__] : ===> \n", __func__);

    int ret = 0;

    seAlgoParamReg *pAlgoParam = nullptr;
    ParamCvtMethod ptrParamCvt;

    int i = compareHashMap_Method(szKey);
    IPSLOG(0, "%s()%d: ## ---------------> compareHashMap_Method = %d \n\n", __FUNCTION__, __LINE__, i);
    printf("%s()%d: ## ---------------> compareHashMap_Method = %d \n\n", __FUNCTION__, __LINE__, i);

    if (-1 != i)
    {
        IPSLOG(0, "[__%s__] : AlgoMethod_TblCnt ===> %02d\n", __func__, i);
        IPSLOG(0, "[__%s__] : AlgoMethod_strCmd ===> %s\n", __func__, gAlgoMethodReg[i].strCmd);
        IPSLOG(0, "[__%s__] : szJsonArg ===>\n %s\n", __func__, szJsonArg);

        pAlgoParam = &gAlgoParamReg[i];

        CAlgoMethodParametr cAlgoMthdParam;
        cAlgoMthdParam.mAlgoMethod = gAlgoMethodReg[i];

        strcpy(cAlgoMthdParam.mAlgoMethod.pJsonBuf, szJsonArg);

        // Copy the pParm of seAlgoParamReg to the seAlgoMethodReg;
        ptrParamCvt = cAlgoMthdParam.mAlgoMethod.ParamConverter;
        if (ptrParamCvt != nullptr)
        {
            (*(*ptrParamCvt)[2])(pAlgoParam->pParam, &cAlgoMthdParam);
        }

        TasksQ_EnQ_Dual(cAlgoMthdParam, iID);

    }

    IPSLOG(0, "[__%s__] : <=== \n\n", __func__);

    return ret;
}