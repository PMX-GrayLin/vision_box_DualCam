/* tests/unit-test.h.  Generated from unit-test.h.in by configure.  */
/*
 * Copyright © 2008-2014 Stephane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#pragma once 

#ifndef __COMMON_H__
#define __COMMON_H__

/* Constants defined by configure.ac */
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# ifndef _MSC_VER
# include <stdint.h>
# else
# include "stdint.h"
# endif
#endif
#include <stdbool.h>
#include <string>

/* define the tty color */
#define NONE          "\033[m"
#define RED           "\033[0;32;31m"
#define LIGHT_RED     "\033[1;31m"
#define GREEN         "\033[0;32;32m"
#define LIGHT_GREEN   "\033[1;32m"
#define BLUE          "\033[0;32;34m"
#define LIGHT_BLUE    "\033[1;34m"
#define DARY_GRAY     "\033[1;30m"
#define CYAN          "\033[0;36m"
#define LIGHT_CYAN    "\033[1;36m"
#define PURPLE        "\033[0;35m"
#define LIGHT_PURPLE  "\033[1;35m"
#define BROWN         "\033[0;33m"
#define YELLOW        "\033[1;33m"
#define LIGHT_GRAY    "\033[0;37m"
#define WHITE         "\033[1;37m"

#define IOD_FIFO_PATH "/var/run/iod_fifo"

#define STATIC_IP	false
#define DHCP_IP 	true

typedef struct{
	unsigned char ssid[32];
	unsigned char auth[32];
	unsigned char pwd[32];
	bool wifi_connected;
}WIFI_STATION_PARA;	

typedef struct{
	unsigned char ssid[32];
	unsigned char auth[32];
	unsigned char pwd[32];
}WIFI_AP_PARA;	

typedef struct {
	unsigned char		version[32];
	unsigned char		hostname[64];
	unsigned char		mode[64];			// AP or Clinet mode
	WIFI_AP_PARA		ap;				    // AP parameters
	WIFI_STATION_PARA	station;		// Clinet(Station) parameter
	bool				mode_changed;
	bool 				wifi_connected;
	bool				enable;
	int					client_timeout;
}WIFI_PARA;

typedef struct {
    bool enable_dhcp;
    unsigned char ethernet_ip[16];
    unsigned char gateway_ip[16];
}ETH_PARA;

typedef enum {
	WRITE_SSID_PW,
	WRITE_AP_SSID_PW,
	WIFI_CLIENT_FAIL,
	WIFI_IPADDR,
	WIFI_CONNECTED,
	WIFI_DISCONNECTED,
	WIFI_TURN_OFF,
	WIFI_TURN_ON,
	WRITE_IPCONFIG,
	READ_IPCONFIG,
	ETH_IPADDR,
	ETH_CONNECTED,
	ETH_DISCONNECTED,
}RESPONSE_TYPE;

typedef unsigned char UINT8;
typedef char INT8;
typedef unsigned short UINT16;
typedef short INT16;
typedef unsigned int UINT32;
typedef int INT32;
typedef bool BOOL;

typedef enum __QNAME__
{
  MSGQ_MAIN = 1000,
  MSGQ_MCU, 
  MSGQ_IOS,
  MSGQ_IPS,
  MSGQ_MAX,
}QNAME;

typedef enum __MCU_ACT__ {
  /* only test */
  MCU_MT_CLOCKWISE_LOW = 0,
  MCU_MT_ANTICLOCKWISE_LOW,
  MCU_MT_STOP_PAN,
  MCU_MT_TILE_UP,
  MCU_MT_TILE_DOWN,
  MCU_ACT_MAX,
}MCU_ACT;

typedef enum __NET_ACT__ {
  NET_CAM_RECORD = 0,
  NET_CAM_SNAPSHOT,
  NET_CAM_STOP_RECORDING, 
  NET_CAM_STOP_SNAPSHOTING, 
  NET_ALLJ_TURN_ON,
  NET_ALLJ_TURN_OFF,
  NET_WIFI_CONNECTED,
  NET_WIFI_DISCONNECTED,
  NET_ACT_MAX,
}NET_ACT;

/* public NET status attribute */
typedef struct __NET_STATUS__ {
  bool  wifiConnected;
}NET_STATUS;


