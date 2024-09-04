#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <json.h>
#include "../common.h"

#include <chrono>
#include <ctime>
#include <string>
#include <deque>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "IOS_CompFunction.h"


pthread_mutex_t _IO_JsonQ_lock = PTHREAD_MUTEX_INITIALIZER;
std::deque<seIO_JsonInfo> BufQueue_IO_Json;
    
extern int IO_AILighting_DutyCycle(char *channel, int value);
extern char trig_DinMode;
extern char trig_DinMode_Dual;

int IO_JsonQ_Init()
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_IO_JsonQ_lock, NULL);
    return res;
}

int IO_JsonQ_EnQ(seIO_JsonInfo seInfo)
{
    //IOSLOG(0, "\n\n[__%s__] %d: ===> \n", __func__, __LINE__);

    int res = 0;
    pthread_mutex_lock(&_IO_JsonQ_lock);

    BufQueue_IO_Json.push_back(seInfo);

    pthread_mutex_unlock(&_IO_JsonQ_lock);

    //IOSLOG(0, "\n\n[__%s__] %d: <=== \n", __func__, __LINE__);

    return res;
}

int IO_JsonQ_DeQ(seIO_JsonInfo *pInfo)
{
    //IOSLOG(0, "\n\n[__%s__] %d: ===> \n", __func__, __LINE__);

    int res = 0;
    pthread_mutex_lock(&_IO_JsonQ_lock);

    if (!BufQueue_IO_Json.empty())
    {

        *pInfo = *BufQueue_IO_Json.begin(); // [0] ;

        BufQueue_IO_Json.pop_front();
    }
    else
    {
        res = -1;
    }

    pthread_mutex_unlock(&_IO_JsonQ_lock);

    //IOSLOG(0, "\n\n[__%s__] %d: <=== \n", __func__, __LINE__);

    return res;
}

/***********************************************************
 *	Function 	: IO_JsonQ_GetSzie
 *	Description : To get queue: BufQueue_IO_Json size
 *	Param 		: None
 *	Return		: BufQueue_IO_Json size
 *************************************************************/
int IO_JsonQ_GetSzie()
{
    int res = 0;
    pthread_mutex_lock(&_IO_JsonQ_lock);
    res = BufQueue_IO_Json.size();
    pthread_mutex_unlock(&_IO_JsonQ_lock);
    return res;
}

/***********************************************************
 *	Function 	: IO_JsonQ_IsEmpty
 *	Description : To check BufQueue_IO_Json is empty or not
 *	Param 		: None
 *	Return		: true, false
 *************************************************************/
int IO_JsonQ_IsEmpty()
{
    int res = 0;
    pthread_mutex_lock(&_IO_JsonQ_lock);
    res = BufQueue_IO_Json.empty();
    pthread_mutex_unlock(&_IO_JsonQ_lock);
    return res;
}

int IO_MqttParse_TRIGGER_SET_PROCESS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }

    MAINLOG(0, "[MAIN] : It's my command : TRIGGER_SET_PROCESS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "inPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : inPin = %d\n", json_object_get_int(j_param));
            ios_trigger.inPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "inMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : inMode = %s\n", json_object_get_string(j_param));
            strcpy(ios_trigger.inMode, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outPin = %d\n", json_object_get_int(j_param));
            ios_trigger.outPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outDelay");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outDelay = %d\n", json_object_get_int64(j_param));
            ios_trigger.outDelay = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
    //MAINLOG(0, "[MAIN] : [%s][%s]\n", ios_CmdInfo, json_object_get_string(j_args));
    
    return 0;
}

