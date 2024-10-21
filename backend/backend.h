#ifndef __AUDIO_HANDLER_H__
#define __AUDIO_HANDLER_H__

#include <linux/types.h>
#include <stdbool.h>

#define MAIN_CTL_BY_CONSOLE_INPUT 1

// #define MAIN_CTL_QUERY_ALLPLAY_NETWORK_STATUS	1

#define COMMAND_BUFFER_SIZE 100

typedef enum
{
    IOS = 0,
    CMS,
    IPS,
    OTAS,
    // AIS,
} SUBSYSTEM_NAME;

static char *subsystem_string[] =
    {
        "IOS",
        "CMS",
        "IPS",
        "OTAS",
        // "AIS",
};

#if 0
/************************************************
    Airplay Module
************************************************/
typedef enum __AIRPLAY_STATUS__
{
	AIR_CONFIGURED,		// has been configured
	AIR_NO_CONFIGURED,
	AIR_WAC,
	AIR_DIRECT,
	AIR_CONFIG,
	AIR_CONNECTING,
	AIR_QUICK_CONNECTING,
	AIR_CONNECTED,
	AIR_DISCONNECTED,
	AIR_NOROUTER,			// no target router state
	AIR_STATUS_UNKNOWN,
}AIRPLAY_STATUS;

char *air_status_str[]={
	"AIR_CONFIGURED",
	"AIR_NO_CONFIGURED",
	"AIR_WAC",
	"AIR_DIRECT",
	"AIR_CONFIG",
	"AIR_CONNECTING",
	"AIR_QUICK_CONNECTING",
	"AIR_CONNECTED",
	"AIR_DISCONNECTED",
	"AIR_NOROUTER",
	"AIR_STATUS_UNKNOWN",
};

/* Airplay send command to handler */
typedef enum __AIRPLAY_TO_HANDLER_CMD__
{
	AIR_IS_OPEN,
	AIR_IS_PLAY,
	AIR_IS_STOP,
	AIR_IS_PAUSE,
	AIR_IS_VDOWN_,
	AIR_IS_VUP_,
	AIR_IS_CONNECTED,
	AIR_IS_DISCONNECTED,
	AIR_IS_VOL_,
	AIR_IS_UNKNOWN,
	AIR_IS_MAX = AIR_IS_UNKNOWN,
}AIRPLAY_TO_HANDLER_CMD;

/* handler send command to Airplay */
typedef enum __HANDLER_TO_AIRPLAY_CMD__
{
	AIR_GO_PLAY,
	AIR_GO_PAUSE,
	AIR_GO_STOP,
	AIR_GO_PREV,
	AIR_GO_NEXT,
	AIR_GO_VDOWN,
	AIR_GO_VUP,
	AIR_GO_BUSY,
	AIR_GO_NONBUSY,
	AIR_GO_UNKNOWN,
}HANDLER_TO_AIRPLAY_CMD;

char *air_2_han_str[]={
	"AIR_OPEN",
	"AIR_PLAY",
	"AIR_STOP",
	"AIR_PAUSE",
	"AIR_VDOWN_",
	"AIR_VUP_",
	"AIR_CONNECTED",
	"AIR_DISCONNECTED",
	"AIR_VOL_"
	"AIR_UNKNOWN",
};

char *han_2_air_str[]={
	"AIR_PLAY",
	"AIR_PAUSE",
	"AIR_STOP",
	"AIR_PREV",
	"AIR_NEXT",
	"AIR_VDOWN_",
	"AIR_VUP_",
	"AIR_BUSY",
	"AIR_NONBUSY",
	"AIR_UNKNOWN",
};               
     
typedef struct __AIRPLY_MODULE__
{
	AIRPLAY_TO_HANDLER_CMD	rec_cmd;	
	HANDLER_TO_AIRPLAY_CMD	send_cmd;
	AIRPLAY_STATUS			status;
	AIRPLAY_TO_HANDLER_CMD	airCurStatus;
	bool					configured;
	bool					playing;
}AIRPLAY_MODULE;	