#define PATH_BUFFER_SIZE		256
/*****************************************************
  main command set
******************************************************/
typedef enum __AUTO_GLUE_INSPECTION_FLOW__{
  GI_START,
  GI_WAIT_TRIGGER,
  GI_EXCUTE_AUTO_GI,
  GI_GET_TEST_RESULT,  
}AUTO_GLUE_INSPECTION_FLOW;

typedef enum __AUTO_MODE__{
  GLUE_INSPECTION,
  BASIC_INSPECTION,
  AI_INSPECTION,
  NON_AUTO,
}AUTO_MODE;

typedef struct _MAIN_PROCESS_{
  bool streaming;     // whether do stream the GigE Camera
  uint8_t camera;     // GigE camera number, 1 or 2
}MAIN_PROCESS;
#if 0
typedef struct _LIGHT_SET_BRIGHTNESS_{
  uint8_t lightSource;
  uint16_t Brightness;
}LIGHT_SET_BRIGHTNESS;
#endif
/*****************************************************
  IOS command set
******************************************************/
typedef struct _IOS_TRIGGER_SET_PROCESS_{
  uint8_t inPin;
  uint8_t inMode[16];
  uint8_t outPin;
  uint32_t outDelay;
}IOS_TRIGGER_SET_PROCESS;

typedef struct _IOS_DI_SET_PROCESS_{
  uint8_t inPin;
  uint8_t inMode[16];
  uint16_t inControlMode;
  uint32_t inDelay;
  uint8_t outPin;
  uint8_t outMode[16];
  uint16_t outControlMode;
  uint32_t outDelay;
  uint8_t onoffSetting;      /* On or Off */
}IOS_DI_SET_PROCESS;

/* Dout manual control using */
typedef struct _IOS_DOUT_SET_PROCESS_{
  uint8_t outPin;           /* DOUT 1 ~ 4 */
  uint8_t outMode[16];      /* NPN or PNP */
  uint8_t onoffSetting;     /* On or Off */
    uint8_t SelectMode[16]; // Latch / OneShot
  uint32_t OneShotPeriod;
}IOS_DOUT_SET_PROCESS;

/* Dout is assigned by test done/result indication */
typedef struct _IOS_DOUT_SET_MODE_{
  uint8_t outPin;           /* DOUT 1 ~ 4 */
  uint8_t polarity[8];      /* NPN or PNP */
  uint8_t selectMode[32];   /* Result-OK, Result-NG, AutoDone */
  uint8_t CameraId;
}IOS_DOUT_SET_MODE;

typedef enum _TRIGGER_PIN_{
  TRIGGER_PIN_1 = 1,
  TRIGGER_PIN_2,
}TRIGGER_PIN;

typedef enum _DIN_PIN_{
  DIN_PIN_1 = 1,
  DIN_PIN_2,
  DIN_PIN_3,
  DIN_PIN_4,
}DIN_PIN;

typedef enum _LIGHT_SOURCE_{
  LIGHT_SOURCE_1 = 1,
  LIGHT_SOURCE_2,
}LIGHT_SOURCE;

typedef enum _OUT_PIN_{
  DOUT_PIN_1 = 1,
  DOUT_PIN_2,
  DOUT_PIN_3,
  DOUT_PIN_4,
  LIGHT_PIN_1,
  LIGHT_PIN_2,
  CAMERA_PIN_1,
  CAMERA_PIN_2,
  OUT_PIN_MAX,
}OUT_PIN;

typedef enum _LED_{
  PowerLED = 1,
  Camera2LED,
  Camera1LED,
  UserDef2LED,
  UserDef1LED,
}LED;

typedef struct _IOS_LIGHT_SET_PWM_{
  uint8_t inLight;
  uint16_t value;
  uint8_t LightSwitch[8];     /* Light switch on or off */
}IOS_LIGHT_SET_PWM;

typedef struct _IOS_GET_STATUS_{
  uint8_t dout1;
  uint8_t dout2;
  uint8_t dout3;
  uint8_t dout4;
  uint8_t din1;
  uint8_t din2;
  uint8_t din3;
  uint8_t din4;
  uint8_t trigger1;
  uint8_t trigger2;
  uint8_t camera1;
  uint8_t camera2;
}IOS_IO_GET_STATUS;

typedef struct _IOS_LED_SET_PROCESS_{
  uint8_t outPin;
  uint8_t outMode[16];
  uint8_t outStatus[16];
  uint32_t outBlinkDelay;
  uint32_t outOffDelay;
}IOS_LED_SET_PROCESS;