int IO_MqttParse_TRIGGER_GET_PROCESS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }

    MAINLOG(0, "[MAIN] : It's my command : TRIGGER_GET_PROCESS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "inPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : inPin = %d\n", json_object_get_int(j_param));
            ios_trigger.inPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outPin = %d\n", json_object_get_int(j_param));
            ios_trigger.outPin = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_DIN_SET_PROCESS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }

    MAINLOG(0, "[MAIN] : It's my command : DIN_SET_PROCESS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "inPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : inPin = %d\n", json_object_get_int(j_param));
            ios_di.inPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "inMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : inMode = %s\n", json_object_get_string(j_param));
            strcpy(ios_di.inMode, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "inControlMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : inControlMode = %d\n", json_object_get_int(j_param));
            ios_di.inControlMode = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "inDelay");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : inDelay = %d\n", json_object_get_int64(j_param));
            ios_di.inDelay = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outPin = %d\n", json_object_get_int(j_param));
            ios_di.outPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outMode = %s\n", json_object_get_string(j_param));
            strcpy(ios_di.outMode, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outControlMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outControlMode = %d\n", json_object_get_int(j_param));
            ios_di.outControlMode = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outDelay");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outDelay = %d\n", json_object_get_int64(j_param));
            ios_di.outDelay = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "onoffSetting");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : onoffSetting = %d\n", json_object_get_int64(j_param));
            ios_di.onoffSetting = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_DIN_GET_PROCESS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }

    MAINLOG(0, "[MAIN] : It's my command : DIN_GET_PROCESS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "inPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : inPin = %d\n", json_object_get_int(j_param));
            ios_di.inPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outPin = %d\n", json_object_get_int(j_param));
            ios_di.outPin = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_IOS_GET_STATUS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }

    MAINLOG(0, "[MAIN] : It's my command : IOS_GET_STATUS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_LIGHT_SET_PWM(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }

    MAINLOG(0, "[MAIN] : It's my command : LIGHT_SET_PWM\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
       j_param = (struct json_object *)json_object_object_get(j_args, "inLight");
       if (j_param != nullptr)
       {
           ios_setpwm.inLight = json_object_get_int(j_param);
       }
       j_param = (struct json_object *)json_object_object_get(j_args, "value");
       if (j_param != nullptr)
       {
           MAINLOG(0, "[MAIN] : value = %d\n", json_object_get_int(j_param));
           ios_setpwm.value = json_object_get_int(j_param);
       }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_LIGHT_GET_PWM(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }

    MAINLOG(0, "[MAIN] : It's my command : LIGHT_GET_PWM\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "inLight");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : inLight = %d\n", json_object_get_int(j_param));
            ios_setpwm.inLight = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_LED_SET_PROCESS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : LED_SET_PROCESS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "outPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outPin = %d\n", json_object_get_int(j_param));
            ios_setled.outPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outMode = %s\n", json_object_get_string(j_param));
            strcpy(ios_setled.outMode, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outStatus");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outStatus = %s\n", json_object_get_string(j_param));
            strcpy(ios_setled.outStatus, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outBlinkDelay");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outBlinkDelay = %d\n", json_object_get_int64(j_param));
            ios_setled.outBlinkDelay = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "outOffDelay");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outOffDelay = %d\n", json_object_get_int64(j_param));
            ios_setled.outOffDelay = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_LED_GET_PROCESS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : LED_GET_PROCESS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "outPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : outPin = %d\n", json_object_get_int(j_param));
            ios_setled.outPin = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_LED_SET_MODE(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : LED_SET_MODE\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "LED");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : LED = %d\n", json_object_get_int(j_param));
            ios_setLedMode.led = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "LedMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : Led Mode = %s\n", json_object_get_string(j_param));
            strcpy((char *)ios_setLedMode.ledMode, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "Indication");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : Indication = %s\n", json_object_get_string(j_param));
            strcpy((char *)ios_setLedMode.Indication, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "Color");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : Color = %s\n", json_object_get_string(j_param));
            strcpy((char *)ios_setLedMode.Color, json_object_get_string(j_param));
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}


