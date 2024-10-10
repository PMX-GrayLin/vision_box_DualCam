/**
 ******************************************************************************
 * @file    mainCrl.c
 * @brief   main controller for vision box.
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
#include <errno.h>
#include <mqueue.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h> //inet_addr
#include <ev.h>
#include <chrono>
#include <deque>
#include <string>
#include <semaphore.h>
#include "ipsCtl/IPS_CompAlgorithm.h"
#include "ipsCtl/IPS_CompFunction.h"
#include "ipsCtl/IPS_MethodStructureDef.h"
#include "IPLAlgoDataStructureDef.h"

#include "common.hpp"
#include "mainCtl.hpp"
#include "spi.h"
#include "ext_mqtt_client.hpp"
#include "global.hpp"
#include "iosCtl.h"
#include "IOS_CompFunction.h"
#include "json.h"


/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */
/* define msgQ type
   1 is from peripheral to main
   2 is from main to peripheral */
#define MSGQ_SEND_MAIN (int)1
#define MSGQ_REC_MAIN (int)2
#define IOS_SEND_MSGQ (int)3
#define IPS_SEND_MSGQ (int)4


/* MACRO DECLARATIONS ---------------------------------------------------------------- */
#define VERSION_PRINT(...) printf("\e[1;34m[%s]\e[m", VSB_VERSION);

#define COUT_MACRO(...) std::cout << __VA_ARGS__

#define COLOR_RED(...) printf("\e[1;31m[%s][%4d] =>\e[m", __func__, __LINE__);


/*********************************************************************
        Tools
*********************************************************************/
#define LOG(x)                           \
    {                                    \
        static char str[32];             \
        if (strcmp(x, &str) != 0)        \
        {                                \
            printf(x);                   \
            memset(str, 0, sizeof(str)); \
            strcpy(str, x);              \
        }                                \
    }


/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef struct __MSGBUF__
{
    long mtype;
    char mtext[MAX_MSG_SIZE];
    /* vision box specific */
    uint8_t mcmd; /* record command from backend */
    void *mpointer;
} MSGBUF;


/* GLOBAL VARIABLE DECLARATIONS ------------------------------------------------------- */
static const char *setfuncStr[]={
  "NO_SETFUNC",
  "CAMERA", 
  "CALIBRATION",
  "ALIGNMENT",
  "THRESHOLD",
  "PROCESS",
  "HARD_GRAB",
  "SOFT_GRAB",
  "STOP_HARD_GRAB",
  "LIGHT",
  "TriggerSetProcess",
  "TriggerGetProcess",
  "DinSetProcess",
  "DinGetProcess",
  "IOSGetStatus",
  "LightSetPWM",
  "LightGetPWM",
  "LedSetProcess",
  "LedGetProcess",
  "DoutSetProcess",
  "ModbusSetParams",
  "ModbusGetParams",
  "Start_Streaming",
  "Stop_Streaming",
  "SETFUNC_MAX",
};

JES jes;
IOS_STATUS ios_status;
IPS_STATUS ips_status;

pthread_t thread1, thread2, thread3, thread4, thread5, thread6;

pthread_mutex_t ip_suspendMutex_Dual[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};
pthread_cond_t ip_resumeCond_Dual[2] = {PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};

pthread_mutex_t ip_suspendMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ip_resumeCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t io_suspendMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t io_resumeCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mp_suspendMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mp_resumeCond = PTHREAD_COND_INITIALIZER;
extern unsigned char ios_cameraid;

int msqMainId; // for mainCtl & MQTT subscriber callback using
int msqIosId;  // for iosCtl & mainCtl using
int msqIpsId;  // for ips_process & mainCtl using
pthread_mutex_t msgQMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t msgQIosMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t msgQIpsMutex = PTHREAD_MUTEX_INITIALIZER;
uint8_t ip_suspendFlag, io_suspendFlag, mp_suspendFlag;
uint8_t ip_doneFlag=0, io_doneFlag=0, mp_doneFlag=0;

uint8_t ip_suspendFlag_Dual[2];
uint8_t ip_doneFlag_Dual[2]={0, 0};

static int mp_FlowStatus = -1;  // default
static uint32_t watchDogCounter = 0; //, count = 0;
bool enable_warchDog = FALSE;
IOS_TRIGGER_SET_PROCESS ios_trigger_previous_value;
bool bTearDown = false;
char trig_pin[6];
char trig_edge[6][16];          // rising / falling
char trig_dout_pin[6];
char trig_dout_active[6][16];   // rising / falling
char trig_DinMode = 0;
char trig_DinMode_Dual = 0;
char trig_trigger1 = 0;
char trig_trigger2 = 0;
uint8_t wakeIPSFlag = 0;
uint8_t wakeIPSFlag_Dual = 0;

extern const char* enum_Publish_CAMReg[];


/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */
extern int ios_triggerSetProcess(IOS_TRIGGER_SET_PROCESS *);
extern int ios_triggerGetProcess(IOS_TRIGGER_SET_PROCESS *);
extern int ios_ledSetProcess(IOS_LED_SET_PROCESS *);
extern int ios_lightSetPWM(IOS_LIGHT_SET_PWM *);
extern int ios_lightGetPWM(IOS_LIGHT_SET_PWM *);
extern int ios_dinSetProcess(IOS_DI_SET_PROCESS *);
extern int ios_doutSetProcess(IOS_DOUT_SET_PROCESS *);
extern int ios_getStatus(IOS_IO_GET_STATUS *);
extern void ios_modbusSetParameters(IOS_MODBUS_SET_PARAMS *);
extern void ios_modbusGetParameters(IOS_MODBUS_SET_PARAMS *);
extern void ios_modbusGITesting(uint8_t GI_Test_Status, uint8_t GI_Testing_Result);
extern void ios_LED_Status_Handler(void);
extern void ios_Control_Light_Handler(uint8_t LightPin, uint8_t Enable);
extern int ios_Control_Dout_Handler(const char *selectmode, uint8_t Enable);
extern int ios_Control_Dout_Handler_Dual(char *selectmode, uint8_t Enable, uint8_t CameraId);
extern int IO_SetLocalTime(bool use_ntp, struct tm *new_tm, char *timezone);
extern int ios_readEthAddr(char *eth, char *jstring);
// extern int ios_readVersionFile(char *jstring);
extern void setStatus_mp(int pStatus);
// extern int32_t ext_mqtt_subscriber_Dual();
extern int ios_sfc_send_msg(IOS_SFC_SET_MODE *ios_sfc);
extern int sfcCtl_serial_port;
extern int tofReadDistance(void);

/*********************************************************************
        Prototype
*********************************************************************/
extern int mqtt_init();
int msgQ_send(uint8_t *msg);


/* EXPORTED SUBPROGRAM BODIES -------------------------------------------------------- */
/***********************************************************
 *	Function 	: sigExit_main
 *	Description : Signal exit handler for the main program
 *	Param 		: int sig : Signal number
 *	Return		: NONE
 *************************************************************/
void sigExit_main(int sig)
{
    fprintf(stderr, "%s()%d: Error: something crash fail.\n", __FUNCTION__, __LINE__);

    suspend_ip_Dual(0);         //Dual camera
    suspend_ip_Dual(1);         //Dual camera
    suspend_io();
    bTearDown = true;
    suspend_mp();
    usleep(30000);
    close(sfcCtl_serial_port);

    ExModeQ_Destory();          //Single camera
    ExModeQ_Destory_Dual(0);    //Dual camera
    ExModeQ_Destory_Dual(1);    //Dual camera
    TasksQ_Destory();           //Single camera
    TasksQ_Destory_Dual(0);     //Dual camera
    TasksQ_Destory_Dual(1);     //Dual camera
    JsonQ_Destory();            //Single camera
    JsonQ_Destory_Dual(0);      //Dual camera
    JsonQ_Destory_Dual(1);      //Dual camera
    innerQ_IOS_Destory();
    innerQ_IPS_Destory_Dual(0); //Dual camera
    innerQ_IPS_Destory_Dual(1); //Dual camera
    innerQ_Main_Destory();
    usleep(30000);

    close_ip_Dual(0);           //Dual camera
    close_ip_Dual(1);           //Dual camera
    close_io();
    close_mp();

    ext_mqtt_release();

    ipsComp_Camera_Release_Dual(0);     //Dual camera >> Camera handle 
    ipsComp_Camera_Release_Dual(1);     //Dual camera >> Camera handle 
    ipsComp_IPL_Release();

    usleep(30000);

    MAINLOG(0, "[MAIN]  pthread_join(thread3, NULL)\n");
    if (pthread_join(thread3, NULL) != 0) { MAINLOG(0, " !! __ Error, pthread_join() of thread 4\n"); }    

    MAINLOG(0, "[MAIN]  pthread_join(thread4, NULL)\n");
    if (pthread_join(thread4, NULL) != 0) { MAINLOG(0, " !! __ Error, pthread_join() of thread 4\n"); }

    MAINLOG(0, "[MAIN]  pthread_join(thread5, NULL)\n");
    if (pthread_join(thread5, NULL) != 0) { MAINLOG(0, " !! __ Error, pthread_join() of thread 5\n"); }

    MAINLOG(0, "[MAIN]  pthread_join(thread2, NULL)\n");
    if (pthread_join(thread2, NULL) != 0) { MAINLOG(0, " !! __ Error, pthread_join() of thread 2\n"); }

    MAINLOG(0, "[MAIN]  pthread_join(thread1, NULL)\n");
    if (pthread_join(thread1, NULL) != 0) { MAINLOG(0, " !! __ Error, pthread_join() of thread 1\n"); }

    bTearDown = true;

    MAINLOG(0, "[MAIN] Got %s signal\n", __func__);
    MAINLOG(0, "[MAIN] signal %d caught\n", sig);

    fflush(stderr);
    fflush(stdout);
    exit(1);
}

/***********************************************************
 *	Function 	: main_gettime_ms
 *	Description : Get current time(ms)
 *	Param 		: NONE
 *	Return		: msec
 *************************************************************/
static uint32_t main_gettime_ms(void)
{
    struct timeval tv;
#if !defined(_MSC_VER)
    gettimeofday(&tv, nullptr);
    return (uint32_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
    return GetTickCount();
#endif
}

/***********************************************************
 *	Function 	: ios_response_json_create
 *	Description : create IOS json format for responsing to backend
 *	Param 		: char * string : generated json string
 *                char *cmd :
 *                const char *pset : copy from backend args contents
 *	Return		: error number
 *************************************************************/
int ios_response_json_create(char *jstring, const char *cmd, const char *pset)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr || cmd == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string(cmd));
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    json_object_object_add(obj_3, "pset", json_object_new_string(pset));

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_trigger_get_process_json_create
 *	Description : create IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_trigger_get_process_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("TRIGGER_SET_PROCESS"));

    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));

    /* only send the command, other parameter has been deposited in the structure */
    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_din_get_process_json_create
 *	Description : create IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_din_get_process_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("DIN_SET_PROCESS"));

    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));

    /* only send the command, other parameter has been deposited in the structure */
    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_get_status_json_create
 *	Description : create IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_get_status_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("DIO_SET_STATUS"));
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));
    json_object_object_add(obj_3, "result", obj_4);
    json_object_object_add(obj_4, "dout1", json_object_new_int(ios_getstatus.dout1));
    json_object_object_add(obj_4, "dout2", json_object_new_int(ios_getstatus.dout2));
    json_object_object_add(obj_4, "dout3", json_object_new_int(ios_getstatus.dout3));
    json_object_object_add(obj_4, "dout4", json_object_new_int(ios_getstatus.dout4));
    json_object_object_add(obj_4, "din1", json_object_new_int(ios_getstatus.din1));
    json_object_object_add(obj_4, "din2", json_object_new_int(ios_getstatus.din2));
    json_object_object_add(obj_4, "din3", json_object_new_int(ios_getstatus.din3));
    json_object_object_add(obj_4, "din4", json_object_new_int(ios_getstatus.din4));
    json_object_object_add(obj_4, "ailed_detect", json_object_new_int(ios_getstatus.ailed_detect));

    /* only send the command, other parameter has been deposited in the structure */
    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_light_set_pwm_json_create
 *	Description : create IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_light_set_pwm_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LIGHT_GET_PWM"));
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));
    /* only send the command, other parameter has been deposited in the structure */

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_light_set_brightness_json_create
 *	Description : create IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_light_get_brightness_json_create(char *jstring, uint16_t *brightness)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LIGHT_SET_BRIGHTNESS"));

    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));
    json_object_object_add(obj_3, "result", obj_4);
    json_object_object_add(obj_4, "lightSource1", json_object_new_int(brightness[0]));
    json_object_object_add(obj_4, "lightSource2", json_object_new_int(brightness[1]));
    /* only send the command, other parameter has been deposited in the structure */

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_light_get_pwm_json_create
 *	Description : create IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_light_get_pwm_json_create(char *jstring)
{
    struct json_object *root, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LIGHT_SET_PWM"));
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_3);
    // obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));
    /* only send the command, other parameter has been deposited in the structure */

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_led_get_mode_json_create
 *	Description : create IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_led_get_mode_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LED_GET_MODE"));
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));
    json_object_object_add(obj_3, "result", obj_4);

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_led_get_process_json_create
 *	Description : create IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_led_get_process_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "IOS", obj_2);
    json_object_object_add(obj_2, "cmd", json_object_new_string("LED_SET_PROCESS"));

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "args", obj_3);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);
    json_object_object_add(obj_4, "outPin", json_object_new_int(ios_setled.outPin));
    json_object_object_add(obj_4, "outMode", json_object_new_string((char *)ios_setled.outMode));
    json_object_object_add(obj_4, "outStatus", json_object_new_string((char *)ios_setled.outStatus));
    json_object_object_add(obj_4, "outBlinkDelay", json_object_new_int64(ios_setled.outBlinkDelay));
    json_object_object_add(obj_4, "outOffDelay", json_object_new_int64(ios_setled.outOffDelay));
    /* only send the command, other parameter has been deposited in the structure */

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_modbus_get_params_json_create
 *	Description : create IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_modbus_get_params_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "IOS", obj_2);
    json_object_object_add(obj_2, "cmd", json_object_new_string("MODBUS_SET_PARAMS"));

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "args", obj_3);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "result", obj_4);
    json_object_object_add(obj_4, "Baudrate", json_object_new_int64(ios_modbusSetParams.RTU_BAUDRATE));
    json_object_object_add(obj_4, "GITriggerRegister", json_object_new_int(ios_modbusSetParams.GI_Testing_Register));
    json_object_object_add(obj_4, "GIStatusRegister", json_object_new_int(ios_modbusSetParams.GI_Test_Status_Register));
    json_object_object_add(obj_4, "GIResultRegister", json_object_new_int(ios_modbusSetParams.GI_Testing_Result_Register));

    /* only send the command, other parameter has been deposited in the structure */

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_ips_json_create
 *	Description : create IPS & IOS json format for testing
 *	Param 		: char * string
 *	Return		: error number
 *************************************************************/
