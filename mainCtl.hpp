
#pragma once

#ifndef __MAIN_HANDLER_H__
#define __MAIN_HANDLER_H__

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include <linux/types.h>
#include <stdbool.h>
#include <cstdint>


/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */
#define COMMAND_BUFFER_SIZE		128

/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef enum {
  IOS=0,
  CMS,
  IPS,
} SUBSYSTEM_NAME;

/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef enum __SETFUNC__{
  /* for IPS */
  NO_SETFUNC,
  CAMERA,
  CALIBRATION,
  ALIGNMENT,
  THRESHOLD,
  PROCESS,
  HARD_GRAB,
  SOFT_GRAB,
  STOP_HARD_GRAB,
  /* for IOS */
  LIGHT,
  TriggerSetProcess,
  TriggerGetProcess,
  DinSetProcess,
  DinGetProcess,
  IOSGetStatus,
  LightSetPWM,
  LightGetPWM,
  LedSetProcess,
  LedGetProcess,
  DoutSetProcess,
  ModbusSetParams,
  ModbusGetParams,
  /* for MAIN */
  START_STREAMING,
  STOP_STREAMING,
  SETFUNC_MAX,
}SETFUNC;

/* main structure */
typedef struct __IPS_PROCESS__ 
{
  bool  mode;   /* audo mdoe */	
  int   intDevId;	// INT device id
  char  ipcamAddr[16];
  char  yearDay[16];
  
}IPS_PROCESS;

typedef enum __IPS_STATE__
{
  CONFIGURE_IOS,          /* would configure IOS */
  CONFIGURE_IPS,          /* would configure IPS */
  IOS_CONFIGURED,         /* IOS has been configured */
  WAIT_IPS_CONFIGURED,    /* IPS has been configured */
  WAIT_IOS_TRIGGER_DONE,  /* IOS report already triggered camera by IO */
  WAIT_GRAB_IMAGE,
  REPORT_MAIN,
  ERROR,
}IPS_STATE;

typedef enum __GPIO_CTL_CMD__
{
  GPIO_GET, 
  GPIO_SET_INPUT,
  GPIO_SET_OUTPUT,
  GPIO_SET_VALUE,
}GPIO_CTL_CMD;

typedef struct _JES_ {
  SETFUNC   new_job;      /* job is IPS, IOS */
  SETFUNC   current_job;  /* job is IPS, IOS */
}JES;

typedef struct _IOS_STATUS_{
  bool  finish;
  SETFUNC new_job;
  uint8_t state;
}IOS_STATUS;

typedef struct _IPS_STATUS_{
  bool  finish;
  SETFUNC new_job;
  uint8_t state;
  bool    ios_configured;       /* ios whether was configured before */
  bool    ips_configured;       /* ips whether was configured before */
  bool    ios_triggered_camera; /* IOS has triggered the camera by IO */
  bool    ips_grabbed_image;    /* IPS has grabbed the camera image */
  
}IPS_STATUS;

/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

	extern JES jes;
	extern IOS_STATUS ios_status;
	extern IPS_STATUS ips_status;

  // Dual camera 
	extern void suspend_ip_Dual(const int iID);
	extern void resume_ip_Dual(const int iID);
	extern void close_ip_Dual(const int iID);

	extern void suspend_io();
	extern void resume_io();
	extern void close_io();

	extern void suspend_mp();
	extern void resume_mp(int iStatus);
	extern void close_mp();
	extern void getStatus_mp(int* pStatus);

	extern void start_watch_dog(uint32_t time);
	extern void stop_watch_dog();

#ifdef __cplusplus
}
#endif


#endif /*__MAIN_HANDLER_H__*/