int IO_MqttParse_DIN_SET_MODE(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : DIN_SET_MODE\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "DinPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : DinPin = %d\n", json_object_get_int(j_param));
            ios_setDinMode.DinPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "DinActive");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : DinActive = %s\n", json_object_get_string(j_param));
            strcpy((char *)ios_setDinMode.DinPolarity, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "SelectMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : SelectMode = %s\n", json_object_get_string(j_param));
            strcpy((char *)ios_setDinMode.SelectMode, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "CameraId");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : CameraId = %d\n", json_object_get_int(j_param));
            char CameraId = json_object_get_int(j_param);
            
            if(CameraId == 1) {
                trig_DinMode = ios_setDinMode.DinPin;
            } else if(CameraId == 2) {
                trig_DinMode_Dual = ios_setDinMode.DinPin;
            } else {
                MAINLOG(0, "[MAIN] : Unknow CameraId [%d] fail.\n", CameraId);
            }
        }
        
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_DOUT_SET_MODE(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : DOUT_SET_MODE\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "DoutPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : DoutPin = %d\n", json_object_get_int(j_param));
            ios_doutMode.outPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "DoutActive");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : DoutActive = %s\n", json_object_get_string(j_param));
            strcpy(ios_doutMode.polarity, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "SelectMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : SelectMode = %s\n", json_object_get_string(j_param));
            strcpy(ios_doutMode.selectMode, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "CameraId");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : CameraId = %d\n", json_object_get_int(j_param));
            ios_doutMode.CameraId = json_object_get_int(j_param) - 1;
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_DOUT_MANUAL_CONTROL(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : DOUT_MANUAL_CONTROL\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "DoutPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : DoutPin = %d\n", json_object_get_int(j_param));
            ios_dout.outPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "DoutActive");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : DoutActive = %s\n", json_object_get_string(j_param));
            strcpy(ios_dout.outMode, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "onoffSetting");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : onoffSetting = %d\n", json_object_get_int(j_param));
            ios_dout.onoffSetting = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "SelectMode");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : SelectMode = %s\n", json_object_get_string(j_param));
            snprintf(&ios_dout.SelectMode[0], sizeof(ios_dout.SelectMode), "%s", json_object_get_string(j_param));
            // ** = json_object_get_int(j_param); => Do nothing Now. 2022/08/25 Jack Hung
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "OneShotPeriod");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : OneShotPeriod = %d\n", json_object_get_int(j_param));
            ios_dout.OneShotPeriod = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_DIO_GET_STATUS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : DIO_GET_STATUS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_LIGHT_SET_BRIGHTNESS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : LIGHT_SET_BRIGHTNESS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "lightSource");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : lightSource = %d\n", json_object_get_int(j_param));
            ios_setpwm.inLight = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "Brightness");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : Brightness = %d\n", json_object_get_int(j_param));
            ios_setpwm.value = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "Switch");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : Switch = %s\n", json_object_get_string(j_param));
            strcpy(ios_setpwm.LightSwitch, json_object_get_string(j_param));
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_LIGHT_GET_BRIGHTNESS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : LIGHT_GET_BRIGHTNESS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_CAMERA_STREAMING_CONTROL(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : CAMERA_STREAMING_CONTROL\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));

        j_param = (struct json_object *)json_object_object_get(j_args, "camera");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : camera = %d\n", json_object_get_int(j_param));
            main_process.camera = json_object_get_int(j_param);
        }

        j_param = (struct json_object *)json_object_object_get(j_args, "streaming");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : streaming = %d\n", json_object_get_int(j_param));
            if (strcmp(json_object_get_string(j_param), "ON") == 0)
                main_process.streaming = true;
            else if (strcmp(json_object_get_string(j_param), "OFF") == 0)
                main_process.streaming = false;
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}