int ios_ips_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5;

    if (jstring == nullptr)
        return -1;
    /* create IOS format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "IOS", obj_2);
    json_object_object_add(obj_2, "cmd", json_object_new_string("TRIGGER_SET_PROCESS"));

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "args", obj_3);
    json_object_object_add(obj_3, "InMode", json_object_new_string("PNP"));
    json_object_object_add(obj_3, "InPin", json_object_new_int(1));
    json_object_object_add(obj_3, "Delay", json_object_new_int(1000));
    json_object_object_add(obj_3, "OutMode", json_object_new_string("PNP"));
    json_object_object_add(obj_3, "OutPin", json_object_new_int(4));
    /* create IPS format */
    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "IPS", obj_4);
    json_object_object_add(obj_4, "cmd", json_object_new_string("CAMERA_SET_MODE"));

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_4, "args", obj_5);
    json_object_object_add(obj_5, "CameraType", json_object_new_string("Mono"));
    json_object_object_add(obj_5, "TriggerMode", json_object_new_string("Software"));
    json_object_object_add(obj_5, "ExposureTime", json_object_new_int(100));
    json_object_object_add(obj_5, "SaveImgOpt", json_object_new_string("Every Image"));
    json_object_object_add(obj_5, "SaveImgPath", json_object_new_string("ExeCurrentFile/ProjectName/Grab/source.png"));

    memset(jstring, '\0', strlen(jstring));
    // sprintf(jstring, (char *)json_object_to_json_string(root));
    strcpy(jstring, (char *)json_object_to_json_string(root));
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_sys_get_params_json_create
 *	Description : create FW version json format
 *	Param 		: char * string
 *	Return		: NONE
 *************************************************************/
int ios_sys_get_params_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IO_SYS_GET_PARAMS"));

    json_object_object_add(root, "args", obj_2);
    char buf[128];
    ios_readVersionFile(&buf[0]);
    json_object_object_add(obj_2, "fw_vision", json_object_new_string(&buf[0]));

    std::string strVSB_VERSION(VSB_VERSION);
    json_object_object_add(obj_2, "vsb_vision", json_object_new_string(strVSB_VERSION.c_str()));

    std::string strIOS_VERSION(IOS_VERSION);
    json_object_object_add(obj_2, "ios_vision", json_object_new_string(strIOS_VERSION.c_str()));
    
    ios_readEthAddr((char *)"eth0", &buf[0]);
    json_object_object_add(obj_2, "eth0_ipaddr", json_object_new_string(&buf[0]));
    
    ios_readEthAddr((char *)"eth1", &buf[0]);
    json_object_object_add(obj_2, "eth1_ipaddr", json_object_new_string(&buf[0]));
    
    ios_readEthAddr((char *)"eth2", &buf[0]);
    json_object_object_add(obj_2, "eth2_ipaddr", json_object_new_string(&buf[0]));
    
    ios_readEthAddr((char *)"wlan0", &buf[0]);
    json_object_object_add(obj_2, "wlan0_ipaddr", json_object_new_string(&buf[0]));

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "pset", obj_4);
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));
    /* only send the command, other parameter has been deposited in the structure */

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_sfc_params_json_create
 *	Description : create FW version json format
 *	Param 		: char * string
 *	Return		: NONE
 *************************************************************/
int ios_sfc_params_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IO_SHOP_FLOOR_CONTROL"));


    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));
    json_object_object_add(obj_3, "msg", json_object_new_string(&ios_sfc.msg[0]));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "pset", obj_4);
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));
    /* only send the command, other parameter has been deposited in the structure */

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

/***********************************************************
 *	Function 	: ios_get_tof_json_create
 *	Description : create ToF json format
 *	Param 		: char * string
 *	Return		: NONE
 *************************************************************/
int ios_get_tof_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IO_TOF_GET_PARAM"));

    json_object_object_add(root, "args", obj_2);
    char buf[128];
    ios_readVersionFile(&buf[0]);
    json_object_object_add(obj_2, "distance", json_object_new_int(ios_tof.distance));

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_3);
    json_object_object_add(obj_3, "status", json_object_new_string("Ack"));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_3, "pset", obj_4);
    json_object_object_add(obj_3, "pset", json_object_new_string((char *)ios_CmdInfo));
    /* only send the command, other parameter has been deposited in the structure */

    memset(jstring, '\0', strlen(jstring));
    const char *buf_cst = json_object_to_json_string(root);
    sprintf(jstring, "%s", buf_cst);
    MAINLOG(0, "%s json : %s\n", __func__, jstring);
    json_object_put(root);
    return 0;
}

pthread_mutex_t _innerQ_main_lock = PTHREAD_MUTEX_INITIALIZER;
std::deque<std::string> buf_innerQ_main;
/***********************************************************
 *	Function 	: innerQ_Main_Init
 *	Description : To initiate _innerQ_main_lock
 *	Param 		: NONE
 *	Return		: error number
 *************************************************************/
int innerQ_Main_Init()
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_innerQ_main_lock, nullptr);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_Main_EnQ
 *	Description : To push back the queue: buf_innerQ_main
 *	Param 		: std::string msg
 *	Return		: error number
 *************************************************************/
