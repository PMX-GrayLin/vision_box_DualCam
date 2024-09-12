#pragma once

#ifndef _IOS_METHOD_STRUCTURE_H_
#define _IOS_METHOD_STRUCTURE_H_

#include <string>

/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef enum __IOS_CMD__ {
  NO_IO_CMD = 0,
  TRIGGER_SET_PROCESS,
  TRIGGER_GET_PROCESS,
  DIN_SET_PROCESS,
  DIN_GET_PROCESS,
  IOS_GET_STATUS,
  LIGHT_SET_PWM,
  LIGHT_GET_PWM,
  LED_SET_PROCESS,
  LED_GET_PROCESS,
  /*** for web backend ****/
  LED_SET_MODE,
  DIN_SET_MODE,
  DOUT_SET_MODE,
  DOUT_MANUAL_CONTROL,
  DIO_GET_STATUS,
  LIGHT_SET_BRIGHTNESS,
  LIGHT_GET_BRIGHTNESS,
  CAMERA_STREAMING_CONTROL,
  MODBUS_SET_PARAMS,
  MODBUS_GET_PARAMS,
  TRIGGER_SET_MODE,
  REPORT_TEST_RESULT,
  AUTO_TEST_SET_MODE,
  
  IO_RTC_SET_MODE,
  IO_SYS_GET_PARAMS,
  IO_SHOP_FLOOR_CONTROL,
  IO_MAINLED_SET_PARAM,
  IO_AILIGHTING_SET_PARAM,
  IO_EXTLIGHTING_SET_PARAM,
  IO_EXTLIGHTING_GET_PARAM,
  IO_TOF_GET_PARAM,

  ENUM_IO_ALGO_END // end of enum
}IOS_CMD;

static char *ios_cmdStr[]={
  (char*)"NO_IO_CMD",
  (char*)"TRIGGER_SET_PROCESS", 
  (char*)"TRIGGER_GET_PROCESS",
  (char*)"DIN_SET_PROCESS",
  (char*)"DIN_GET_PROCESS",
  (char*)"IOS_GET_STATUS",
  (char*)"LIGHT_SET_PWM",
  (char*)"LIGHT_GET_PWM",
  (char*)"LED_SET_PROCESS",
  (char*)"LED_GET_PROCESS",
  /*** for web backend ****/
  (char*)"LED_SET_MODE",
  (char*)"DIN_SET_MODE",
  (char*)"DOUT_SET_MODE",
  (char*)"DOUT_MANUAL_CONTROL",
  (char*)"DIO_GET_STATUS",
  (char*)"LIGHT_SET_BRIGHTNESS",
  (char*)"LIGHT_GET_BRIGHTNESS",
  (char*)"CAMERA_STREAMING_CONTROL",
  (char*)"MODBUS_SET_PARAMS",
  (char*)"MODBUS_GET_PARAMS",
  (char*)"TRIGGER_SET_MODE",
  (char*)"REPORT_TEST_RESULT",
  (char*)"AUTO_TEST_SET_MODE",
  
  (char*)"IO_RTC_SET_MODE",
  (char*)"IO_SYS_GET_PARAMS",
  (char*)"IO_SHOP_FLOOR_CONTROL",
  (char*)"IO_MAINLED_SET_PARAM",
  (char*)"IO_AILIGHTING_SET_PARAM",
  (char*)"IO_EXTLIGHTING_SET_PARAM",
  (char*)"IO_EXTLIGHTING_GET_PARAM",
  (char*)"IO_TOF_GET_PARAM",

};

static char *ios_respStr[]={
  (char*)"NO_IO_CMD",
  (char*)"TRIGGER_GET_PROCESS", 
  (char*)"TRIGGER_SET_PROCESS",
  (char*)"DIN_GET_PROCESS",
  (char*)"DIN_SET_PROCESS",
  (char*)"IOS_SET_STATUS",
  (char*)"LIGHT_GET_PWM",
  (char*)"LIGHT_SET_PWM",
  (char*)"LED_GET_PROCESS",
  (char*)"LED_SET_PROCESS",
  /*** for web backend ****/
  (char*)"LED_GET_MODE",
  (char*)"DIN_GET_MODE",
  (char*)"DOUT_GET_MODE",
  (char*)"DOUT_GET_MANUAL_CONTROL",
  (char*)"DIO_SET_STATUS",
  (char*)"LIGHT_GET_BRIGHTNESS",
  (char*)"LIGHT_SET_BRIGHTNESS",
  (char*)"CAMERA_STREAMING_CONTROL",
  (char*)"MODBUS_SET_PARAMS",
  (char*)"MODBUS_GET_PARAMS",
  (char*)"TRIGGER_SET_MODE",
  (char*)"REPORT_TEST_RESULT",
  (char*)"AUTO_TEST_SET_MODE",
  
  (char*)"IO_RTC_SET_MODE",
  (char*)"IO_SYS_GET_PARAMS",
  (char*)"IO_SHOP_FLOOR_CONTROL",
  (char*)"IO_MAINLED_SET_PARAM",
  (char*)"IO_AILIGHTING_SET_PARAM",
  (char*)"IO_EXTLIGHTING_SET_PARAM",
  (char*)"IO_EXTLIGHTING_GET_PARAM",
  (char*)"IO_TOF_GET_PARAM",
  
};
typedef struct tagIO_JsonInfo
{
    tagIO_JsonInfo()
    {
        emAlgoId = ENUM_IO_ALGO_END;
        memset(szCmd, 0x00, sizeof(szCmd) / sizeof(szCmd[0]));
        memset(szJsonBuf, 0x00, sizeof(szJsonBuf) / sizeof(szJsonBuf[0]));
    }

    IOS_CMD emAlgoId;

    char szCmd[200];

    char szJsonBuf[4096];

} seIO_JsonInfo, *LPIO_JsonInfo;

typedef struct tagIO_AlgoParamRegister
{
    IOS_CMD emAlgoId;

    const char *strCmd;

    // Parameter structure.
    void *pParam;

    // Json text info.
    char *szJsonInfo;

    // MQTT command parses function.
    int (*MqttParsesFunc)(const char *pCmd, const void *pJson_Obj, void *pParam, void *Json_CmdInfo_Out);

} seIO_AlgoParamReg, *LPIO_AlgoParamReg;


#endif //_IOS_METHOD_STRUCTURE_H_