int IO_MqttParse_MODBUS_SET_PARAMS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : MODBUS_SET_PARAMS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        j_param = (struct json_object *)json_object_object_get(j_args, "Baudrate");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : Baudrate = %d\n", json_object_get_int(j_param));
            ios_modbusSetParams.RTU_BAUDRATE = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "GITriggerRegister");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : GITriggerRegister = %s\n", json_object_get_string(j_param));
            ios_modbusSetParams.GI_Testing_Register = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "GIStatusRegister");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : GIStatusRegister = %s\n", json_object_get_string(j_param));
            ios_modbusSetParams.GI_Test_Status_Register = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "GIResultRegister");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : GIResultRegister = %s\n", json_object_get_string(j_param));
            ios_modbusSetParams.GI_Testing_Result_Register = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_MODBUS_GET_PARAMS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : MODBUS_GET_PARAMS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_TRIGGER_SET_MODE(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : TRIGGER_SET_MODE\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        
        j_param = (struct json_object *)json_object_object_get(j_args, "TriggerPin");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : TriggerPin = %d\n", json_object_get_int(j_param));
            ios_trigger.inPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "TriggerActive");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : TriggerActive = %s\n", json_object_get_string(j_param));
            strcpy(ios_trigger.inMode, json_object_get_string(j_param));
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "LightSource");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : LightSource = %d\n", json_object_get_int(j_param));
            ios_trigger.outPin = json_object_get_int(j_param);
        }
        j_param = (struct json_object *)json_object_object_get(j_args, "LightPeriod");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : LightPeriod = %d\n", json_object_get_int(j_param));
            ios_trigger.outDelay = json_object_get_int(j_param);
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_REPORT_TEST_RESULT(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : REPORT_TEST_RESULT\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        
        j_param = (struct json_object *)json_object_object_get(j_args, "Result");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : Result = %s\n", json_object_get_string(j_param));
            strcpy(Process_Node.TestResult, json_object_get_string(j_param));
        }
                
        j_param = (struct json_object *)json_object_object_get(j_args, "CameraId");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : CameraId = %s\n", json_object_get_string(j_param));
            char CameraId = json_object_get_int(j_param);
            if(CameraId == 1) {
                j_param = (struct json_object *)json_object_object_get(j_args, "Result");
                MAINLOG(0, "[MAIN] : Result = %s\n", json_object_get_string(j_param));
                strcpy(Process_Node_Dual.TestResult, json_object_get_string(j_param));
            } else if(CameraId == 2) {
                j_param = (struct json_object *)json_object_object_get(j_args, "Result");
                MAINLOG(0, "[MAIN] : Result = %s\n", json_object_get_string(j_param));
                strcpy(Process_Node_Dual.TestResult, json_object_get_string(j_param));
            } else {
                MAINLOG(0, "[MAIN] : Unknow CameraId [%d] fail.\n", CameraId);
            }
        }
                
        
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_AUTO_TEST_SET_MODE(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : AUTO_TEST_SET_MODE\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
        
        j_param = (struct json_object *)json_object_object_get(j_args, "onoffSetting");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : onoffSetting = %d\n", json_object_get_int(j_param));
            Process_Node_Dual.Active = Process_Node.Active = json_object_get_int(j_param);
        }
        
        j_param = (struct json_object *)json_object_object_get(j_args, "CameraId");
        if (j_param != nullptr)
        {
            MAINLOG(0, "[MAIN] : CameraId = %d\n", json_object_get_int(j_param));
            char CameraId = json_object_get_int(j_param);
            
            if(CameraId == 1) {
                j_param = (struct json_object *)json_object_object_get(j_args, "onoffSetting");
                MAINLOG(0, "[MAIN] : onoffSetting = %d\n", json_object_get_int(j_param));
                Process_Node_Dual.Active = json_object_get_int(j_param);
            } else if(CameraId == 2) {
                j_param = (struct json_object *)json_object_object_get(j_args, "onoffSetting");
                MAINLOG(0, "[MAIN] : onoffSetting = %d\n", json_object_get_int(j_param));
                Process_Node_Dual.Active = json_object_get_int(j_param);
            } else {
                MAINLOG(0, "[MAIN] : Unknow CameraId [%d] fail.\n", CameraId);
            }
        }
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_IO_RTC_SET_MODE(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : IO_RTC_SET_MODE\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        ios_rtc.local_time;
        
        // parsing "args"
        j_args = (struct json_object *)json_object_object_get(root, "args");
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));

        j_param = (struct json_object *)json_object_object_get(j_args, "use_ntp");
        if (j_param)
        {
            ios_rtc.use_ntp = json_object_get_int(j_param);
        }

        j_param = (struct json_object *)json_object_object_get(j_args, "year");
        if (j_param)
        {
            ios_rtc.local_time.tm_year = json_object_get_int(j_param) - 1900;
        }

        j_param = (struct json_object *)json_object_object_get(j_args, "month");
        if (j_param)
        {
            ios_rtc.local_time.tm_mon = json_object_get_int(j_param);
        }

        j_param = (struct json_object *)json_object_object_get(j_args, "date");
        if (j_param)
        {
            ios_rtc.local_time.tm_mday = json_object_get_int(j_param);
        }

        j_param = (struct json_object *)json_object_object_get(j_args, "hour");
        if (j_param)
        {
            ios_rtc.local_time.tm_hour = json_object_get_int(j_param);
        }

        j_param = (struct json_object *)json_object_object_get(j_args, "minute");
        if (j_param)
        {
            ios_rtc.local_time.tm_min = json_object_get_int(j_param);
        }

        j_param = (struct json_object *)json_object_object_get(j_args, "second");
        if (j_param)
        {
            ios_rtc.local_time.tm_sec = json_object_get_int(j_param);
        }

        j_param = (struct json_object *)json_object_object_get(j_args, "timezone");
        if (j_param)
        {
            ios_rtc.local_time.tm_zone = json_object_get_string(j_param);
            snprintf(&ios_rtc.timezone[0], sizeof(ios_rtc.timezone), "%s", json_object_get_string(j_param));
        }

        MAINLOG(0, "[__%s__] %d: [%d][%d][%d] [%d][%d][%d] [%s]\n", __func__, __LINE__
            , ios_rtc.local_time.tm_year, ios_rtc.local_time.tm_mon, ios_rtc.local_time.tm_mday
            , ios_rtc.local_time.tm_hour, ios_rtc.local_time.tm_min, ios_rtc.local_time.tm_sec
            , ios_rtc.local_time.tm_zone);

    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_IO_SYS_GET_PARAMS(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : IO_SYS_GET_PARAMS\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        ios_rtc.local_time;
        
        // parsing "args"
        j_args = (struct json_object *)json_object_object_get(root, "args");
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));
    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}