int innerQ_Main_EnQ(std::string msg)
{
    int res = 0;
    pthread_mutex_lock(&_innerQ_main_lock);

    buf_innerQ_main.push_back(msg);

    pthread_mutex_unlock(&_innerQ_main_lock);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_Main_DeQ
 *	Description : To pop front of the queue: buf_innerQ_main
 *	Param 		: return message: std::string *msg
 *	Return		: error number
 *************************************************************/
int innerQ_Main_DeQ(std::string *msg)
{
    int res = 0;
    pthread_mutex_lock(&_innerQ_main_lock);

    if (!buf_innerQ_main.empty())
    {

        *msg = *buf_innerQ_main.begin();

        buf_innerQ_main.pop_front();
    }
    else
    {

        res = -1;
    }

    pthread_mutex_unlock(&_innerQ_main_lock);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_Main_IsEmpty
 *	Description : To check buf_innerQ_main is empty or not
 *	Param 		: NONE
 *	Return		: true, false
 *************************************************************/
bool innerQ_Main_IsEmpty()
{
    bool isEmpty = false;
    pthread_mutex_lock(&_innerQ_main_lock);
    isEmpty = buf_innerQ_main.empty();
    pthread_mutex_unlock(&_innerQ_main_lock);
    return isEmpty;
}

/***********************************************************
 *	Function 	: innerQ_Main_Destory
 *	Description : To release the lock: _innerQ_main_lock
 *	Param 		: NONE
 *	Return		: NONE
 *************************************************************/
void innerQ_Main_Destory()
{
    pthread_mutex_destroy(&_innerQ_main_lock);
}

pthread_mutex_t _innerQ_ips_lock = PTHREAD_MUTEX_INITIALIZER;
std::deque<std::string> buf_innerQ_ips;
/***********************************************************
 *	Function 	: innerQ_IPS_Init
 *	Description : To initiate _innerQ_ips_lock
 *	Param 		: NONE
 *	Return		: error number
 *************************************************************/
int innerQ_IPS_Init()
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_innerQ_ips_lock, nullptr);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_IPS_EnQ
 *	Description : To push back the queue: buf_innerQ_ips
 *	Param 		: std::string msg
 *	Return		: error number
 *************************************************************/
int innerQ_IPS_EnQ(std::string msg)
{
    int res = 0;
    pthread_mutex_lock(&_innerQ_ips_lock);

    buf_innerQ_ips.push_back(msg);

    pthread_mutex_unlock(&_innerQ_ips_lock);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_IPS_DeQ
 *	Description : To pop front of the queue: buf_innerQ_ips
 *	Param 		: return message: std::string *msg
 *	Return		: error number
 *************************************************************/
int innerQ_IPS_DeQ(std::string *msg)
{
    int res = 0;
    pthread_mutex_lock(&_innerQ_ips_lock);

    if (!buf_innerQ_ips.empty())
    {

        *msg = *buf_innerQ_ips.begin();

        buf_innerQ_ips.pop_front();
    }
    else
    {

        res = -1;
    }

    pthread_mutex_unlock(&_innerQ_ips_lock);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_IPS_IsEmpty
 *	Description : To check buf_innerQ_ips is empty or not
 *	Param 		: NONE
 *	Return		: true, false
 *************************************************************/
bool innerQ_IPS_IsEmpty()
{

    bool isEmpty = false;
    pthread_mutex_lock(&_innerQ_ips_lock);
    isEmpty = buf_innerQ_ips.empty();
    pthread_mutex_unlock(&_innerQ_ips_lock);
    return isEmpty;
}

/***********************************************************
 *	Function 	: innerQ_IPS_Destory
 *	Description : To release the lock: _innerQ_ips_lock
 *	Param 		: NONE
 *	Return		: NONE
 *************************************************************/
void innerQ_IPS_Destory()
{
    pthread_mutex_destroy(&_innerQ_ips_lock);
}

pthread_mutex_t _innerQ_ips_lock_Dual[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};
std::vector<std::deque<std::string>> buf_innerQ_ips_Dual(2);
/***********************************************************
 *	Function 	: innerQ_IPS_Init_Dual
 *	Description : To initiate _innerQ_ips_lock_Dual
 *	Param 		: NONE
 *	Return		: error number
 *************************************************************/
int innerQ_IPS_Init_Dual(const int iID)
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_innerQ_ips_lock_Dual[iID], nullptr);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_IPS_EnQ_Dual
 *	Description : To push back the queue: buf_innerQ_ips_Dual
 *	Param 		: std::string msg
 *	Return		: error number
 *************************************************************/
int innerQ_IPS_EnQ_Dual(std::string msg, const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_innerQ_ips_lock_Dual[iID]);

    buf_innerQ_ips_Dual[iID].push_back(msg);

    pthread_mutex_unlock(&_innerQ_ips_lock_Dual[iID]);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_IPS_DeQ_Dual
 *	Description : To pop front of the queue: _innerQ_ips_lock_Dual
 *	Param 		: return message: std::string *msg
 *	Return		: error number
 *************************************************************/
int innerQ_IPS_DeQ_Dual(std::string *msg, const int iID)
{
    int res = 0;
    pthread_mutex_lock(&_innerQ_ips_lock_Dual[iID]);

    if (!buf_innerQ_ips_Dual[iID].empty())
    {
        *msg = *buf_innerQ_ips_Dual[iID].begin();

        buf_innerQ_ips_Dual[iID].pop_front();
    }
    else
    {
        res = -1;
    }

    pthread_mutex_unlock(&_innerQ_ips_lock_Dual[iID]);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_IPS_IsEmpty_Dual
 *	Description : To check buf_innerQ_ips_Dual is empty or not
 *	Param 		: NONE
 *	Return		: true, false
 *************************************************************/
bool innerQ_IPS_IsEmpty_Dual(const int iID)
{

    bool isEmpty = false;
    pthread_mutex_lock(&_innerQ_ips_lock_Dual[iID]);
    isEmpty = buf_innerQ_ips_Dual[iID].empty();
    pthread_mutex_unlock(&_innerQ_ips_lock_Dual[iID]);
    return isEmpty;
}

/***********************************************************
 *	Function 	: innerQ_IPS_Destory_Dual
 *	Description : To release the lock: _innerQ_ips_lock_Dual
 *	Param 		: NONE
 *	Return		: true, false
 *************************************************************/
void innerQ_IPS_Destory_Dual(const int iID)
{
    pthread_mutex_destroy(&_innerQ_ips_lock_Dual[iID]);
}

pthread_mutex_t _innerQ_ios_lock = PTHREAD_MUTEX_INITIALIZER;
std::deque<std::string> buf_innerQ_ios;
/***********************************************************
 *	Function 	: innerQ_IOS_Init
 *	Description : To initiate _innerQ_ios_lock
 *	Param 		: NONE
 *	Return		: error number
 *************************************************************/
int innerQ_IOS_Init()
{
    /* init mutex lock */
    int res = pthread_mutex_init(&_innerQ_ios_lock, nullptr);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_IOS_EnQ
 *	Description : To push back the queue: buf_innerQ_ios
 *	Param 		: std::string msg
 *	Return		: error number
 *************************************************************/
int innerQ_IOS_EnQ(std::string msg)
{
    int res = 0;
    pthread_mutex_lock(&_innerQ_ios_lock);

    buf_innerQ_ios.push_back(msg);

    pthread_mutex_unlock(&_innerQ_ios_lock);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_IOS_DeQ
 *	Description : To pop front of the queue: buf_innerQ_ios
 *	Param 		: return message: std::string *msg
 *	Return		: error number
 *************************************************************/
int innerQ_IOS_DeQ(std::string *msg)
{
    int res = 0;
    pthread_mutex_lock(&_innerQ_ios_lock);

    if (!buf_innerQ_ios.empty())
    {
        *msg = *buf_innerQ_ios.begin();

        buf_innerQ_ios.pop_front();
    }
    else
    {

        res = -1;
    }

    pthread_mutex_unlock(&_innerQ_ios_lock);
    return res;
}

/***********************************************************
 *	Function 	: innerQ_IOS_IsEmpty
 *	Description : To check buf_innerQ_ios is empty or not
 *	Param 		: NONE
 *	Return		: true, false
 *************************************************************/
bool innerQ_IOS_IsEmpty()
{

    bool isEmpty = false;
    pthread_mutex_lock(&_innerQ_ios_lock);
    isEmpty = buf_innerQ_ios.empty();
    pthread_mutex_unlock(&_innerQ_ios_lock);
    return isEmpty;
}

/***********************************************************
 *	Function 	: innerQ_IOS_Destory
 *	Description : To release the lock: _innerQ_ios_lock
 *	Param 		: NONE
 *	Return		: true, false
 *************************************************************/
void innerQ_IOS_Destory()
{
    pthread_mutex_destroy(&_innerQ_ios_lock);
}

/***********************************************************
 *	Function 	: kbhit
 *	Description : non-block program to get the keyborad input
 *                to check the key buffer, once large than 0
 *                meaning the keyboard is pressed
 *	Param 		: NONE
 *	Return		: 0, 1
 *************************************************************/
int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

/***********************************************************
 *	Function 	: Show_Process_State
 *	Description : To show the current process state.
 *	Param 		: uint8_t currentState: the current process state.
 *	Return		: NONE
 *************************************************************/
uint32_t ShowProcessCounter = 0;
uint32_t ShowProcessCountTarget = 1000;
uint8_t PreState = 254;
void Show_Process_State(uint8_t currentState)
{
    if (PreState != currentState)
    {
        PreState = currentState;
        MAINLOG(0, RED "[Main](%s):(Diff)  Current state of Process is : %d\n", __func__, currentState);
    }

    if (ShowProcessCounter == ShowProcessCountTarget)
    {
        MAINLOG(0, RED "[Main](%s):(Times) Current state of Process is : %d\n", __func__, currentState);
        ShowProcessCounter = 0;
    }
    else
    {
        ShowProcessCounter += 1;
    }
}

/***********************************************************
 *	Function 	: Process_Flow_Handler
 *	Description : Process state machine
 *	Param 		: NONE
 *	Return		: NONE
 *************************************************************/
void Process_Flow_Handler(void)
{
    std::string strInfo;

    if (!Process_Node.Active)
    {
        return;
    }
    switch (Process_Node.autoMode)
    {
        case GLUE_INSPECTION:
        {
            int iId = 0, iStatus = 0, m;
            uint8_t rec_data[MAX_MSG_SIZE];
            seAlgoParamReg *pAlgoParam = nullptr;
            seMode_TriggerModeType *pJP = nullptr;
            seExpansionMode iEnbExMode = tagExpansionMode();

            
            switch (Process_Node.giFlow)
            {
                case GI_START: {
                    /* firstall, check msqIosID whether exists remaining queue */
                    m = innerQ_IOS_IsEmpty();
                    if (m != 1)
                    {

                        for (int i = 0; i < m; i++)
                        {
                            innerQ_IOS_DeQ(&strInfo);
                            strcpy((char*)rec_data, strInfo.c_str());
                            MAINLOG(0, BLUE "[MAINCTL](%s) : removed one Q from msqIosId\n", __func__);
                        }
                    }
                    
                    if (ios_Control_Dout_Handler_Dual((char *)"AutoDone", FALSE, 0) == 1)
                    {
                        
                        strcpy((char*)Process_Node.TestResult, "");
                        ios_Control_Light_Handler(LIGHT_SOURCE_1, FALSE);
                        ios_modbusGITesting(0, 0); /* Test Status: VB Ready; Testing Result: No Result */
                        wakeIPSFlag = 0;
                        setStatus_mp(-1);
                        /* go next step */
                        Process_Node.giFlow = GI_WAIT_TRIGGER;
                        
                    }
                    
                    break;
                }
                case GI_WAIT_TRIGGER: {
                    m = innerQ_IOS_IsEmpty();
                    if (m != 1)
                    {

                        printf("%s()%d: msqIosId has received...\n", __FUNCTION__, __LINE__);

                        innerQ_IOS_DeQ(&strInfo);
                        strcpy((char*)rec_data, strInfo.c_str());
                        printf("%s()%d: rec_data=%s\n", __FUNCTION__, __LINE__, rec_data);
                        if (strcmp((char*)rec_data, "Din_Trigger") == 0)
                        {
                            ios_Control_Dout_Handler_Dual((char *)"Result", FALSE, 0);
                            ios_Control_Light_Handler(LIGHT_SOURCE_1, TRUE);
                            ios_modbusGITesting(1, 0); /* Test Status: VB Busy; Testing Result: No Result */
                            Process_Node.giFlow = GI_EXCUTE_AUTO_GI;
                        }
                        else
                        {
                            usleep(10000);
                            innerQ_IOS_EnQ((char *)"Din_Trigger_Dual");
                            MAINLOG(0, BLUE "[MAINCTL](%s) : _____GI_WAIT_TRIGGER\n", __func__);
                        }
                    }
                    break;
                }
                case GI_EXCUTE_AUTO_GI: {
                    usleep(10000);
                    /* waiting for Rex release audo mode API */ //<---TBD
                    MAINLOG(0, BLUE "[MAINCTL](%s) : GI_EXCUTE_AUTO_GI ===> \n", __func__);

                    if (wakeIPSFlag == 0) {
                        iId = (enum_IdReg)FLAGE_TRIGGERMODETYPE;
                        pAlgoParam = &gAlgoParamReg[iId];
                        pJP = (seMode_TriggerModeType *)pAlgoParam->pParam;
                        pJP->bFlg_TriggerMode_Activate = true;
                        iEnbExMode.flg_TriggerMode_Activat = pJP->bFlg_TriggerMode_Activate;
                        ExModeQ_EnQ_Dual(iEnbExMode, 0);
                        //setAlgo_MethodAssign(enum_Subscribe_CAMReg[iId], "{ \"Enb_TriggerMode_Activate\": 1 }");
                        setAlgo_MethodAssign_Dual(enum_Subscribe_CAMReg[iId], "{ \"Enb_TriggerMode_Activate\": 1 }", 0);
                        resume_ip_Dual(0);
                        wakeIPSFlag = 1;
                    }
                    // suspend_mp();

                    // pthread_mutex_lock(&mp_suspendMutex);
                    // while (mp_suspendFlag != 0)
                    // {
                    //     pthread_cond_wait(&mp_resumeCond, &mp_suspendMutex);
                    // }
                    // pthread_mutex_unlock(&mp_suspendMutex);

                    getStatus_mp(&iStatus);
                    if (iStatus == 0)
                    { // This number of -1 or 0 is mean not finish the AutoRun Mode flow.
                        MAINLOG(0, BLUE "[MAINCTL](%s) : AutoRunMode is Error!! Go to GI_GET_TEST_RESULT\n", __func__);
                        ios_modbusGITesting(1, 2); /* Test Status: VB Busy; Testing Result: Fail */
                        Process_Node.giFlow = GI_GET_TEST_RESULT;
                    }
                    else if (iStatus == 1)
                    { // This numbe of 1 is mean done the AutiRun Mode flow.
                        MAINLOG(0, BLUE "[MAINCTL](%s) : AutoRunMode is Done!! Go to GI_GET_TEST_RESULT\n", __func__);
                        ios_modbusGITesting(1, 1); /* Test Status: VB Busy; Testing Result: Pass */
                        Process_Node.giFlow = GI_GET_TEST_RESULT;
                    } else {
                        Process_Node.giFlow = GI_EXCUTE_AUTO_GI;
                    }
                            // MAINLOG(0, BLUE "[MAINCTL](%s) : GI_EXCUTE_AUTO_GI iStatus=[%d] <=== \n", __func__, iStatus);
                    break;
                }
                case GI_GET_TEST_RESULT: {
                    /* result be judged by web backend, we dirctly go back to GI_START */

                    m = innerQ_IPS_IsEmpty_Dual(0);
                    if (m != 1)
                    {
                        innerQ_IPS_DeQ_Dual(&strInfo, 0);
                        strcpy((char*)rec_data, strInfo.c_str());

                        if (strcmp((char*)rec_data, "Auto_Mode_Done") == 0)
                        {
                            MAINLOG(0, BLUE "[MAINCTL](%s) : Auto mode done \n", __func__);
                        }
                        else
                        {
                            MAINLOG(0, BLUE "[MAINCTL](%s) : Auto mode fail \n", __func__);
                        }

                        ios_Control_Light_Handler(LIGHT_SOURCE_1, FALSE);
                        /* Dout ON for indicating auto mode done */
                        /*if (ios_Control_Dout_Handler_Dual((char *)"AutoDone", TRUE, 0) == 1)
                        {
                            MAINLOG(0, BLUE "[MAINCTL](%s) : Set Dout ON for AutoDone \n", __func__);
                            unsigned int rvalue = 0;
                            char job_t[512];
                            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"CAM1_AUTODONE_RESP\", \"args\":{ \"result\":%d }}", rvalue);
                            ext_mqtt_publisher_Dual(&job_t[0], 0);
                        }*/
                    }
                    /* wait for test result */
                    if ((strcmp((const char*)Process_Node.TestResult, "PASS") == 0) ||
                        (strcmp((const char*)Process_Node.TestResult, "NG") == 0))
                    {
                        if (strcmp((const char*)Process_Node.TestResult, "PASS") == 0)
                        {
                            if (ios_Control_Dout_Handler_Dual((char *)"Result", TRUE, 0) == 1)
                            {
                                MAINLOG(0, BLUE "[MAINCTL](%s) : Test Result: %s\n", __func__, Process_Node.TestResult);
                                /* go back to start state */
                                Process_Node.giFlow = GI_START;
                                MAINLOG(0, BLUE "[MAINCTL](%s) : Go to Start State!!!\n", __func__);
                            }
                        }
                        else
                        {
                            if (ios_Control_Dout_Handler_Dual((char *)"Result", FALSE, 0) == 1)
                            {
                                MAINLOG(0, BLUE "[MAINCTL](%s) : Test Result: %s\n", __func__, Process_Node.TestResult);
                                /* go back to start state */
                                Process_Node.giFlow = GI_START;
                                MAINLOG(0, BLUE "[MAINCTL](%s) : Go to Start State!!!\n", __func__);
                            }
                        }
                        usleep(500000);
                        Process_Node.giFlow = GI_START;
                        if (ios_Control_Dout_Handler_Dual((char *)"AutoDone", TRUE, 0) == 1)
                        {
                            // MAINLOG(0, "Turn ON Dout\n");
                            MAINLOG(0, BLUE "[MAINCTL](%s) : Set Dout ON for AutoDone \n", __func__);
                            unsigned int rvalue = 0;
                            char job_t[512];
                                    snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"CAM1_AUTODONE_RESP\", \"args\":{ \"result\":%d }}", rvalue);
                                    ext_mqtt_publisher_Dual(&job_t[0], 0);
                                }
                    }
                    //MAINLOG(0, BLUE "[MAINCTL](%s) : GI_GET_TEST_RESULT Result=[%s] <=== \n", __func__, Process_Node.TestResult);
                    break;
                }
                default: {
                    break;
                }
            }
            
            Show_Process_State(Process_Node.giFlow);
        }
        break;

        case NON_AUTO:
        default:
            break;
    }
}

/***********************************************************
 *	Function 	: Process_Flow_Handler_Dual
 *	Description : Process state machine
 *	Param 		: NONE
 *	Return		: NONE
 *************************************************************/
void Process_Flow_Handler_Dual(void)
{
    std::string strInfo;

    if (!Process_Node_Dual.Active)
    {
        return;
    }    

    switch (Process_Node_Dual.autoMode)
    {
        case GLUE_INSPECTION:
        {
            int iId = 0, iStatus = 0, m;
            uint8_t rec_data[MAX_MSG_SIZE];
            seAlgoParamReg *pAlgoParam = nullptr;
            seMode_TriggerModeType *pJP = nullptr;
            seExpansionMode iEnbExMode = tagExpansionMode();

            switch (Process_Node_Dual.giFlow)
            {   
                case GI_START: {
                    /* firstall, check msqIosID whether exists remaining queue */
                    m = innerQ_IOS_IsEmpty();
                    if (m != 1)
                    {

                        for (int i = 0; i < m; i++)
                        {
                            innerQ_IOS_DeQ(&strInfo);
                            strcpy((char*)rec_data, strInfo.c_str());
                            MAINLOG(0, BLUE "[MAINCTL](%s) : removed one Q from msqIosId\n", __func__);
                        }
                    }

                    if (ios_Control_Dout_Handler_Dual((char *)"AutoDone", FALSE, 1) == 1)
                    {
                        strcpy((char*)Process_Node_Dual.TestResult, "");
                        ios_Control_Light_Handler(LIGHT_SOURCE_2, FALSE);
                        ios_modbusGITesting(0, 0); /* Test Status: VB Ready; Testing Result: No Result */
                        wakeIPSFlag_Dual = 0;
                        setStatus_mp(-1);
                        /* go next step */
                        Process_Node_Dual.giFlow = GI_WAIT_TRIGGER;
                    }
                    break;
                }
                case GI_WAIT_TRIGGER: {
                    m = innerQ_IOS_IsEmpty();
                    if (m != 1)
                    {

                        printf("%s()%d: msqIosId has received dual...\n", __FUNCTION__, __LINE__);

                        innerQ_IOS_DeQ(&strInfo);
                        strcpy((char*)rec_data, strInfo.c_str());

                        printf("%s()%d: rec_data dual=%s\n", __FUNCTION__, __LINE__, rec_data);
                        if (strcmp((char*)rec_data, "Din_Trigger_Dual") == 0)
                        {
                            ios_Control_Dout_Handler_Dual((char *)"Result", FALSE, 1);
                            ios_Control_Light_Handler(LIGHT_SOURCE_2, TRUE);
                            ios_modbusGITesting(1, 0); /* Test Status: VB Busy; Testing Result: No Result */
                            Process_Node_Dual.giFlow = GI_EXCUTE_AUTO_GI;
                        }
                        else
                        {
                            usleep(10000);
                            innerQ_IOS_EnQ("Din_Trigger");
                            MAINLOG(0, BLUE "[MAINCTL](%s) : _____GI_WAIT_TRIGGER\n", __func__);
                        }
                    }
                    break;
                }
                case GI_EXCUTE_AUTO_GI: {
                    usleep(10000);
                    /* waiting for Rex release audo mode API */ //<---TBD
                    MAINLOG(0, BLUE "[MAINCTL](%s) : GI_EXCUTE_AUTO_GI Dual ===> \n", __func__);

                    if (wakeIPSFlag_Dual == 0) {
                        iId = (enum_IdReg)FLAGE_TRIGGERMODETYPE;
                        pAlgoParam = &gAlgoParamReg[iId];
                        pJP = (seMode_TriggerModeType *)pAlgoParam->pParam;
                        pJP->bFlg_TriggerMode_Activate = true;
                        iEnbExMode.flg_TriggerMode_Activat = pJP->bFlg_TriggerMode_Activate;
                        ExModeQ_EnQ_Dual(iEnbExMode, 1);
                        // setAlgo_MethodAssign(enum_Subscribe_CAMReg[iId], "{ \"Enb_TriggerMode_Activate\": 1 }");
                        setAlgo_MethodAssign_Dual(enum_Subscribe_CAMReg[iId], "{ \"Enb_TriggerMode_Activate\": 1 }", 1);
                        resume_ip_Dual(1);
                        wakeIPSFlag_Dual = 1;
                    }         
                    // suspend_mp();

                    // pthread_mutex_lock(&mp_suspendMutex);
                    // while (mp_suspendFlag != 0)
                    // {
                    //     pthread_cond_wait(&mp_resumeCond, &mp_suspendMutex);
                    // }
                    // pthread_mutex_unlock(&mp_suspendMutex);

                    getStatus_mp(&iStatus);
                    if (iStatus == 0)
                    { // This number of -1 or 0 is mean not finish the AutoRun Mode flow.
                        MAINLOG(0, BLUE "[MAINCTL](%s) : AutoRunMode is Error!! Go to GI_GET_TEST_RESULT\n", __func__);
                        ios_modbusGITesting(1, 2); /* Test Status: VB Busy; Testing Result: Fail */
                        Process_Node_Dual.giFlow = GI_GET_TEST_RESULT;
                    }
                    else if (iStatus == 1)
                    { // This numbe of 1 is mean done the AutiRun Mode flow.
                        MAINLOG(0, BLUE "[MAINCTL](%s) : AutoRunMode is Done!! Go to GI_GET_TEST_RESULT\n", __func__);
                        ios_modbusGITesting(1, 1); /* Test Status: VB Busy; Testing Result: Pass */
                        Process_Node_Dual.giFlow = GI_GET_TEST_RESULT;
                    } else {
                        Process_Node_Dual.giFlow = GI_EXCUTE_AUTO_GI;
                    }
                    // MAINLOG(0, BLUE "[MAINCTL](%s) : GI_EXCUTE_AUTO_GI iStatus=[%d] <=== \n", __func__, iStatus);
                    break;
                }
                case GI_GET_TEST_RESULT: {
                    /* result be judged by web backend, we dirctly go back to GI_START */

                    m = innerQ_IPS_IsEmpty_Dual(1);
                    if (m != 1)
                    {
                        innerQ_IPS_DeQ_Dual(&strInfo, 1);
                        strcpy((char*)rec_data, strInfo.c_str());

                        if (strcmp((char*)rec_data, "Auto_Mode_Done") == 0)
                        {
                            MAINLOG(0, BLUE "[MAINCTL](%s) : Auto mode done \n", __func__);
                        }
                        else
                        {
                            MAINLOG(0, BLUE "[MAINCTL](%s) : Auto mode fail \n", __func__);
                        }

                        ios_Control_Light_Handler(LIGHT_SOURCE_2, FALSE);

                        /* Dout ON for indicating auto mode done */
                        /*if (ios_Control_Dout_Handler_Dual((char *)"AutoDone", TRUE, 1) == 1)
                        {
                            MAINLOG(0, BLUE "[MAINCTL](%s) : Set Dout ON for AutoDone \n", __func__);
                            unsigned int rvalue = 0;
                            char job_t[512];
                            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"CAM2_AUTODONE_RESP\", \"args\":{ \"result\":%d }}", rvalue);
                            ext_mqtt_publisher_Dual(&job_t[0], 0);
                        }*/
                    }
                    /* wait for test result */
                    if ((strcmp((const char*)Process_Node_Dual.TestResult, (char *)"PASS") == 0) ||
                        (strcmp((const char*)Process_Node_Dual.TestResult, (char *)"NG") == 0))
                    {
                        if (strcmp((const char*)Process_Node_Dual.TestResult, (char *)"PASS") == 0)
                        {
                            if (ios_Control_Dout_Handler_Dual((char *)"Result", TRUE, 1) == 1)
                            {
                                MAINLOG(0, BLUE "[MAINCTL](%s) : Test Result: %s\n", __func__, Process_Node_Dual.TestResult);
                                /* go back to start state */
                                Process_Node_Dual.giFlow = GI_START;
                                MAINLOG(0, BLUE "[MAINCTL](%s) : Go to Start State!!!\n", __func__);
                            }
                        }
                        else
                        {
                            if (ios_Control_Dout_Handler_Dual((char *)"Result", FALSE, 1) == 1)
                            {
                                MAINLOG(0, BLUE "[MAINCTL](%s) : Test Result: %s\n", __func__, Process_Node_Dual.TestResult);
                                /* go back to start state */
                                Process_Node_Dual.giFlow = GI_START;
                                MAINLOG(0, BLUE "[MAINCTL](%s) : Go to Start State!!!\n", __func__);
                            }
                        }
                        usleep(500000);
                        Process_Node_Dual.giFlow = GI_START;
                        if (ios_Control_Dout_Handler_Dual((char *)"AutoDone", TRUE, 1) == 1)
                        {
                            // MAINLOG(0, "Turn ON Dout\n");
                            MAINLOG(0, BLUE "[MAINCTL](%s) : Set Dout ON for AutoDone \n", __func__);
                            unsigned int rvalue = 0;
                            char job_t[512];
                            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"CAM2_AUTODONE_RESP\", \"args\":{ \"result\":%d }}", rvalue);
                            ext_mqtt_publisher_Dual(&job_t[0], 0);
                        }
                    }
                    // MAINLOG(0, BLUE "[MAINCTL](%s) : GI_GET_TEST_RESULT Dual Result=[%s] <=== \n", __func__, Process_Node.TestResult);
                    break;
                }
                default: {
                    break;
                }
            }
            Show_Process_State(Process_Node_Dual.giFlow);
        }
        break;

        case NON_AUTO:
        default:
            break;
    }
}

/***********************************************************
 *	Function 	: FW_Mqtt_PriorityPass_Internal
 *	Description : Priority pass for internal Mqtt commmand
 *	Param 		: int nIsEndFunc
 *	Return		: error number
 *************************************************************/
int FW_Mqtt_PriorityPass_Internal(int nIsEndFunc = 0)
{
    int nRet = 0;

    if (!nIsEndFunc)
    {

        return 0;
    }

    seExpansionMode iEnbExMode;

    int iId_PriorityPass[] = {
        (enum_IdReg)METHOD_GigeCam_Inquiry,
    };

    const int PriorityPass_Cnt = (sizeof(iId_PriorityPass) / sizeof(iId_PriorityPass[0]));

    for (int i = 0; i < PriorityPass_Cnt; i++)
    {

        int iId = iId_PriorityPass[i];
        ExModeQ_EnQ(iEnbExMode);
        setAlgo_MethodAssign(enum_Subscribe_CAMReg[iId], "{ \"status\": \"FW_Mqtt_PriorityPass\" }");
    }

    resume_ip_Dual(0);
    resume_ip_Dual(1);

    return nRet;
}

/***********************************************************
 *	Function 	: mainCtl
 *	Description : handle all subsystem's sequency and
 *                communicate with web backend
 *	Param     : void *argu --> none
 *	Return    : NONE
 *************************************************************/
void *mainCtl(void *argu)
{
    uint8_t rec_data[MAX_MSG_SIZE];
    int m;

    char job_t[512];        /* need to define this buffer size */
      
    /* init mutex lock */
    pthread_mutex_init(&msgQMutex, nullptr);
    pthread_mutex_init(&msgQIosMutex, nullptr);

    /* init value */
    ips_status.new_job = NO_SETFUNC;
    jes.new_job = NO_SETFUNC;
    memset(&Process_Node, 0, sizeof(Process_Node));
    Process_Node.autoMode = NON_AUTO;
    Process_Node.giFlow = GI_START;
    
    memset(&Process_Node_Dual, 0, sizeof(Process_Node_Dual));
    Process_Node_Dual.autoMode = NON_AUTO;
    Process_Node_Dual.giFlow = GI_START;

    seJsonInfo seInfo;

    std::string strInfo;

    while (!mp_doneFlag)
    {
        m = innerQ_Main_IsEmpty();

        if (m != 1)
        {

            innerQ_Main_DeQ(&strInfo);
            strcpy((char*)rec_data, strInfo.c_str());

            {
                //=================   IOS process.START (Single camera)   ===================
                //seIO_JsonInfo seIOInfo;
                //if (!IO_JsonQ_DeQ(&seIOInfo))
                if(!IO_JsonQ_IsEmpty())
                {

                    MAINLOG(0, "%s : ======   IOS process.START   ======\n", __func__);

                    //MAINLOG(0, "%s : enum_IdReg emAlgoId : %d\n", __func__, seIOInfo.emAlgoId);
                    //MAINLOG(0, "%s : szCmd : %s\n", __func__, seIOInfo.szCmd);
                    MAINLOG(0, "%s : %s ---> start\n", __func__, "resume_ip()");

                    MAINLOG(0, "%s : ======   IOS process.END   ======\n", __func__);
                    resume_io();
                }
                //=================   IOS process.END (Single camera)    ===================

                //=================   IPS process.START (Single camera)   ===================

                if (!JsonQ_DeQ(&seInfo))
                {

                    MAINLOG(0, "%s : ======   IPS process.START   ======\n", __func__);

                    MAINLOG(0, "%s : enum_IdReg emAlgoId : %d\n", __func__, seInfo.emAlgoId);
                    MAINLOG(0, "%s : szCmd : %s\n", __func__, seInfo.szCmd);
                    MAINLOG(0, "%s : %s ---> start\n", __func__, "resume_ip()");

                    MAINLOG(0, "%s : ======   IPS process.END   ======\n", __func__);
                }
                //=================   IPS process.END (Single camera)    ===================

                //=================   IPS process.START (Dual camera)    ===================
                if (!JsonQ_DeQ_Dual(&seInfo, 0))
                {

                    MAINLOG(0, "%s : ======   IPS process.START (JsonQ_DeQ_Dual 0)   ======\n", __func__);

                    MAINLOG(0, "%s : enum_IdReg emAlgoId : %d\n", __func__, seInfo.emAlgoId);
                    MAINLOG(0, "%s : szCmd : %s\n", __func__, seInfo.szCmd);
                    MAINLOG(0, "%s : %s ---> start\n", __func__, "resume_ip()");

                    resume_ip_Dual(0);

                    MAINLOG(0, "%s : ======   IPS process.END (JsonQ_DeQ_Dual 0)   ======\n", __func__);
                }

                if (!JsonQ_DeQ_Dual(&seInfo, 1))
                {

                    MAINLOG(0, "%s : ======   IPS process.START (JsonQ_DeQ_Dual 1)   ======\n", __func__);

                    MAINLOG(0, "%s : enum_IdReg emAlgoId : %d\n", __func__, seInfo.emAlgoId);
                    MAINLOG(0, "%s : szCmd : %s\n", __func__, seInfo.szCmd);
                    MAINLOG(0, "%s : %s ---> start\n", __func__, "resume_ip()");

                    resume_ip_Dual(1);

                    MAINLOG(0, "%s : ======   IPS process.END  (JsonQ_DeQ_Dual 1)   ======\n", __func__);
                }                

                //=================   IPS process.END (Dual camera)    ===================
            }
            //=================   end of IPS process    ===================
        }

        ///******** for auto mode process **********/
        Process_Flow_Handler();
        Process_Flow_Handler_Dual();
        ios_LED_Status_Handler();

        usleep(1000); /* delay 1 ms */
    }

    MAINLOG(0, " ## exit of mainCtl ##\n");
    return NULL;
}

/***********************************************************
 *	Function 	: ext_mqtt_sub_Dual
 *	Description : subscribes MQTT from web baclend (external)
 *	Param       : void *argu --> none
 *	Return      : NONE
 *************************************************************/
void *ext_mqtt_sub_Dual(void *argu)
{
    /* for backend */
    ext_mqtt_subscriber_Dual();

    MAINLOG(0, " ## exit of ext_mqtt_sub ##\n");
    return NULL;
}

/***********************************************************
 *	Function 	: start_watch_dog
 *	Description : start watch dog to monitor subsystem execution
 *                whether is timeout
 *	Param       : uint32_t time
 *	Return      : time (unit is ms)
 *************************************************************/
void start_watch_dog(uint32_t time)
{
    watchDogCounter = time;
    enable_warchDog = TRUE;
}

/***********************************************************
 *	Function 	: stop_watch_dog
 *	Description : stop watch dog to monitor subsystem
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void stop_watch_dog()
{
    watchDogCounter = 0;
    enable_warchDog = FALSE;
}

/***********************************************************
 *	Function 	: ips_process_Dual
 *	Description : Image process Dual camera subsystem 
 *                thread fucntion
 *	Param       : void *argu --> none
 *	Return      : NONE
 *************************************************************/
void *ips_process_Dual(void *argu)
{
    int res = 0;

    /* init value */
    /* once has been configured , will do not configure anymore,
           unless changed the project name (recipe)*/
    ips_status.ios_configured = FALSE;
    ips_status.ips_configured = FALSE;

    int iCamId = 0;
    if(argu != nullptr) {
        iCamId = *(int*) argu;
    }
    
    fprintf(stderr, "%s()%d: >> *(int*) argu = %d\n", __FUNCTION__, __LINE__, *(int*) argu);
    fprintf(stderr, "%s()%d: >> iCamId = %d\n", __FUNCTION__, __LINE__, iCamId);    

    void *phandler = nullptr;

    ParamCvtMethod ptrParamCvt;

    static bool bEnb_AutoRunningMode = 0;
    static bool bPrev_AutoRunningMode = 0;

    static bool bEnb_AutoRunningMode_Activate = 0;
    static bool bPrev_AutoRunningMode_Activate = 0;

    static bool bEnb_TriggerMode = 0;
    static bool bPrev_TriggerMode = 0;

    static bool bEnb_TriggerMode_Activate = 0;

    static int bStatus_AutoRunMode = -1;

    seAllParamTable_MeasGW_Annulus seAllParam{};

    seExpansionMode iEnbExMode{};

    std::vector<CAlgoMethodParametr> vecMthParmTasksRespo;

    seImageInfo imgBuf = seImageInfo();

    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration(0);
    std::string strtmp;

    while (!ip_doneFlag_Dual[iCamId])
    {

        pthread_mutex_lock(&ip_suspendMutex_Dual[iCamId]);
        while (ip_suspendFlag_Dual[iCamId] != 0)
            pthread_cond_wait(&ip_resumeCond_Dual[iCamId], &ip_suspendMutex_Dual[iCamId]);
        pthread_mutex_unlock(&ip_suspendMutex_Dual[iCamId]);

        // cycletime_start >>
        start = std::chrono::high_resolution_clock::now();

        MAINLOG(0, " ## >> ## >> pAlgoMthd ==. ==> Start \n");
        bStatus_AutoRunMode = -1;

        while (!TasksQ_IsEmpty_Dual(iCamId))
        {
            // deque pop_front
            CAlgoMethodParametr cAlgoMthParam;
            TasksQ_DeQ_Dual(&cAlgoMthParam, iCamId);

            ExModeQ_DeQ_Dual(&iEnbExMode, iCamId);

            /////////////////////////////////////////////////////
            //< Trigger Mode_Activate > AutoRun Mode.
            /////////////////////////////////////////////////////

            if (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[FLAGE_TRIGGERMODETYPE]))
            {

                MAINLOG(0, "@ === ===> AutoRun Mode === ===>\n");

                bEnb_TriggerMode_Activate = iEnbExMode.flg_TriggerMode_Activat;

                if (bEnb_TriggerMode_Activate)
                {
                    bEnb_AutoRunningMode_Activate = true;
                }
                else
                {
                    /* send msgQ to mainCtl for indicating the fail of auto mode */
                    innerQ_IPS_EnQ_Dual("Auto_Mode_Fail", iCamId);   // Dual camera
                    bEnb_AutoRunningMode_Activate = false;
                    bStatus_AutoRunMode = 0;
                    resume_mp(bStatus_AutoRunMode);
                    continue;
                }

                int szVector = vecMthParmTasksRespo.size();
                if (0 == szVector)
                {
                    /* send msgQ to mainCtl for indicating the fail of auto mode */
                    innerQ_IPS_EnQ_Dual("Auto_Mode_Fail", iCamId);   // Dual camera
                    bEnb_AutoRunningMode_Activate = false;
                    bStatus_AutoRunMode = 0;
                    resume_mp(bStatus_AutoRunMode);
                    continue;
                }

                ///////////////////////////////////////////////////////////////////////////////////////
                // Send the state of Auto-Run Mode was enabled to the Backend.

                int nID = cAlgoMthParam.mAlgoMethod.emAlgoId;
                cAlgoMthParam.mAlgoMethod.JsonGenerator(enum_Publish_CAMReg[nID], cAlgoMthParam.mAlgoMethod.pRes, cAlgoMthParam.mAlgoMethod.pJsonBuf, iCamId);

                ///////////////////////////////////////////////////////////////////////////////////////
                int iLoopCnt = 1;
                for (int i = 0; i < szVector; i++)
                {

                    CAlgoMethodParametr vecCAlgoMthdParm;

                    vecCAlgoMthdParm = vecMthParmTasksRespo[i];

                    if (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[FLAGE_AUTO_RUNNING]))
                    {

                        if (!bPrev_AutoRunningMode_Activate && bEnb_AutoRunningMode_Activate)
                        {

                            bPrev_AutoRunningMode_Activate = bEnb_AutoRunningMode_Activate;
                            seAllParam = seAllParamTable_MeasGW_Annulus();
                        }
                        else if (bPrev_AutoRunningMode_Activate && !bEnb_AutoRunningMode_Activate)
                        {

                            bPrev_AutoRunningMode_Activate = bEnb_AutoRunningMode_Activate;
                            seAllParam = seAllParamTable_MeasGW_Annulus();
                        }
                        else
                        {
                        }
                    }

                    // Selection the handle pointer
                    if ((!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Inquiry])) ||
                        (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Config])) ||
                        (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Capture])) ||
                        (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Release])) ||

                        (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Initialize])) ||
                        (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Start])) ||
                        (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Capture])) ||
                        (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Stop])) ||
                        (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Release])))
                    {

                        MAINLOG(0, "@@@@ === !===> phandler = pter_hdl_GigE_Dual[%d]\n", iCamId);
                        phandler = pter_hdl_GigE_Dual[iCamId];

                        if (imgBuf.pbImgBuf)
                        {

                            MAINLOG(0, "@===> Release imgBuf.pbImgBuf\n");
                            delete[] imgBuf.pbImgBuf;
                            imgBuf.pbImgBuf = nullptr;
                        }
                    }
                    else
                    {

                        MAINLOG(0, "### === !===> phandler = pter_hdl_IPL\n");
                        phandler = pter_hdl_IPL;
                    }

                    // Get the handle of Parameter convert method.
                    ptrParamCvt = vecCAlgoMthdParm.mAlgoMethod.ParamConverter;
                    nID = vecCAlgoMthdParm.mAlgoMethod.emAlgoId;

                    if (bEnb_AutoRunningMode_Activate)
                    {
                        // Convert the all parameter to function parameter.
                        //[[ AP to P ]]
                        if (ptrParamCvt != nullptr)
                        {
                            (*(*ptrParamCvt)[0])(&vecCAlgoMthdParm, &seAllParam);
                        }
                    }

                    // Algorithm Running !!!
                    res = vecCAlgoMthdParm.mAlgoMethod.AlgoMthd(phandler, &imgBuf, &vecCAlgoMthdParm, vecCAlgoMthdParm.mAlgoMethod.pRes, &imgBuf);
                    if (res)
                    {

                        vecCAlgoMthdParm.mAlgoMethod.JsonGenerator(enum_Publish_CAMReg[nID], vecCAlgoMthdParm.mAlgoMethod.pRes, vecCAlgoMthdParm.mAlgoMethod.pJsonBuf, iCamId);
                        bEnb_AutoRunningMode_Activate = 0;
                        bPrev_AutoRunningMode_Activate = 0;

                        if ((!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Inquiry])) ||
                            (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Config])) ||
                            (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Capture])) ||

                            (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Initialize])) ||
                            (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Start])) ||
                            (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Capture])) ||
                            (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Stop])) ||
                            (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Release])))
                        {

                            seGigECamCapture_Ret *pResult = (seGigECamCapture_Ret *)cAlgoMthParam.mAlgoMethod.pRes;

                            if (pResult->retState)
                            {
                                ipsComp_IPL_Release();
                                ipsComp_Camera_Release_Dual(iCamId);
                                usleep(2000); /* delay 2 ms */
                                ipsComp_IPL_Init();
                                ipsComp_Camera_Init_Dual(iCamId);

                                MAINLOG(0, "@@@@ === !===> phandler = pter_hdl_GigE_Dual[%d]\n", iCamId);
                            }
                        }
                        printf("Error ==> %s  <===\n", enum_Publish_CAMReg[nID]);
                        /* send msgQ to mainCtl for indicating the fail of auto mode */
                        innerQ_IPS_EnQ_Dual("Auto_Mode_Fail", iCamId);   // Dual camera
                        bStatus_AutoRunMode = -1;
                        resume_mp(bStatus_AutoRunMode);
                        break;
                    }

                    if (bEnb_AutoRunningMode_Activate)
                    {
                        // Convert the Result Parameter to All parameter.
                        //[[ RetP to AP ]]
                        if (ptrParamCvt != nullptr)
                        {
                            (*(*ptrParamCvt)[1])(vecCAlgoMthdParm.mAlgoMethod.pRes, &seAllParam);
                        }
                    }

                    if (bEnb_AutoRunningMode_Activate)
                    {

                        // When the camera capture image was finished, send the result immediately to Back-end.
                        if (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Capture]))
                        {
                            vecCAlgoMthdParm.mAlgoMethod.JsonGenerator(enum_Publish_CAMReg[nID], vecCAlgoMthdParm.mAlgoMethod.pRes, vecCAlgoMthdParm.mAlgoMethod.pJsonBuf, iCamId);
                        }

                        // # MeasGlueWidth_Annulus
                        if (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[ALGO_MeasGW_Annulus]))
                        {

                            // seAlgoMthd.pJsonBuf is "TriggerMode_Activate" and the Json context has important of "msgId" info.
                            // The value of roi_id in pJsonBuf only for continuity testing mode.
                            std::string strPSetInfo(cAlgoMthParam.mAlgoMethod.pJsonBuf);

                            printf("ALGO_MeasGW_Annulus:\n %s\n\n", strPSetInfo.c_str());

                            std::size_t found = strPSetInfo.rfind("}");
                            std::string strAddROI_Id = strPSetInfo.substr(0, found) + ",\"roi_id\":" + std::to_string(iLoopCnt) + "}";

                            vecCAlgoMthdParm.mAlgoMethod.JsonGenerator(enum_Publish_CAMReg[nID], vecCAlgoMthdParm.mAlgoMethod.pRes, strAddROI_Id.c_str(), iCamId);

                            iLoopCnt++;
                        }

                        // # MeasGlueWidth_Rectangle
                        if (!strcmp(vecCAlgoMthdParm.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[ALGO_MeasGW_Rect]))
                        {

                            // seAlgoMthd.pJsonBuf is "TriggerMode_Activate" and the Json context has important of "msgId" info.
                            // The value of roi_id in pJsonBuf only for continuity testing mode.
                            std::string strPSetInfo(cAlgoMthParam.mAlgoMethod.pJsonBuf);

                            printf("ALGO_MeasGW_Rectangle:\n %s\n\n", strPSetInfo.c_str());

                            std::size_t found = strPSetInfo.rfind("}");
                            std::string strAddROI_Id = strPSetInfo.substr(0, found) + ",\"roi_id\":" + std::to_string(iLoopCnt) + "}";

                            vecCAlgoMthdParm.mAlgoMethod.JsonGenerator(enum_Publish_CAMReg[nID], vecCAlgoMthdParm.mAlgoMethod.pRes, strAddROI_Id.c_str(), iCamId);

                            iLoopCnt++;
                        }
                    }
                }
                if (0 == res)
                {
                    /* send msgQ to mainCtl for indicating the done of auto mode */
                    innerQ_IPS_EnQ_Dual("Auto_Mode_Done", iCamId);   // Dual camera
                    bStatus_AutoRunMode = 1;
                    resume_mp(bStatus_AutoRunMode);
                }

                MAINLOG(0, "@ <=== === AutoRun Mode <=== ===\n");

                usleep(1000); /* delay 1 ms */

                continue;
            }

            /////////////////////////////////////////////////////////////////
            //<Without Trigger Mode_Activate> Normal Mode and AutoRunn Mode.
            /////////////////////////////////////////////////////////////////
            if (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[FLAGE_AUTO_RUNNING]))
            {

                bEnb_AutoRunningMode = iEnbExMode.flg_AutoRunning;
                bEnb_TriggerMode = iEnbExMode.flg_Enb_TriggerMode;

                if (!bPrev_AutoRunningMode && bEnb_AutoRunningMode)
                {

                    bPrev_AutoRunningMode = bEnb_AutoRunningMode;
                    seAllParam = seAllParamTable_MeasGW_Annulus();
                }
                else if (bPrev_AutoRunningMode && !bEnb_AutoRunningMode)
                {

                    bPrev_AutoRunningMode = bEnb_AutoRunningMode;
                    seAllParam = seAllParamTable_MeasGW_Annulus();
                }
                else
                {
                }
            }

            // Record All algoritm Mothod and Parameter.
            if (!bPrev_TriggerMode && bEnb_TriggerMode)
            { // 01

                MAINLOG(0, " bEnb_TriggerMode = %d\n", bEnb_TriggerMode);
                MAINLOG(0, " =>>> Clear vector of vecMthParmTasksRespo() <<<= \n");
                if (!vecMthParmTasksRespo.empty())
                {
                    vecMthParmTasksRespo.clear();
                }

                bPrev_TriggerMode = bEnb_TriggerMode;
                vecMthParmTasksRespo.push_back(cAlgoMthParam);

                MAINLOG(0, " bEnb_TriggerMode = %d\n", bEnb_TriggerMode);
                MAINLOG(0, " Continue... ...\n");
                continue;
            }
            else if (bPrev_TriggerMode && bEnb_TriggerMode)
            { // 11

                vecMthParmTasksRespo.push_back(cAlgoMthParam);

                MAINLOG(0, " bEnb_TriggerMode = %d\n", bEnb_TriggerMode);
                MAINLOG(0, " Continue... ...\n");
                continue;
            }
            else if (bPrev_TriggerMode && !bEnb_TriggerMode)
            { // 10

                MAINLOG(0, " bEnb_TriggerMode = %d\n", bEnb_TriggerMode);
                MAINLOG(0, " The End!!! \n");

                bPrev_TriggerMode = bEnb_TriggerMode;
                vecMthParmTasksRespo.push_back(cAlgoMthParam);
            }
            else if (!bPrev_TriggerMode && !bEnb_TriggerMode)
            { // 00

                MAINLOG(0, " bEnb_TriggerMode = %d\n", bEnb_TriggerMode);
                MAINLOG(0, " Clear vector of vecMthParmTasksRespo() ... ...\n");

                if (!vecMthParmTasksRespo.empty())
                {
                    vecMthParmTasksRespo.clear();
                }
            }

            MAINLOG(0, " cAlgoMthParam.mAlgoMethod.strCmd = %s\n", cAlgoMthParam.mAlgoMethod.strCmd);
            MAINLOG(0, " iEnbExMode.flg_AutoRunning = %d\n", iEnbExMode.flg_AutoRunning);
            MAINLOG(0, " bEnb_AutoRunningMode = %d\n", bEnb_AutoRunningMode);
            MAINLOG(0, " bPrev_AutoRunningMode = %d\n", bPrev_AutoRunningMode);
            MAINLOG(0, " >>>!!!>>> vecMthParmTasksRespo.szie() = %d <<<!!!<<< \n", (int)vecMthParmTasksRespo.size());

            // Selection the handle pointer
            if ((!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Inquiry])) ||
                (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Config])) ||
                (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Capture])) ||
                (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Release])) ||

                (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Initialize])) ||
                (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Start])) ||
                (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Capture])) ||
                (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Stop])) ||
                (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Release]))

            )
            {

                MAINLOG(0, "@@@@ === !===> phandler = pter_hdl_GigE_Dual[%d]\n", iCamId);

                phandler = pter_hdl_GigE_Dual[iCamId];
            }
            else
            {

                MAINLOG(0, "### === !===> phandler = pter_hdl_IPL\n");

                phandler = pter_hdl_IPL;
            }

            // Get the handle of Parameter convert method.
            ptrParamCvt = cAlgoMthParam.mAlgoMethod.ParamConverter;
            int nID = cAlgoMthParam.mAlgoMethod.emAlgoId;

            if (bEnb_AutoRunningMode)
            {
                // Convert the all parameter to function parameter.
                //[[ AP to P ]]
                if (ptrParamCvt != nullptr)
                {
                    (*(*ptrParamCvt)[0])(&cAlgoMthParam, &seAllParam);
                }
            }

            // Algorithm Running !!!
            res = cAlgoMthParam.mAlgoMethod.AlgoMthd(phandler, nullptr, &cAlgoMthParam, cAlgoMthParam.mAlgoMethod.pRes, nullptr);

            if (res)
            {

                cAlgoMthParam.mAlgoMethod.JsonGenerator(enum_Publish_CAMReg[nID], cAlgoMthParam.mAlgoMethod.pRes, cAlgoMthParam.mAlgoMethod.pJsonBuf, iCamId);
                bEnb_AutoRunningMode = 0;
                bPrev_AutoRunningMode = 0;

                if ((!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Inquiry])) ||
                    (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Config])) ||
                    (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Capture])) ||

                    (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Initialize])) ||
                    (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Start])) ||
                    (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Capture])) ||
                    (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Stop])) ||
                    (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[METHOD_GigeCam_Streaming_Release])))
                {

                    seGigECamCapture_Ret *pResult = (seGigECamCapture_Ret *)cAlgoMthParam.mAlgoMethod.pRes;

                    if (pResult->retState)
                    {

                        ipsComp_IPL_Release();
                        ipsComp_Camera_Release_Dual(iCamId);
                        usleep(2000); /* delay 2 ms */
                        ipsComp_IPL_Init();
                        ipsComp_Camera_Init_Dual(iCamId);

                        MAINLOG(0, "@@@@ === !===> phandler = pter_hdl_GigE_Dual[%d]\n", iCamId);
                    }
                }

                break;
            }

            if (bEnb_AutoRunningMode)
            {
                // Convert the Result Parameter to All parameter.
                //[[ RetP to AP ]]
                if (ptrParamCvt != nullptr)
                {
                    (*(*ptrParamCvt)[1])(cAlgoMthParam.mAlgoMethod.pRes, &seAllParam);
                }
            }

            if (bEnb_AutoRunningMode)
            {

                // # MeasGlueWidth_Annulus
                if (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[ALGO_MeasGW_Annulus]))
                {

                    cAlgoMthParam.mAlgoMethod.JsonGenerator(enum_Publish_CAMReg[nID], cAlgoMthParam.mAlgoMethod.pRes, cAlgoMthParam.mAlgoMethod.pJsonBuf, iCamId);
                }

                // # MeasGlueWidth_Rectangle
                if (!strcmp(cAlgoMthParam.mAlgoMethod.strCmd, enum_Subscribe_CAMReg[ALGO_MeasGW_Rect]))
                {

                    cAlgoMthParam.mAlgoMethod.JsonGenerator(enum_Publish_CAMReg[nID], cAlgoMthParam.mAlgoMethod.pRes, cAlgoMthParam.mAlgoMethod.pJsonBuf, iCamId);
                }
            }
            else
            {

                cAlgoMthParam.mAlgoMethod.JsonGenerator(enum_Publish_CAMReg[nID], cAlgoMthParam.mAlgoMethod.pRes, cAlgoMthParam.mAlgoMethod.pJsonBuf, iCamId);
            }
        }

        MAINLOG(0, " ## << ## << pAlgoMthd <== . <== End \n");

        // # cycletime_end <<
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        strtmp = std::to_string(duration.count());
        MAINLOG(0, " # Total_Methed(...)_CycleTime : %s (ms)\n", strtmp.c_str());

        if (TasksQ_IsEmpty_Dual(iCamId))
        {
            if (ip_doneFlag_Dual[iCamId])
            {
                MAINLOG(0, " ## break of ips_process ##\n");
                break;
            }

            suspend_ip_Dual(iCamId);
        }

        MAINLOG(0, "*****   IP processing App.    *****\n");

        usleep(1000); /* delay 10 ms */
    }

    MAINLOG(0, " ## exit of ips_process ##\n");
    return NULL;
}