/************************************************
				BT Module
************************************************/
typedef enum __BT_STATUS__
{
	BT_PAIRING,
	BT_CONNECTED,
	BT_DISCONNECTED,
	BT_PAUSE,
	BT_PLAY,
	BT_STATUS_UNKNOWN,
}BT_STATUS;
/* BT send command to handler */
typedef enum __BT_TO_HANDLER_CMD__
{
	BT_IS_PAIRING,
	BT_IS_CONNECTED,
	BT_IS_DISCONNECTED,
	BT_IS_PAUSE,
	BT_IS_PLAY,
	BT_IS_VOLUME,
	BT_IS_MAX,
}BT_TO_HANDLER_CMD;

/* handler send command to Airplay */
typedef enum __HANDLER_TO_BT_CMD__
{
	BT_GO_PLAY,
	BT_GO_PAUSE,
	BT_GO_STOP,
	BT_GO_PREV,
	BT_GO_NEXT,
	BT_GO_VDOWN,
	BT_GO_VUP,
	BT_GO_DISCONNECT,		
	BT_GO_CONNECT,
	BT_GO_PAIRING,
	BT_GO_UNKNOWN,
}HANDLER_TO_BT_CMD;

char *bt_status_str[]={
	"BT_PAIRING",
	"BT_CONNECTED",
	"BT_DISCONNECTED",
	"BT_STATUS_UNKNOWN",
};

char *bt_2_han_str[]={
	"BT_PAIRING",
	"BT_CONNECTED",
	"BT_DISCONNECTED",
	"BT_PAUSE",
	"BT_PLAY",
	"BT_VOLUME_"
};

char *han_2_bt_str[]={
	"BT_PLAY",
	"BT_PAUSE",
	"BT_STOP",
	"BT_PREV",
	"BT_NEXT",
	"BT_VDOWN",
	"BT_VUP",
	"BT_DISCONNECT",
	"BT_CONNECT",
	"BT_PAIRING",
	"BT_UNKNOWN",
};               

typedef struct __BT_MODULE__
{
	BT_TO_HANDLER_CMD	rec_cmd;	
	HANDLER_TO_BT_CMD	send_cmd;
	BT_STATUS			status;
	bool				playing;
}BT_MODULE;	

/************************************************
				DLNA Module
************************************************/
typedef enum __DLNA_STATUS__
{
	DLNA_OPENING,
	DLNA_OPENED,
	DLNA_OPEN_ERROR,
}DLNA_STATUS;

char *dlna_status_str[]={
	"DLNA_OPENING",
	"DLNA_OPENED",
	"DLNA_OPEN_ERROR",
};

/* DLNA send command to handler */
typedef enum __DLNA_TO_HANDLER_CMD__
{
	DLNA_IS_OPEN,
	DLNA_IS_OPEN_READY,
	DLNA_IS_PLAY,
	DLNA_IS_STOP,
	DLNA_IS_PAUSE,
	DLNA_IS_VDOWN_,
	DLNA_IS_VUP_,
	DLNA_IS_CONNECTED,
	DLNA_IS_DISCONNECTED,
	DLNA_IS_VOL_,
	DLNA_IS_UNKNOWN,
	DLNA_IS_MAX = DLNA_IS_UNKNOWN,
}DLNA_TO_HANDLER_CMD;

/* handler send command to Airplay */
typedef enum __HANDLER_TO_DLNA_CMD__
{
	DLNA_GO_PLAY,
	DLNA_GO_PAUSE,
	DLNA_GO_STOP,
	DLNA_GO_PREV,
	DLNA_GO_NEXT,
	DLNA_GO_VDOWN_,
	DLNA_GO_VUP_,
	DLNA_GO_BUSY,
	DLNA_GO_NONBUSY,
	DLNA_GO_UNKNOWN,
}HANDLER_TO_DLNA_CMD;

char *dlna_2_han_str[]={
	"DLNA_OPEN",
	"DLNA_OPEN_READY",
	"DLNA_PLAY",
	"DLNA_STOP",
	"DLNA_PAUSE",
	"DLNA_VDOWN_",
	"DLNA_VUP_",
	"DLNA_CONNECTED",
	"DLNA_DISCONNECTED",
	"DLNA_VOL_",
	"DLNA_UNKNOWN",
};

typedef struct __DLNA_MODULE__
{
	DLNA_TO_HANDLER_CMD	rec_cmd;	
	HANDLER_TO_DLNA_CMD	send_cmd;
	DLNA_STATUS			status;
	bool				playing;
}DLNA_MODULE;	