int IO_MqttParse_IO_SHOP_FLOOR_CONTROL(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out)
{
    if (pCmd == nullptr || pJson_Obj == nullptr || pParam == nullptr || Json_CmdInfo_Out == nullptr)
    {
        IOSLOG(0, " %s : >>> Error!!!, the pointer define is nullptr.\n", __func__);
        return -1;
    }
    MAINLOG(0, "[MAIN] : It's my command : IO_SHOP_FLOOR_CONTROL\n");
    struct json_object *root, *j_args, *j_param;
    root = (struct json_object *)pJson_Obj;

    /* parsing "args" */
    j_args = (struct json_object *)json_object_object_get(root, "args");
    if (j_args != nullptr)
    {
        // parsing "args"
        j_args = (struct json_object *)json_object_object_get(root, "args");
        MAINLOG(0, "[MAIN] : args=%s\n", json_object_get_string(j_args));

        j_param = (struct json_object *)json_object_object_get(j_args, "msg");
        if (j_param)
        {
            MAINLOG(0, "[MAIN] : msg=[%s]\n", json_object_get_string(j_param));
            strcpy(ios_sfc.msg, json_object_get_string(j_param));
        }

    }
    memset((char *)ios_CmdInfo, '\0', sizeof((char *)ios_CmdInfo));
    strcpy((char *)ios_CmdInfo, json_object_get_string(j_args));
}



////////////////////////////////////////////////////////////////////////////////////
// Json text information by the MQTT subscribe
////////////////////////////////////////////////////////////////////////////////////
static const int gJson_MAXIMUM_SIZE = 4096;

// Vision Box Mode
static char szJson_IO_Function[gJson_MAXIMUM_SIZE];