/***********************************************************
 *	Function 	: ios_process
 *	Description : IO subsystem thread fucntion
 *	Param       : void *argu --> none
 *	Return      : NONE
 *************************************************************/
void* ios_process(void *argu)
{
    uint8_t rec_data[MAX_MSG_SIZE];
    char job_t[4096] = {'\0'}; /* need to define this buffer size */
    
    while (!io_doneFlag)
    {
        pthread_mutex_lock(&io_suspendMutex);
        while (io_suspendFlag != 0)
            pthread_cond_wait(&io_resumeCond, &io_suspendMutex);
        pthread_mutex_unlock(&io_suspendMutex);

        // for(int i = 0; i < cnt_Tasks; i++)
        while (!IO_JsonQ_IsEmpty())
        {
            seIO_JsonInfo seInfo;
            IO_JsonQ_DeQ(&seInfo);
            strcpy((char *)rec_data, seInfo.szCmd);
            MAINLOG(0, YELLOW " @@@ === >>> IO_JsonQ_DeQ() = [%d][%s]\n", seInfo.emAlgoId, seInfo.szCmd);
            
            if (seInfo.emAlgoId == TRIGGER_SET_PROCESS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[TRIGGER_SET_PROCESS]);
                jes.new_job = TriggerSetProcess;
                ios_triggerSetProcess(&ios_trigger);
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_respStr[TRIGGER_SET_PROCESS]);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", setfuncStr[jes.new_job]);
            } else if (seInfo.emAlgoId == TRIGGER_GET_PROCESS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[TRIGGER_GET_PROCESS]);
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), setfuncStr[jes.new_job]);
                MAINLOG(0, "[MAINCTL] : current_job=%s\n", setfuncStr[jes.current_job]);
                jes.new_job = TriggerGetProcess;
                ios_triggerGetProcess(&ios_trigger);
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_respStr[TRIGGER_GET_PROCESS]);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", setfuncStr[jes.new_job]);
            } else if (seInfo.emAlgoId == DIN_SET_PROCESS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[DIN_SET_PROCESS]);
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), setfuncStr[jes.new_job]);
                jes.new_job = DinSetProcess;
                //ios_response_json_create(job_t, (char *)rec_data, (char *)ios_respStr[DIN_SET_PROCESS]);
                ios_din_get_process_json_create((char *)job_t);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == DIN_GET_PROCESS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[DIN_GET_PROCESS]);
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), setfuncStr[jes.new_job]);
                jes.new_job = DinGetProcess;
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_respStr[DIN_GET_PROCESS]);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == IOS_GET_STATUS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[IOS_GET_STATUS]);
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), setfuncStr[jes.new_job]);
                jes.new_job = IOSGetStatus;
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_respStr[IOS_GET_STATUS]);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == LIGHT_SET_PWM) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[LIGHT_SET_PWM]);
                Process_Node.Brightness[0] = Process_Node.Brightness[1] = ios_setpwm.value;
                ios_Control_Light_Handler(ios_setpwm.inLight, TRUE);
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_respStr[LIGHT_SET_PWM]);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == LIGHT_GET_PWM || seInfo.emAlgoId == LIGHT_GET_BRIGHTNESS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[LIGHT_GET_PWM]);
                jes.new_job = LightGetPWM;
                uint16_t brightness[2];

                IOS_LIGHT_SET_PWM  ios_setpwm_tmp;
                ios_setpwm_tmp.inLight = LIGHT_SOURCE_1;
                ios_lightGetPWM(&ios_setpwm_tmp);
                brightness[0] = ios_setpwm_tmp.value;
                ios_setpwm_tmp.inLight = LIGHT_SOURCE_2;
                ios_lightGetPWM(&ios_setpwm_tmp);
                brightness[1] = ios_setpwm_tmp.value;
                
                Process_Node.Brightness[0] = brightness[0];
                Process_Node.Brightness[1] = brightness[1];

                ios_light_get_brightness_json_create(job_t, brightness);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == LED_SET_PROCESS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), setfuncStr[jes.new_job]);
                jes.new_job = LedSetProcess;
                ios_ledSetProcess(&ios_setled);
                ios_led_get_process_json_create(job_t);
                // # ext_mqtt_publisher(job_t);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", setfuncStr[jes.new_job]);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == LED_GET_PROCESS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), setfuncStr[jes.new_job]);
                jes.new_job = LedGetProcess;
                //ios_led_get_process_json_create(job_t);
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_respStr[LED_GET_PROCESS]);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", setfuncStr[jes.new_job]);
            } else if (seInfo.emAlgoId == LED_SET_MODE) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[LED_SET_MODE]);
                // MAINLOG(0, "%d ios_setLedMode.Indication = %d\n", __LINE__, strlen(ios_setLedMode.Indication));
                MAINLOG(0, "%d ios_setLedMode.Indication = %s\n", __LINE__, ios_setLedMode.Indication);
                if (strlen((char *)ios_setLedMode.Indication) != 0)
                {
                    // printf("ios_setLedMode.Indication\n");
                    if (strcmp((char *)ios_setLedMode.Indication, (char *)"ON") == 0)
                    {
                        if (strcmp((char *)ios_setLedMode.Color, (char *)"Red") == 0)
                        {MAINLOG(0, "%d \n", __LINE__);
                            sprintf((char *)ios_setled.outStatus, "%s", "Red ON");
                        }
                        else if (strcmp((char *)ios_setLedMode.Color, (char *)"Green") == 0)
                        {
                            sprintf((char *)ios_setled.outStatus, "%s", (char *)"Green ON");
                        }
                        else if (strcmp((char *)ios_setLedMode.Color, (char *)"Orange") == 0)
                        {
                            sprintf((char *)ios_setled.outStatus, "%s", (char *)"Orange ON");
                        }
                        ios_setled.outBlinkDelay = 0;
                        ios_setled.outOffDelay = 0;
                    }
                    else if (strcmp((char *)ios_setLedMode.Indication, (char *)"OFF") == 0)
                    {
                        sprintf((char *)ios_setled.outStatus, "%s", "OFF");
                        ios_setled.outBlinkDelay = 0;
                        ios_setled.outOffDelay = 0;
                    }
                    else if (strcmp((char *)ios_setLedMode.Indication, (char *)"Blink") == 0)
                    {
                        if (strcmp((char *)ios_setLedMode.Color, "Red") == 0)
                        {
                            sprintf((char *)ios_setled.outStatus, "%s", "Red Blinking");
                        }
                        else if (strcmp((char *)ios_setLedMode.Color, (char *)"Green") == 0)
                        {
                            sprintf((char *)ios_setled.outStatus, "%s", "Green Blinking");
                        }
                        else if (strcmp((char *)ios_setLedMode.Color, "Orange") == 0)
                        {
                            sprintf((char *)ios_setled.outStatus, "%s", "Orange Blinking");
                        }
                        ios_setled.outBlinkDelay = 10000;
                        ios_setled.outOffDelay = 0;
                        /* so far, must fill out outMode to indicated LED */
                        sprintf((char *)ios_setled.outMode, "%s", "AI status");
                    }
                }
                else
                {
                    // printf("ios_setLedMode.ledMode=%s\n", ios_setLedMode.ledMode);
                    if (strcmp((char *)ios_setLedMode.ledMode, (char *)"AI status") == 0)
                    {
                        sprintf((char *)ios_setled.outMode, "%s", "AI status");
                        sprintf((char *)ios_setled.outStatus, "%s", "Red ON");
                    }
                    else if (strcmp((char *)ios_setLedMode.ledMode, (char *)"Light status") == 0)
                    {MAINLOG(0, "%d \n", __LINE__);
                        /* turn off LED */
                        usleep(50000); /* delay 50  ms */
                        sprintf((char *)ios_setled.outStatus, "%s", "OFF");
                        ios_setled.outPin = UserDef2LED;
                        ios_ledSetProcess(&ios_setled);
                        usleep(50000); /* delay 50  ms */
                        ios_setled.outPin = UserDef1LED;
                        ios_ledSetProcess(&ios_setled);
                        /* fills other LED later */
                    }
                    ios_setled.outBlinkDelay = 0;
                    ios_setled.outOffDelay = 0;
                }
                if (ios_setLedMode.led == 1)
                {
                    ios_setled.outPin = UserDef1LED;
                    strcpy((char *)Process_Node.UsrDef1Mode, (char *)ios_setLedMode.ledMode);
                }
                else if (ios_setLedMode.led == 2)
                {
                    ios_setled.outPin = UserDef2LED;
                    strcpy((char *)Process_Node.UsrDef2Mode, (char *)ios_setLedMode.ledMode);
                }
                else
                {
                    MAINLOG(0, "[MAINCTL] : user define LED number is wrong !!");
                }
                        
                        
    if(!strcasecmp((char *)&ios_setled.outStatus[0], "OFF")) {IOSLOG(0, "[IOS](%s)%d: \n", __func__, __LINE__);
        ios_setStatusLed(ios_setLedMode.led, 0);
    } else if(!strcasecmp((char *)&ios_setled.outStatus[0], "Red ON")) {IOSLOG(0, "[IOS](%s)%d: \n", __func__, __LINE__);
        ios_setStatusLed(ios_setLedMode.led, 1);
    } else if(!strcasecmp((char *)&ios_setled.outStatus[0], "Green ON")) {IOSLOG(0, "[IOS](%s)%d: \n", __func__, __LINE__);
        ios_setStatusLed(ios_setLedMode.led, 2);
    } else if(!strcasecmp((char *)&ios_setled.outStatus[0], "Orange ON")) {IOSLOG(0, "[IOS](%s)%d: \n", __func__, __LINE__);
        ios_setStatusLed(ios_setLedMode.led, 3);
    } else {
        MAINLOG(0, "[MAINCTL] : ios_setled outStatus [%s] is wrong !!", ios_setled.outStatus);
    }
      
                
                usleep(50000); /* delay 50  ms */
                ios_LED_Status_Handler();
                //ios_ledSetProcess(&ios_setled);
                ios_response_json_create(job_t, ios_respStr[LED_SET_MODE], (char *)ios_CmdInfo);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", setfuncStr[jes.new_job]);
            } else if (seInfo.emAlgoId == DIN_SET_MODE) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[DIN_SET_MODE]);
                ios_di.inPin = ios_setDinMode.DinPin;
                strcpy((char *)ios_di.inMode, (char *)ios_setDinMode.DinPolarity);
                ios_di.inControlMode = 0;
                ios_di.outPin = 0;                         // inform MCU don't handle Dout pin when Din is triggered
                                                           // strcpy(ios_di.outMode, ios_setDinMode.DinPolarity); // this line is free when outPin = 0
                sprintf((char *)ios_di.outMode, "%s", ""); // this line is free when outPin = 0
                ios_di.outControlMode = 0;
                ios_di.outDelay = 0;
                ios_di.onoffSetting = 0;
                ios_dinSetProcess(&ios_di);
                if (strcmp((char *)ios_setDinMode.SelectMode, (char *)"Glue inspection") == 0)
                {
                    /* avoiding SelectMode's string be modified by backend, we always keep the internal defined string */
                    if(ios_cameraid == 0) {
                        Process_Node.autoMode = GLUE_INSPECTION;
                    } else if(ios_cameraid == 0) {
                        Process_Node_Dual.autoMode = GLUE_INSPECTION;
                    }
                    Process_Node_Dual.autoMode = Process_Node.autoMode = GLUE_INSPECTION;
                }
                ios_response_json_create(job_t, ios_respStr[DIN_SET_MODE], (char *)ios_CmdInfo);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[DIN_SET_MODE]);
            } else if (seInfo.emAlgoId == DOUT_SET_MODE) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[DOUT_SET_MODE]);
                if(ios_doutMode.CameraId == 0) {
                    if (ios_doutMode.outPin == 1)
                    {
                        strcpy((char *)Process_Node.Dout1Mode, (char *)ios_doutMode.polarity);
                        strcpy((char *)Process_Node.Dout1selectMode, (char *)ios_doutMode.selectMode);
                    }
                    else if (ios_doutMode.outPin == 2)
                    {
                        strcpy((char *)Process_Node.Dout2Mode, (char *)ios_doutMode.polarity);
                        strcpy((char *)Process_Node.Dout2selectMode, (char *)ios_doutMode.selectMode);
                    }
                    else if (ios_doutMode.outPin == 3)
                    {
                        strcpy((char *)Process_Node.Dout3Mode, (char *)ios_doutMode.polarity);
                        strcpy((char *)Process_Node.Dout3selectMode, (char *)ios_doutMode.selectMode);
                    }
                    else if (ios_doutMode.outPin == 4)
                    {
                        strcpy((char *)Process_Node.Dout4Mode, (char *)ios_doutMode.polarity);
                        strcpy((char *)Process_Node.Dout4selectMode, (char *)ios_doutMode.selectMode);
                    }
                } else {
                    if (ios_doutMode.outPin == 1)
                    {
                        strcpy((char *)Process_Node_Dual.Dout1Mode, (char *)ios_doutMode.polarity);
                        strcpy((char *)Process_Node_Dual.Dout1selectMode, (char *)ios_doutMode.selectMode);
                    }
                    else if (ios_doutMode.outPin == 2)
                    {
                        strcpy((char *)Process_Node_Dual.Dout2Mode, (char *)ios_doutMode.polarity);
                        strcpy((char *)Process_Node_Dual.Dout2selectMode, (char *)ios_doutMode.selectMode);
                    }
                    else if (ios_doutMode.outPin == 3)
                    {
                        strcpy((char *)Process_Node_Dual.Dout3Mode, (char *)ios_doutMode.polarity);
                        strcpy((char *)Process_Node_Dual.Dout3selectMode, (char *)ios_doutMode.selectMode);
                    }
                    else if (ios_doutMode.outPin == 4)
                    {
                        strcpy((char *)Process_Node_Dual.Dout4Mode, (char *)ios_doutMode.polarity);
                        strcpy((char *)Process_Node_Dual.Dout4selectMode, (char *)ios_doutMode.selectMode);
                    }
                }
                ios_Control_Dout_Handler_Dual((char* )ios_doutMode.selectMode, TRUE, ios_doutMode.CameraId);
                usleep(50000);
                ios_Control_Dout_Handler_Dual((char* )ios_doutMode.selectMode, FALSE, ios_doutMode.CameraId);
                ios_response_json_create(job_t, ios_respStr[DOUT_SET_MODE], (char *)ios_CmdInfo);
                ext_mqtt_publisher_Dual(job_t, ios_doutMode.CameraId);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[DOUT_SET_MODE]);
            } else if (seInfo.emAlgoId == DIO_GET_STATUS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[DIO_GET_STATUS]);
                ios_getStatus(&ios_getstatus);
                // printf("ios_getstatus.dout1=%d\n", ios_getstatus.dout1);
                // printf("ios_getstatus.dout2=%d\n", ios_getstatus.dout2);
                // printf("ios_getstatus.dout3=%d\n", ios_getstatus.dout3);
                // printf("ios_getstatus.dout4=%d\n", ios_getstatus.dout4);
                // printf("ios_getstatus.din1=%d\n", ios_getstatus.din1);
                // printf("ios_getstatus.din2=%d\n", ios_getstatus.din2);
                // printf("ios_getstatus.din3=%d\n", ios_getstatus.din3);
                // printf("ios_getstatus.din4=%d\n", ios_getstatus.din4);
                ios_get_status_json_create(job_t);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[DIO_GET_STATUS]);
            } else if (seInfo.emAlgoId == DOUT_MANUAL_CONTROL) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[DOUT_MANUAL_CONTROL]);
                ios_doutSetProcess(&ios_dout);
                ios_response_json_create(job_t, ios_respStr[DOUT_MANUAL_CONTROL], (char *)ios_CmdInfo);
                // ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[DOUT_MANUAL_CONTROL]);
            } else if (seInfo.emAlgoId == LIGHT_SET_BRIGHTNESS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[LIGHT_SET_BRIGHTNESS]);
                jes.new_job = LightSetPWM;

                if (ios_setpwm.inLight == LIGHT_SOURCE_1)
                {
                    Process_Node.Brightness[0] = ios_setpwm.value;
                }
                else if (ios_setpwm.inLight == LIGHT_SOURCE_2)
                {
                    Process_Node.Brightness[1] = ios_setpwm.value;
                }

                if (strcmp((char *)ios_setpwm.LightSwitch, (char *)"ON") == 0)
                {
                    ios_Control_Light_Handler(ios_setpwm.inLight, TRUE);
                }
                else
                {
                    ios_Control_Light_Handler(ios_setpwm.inLight, FALSE);
                }
                ios_response_json_create(job_t, ios_respStr[LIGHT_SET_BRIGHTNESS], (char *)ios_CmdInfo);
                // ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[LIGHT_SET_BRIGHTNESS]);
            } else if (seInfo.emAlgoId == LIGHT_GET_BRIGHTNESS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[LIGHT_GET_BRIGHTNESS]);
                jes.new_job = LightGetPWM;
                uint16_t brightness[2];
                brightness[0] = Process_Node.Brightness[0];
                brightness[1] = Process_Node.Brightness[1];

                ios_light_get_brightness_json_create(job_t, brightness);
                // # ext_mqtt_publisher(job_t);
            } else if (seInfo.emAlgoId == MODBUS_SET_PARAMS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[MODBUS_SET_PARAMS]);
                ios_modbusSetParameters(&ios_modbusSetParams);
                ios_response_json_create(job_t, ios_respStr[MODBUS_SET_PARAMS], (char *)ios_CmdInfo);
                // # ext_mqtt_publisher(job_t);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[MODBUS_SET_PARAMS]);
            } else if (seInfo.emAlgoId == MODBUS_GET_PARAMS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[MODBUS_GET_PARAMS]);
                ios_modbusGetParameters(&ios_modbusSetParams);
                ios_modbus_get_params_json_create(job_t);
                // # ext_mqtt_publisher(job_t);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[MODBUS_GET_PARAMS]);
            } else if (seInfo.emAlgoId == TRIGGER_SET_MODE) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[TRIGGER_SET_MODE]);
                if (ios_trigger.outPin == LIGHT_SOURCE_1)
                {
                    ios_trigger.outPin = LIGHT_PIN_1;
                }
                else
                {
                    ios_trigger.outPin = LIGHT_PIN_2;
                }
                ios_triggerSetProcess(&ios_trigger);
                ios_response_json_create(job_t, ios_respStr[TRIGGER_SET_MODE], (char *)ios_CmdInfo);
                // # ext_mqtt_publisher(job_t);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[TRIGGER_SET_MODE]);
            } else if (seInfo.emAlgoId == REPORT_TEST_RESULT) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[REPORT_TEST_RESULT]);
                ios_response_json_create(job_t, ios_respStr[REPORT_TEST_RESULT], (char *)ios_CmdInfo);
                UpdateLEDStatus_Flg = 1;
                // # ext_mqtt_publisher(job_t);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[REPORT_TEST_RESULT]);
            } else if (seInfo.emAlgoId == AUTO_TEST_SET_MODE) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[AUTO_TEST_SET_MODE]);
                ios_response_json_create(job_t, ios_respStr[AUTO_TEST_SET_MODE], (char *)ios_CmdInfo);
                if(ios_cameraid == 0) {
                    Process_Node.giFlow = GI_START;
                } else if(ios_cameraid == 0) {
                    Process_Node_Dual.giFlow = GI_START;
                }
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[AUTO_TEST_SET_MODE]);
            } else if (seInfo.emAlgoId == IO_RTC_SET_MODE) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[IO_RTC_SET_MODE]);
                IO_SetLocalTime(ios_rtc.use_ntp, &ios_rtc.local_time, &ios_rtc.timezone[0]);
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_CmdInfo);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == IO_SYS_GET_PARAMS) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[IO_SYS_GET_PARAMS]);
                ios_sys_get_params_json_create(job_t);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == IO_SHOP_FLOOR_CONTROL) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[IO_SHOP_FLOOR_CONTROL]);
                ios_sfc_send_msg(&ios_sfc);
                //ios_response_json_create(job_t, ios_respStr[IO_SHOP_FLOOR_CONTROL], (char *)ios_CmdInfo);
                ios_sfc_params_json_create(job_t);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == IO_MAINLED_SET_PARAM) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[IO_MAINLED_SET_PARAM]);
                ios_setMainLightLevel(ios_mainled.intBrightness);
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_CmdInfo);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == IO_AILIGHTING_SET_PARAM) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[IO_AILIGHTING_SET_PARAM]);
                ios_setAiLightLevel_withChannel(ios_ailighting.intBrightness, ios_ailighting.intBrightness);
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_CmdInfo);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == IO_EXTLIGHTING_SET_PARAM) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[IO_EXTLIGHTING_SET_PARAM]);
                    
                __LIGHT_MANUFACTURER__  manufacturer;
                if(!strncasecmp(ios_ailighting.strlightSource.c_str(), "opt", strlen("opt"))) {
                    manufacturer = OPT;
                }

                ios_setExtLightLevel_withChannel(manufacturer, ios_ailighting.intBrightness, ios_ailighting.channel);
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_CmdInfo);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == IO_EXTLIGHTING_GET_PARAM) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[IO_EXTLIGHTING_GET_PARAM]);
                    
                __LIGHT_MANUFACTURER__  manufacturer;
                if(!strncasecmp(ios_ailighting.strlightSource.c_str(), "opt", strlen("opt"))) {
                    manufacturer = OPT;
                }

                ios_readExtLightLevel_withChannel(manufacturer, ios_ailighting.channel);
                ios_response_json_create(job_t, (char *)rec_data, (char *)ios_CmdInfo);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            } else if (seInfo.emAlgoId == IO_TOF_GET_PARAM) {
                MAINLOG(0, "%d [MAINCTL] : new_job=%s\n", main_gettime_ms(), ios_cmdStr[IO_TOF_GET_PARAM]);
                ios_tof.distance = tofReadDistance();
                MAINLOG(0, "[MAIN] : Distance=[%d]\n", ios_tof.distance);
                ios_get_tof_json_create(job_t);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
                MAINLOG(0, YELLOW "[MAIN] : IOS processing %s done..\n", ios_cmdStr[IO_TOF_GET_PARAM]);
            } else {
                MAINLOG(0, RED "[MAIN] : Unknow cmd emAlgoId=[%d] szCmd=[%s] fail.\n", seInfo.emAlgoId, seInfo.szCmd);
                ios_sys_get_params_json_create(job_t);
                ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            }
            MAINLOG(0, " ## break of ios_process ##\n");
        }

        if (IO_JsonQ_IsEmpty())
        {
            if (io_doneFlag)
            {
                MAINLOG(0, " ## break of ips_process ##\n");
                break;
            }

            suspend_io();
        }

        MAINLOG(0, "*****   IO processing App.    *****\n");
        
        usleep(1000); /* delay 1 ms */
    }

    MAINLOG(0, " ## exit of ios_process ##\n");
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////
/***********************************************************
 *	Function 	: suspend_ip_Dual
 *	Description : Suspend image dual camera process
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void suspend_ip_Dual(const int iID)
{ // tell the thread to suspend
    MAINLOG(0, "%s\n", __func__);
    pthread_mutex_lock(&ip_suspendMutex_Dual[iID]);
    ip_suspendFlag_Dual[iID] = 1;
    pthread_mutex_unlock(&ip_suspendMutex_Dual[iID]);
}
/***********************************************************
 *	Function 	: resume_ip_Dual
 *	Description : Resume image dual camera process
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void resume_ip_Dual(const int iID)
{ // tell the thread to resume
    MAINLOG(0, "%s\n", __func__);
    pthread_mutex_lock(&ip_suspendMutex_Dual[iID]);
    ip_suspendFlag_Dual[iID] = 0;
    pthread_cond_broadcast(&ip_resumeCond_Dual[iID]);
    pthread_mutex_unlock(&ip_suspendMutex_Dual[iID]);
}
/***********************************************************
 *	Function 	: close_ip_Dual
 *	Description : Close image dual camera process
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void close_ip_Dual(const int iID)
{
    MAINLOG(0, "%s\n", __func__);
    pthread_mutex_lock(&ip_suspendMutex_Dual[iID]);
    ip_suspendFlag_Dual[iID] = 0;
    ip_doneFlag_Dual[iID] = 1;
    pthread_cond_broadcast(&ip_resumeCond_Dual[iID]);
    pthread_mutex_unlock(&ip_suspendMutex_Dual[iID]);
}

/***********************************************************
 *	Function 	: suspend_io
 *	Description : Suspend io process
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void suspend_io()
{ // tell the thread to suspend
    MAINLOG(0, "%s\n", __func__);
    pthread_mutex_lock(&io_suspendMutex);
    io_suspendFlag = 1;
    pthread_mutex_unlock(&io_suspendMutex);
}
/***********************************************************
 *	Function 	: resume_io
 *	Description : Resume io process
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void resume_io()
{ // tell the thread to resume
    MAINLOG(0, "%s\n", __func__);
    pthread_mutex_lock(&io_suspendMutex);
    io_suspendFlag = 0;
    pthread_cond_broadcast(&io_resumeCond);
    pthread_mutex_unlock(&io_suspendMutex);
}
/***********************************************************
 *	Function 	: close_io
 *	Description : Close io process
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void close_io()
{
    MAINLOG(0, "%s\n", __func__);
    pthread_mutex_lock(&io_suspendMutex);
    io_suspendFlag = 0;
    io_doneFlag = 1;
    pthread_cond_broadcast(&io_resumeCond);
    pthread_mutex_unlock(&io_suspendMutex);
}

/***********************************************************
 *	Function 	: suspend_mp
 *	Description : Suspend main process
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void suspend_mp()
{ // tell the thread to suspend
    MAINLOG(0, "%s\n", __func__);
    pthread_mutex_lock(&mp_suspendMutex);
    mp_suspendFlag = 1;
    pthread_mutex_unlock(&mp_suspendMutex);
}

/***********************************************************
 *	Function 	: resume_mp
 *	Description : Resume main process
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void resume_mp(int iStatus)
{ // tell the thread to resume
    MAINLOG(0, "%s\n", __func__);
    pthread_mutex_lock(&mp_suspendMutex);
    mp_suspendFlag = 0;
    mp_FlowStatus = iStatus;
    pthread_cond_broadcast(&mp_resumeCond);
    pthread_mutex_unlock(&mp_suspendMutex);
}

/***********************************************************
 *	Function 	: close_mp
 *	Description : Close main process
 *	Param       : NONE
 *	Return      : NONE
 *************************************************************/