/************************************************
				System
************************************************/
typedef enum __STATUS__
{
	SLEEP,
	WAKEUP,
}STATUS;

/* Audio source selection */
typedef enum __SOURCE__
{
	WIFI,
	BT,
	AUX,
	UNKNOWN,
}SOURCE;

char *sourceStr[]={
	"WIFI",
	"BT",
	"AUX",
	"UNKNOWN"
};

typedef enum __WIFI_MODE__
{
	IS_CLIENT_MODE,
	IS_AP_MODE,
}WIFI_MODE;
	
typedef struct __WIFI_PARAS__
{
	char 		ssid_config[32];	// AIR_CONFIG SSID
	char 		ssid_direct[32];	// AIR_DIRECT SSID
	char 		ssid[32];			// for test
	WIFI_MODE	mode;
}WIFI_PARAS;
	
/* define LED's GPIO pin number */
typedef enum __LEDS__
{
	PWR_WHITE,	// GPIO#26 (UI panel)
	WIFI_R,	// GPIO#30 (UI panel
	WIFI_G,	// GPIO#31 (UI panel)
	BT_BLUE,	// GPIO#32 (UI panel)
	WIFI_GREEN,	// GPIO#35 (ON Board)
	MAX_LED,
}LEDS;
/* set LED's GPIO pin mumber */

unsigned char 		led_map[5]={26, 30, 31, 32, 35};	
typedef enum __LED_ACTION__
{
	LED_ON,
	LED_OFF,
	SBLK,	//2 sec ON, 1 sec OFF
	BLK,	//1 sec ON, 1 sec OFF (0.5 Hz, 50% on/off)
	FBLK,	//0.5 sec ON, 0.5 sec OFF (1Hz, 50% on/off)
	NO_ACTION,
}LED_ACTION;

typedef struct __LED_STATUS__
{
	LEDS 		color1;
	LEDS 		color2;
	LED_ACTION	ledAct1;
	LED_ACTION	ledAct2;
}LED_STATUS;

typedef enum __VOLUME__
{
	VOLUMEUP,
	VOLUMEDOWN,
	MUTE,
}VOLUME;

typedef enum __HW_SWITCH_SRC__
{
	I2S_SRC_BT_DLNA=0,
	I2S_SRC_AIR_PLAY,
}HW_SWITCH_SRC;

typedef enum __PA_SWITCH_SRC__
{
	PA_SRC_AIR_BT_DLNA=0,
	PA_SRC_AUX_IN,
}PA_SWITCH_SRC;

typedef enum __PA_MUTE_STATUS__
{
	PA_IS_MUTE=0,
	PA_IS_NOT_MUTE,
}PA_MUTE_STATUS;

typedef enum __DMR_WIFI_MODE__
{
	DMR_BR0_MODE=0,
	DMR_APCLI0_MODE,
}DMR_WIFI_MODE;

typedef struct __AUDIO_HANDLER__
{
//	DLNA_STATUS			dlna;
	STATUS				power;
	SOURCE				source;		// current source
	SOURCE				new_source;	// change source to xxx
	AIRPLAY_MODULE		air;
	BT_MODULE			bt;
	DLNA_MODULE			dlna;	
	WIFI_PARAS			wifi;		
//	unsigned char 		led;
	//unsigned char		volume;
	int16_t 			air_Vol;
	int16_t 			aux_Vol;
	int16_t				volume;
	LED_STATUS			ledStatus;
	bool				quicklyAir;	// quickly start airplay if configured
}AUDIO_HANDLER;

#define STM32_I2C_SLAVE_ADDR 0x28

#define STM32_SET_PWR_STATUS_COMMAND 0x3c
#define STM32_GET_PWR_STATUS_COMMAND 0x3d
#define STM32_SET_PWR_LED_COMMAND 0x4e

#define DISABLE_STM32_PWR_SW_DETECTION 0x00
#define ENABLE_STM32_PWR_SW_DETECTION 0x01

#define POWER_STATUS_OFF 0x00
#define POWER_STATUS_ON 0x01

#define SET_F2_PWR_LED_ON 0x00
#define SET_F2_PWR_LED_OFF 0x01
#define SET_F2_PWR_LED_BLK 0x02
#define SET_F2_PWR_LED_INPUT 0x03
#endif

#endif