/* for backend */
typedef enum __LED_MODE__{
  WIFI_STATUS,
  AI_STATUS,
  POE_STATUS,
  LIGHT_STATUS,
  TRIGGER_STATUS,
  DI_STATUS,
  DO_STATUS,
}LED_MODE;
  
/********** for web backend ***************/
typedef struct _IOS_LED_SET_MODE_{
  uint8_t led;
  uint8_t ledMode[16];
  uint8_t Indication[8];
  uint8_t Color[8];
}IOS_LED_SET_MODE;

typedef struct _IOS_DIN_SET_MODE_{
  uint8_t DinPin;
  uint8_t DinPolarity[8];
  uint8_t SelectMode[32];
}IOS_DIN_SET_MODE;

enum{
  TCP,
  RTU
};

typedef struct __IOS_MODBUS_SET_PARAMS__{
  uint8_t   GI_Testing_Register;
  uint8_t   GI_Test_Status_Register;
  uint8_t   GI_Testing_Result_Register;
  uint8_t   FrameFormat;
  uint16_t  nb_connections;
  uint16_t  TCP_IP_PORT;
  uint32_t  RTU_BAUDRATE;
} IOS_MODBUS_SET_PARAMS;

/*****************************************************
  IPS command set
******************************************************/
typedef enum _CAMERA_PARA_{
  NO_DEFINE,
  MONO,
  COLOR,
  SOFTWARE,
  HARDWARE,
  EVERY_IMAGE,
}CAMERA_PARA;

typedef enum __IPS_CMD__ {
  NO_IP_CMD = 0,
  CAMERA_SET_MODE,
  
}IPS_CMD;

typedef struct _CAMERA_GRAB_{
  uint8_t type;
  uint8_t triggerMode;
  uint8_t exposureTime;
  uint8_t saveImgOpt;
  uint8_t saveImgPath[PATH_BUFFER_SIZE];
}CAMERA_GRAB;

typedef struct __PROCESS_NODE__{
  uint8_t Active;
  AUTO_MODE autoMode;
  AUTO_GLUE_INSPECTION_FLOW giFlow; // glue inspection flow
  uint8_t TestResult[8];
  uint8_t Dout1Mode[16];
  uint8_t Dout2Mode[16];
  uint8_t Dout3Mode[16];
  uint8_t Dout4Mode[16];
  uint8_t Dout1selectMode[32];
  uint8_t Dout2selectMode[32];
  uint8_t Dout3selectMode[32];
  uint8_t Dout4selectMode[32];
  uint8_t UsrDef1Mode[16];
  uint8_t UsrDef2Mode[16];
  uint16_t Brightness[2];
}PROCESS_NODE;


typedef struct __IOS_RTC_SET_MODE__{
    bool use_ntp;
    struct tm local_time;
    char timezone[64];
}IOS_RTC_SET_MODE;

typedef struct __IOS_SFC_SET_MODE__{
    char msg[1024];
}IOS_SFC_SET_MODE;

#ifdef __cplusplus
extern "C" {
#endif

extern MAIN_PROCESS main_process;

extern CAMERA_GRAB camera_grab;
extern IOS_TRIGGER_SET_PROCESS ios_trigger;
extern IOS_DI_SET_PROCESS ios_di;
extern IOS_LIGHT_SET_PWM ios_setpwm;
extern IOS_IO_GET_STATUS ios_getstatus;
extern IOS_LED_SET_PROCESS ios_setled;
extern IOS_DOUT_SET_PROCESS ios_dout;
extern IOS_DOUT_SET_MODE ios_doutMode;
extern IOS_LED_SET_MODE ios_setLedMode;
extern IOS_DIN_SET_MODE ios_setDinMode;
extern IOS_MODBUS_SET_PARAMS ios_modbusSetParams;
extern PROCESS_NODE Process_Node;
extern PROCESS_NODE Process_Node_Dual;
extern uint8_t ios_CmdInfo[];
extern uint8_t UpdateLEDStatus_Flg;
extern IOS_RTC_SET_MODE ios_rtc;
extern IOS_SFC_SET_MODE ios_sfc;

#ifdef __cplusplus
}
#endif




#define CLRMEM(x) \
memset(x, '\0', sizeof(x))

#define SETMSG(x, y) \
memset(x, '\0', sizeof(x));\
strcpy(x, y)