////////////////////////////////////////////////////////////////////////////////////
// Parameter structure register
////////////////////////////////////////////////////////////////////////////////////
static seIO_AlgoParamReg gIO_AlgoParamReg[] = {


    // IOPP function initialize
    {TRIGGER_SET_PROCESS, ios_cmdStr[TRIGGER_SET_PROCESS], &ios_trigger, szJson_IO_Function, IO_MqttParse_TRIGGER_SET_PROCESS},
    {TRIGGER_GET_PROCESS, ios_cmdStr[TRIGGER_GET_PROCESS], &ios_trigger, szJson_IO_Function, IO_MqttParse_TRIGGER_GET_PROCESS},
    {DIN_SET_PROCESS,     ios_cmdStr[DIN_SET_PROCESS],     &ios_trigger, szJson_IO_Function, IO_MqttParse_DIN_SET_PROCESS},
    {DIN_GET_PROCESS,     ios_cmdStr[DIN_GET_PROCESS],     &ios_trigger, szJson_IO_Function, IO_MqttParse_DIN_GET_PROCESS},
    {IOS_GET_STATUS,      ios_cmdStr[IOS_GET_STATUS],      &ios_trigger, szJson_IO_Function, IO_MqttParse_IOS_GET_STATUS},
    {LIGHT_SET_PWM,       ios_cmdStr[LIGHT_SET_PWM],       &ios_trigger, szJson_IO_Function, IO_MqttParse_LIGHT_SET_PWM},
    {LIGHT_GET_PWM,       ios_cmdStr[LIGHT_GET_PWM],       &ios_trigger, szJson_IO_Function, IO_MqttParse_LIGHT_GET_PWM},
    {LED_SET_PROCESS,     ios_cmdStr[LED_SET_PROCESS],     &ios_trigger, szJson_IO_Function, IO_MqttParse_LED_SET_PROCESS},
    {LED_GET_PROCESS,     ios_cmdStr[LED_GET_PROCESS],     &ios_trigger, szJson_IO_Function, IO_MqttParse_LED_GET_PROCESS},
    {LED_SET_MODE,        ios_cmdStr[LED_SET_MODE],        &ios_trigger, szJson_IO_Function, IO_MqttParse_LED_SET_MODE},
    {DIN_SET_MODE,        ios_cmdStr[DIN_SET_MODE],        &ios_trigger, szJson_IO_Function, IO_MqttParse_DIN_SET_MODE},
    {DOUT_SET_MODE,       ios_cmdStr[DOUT_SET_MODE],       &ios_trigger, szJson_IO_Function, IO_MqttParse_DOUT_SET_MODE},
    {DOUT_MANUAL_CONTROL, ios_cmdStr[DOUT_MANUAL_CONTROL], &ios_trigger, szJson_IO_Function, IO_MqttParse_DOUT_MANUAL_CONTROL},
    {DIO_GET_STATUS,      ios_cmdStr[DIO_GET_STATUS],      &ios_trigger, szJson_IO_Function, IO_MqttParse_DIO_GET_STATUS},
    {LIGHT_SET_BRIGHTNESS,ios_cmdStr[LIGHT_SET_BRIGHTNESS],&ios_trigger, szJson_IO_Function, IO_MqttParse_LIGHT_SET_BRIGHTNESS},
    {LIGHT_GET_BRIGHTNESS,ios_cmdStr[LIGHT_GET_BRIGHTNESS],&ios_trigger, szJson_IO_Function, IO_MqttParse_LIGHT_GET_BRIGHTNESS},
    {CAMERA_STREAMING_CONTROL,ios_cmdStr[CAMERA_STREAMING_CONTROL],&ios_trigger, szJson_IO_Function, IO_MqttParse_CAMERA_STREAMING_CONTROL},
    {MODBUS_SET_PARAMS,   ios_cmdStr[MODBUS_SET_PARAMS],   &ios_trigger, szJson_IO_Function, IO_MqttParse_MODBUS_SET_PARAMS},
    {MODBUS_GET_PARAMS,   ios_cmdStr[MODBUS_GET_PARAMS],   &ios_trigger, szJson_IO_Function, IO_MqttParse_MODBUS_GET_PARAMS},
    {TRIGGER_SET_MODE,    ios_cmdStr[TRIGGER_SET_MODE],    &ios_trigger, szJson_IO_Function, IO_MqttParse_TRIGGER_SET_MODE},
    {REPORT_TEST_RESULT,  ios_cmdStr[REPORT_TEST_RESULT],  &ios_trigger, szJson_IO_Function, IO_MqttParse_REPORT_TEST_RESULT},
    {AUTO_TEST_SET_MODE,  ios_cmdStr[AUTO_TEST_SET_MODE],  &ios_trigger, szJson_IO_Function, IO_MqttParse_AUTO_TEST_SET_MODE},
    {IO_RTC_SET_MODE,     ios_cmdStr[IO_RTC_SET_MODE],     &ios_trigger, szJson_IO_Function, IO_MqttParse_IO_RTC_SET_MODE},
    {IO_SYS_GET_PARAMS,   ios_cmdStr[IO_SYS_GET_PARAMS],   &ios_trigger, szJson_IO_Function, IO_MqttParse_IO_SYS_GET_PARAMS},
    {IO_SHOP_FLOOR_CONTROL,ios_cmdStr[IO_SHOP_FLOOR_CONTROL],   &ios_trigger, szJson_IO_Function, IO_MqttParse_IO_SHOP_FLOOR_CONTROL},
    
    
    
/*
  TRIGGER_SET_PROCESS,
  TRIGGER_GET_PROCESS,
  DIN_SET_PROCESS,
  DIN_GET_PROCESS,
  IOS_GET_STATUS,
  LIGHT_SET_PWM,
  LIGHT_GET_PWM,
  LED_SET_PROCESS,
  LED_GET_PROCESS,

  LED_SET_MODE,
  DIN_SET_MODE,
  DOUT_SET_MODE,
  DOUT_MANUAL_CONTROL,
  DIO_GET_STATUS,
  LIGHT_SET_BRIGHTNESS,
  LIGHT_GET_BRIGHTNESS,
  CAMERA_STREAMING_CONTROL,
  MODBUS_SET_PARAMS,
  */
  
  /*
  MODBUS_GET_PARAMS,
  TRIGGER_SET_MODE,
  REPORT_TEST_RESULT,
  AUTO_TEST_SET_MODE,
  
  IO_RTC_SET_MODE,
  IO_SYS_GET_PARAMS,

*/
    // The End of AlgoParam register
    {ENUM_IO_ALGO_END, ios_cmdStr[ENUM_IO_ALGO_END], NULL, NULL, NULL}

};