void close_mp()
{
    MAINLOG(0, "%s\n", __func__);
    pthread_mutex_lock(&mp_suspendMutex);
    mp_suspendFlag = 0;
    mp_doneFlag = 1;
    pthread_cond_broadcast(&mp_resumeCond);
    pthread_mutex_unlock(&mp_suspendMutex);
}

/***********************************************************
 *	Function 	: getStatus_mp
 *	Description : Get main process status
 *	Param       : int *pStatus
 *	Return      : NONE
 *************************************************************/
void getStatus_mp(int *pStatus)
{
    // int retStatus;
    pthread_mutex_lock(&mp_suspendMutex);
    *pStatus = mp_FlowStatus;
    pthread_mutex_unlock(&mp_suspendMutex);
}

/***********************************************************
 *	Function 	: setStatus_mp
 *	Description : Set main process status
 *	Param       : int *pStatus
 *	Return      : NONE
 *************************************************************/
void setStatus_mp(int pStatus)
{
    // int retStatus;
    pthread_mutex_lock(&mp_suspendMutex);
    mp_FlowStatus = pStatus;
    pthread_mutex_unlock(&mp_suspendMutex);
}

/***********************************************************
 *	Function 	: init_value_set_to_default
 *	Description : Reset IO subsystem process status
 *	Param       : int *pStatus
 *	Return      : NONE
 *************************************************************/