#define MAX_MSG_SIZE		512	// max message Queue length 	
#define TRUE				1
#define FALSE				0


// static uint32_t debugLevel = 0;
// #define DEBUG(level, format, b...)     if ( (debugLevel+1) >= (level+1) )  printf("%s:%s()%d:" format, __FILE__, __FUNCTION__, __LINE__, ##b)

extern uint32_t mainDebugLevel;
#ifdef ALGO_Enable_MAINLOG_DEBUG
#define MAINLOG(level, format, b...)     if ( (mainDebugLevel+1) >= (level+1) )  printf("%s:%s()%d:" LIGHT_CYAN format NONE, __FILE__, __FUNCTION__, __LINE__, ##b)
#else
#define MAINLOG(level, format, b...)
#endif

extern uint32_t iosDebugLevel;
#define IOSLOG(level, format, b...)     if ( (iosDebugLevel+1) >= (level+1) )  printf("%s:%s()%d:" GREEN format NONE, __FILE__, __FUNCTION__, __LINE__, ##b)


extern uint32_t ipsDebugLevel;
#ifdef ALGO_Enable_IPSLOG_DEBUG
#define IPSLOG(level, format, b...)     if ( (ipsDebugLevel+1) >= (level+1) )  printf("__ips_# %s: %s(): ln:%d :" CYAN format NONE, __FILE__, __FUNCTION__, __LINE__, ##b)
#else
#define IPSLOG(level, format, b...)
#endif

// static uint32_t cmsDebugLevel = 0;
// #define CMSLOG(level, format, b...)     if ( (cmsDebugLevel+1) >= (level+1) )  printf("%s:%s()%d:" BROWN format NONE, __FILE__, __FUNCTION__, __LINE__, ##b)
// static uint32_t otasDebugLevel = 0;
// #define OTASLOG(level, format, b...)     if ( (otasDebugLevel+1) >= (level+1) )  printf("%s:%s()%d:" LIGHT_RED format NONE, __FILE__, __FUNCTION__, __LINE__, ##b)


extern uint32_t backendDebugLevel;
#define BACKENDLOG(level, format, b...)     if ( (backendDebugLevel+1) >= (level+1) )  printf("%s:%s()%d:" LIGHT_BLUE format NONE, __FILE__, __FUNCTION__, __LINE__, ##b)

#ifdef FN
    #undef FN
#endif
#define FN \
    { \
        printf(LIGHT_GREEN "[%s]: %s(%d)\r\n" NONE, __FILE__, __FUNCTION__, __LINE__); \
    }
    
#ifdef __cplusplus
	extern "C" {
#endif
extern int msgQ_send(uint8_t  *msg);
extern int msgQ_rec(uint8_t *msg);
extern int msqQ_remove(void);
extern int msgQ_init();
extern int msgQ_iosSend(uint8_t  *msg);
extern int msgQ_ipsSend(uint8_t  *msg);
extern int msgQ_recIos(uint8_t *msg);
extern int msgQ_recIps(uint8_t *msg);

extern UINT32 mcMsgQParser(char *buf);

extern int iosCtl_init();
extern int ipsCtl_init();
extern int cmsCtl_init();
// extern int aisCtl_init();
extern int otasCtl_init();

extern int innerQ_Main_Init();
extern int innerQ_Main_EnQ(std::string msg);
extern int innerQ_Main_DeQ(std::string* msg);
extern bool innerQ_Main_IsEmpty();
extern void innerQ_Main_Destory();

//Single camera
extern int innerQ_IPS_Init();
extern int innerQ_IPS_EnQ(std::string msg);
extern int innerQ_IPS_DeQ(std::string* msg);
extern bool innerQ_IPS_IsEmpty();
extern void innerQ_IPS_Destory();

//Dual camera
extern int innerQ_IPS_Init_Dual(const int iID);
extern int innerQ_IPS_EnQ_Dual(std::string msg, const int iID);
extern int innerQ_IPS_DeQ_Dual(std::string* msg, const int iID);
extern bool innerQ_IPS_IsEmpty_Dual(const int iID);
extern void innerQ_IPS_Destory_Dual(const int iID);

extern int innerQ_IOS_Init();
extern int innerQ_IOS_EnQ(std::string msg);
extern int innerQ_IOS_DeQ(std::string* msg);
extern bool innerQ_IOS_IsEmpty();
extern void innerQ_IOS_Destory();



#ifdef __cplusplus
}
#endif
#endif