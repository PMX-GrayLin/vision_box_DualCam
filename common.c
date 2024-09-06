#include "common.h"

MAIN_PROCESS main_process;
CAMERA_GRAB camera_grab;
IOS_TRIGGER_SET_PROCESS ios_trigger;
IOS_DI_SET_PROCESS ios_di;
IOS_LIGHT_SET_PWM ios_setpwm;
IOS_IO_GET_STATUS ios_getstatus;
IOS_LED_SET_PROCESS ios_setled;
IOS_DOUT_SET_PROCESS ios_dout;
IOS_DOUT_SET_MODE ios_doutMode;
IOS_LED_SET_MODE ios_setLedMode;
IOS_DIN_SET_MODE ios_setDinMode;
IOS_MODBUS_SET_PARAMS ios_modbusSetParams;
PROCESS_NODE Process_Node;
PROCESS_NODE Process_Node_Dual;
IOS_DOUT_SET_MODE ios_doutMode_tmp;
IOS_RTC_SET_MODE ios_rtc;
IOS_SFC_SET_MODE ios_sfc;

uint8_t ios_CmdInfo[512];
uint8_t UpdateLEDStatus_Flg = 0;

// ??
uint32_t mainDebugLevel = 10;
uint32_t iosDebugLevel = 10;
uint32_t ipsDebugLevel = 10;
uint32_t rextyDebugLevel = 10;
uint32_t backendDebugLevel = 10;