void init_value_set_to_default()
{
    ios_status.finish = FALSE;
}

/***********************************************************
 *	Function 	: main
 *	Description : initial subsystem and create thread for MQTT
 *                subcriber MQTT and create a command testing 
 *                by keyboard (for subsystem or web backend)
 *	Param     : void *argu --> none
 *	Return    : NONE
 *************************************************************/
int main(int argc, char **argv)
{
    int ret;
    
    std::string strFWVersion(FW_VERSION);
    std::string strIPSVersion(VSB_VERSION);
    std::string strIOSVersion(IOS_VERSION);

    MAINLOG(0, "*******************************************\n");
    MAINLOG(0, "*****   Main App. [%s] *****\n", strFWVersion.c_str());
    MAINLOG(0, "***** >> IPS_ver: %s *****\n", strIPSVersion.c_str());
    MAINLOG(0, "***** >> IOS_ver: %s *****\n", strIOSVersion.c_str());
    MAINLOG(0, "***** >> (Compile time: %s,%s) *****\n", __DATE__, __TIME__);
    MAINLOG(0, "*******************************************\n");

    /* Signal handling */
    signal(SIGKILL, sigExit_main);
    signal(SIGTERM, sigExit_main);
    signal(SIGSEGV, sigExit_main);
    signal(SIGINT, sigExit_main);

    /* init value */
    jes.new_job = NO_SETFUNC;
    jes.current_job = NO_SETFUNC;
    init_value_set_to_default();

    xlog("%s:%d \n\r", __func__, __LINE__);

    /* initial vailable */
    ipsComp_IPL_Init();
    ipsComp_Camera_Init_Dual(0);    //Dual camera >> Camera handle
    ipsComp_Camera_Init_Dual(1);    //Dual camera >> Camera handle

    xlog("%s:%d \n\r", __func__, __LINE__);

    /* initial inner queue(std::deque)  */
    innerQ_Main_Init();
    
    innerQ_IPS_Init_Dual(0);    //Dual camera
    innerQ_IPS_Init_Dual(1);    //Dual camera

    innerQ_IOS_Init();

    xlog("%s:%d \n\r", __func__, __LINE__);

    // /* initial IPS task queue(std::deque)  */
    //Dual camera for IPS_Dual
    JsonQ_Init_Dual(0);
    JsonQ_Init_Dual(1);
    TasksQ_Init_Dual(0);
    TasksQ_Init_Dual(1);
    ExModeQ_Init_Dual(0);
    ExModeQ_Init_Dual(1);
    
    xlog("%s:%d \n\r", __func__, __LINE__);

    // /* initial IOS task queue(std::deque)  */
    IO_JsonQ_Init();

    // /* initial AIS task queue(std::deque)  */


    xlog("%s:%d \n\r", __func__, __LINE__);
    /* initial hash tabe of Mqtt parse and Method assign */
    createHashMap_Param();
    createHashMap_Method();
    
    createHashMap_IO_Param();

    usleep(100000);

    xlog("%s:%d \n\r", __func__, __LINE__);
    /* internal Mqtt commmand for StreamingMode enable. */
    FW_Mqtt_PriorityPass_Internal(1);

    usleep(100000);

    xlog("%s:%d \n\r", __func__, __LINE__);
    /* create a thread for main handler  */
    ret = pthread_create(&thread1, nullptr, mainCtl, nullptr);
    if (ret < 0)
    {
        perror("Cannot create thread 1 !!\n");
        exit(1);
    }

    ret = pthread_create(&thread2, nullptr, ext_mqtt_sub_Dual, nullptr);
    if (ret < 0)
    {
        perror("Cannot create thread 2 !!\n");
        exit(1);
    }

    xlog("%s:%d \n\r", __func__, __LINE__);
    /* create a thread for IP process */
    int iCamId[2] = {0 ,1}; //dual camera
    fprintf(stderr, "%s()%d: >> thread3() iCamId = %d\n", __FUNCTION__, __LINE__, iCamId[0]);
    ret = pthread_create(&thread3, nullptr, ips_process_Dual, &iCamId[0]);
    if (ret < 0)
    {
        perror("Cannot create thread 3 _ ips_process_Dual(...) !!\n");
        exit(1);
    }

    usleep(10000);

    fprintf(stderr, "%s()%d: >> thread4() iCamId = %d\n", __FUNCTION__, __LINE__, iCamId[1]);    
    ret = pthread_create(&thread4, nullptr, ips_process_Dual, &iCamId[1]);
    if (ret < 0)
    {
        perror("Cannot create thread 4 _ ips_process_Dual(...) !!\n");
        exit(1);
    }    

    fprintf(stderr, "%s()%d: >> thread4() ios\n", __FUNCTION__, __LINE__);    
    ret = pthread_create(&thread5, nullptr, ios_process, &iCamId[1]);
    if (ret < 0)
    {
        perror("Cannot create thread 5 _ ios_process(...) !!\n");
        exit(1);
    }    

    xlog("%s:%d \n\r", __func__, __LINE__);
    /* init mcuCtl */
    ret = iosCtl_init();
    if (ret < 0)
    {
        perror("iosCtl_init");
        exit(1);
    }

    xlog("%s:%d \n\r", __func__, __LINE__);

    suspend_ip_Dual(0);
    suspend_ip_Dual(1);
    suspend_io();
    /* read the input character from keyborad be pressed  */

    while (1)
    {

        if (bTearDown)
        {
            break;
        }

        sleep(1);
    };

    return 0;
}