////////////////////////////////////////////////////////////////////////////////////
// Parameter Tasks assign function
////////////////////////////////////////////////////////////////////////////////////

static std::unordered_map<std::string, int> gIO_HashMap_Param;

const int IO_AlgoParam_TblCnt = (sizeof(gIO_AlgoParamReg) / sizeof(gIO_AlgoParamReg[0]));

const int ioParam_TblCnt = (sizeof(ios_cmdStr) / sizeof(ios_cmdStr[0]));

int createHashMap_IO_Param()
{

    if (!gIO_HashMap_Param.empty())
    {

        IOSLOG(0, " ## Param__The gIO_HashMap_Param is already create.\n");
        return 0;
    }

    for (int i = 0; i < ioParam_TblCnt; i++)
    {

        std::string strTmp = ios_cmdStr[i];

        // IOSLOG(0, "%s()%d: ## gIO_HashMap_Param[ %s ] = %d\n", __FUNCTION__, __LINE__, strTmp.c_str(), i);

        gIO_HashMap_Param[strTmp] = i;
    }

    return 0;
}

int compareHashMap_IO_Param(std::string strKey)
{
    auto it = gIO_HashMap_Param.find(strKey);

    if (it != gIO_HashMap_Param.end())
    {

        IOSLOG(0, " ## Param__Element: %s, has ID: %d\n", strKey.c_str(), it->second);
        printf("%s()%d: ## Param__Element: %s, has ID: %d\n", __FUNCTION__, __LINE__, strKey.c_str(), it->second);
        return it->second;
    }
    else
    {

        IOSLOG(0, " ## Param__Element: %s, not found in the map.\n", strKey.c_str());
        printf("%s()%d: ## Param__Element: %s, not found in the map.\n", __FUNCTION__, __LINE__, strKey.c_str());
        return -1;
    }
}

int setIO_ParamAssign(const char *szKey, struct json_object *j_subsystem, seIO_JsonInfo *pInfo)
{
    IOSLOG(0, "\n\n[__%s()__] %d: ===> start [%s]\n", __func__, __LINE__, szKey);

    int ret = -1;

    seIO_AlgoParamReg *pIO_AlgoParam = nullptr;

    for (int i = 0; i < IO_AlgoParam_TblCnt; i++)
    {

        int res = 0;
        // IOSLOG(0, "%s()%d: [%s][%s][%d]\n", __FUNCTION__, __LINE__, szKey, gIO_AlgoParamReg[i].strCmd, IO_AlgoParam_TblCnt);
        if (!strcmp(szKey, gIO_AlgoParamReg[i].strCmd))
        {

            pIO_AlgoParam = &gIO_AlgoParamReg[i];
            res = pIO_AlgoParam->MqttParsesFunc(pIO_AlgoParam->strCmd, j_subsystem, pIO_AlgoParam->pParam, pIO_AlgoParam->szJsonInfo);

            if (pIO_AlgoParam->emAlgoId != ENUM_IO_ALGO_END)
            {

                pInfo->emAlgoId = pIO_AlgoParam->emAlgoId;

                // Copy the MQTT "Key" ID of Algorithm.
                strcpy(pInfo->szCmd, pIO_AlgoParam->strCmd);

                // Copy the Json information by the MQTT subscribe function.
                strcpy(pInfo->szJsonBuf, pIO_AlgoParam->szJsonInfo);
                
                // IOSLOG(0, "%s()%d: [%s][%s]\n", __FUNCTION__, __LINE__, szKey, gIO_AlgoParamReg[i].strCmd);
                IOSLOG(0, "%s()%d: [%d][%s][%s]\n", __FUNCTION__, __LINE__, pInfo->emAlgoId, pInfo->szCmd, pInfo->szJsonBuf);
                ret = 0;
                break;
            }
            else
            {
                ret = -1;
                // IOSLOG(0, "%s()%d: Unknow cmd=[%s][%s][%d]\n", __func__, __LINE__, szKey, gIO_AlgoParamReg[i].strCmd, IO_AlgoParam_TblCnt);
                break;
            }
        }
    }

    IOSLOG(0, "[__%s__] %d: end <===  \n\n", __func__, __LINE__);

    return ret;
}
