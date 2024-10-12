/**
 ******************************************************************************
 * @file    iosCtl.c
 * @brief   Peripheral IO controller for vision box.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 Primax Technology Ltd.
 * All rights reserved.
 *
 ******************************************************************************
 */

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */

#include "iosCtl.h"

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
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <linux/input.h>
#include <linux/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <modbus.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/watchdog.h> 
#include <poll.h>
#include <chrono>
#include <ctime>
#include <ifaddrs.h>
#include "common.hpp"
#include "global.hpp"
#include "gpio.h"
#include "pwm.h"
#include "spi.h"

#if defined (__cplusplus)
extern "C" {
#endif
    #include <i2c/smbus.h>
    // #include "i2cbusses.h"
    #include "ext_mqtt_client.hpp"

    #include "VL53L1X_api.h"
    #include "VL53L1X_calibration.h"
    #include "vl53l1_platform.h"

#if defined (__cplusplus)
}
#endif

volatile char rbuf[IOS_BUF_SIZE];
volatile char wbuf[IOS_BUF_SIZE];

/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */
pthread_t iosThread;
pthread_t didoThread;
pthread_t wdtThread;
pthread_t ledThread;
pthread_t trigThread;
extern bool bTearDown;
extern char trig_pin[6];
extern char trig_edge[6][16];
extern char trig_dout_pin[6];
extern char trig_dout_active[6][16]; // rising / falling
extern char trig_DinMode;
extern char trig_DinMode_Dual;
extern char trig_trigger1;
extern char trig_trigger2;
int iosLED_blink_bit = 0;
unsigned int iosDO1_blink_time = 0;
unsigned int iosDO2_blink_time = 0;
unsigned int iosDO3_blink_time = 0;
unsigned int iosDO4_blink_time = 0;
unsigned char ios_cameraid = 0;
extern void* pter_hdl_GigE;

/* MACRO DECLARATIONS ---------------------------------------------------------------- */
#define PCA64_P0  0x02
#define PCA64_P1  0x03
char iosLED_val0;
char iosLED_val1;
int iosLED_file;
int led_i2cbus;
int led_Blinking;
int sfcCtl_serial_port = 0;

/* GLOBAL VARIABLE DECLARATIONS ------------------------------------------------------- */
bool doTrigger = false;

int fd = 0; // for rpmsg driver
char ip[IP_SIZE];
const char *test_eth = "eth1";
/*------- Modbus parameter init --------*/
static            modbus_t *ctx = NULL;
static            modbus_mapping_t *mb_mapping;
static int        server_socket = -1;
uint8_t           modbus_status = 0;        /* 0: disable 1: enable */
uint8_t           ios_query[MODBUS_TCP_MAX_ADU_LENGTH];
static uint8_t    GI_Testing_Register = 1;
static uint8_t    GI_Test_Status_Register = 2;
static uint8_t    GI_Testing_Result_Register = 3;
uint8_t           FrameFormat = RTU;
uint16_t          nb_connections = 10;
uint16_t          TCP_IP_PORT = 1502;
uint32_t          RTU_BAUDRATE = 115200;
int               rc;
int               master_socket;
int               fdmax;
fd_set            refset;
fd_set            rdset;

/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */
extern void Process_Flow_Handler(void);

/* EXPORTED SUBPROGRAM BODIES -------------------------------------------------------- */
/***********************************************************
 *	Function 	: get_loacl_ip
 *	Description : Retrieve the local IP address of a specified network interface
 *	Param 		: such as the Ethernet interface eth0
 *	Return		: NONE
 *************************************************************/
void get_loacl_ip (const char *eth_inf, char *ip)
{
  int sd;
  struct sockaddr_in sin;
  struct ifreq ifr;

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sd == -1) {
    printf("[IOS](%s): socket error: %s \r\n", __func__, strerror(errno));
    return ;
  }

  strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
  ifr.ifr_name[IFNAMSIZ - 1] = 0;

  if (ioctl(sd, SIOCGIFADDR, &ifr) < 0) {
    printf("[IOS](%s): ioctl error: %s \r\n", __func__, strerror(errno));
    close(sd);
    return ;
  }

  memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
  snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

  printf("[IOS](%s): Local %s ip: %s \n", __func__, test_eth, ip);

  close(sd);
}

/***********************************************************
 *	Function 	: ios_modbus_free
 *	Description : Used to close the Modbus connection and release associated resources
 *	Param 		: NONE
 *	Return		: NONE
 *************************************************************/
void ios_modbus_free(void)
{
    IOSLOG(0, "[IOS](%s): Close Modbus! \n", __func__);
      
    if(modbus_status == 0) {
        return ;
    }
      
    if (server_socket != -1) {
        close(server_socket);
    }

    /* Free Memory */
    modbus_mapping_free(mb_mapping);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);
    modbus_status = 0;
}

/***********************************************************
 *	Function 	: close_sigint
 *	Description : Catch signals and free Modbus
 *	Param 		: NONE
 *	Return		: NONE
 *************************************************************/
void close_sigint(int sig)
{
    IOSLOG(0, "[IOS](%s): Signal %d caught! \n", __func__, sig);
    ios_modbus_free();
    SPI_Close();
    sleep(2);
    exit(1);
}

/***********************************************************
 *	Function 	: set_nonblocking
 *	Description : Set the given socket file descriptor to non-blocking mode.
 *	Param 		: int socket : socket number
 *	Return		: integer value
 *************************************************************/
int set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

/***********************************************************
 *	Function 	: ios_modbus_init
 *	Description : ModBus init function
 *	Param 		: NONE
 *	Return		: integer value
 *************************************************************/
int ios_modbus_init(void)
{
  if (FrameFormat == TCP) {
    get_loacl_ip(test_eth, ip);
    ctx = modbus_new_tcp(ip, TCP_IP_PORT);
        if (ctx == NULL) {
      IOSLOG(0, "[IOS](%s): Unable to create the libmodbus context! \r\n", __func__);
      return -1;
    }

    server_socket = modbus_tcp_listen(ctx, nb_connections);
    if (server_socket == -1) {
      IOSLOG(0, "[IOS](%s): Unable to listen TCP connection! \r\n", __func__);
      modbus_free(ctx);
      return -1;
    }

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;
  } else if (FrameFormat == RTU) {
    ctx = modbus_new_rtu(RTU_COM_PORT, RTU_BAUDRATE, RTU_PARITY, RTU_DATA_BIT, RTU_STOP_BIT);
        if (ctx == NULL) {
      IOSLOG(0, "[IOS](%s): Unable to create the libmodbus context! \r\n", __func__);
      return -1;
    }

    if (modbus_set_slave(ctx, RTU_SERVER_ID) == -1) {
      IOSLOG(0, "[IOS](%s): Invalid slave ID! \r\n", __func__);
      modbus_free(ctx);
      return -1;
    }

    if (modbus_connect(ctx) == -1) {
      IOSLOG(0, "[IOS](%s): Connection failed: %s \r\n", __func__, modbus_strerror(errno));
      modbus_free(ctx);
      return -1;
    }
  }

#ifdef MODBUS_DEBUG_MODE_ENABLE
  modbus_set_debug(ctx, TRUE);
#endif

  mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0, MODBUS_MAX_READ_REGISTERS, 0);
    if (mb_mapping == NULL) {
    IOSLOG(0, "[IOS](%s): Failed to allocate the mapping: %s\n", __func__, modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
  }

  mb_mapping->tab_registers[GI_Test_Status_Register] = 0;
  mb_mapping->tab_registers[GI_Testing_Result_Register] = 0;
  modbus_status = 1;
  return 0;
}

/***********************************************************
 *	Function 	: ios_WriteBuftoKernel
 *	Description : ModBus init function
 *	Param 		: NONE
 *	Return		: integer value
 *************************************************************/
int ios_WriteBuftoKernel() {
  uint8_t retry = 3;
  int ret;

  while (retry > 0) {
    ret = write(fd, (char *)wbuf, IOS_BUF_SIZE);
    if (ret >= 0) {
      retry = 0;
    } else {
      retry--;
    }
    usleep(50000); /* delay 50  ms */
  }
  return ret;
}

/***********************************************************
 *	Function 	: ios_triggerSetProcess
 *	Description : Set Trigger Pin Parameter High / Low
 *	Param 		: structure IOS_TRIGGER_SET_PROCESS
 *	Return		: integer value
 *************************************************************/
int ios_triggerSetProcess(IOS_TRIGGER_SET_PROCESS * ios_trigger)
{
  int ret = 0;

  if((ios_trigger->inPin == TRIGGER_PIN_1) || (ios_trigger->inPin == TRIGGER_PIN_2)){
    IOSLOG(0, "[IOS](%s): new command is IOS_TRIGGER_SET_PROCESS. \n", __func__);
    IOSLOG(0, YELLOW "[IOS](%s): do something... \n", __func__);
    if(!strncasecmp((char *)ios_trigger->inMode, "High", sizeof("High")))
    {
      IOSLOG(0, "[IOS](%s): do something... \n", __func__);
    }
    else{
      IOSLOG(0, "[IOS](%s): do something... \n", __func__);
    }

    IOSLOG(0, "[IOS](%s): ret=%d\n", __func__, ret);
    return ret;
  }
  return ret;
}

/***********************************************************
 *	Function 	: ios_triggerGetProcess
 *	Description : Get Trigger Pin Parameter
 *	Param 		: structure IOS_TRIGGER_SET_PROCESS
 *	Return		: integer value
 *************************************************************/
/* basically, this "get" command has not any help, mainCtl can get structure's value and return to backend directly */
int ios_triggerGetProcess(IOS_TRIGGER_SET_PROCESS * ios_trigger)
{
  if((ios_trigger->inPin == TRIGGER_PIN_1) || (ios_trigger->inPin == TRIGGER_PIN_2)) {
    IOSLOG(0, "[IOS](%s): new command is TRIGGER_GET. \n", __func__);
    IOSLOG(0, YELLOW "[IOS](%s): do something... \n", __func__);
  }
  return 0;
}

/***********************************************************
 *	Function 	: ios_dinSetProcess
 *	Description : DI input Pin triggers rising or falling
 *	Param 		: structure IOS_TRIGGER_SET_PROCESS
 *	Return		: integer value
 *************************************************************/
int ios_dinSetProcess(IOS_DI_SET_PROCESS * ios_di)
{
    int ret = 0;
    
    if((ios_di->inPin >= DIN_PIN_1) && (ios_di->inPin <= DIN_PIN_4)) {
        trig_pin[ios_di->inPin] = ios_setDinMode.DinPin;

        if(!strcmp((char *)&ios_setDinMode.DinPolarity[0], "High")) {
            snprintf(trig_edge[ios_di->inPin], sizeof(trig_edge[ios_di->inPin]), "rising");
        } else {
            snprintf(trig_edge[ios_di->inPin], sizeof(trig_edge[ios_di->inPin]), "falling");
        }
        printf("%s()%d: trig_pin[%d]=[%d] trig_edge[%d]=[%s]\n", __FUNCTION__, __LINE__, ios_di->inPin, trig_pin[ios_di->inPin], ios_di->inPin, trig_edge[ios_di->inPin]);
        return 1;
    }
    return ret;
}

/***********************************************************
 *	Function 	: ios_dinGetProcess
 *	Description : DI input Get Status
 *	Param 		: structure IOS_DI_SET_PROCESS
 *	Return		: integer value
 *************************************************************/
/* basically, this "get" command has not any help, mainCtl can get structure's value and return to backend directly */
int ios_dinGetProcess(IOS_DI_SET_PROCESS * ios_di)
{
  if((ios_di->inPin >= DIN_PIN_1) && (ios_di->inPin <= DIN_PIN_4)) {
    IOSLOG(0, "[IOS](%s): new command is DIN_GET. do something... \r\n", __func__);
    return -1;
  }
  return 0;
}    

int ios_lightSetPWM(IOS_LIGHT_SET_PWM *ios_setpwm)
{
  int ret = 0;

  if((ios_setpwm->inLight == LIGHT_SOURCE_1) || (ios_setpwm->inLight == LIGHT_SOURCE_2)) {
    int inLight;
    if(ios_setpwm->inLight == LIGHT_SOURCE_1) {
        inLight = LIGHTING_PWM1_NUM;
    } else if(ios_setpwm->inLight == LIGHT_SOURCE_2) {
        inLight = LIGHTING_PWM2_NUM;
    }

    if ((ret = pwm_write_duty_cycle(inLight, ios_setpwm->value * AILED_STEP_LEVEL)) != 0)
    {
        IOSLOG(0, "[IOS](%s)%d: pwm_write_duty_cycle ret=[%d] fail.\n", __func__, __LINE__, ret);
        return -1;
    }
  }
  return ret;
}  
/* basically, this "get" command has not any help, mainCtl can get structure's value and return to backend directly */
int ios_lightGetPWM(IOS_LIGHT_SET_PWM *ios_setpwm)
{
  int ret;
#if 0
  if((ios_setpwm->inLight == LIGHT_SOURCE_1) || (ios_setpwm->inLight == LIGHT_SOURCE_2)) {
    /* read MCU's content via rpmsg driver */
        ret = read(fd, (char *)rbuf, IOS_BUF_SIZE);
    uint8_t index = ios_setpwm->inLight == LIGHT_SOURCE_1 ? 0 : 2;
    IOSLOG(0, "[IOS](%s): new command is LIGHT_GET_PWM %d \n", __func__, ios_setpwm->inLight);
    // IOSLOG(0, "[IOS](%s): LIGHT_SET_PWM read PWM[0]=%d \n", __func__, rbuf[IOS_LIGHT1_PWM_INDEX + index]);
    // IOSLOG(0, "[IOS](%s): LIGHT_SET_PWM read PWM[1]=%d \n", __func__, rbuf[IOS_LIGHT1_PWM_INDEX + index + 1]);
    ios_setpwm->value = ((rbuf[IOS_LIGHT1_PWM_INDEX + index + 1] << 8) + rbuf[IOS_LIGHT1_PWM_INDEX + index]) / PWM_ONE_STEP;
    IOSLOG(0, "[IOS](%s): get Light %d PWM = %d \n", __func__, ios_setpwm->inLight, ios_setpwm->value);
  }
#else
  if((ios_setpwm->inLight == LIGHT_SOURCE_1) || (ios_setpwm->inLight == LIGHT_SOURCE_2)) {
    if ((ret = pwm_read_duty_cycle(ios_setpwm->inLight)) < 0)
    {
        IOSLOG(0, "[IOS](%s)%d: pwm_write_duty_cycle ret=[%d] fail.\n", __func__, __LINE__, ret);
        return -1;
    } else {
        ios_setpwm->value = ret / AILED_STEP_LEVEL;
    }
    IOSLOG(0, "[IOS](%s)%d: get Light %d PWM = %d \n", __func__, __LINE__, ios_setpwm->inLight, ios_setpwm->value);
  }
#endif
  return ret;
}    

int iosLED_init()
{
    int ret = 0;
    int address;
    int daddress;
    char *end;
    char filename[20];
    int force = 1;
#if 1
    ios_setStatusLed(LED1_PWR, LED_OFF);
    ios_setStatusLed(LED2_STAT, LED_OFF);
    ios_setStatusLed(LED3_COM, LED_OFF);
    ios_setStatusLed(LED4_TRIG, LED_OFF);
    ios_setStatusLed(LED5_ERR, LED_OFF);
#else
    do {
        led_i2cbus = lookup_i2c_bus("2");
        if (led_i2cbus < 0) {
            printf("%s()%d: lookup_i2c_bus led_i2cbus=[%d] fail.\n", __FUNCTION__, __LINE__, led_i2cbus);
            break;
        }
        address = parse_i2c_address("0x20");
        if (address < 0) {
            printf("%s()%d: lookup_i2c_bus address=[%d] fail.\n", __func__, __LINE__, address);
            break;
        }
        daddress = strtol("0x02", &end, 0);
        if (*end || daddress < 0 || daddress > 0xff) {
        	printf("%s()%d: Error: Data address=[%d] invalid fail.\n", __func__, __LINE__, daddress);
        	break;
        }
        iosLED_file = open_i2c_dev(led_i2cbus, filename, sizeof(filename), 0);
        if (iosLED_file < 0 || set_slave_addr(iosLED_file, address, force)) {
        	printf("%s()%d: Error: open_i2c_dev iosLED_file=[%d] fail.\n", __func__, __LINE__, iosLED_file);
        	break;
        }
    } while(0);
    
    if ((ret = i2c_smbus_write_byte_data(iosLED_file, 0x06, (char)0x00)) < 0) {
        printf("%s()%d: Error: i2c_smbus_write_byte_data ret=[%d]\n", __func__, __LINE__, ret);
    }
    if ((ret = i2c_smbus_write_byte_data(iosLED_file, 0x07, (char)0x00)) < 0) {
        printf("%s()%d: Error: i2c_smbus_write_byte_data ret=[%d]\n", __func__, __LINE__, ret);
    }
    
    if ((ret = i2c_smbus_write_byte_data(iosLED_file, PCA64_P0, (char)0x00)) < 0) {
        printf("%s()%d: Error: i2c_smbus_write_byte_data ret=[%d]\n", __func__, __LINE__, ret);
    }
    if ((ret = i2c_smbus_write_byte_data(iosLED_file, PCA64_P1, (char)0x00)) < 0) {
        printf("%s()%d: Error: i2c_smbus_write_byte_data ret=[%d]\n", __func__, __LINE__, ret);
    }
#endif
    return ret;
}

int iosLED_i2cset(int led, int active)
{
    int ret = 0;
    if ((iosLED_val0 = i2c_smbus_read_byte_data(iosLED_file, PCA64_P0)) < 0) {
        printf("%s()%d: Error: i2c_smbus_read_byte_data iosLED_val0=[%d]\n", __func__, __LINE__, iosLED_val0);
    }
    if ((iosLED_val1 = i2c_smbus_read_byte_data(iosLED_file, PCA64_P1)) < 0) {
        printf("%s()%d: Error: i2c_smbus_read_byte_data iosLED_val1=[%d]\n", __func__, __LINE__, iosLED_val1);
    }
    
    //DebugPrint(9, "%s()%d: 1 [%d][%d] iosLED_val0=[%d] iosLED_val1=[%d]", __func__, __LINE__, led, active, iosLED_val0, iosLED_val1);
    if(active == 0) {
        if(led <= 7) {
            iosLED_val0 = iosLED_val0 & ~(1 << led);
        } else {
            iosLED_val1 = iosLED_val1 & ~(1 << (led - 8));
        }
    } else {
        if(led <= 7) {
            iosLED_val0 = iosLED_val0 | (1 << led);
        } else {
            iosLED_val1 = iosLED_val1 | (1 << (led - 8));
        }
    }
    //DebugPrint(9, "%s()%d: 2 [%d][%d] iosLED_val0=[%d] iosLED_val1=[%d]", __func__, __LINE__, led, active, iosLED_val0, iosLED_val1);

    //iosLED_val1 = iosLED_val0 = count++;
    if ((ret = i2c_smbus_write_byte_data(iosLED_file, PCA64_P0, (char)iosLED_val0)) < 0) {
        printf("%s()%d: Error: i2c_smbus_write_byte_data ret=[%d]\n", __func__, __LINE__, ret);
    }
    if ((ret = i2c_smbus_write_byte_data(iosLED_file, PCA64_P1, (char)iosLED_val1)) < 0) {
        printf("%s()%d: Error: i2c_smbus_write_byte_data ret=[%d]\n", __func__, __LINE__, ret);
    }
    return ret;
}

int iosLED_i2cset_blink(int val0, int val1)
{
    int ret = 0;

    if ((ret = i2c_smbus_write_byte_data(iosLED_file, PCA64_P0, (char)val0)) < 0) {
        printf("%s()%d: Error: i2c_smbus_write_byte_data ret=[%d]\n", __func__, __LINE__, ret);
    }
    if ((ret = i2c_smbus_write_byte_data(iosLED_file, PCA64_P1, (char)val1)) < 0) {
        printf("%s()%d: Error: i2c_smbus_write_byte_data ret=[%d]\n", __func__, __LINE__, ret);
    }
    return ret;
}

int iosLED_i2cget(int port)
{
    if(port == 0) {
        if ((iosLED_val0 = i2c_smbus_read_byte_data(iosLED_file, PCA64_P0)) < 0) {
            printf("%s()%d: Error: i2c_smbus_read_byte_data iosLED_val0=[%d]\n", __func__, __LINE__, iosLED_val0);
        }
        return iosLED_val0; 
    } else {
        if ((iosLED_val0 = i2c_smbus_read_byte_data(iosLED_file, PCA64_P1)) < 0) {
            printf("%s()%d: Error: i2c_smbus_read_byte_data iosLED_val0=[%d]\n", __func__, __LINE__, iosLED_val0);
        }
        return iosLED_val0; 
    }
}

int ios_ledSetProcess(IOS_LED_SET_PROCESS *ios_setled)
{
  int ret = 0;
    printf("[IOS](%s)%d: ios_setled.outPin = %d \n", __func__, __LINE__, ios_setled->outPin);
    printf("[IOS](%s)%d: ios_setled.outStatus = %s \n", __func__, __LINE__, ios_setled->outStatus);
    printf("[IOS](%s)%d: ios_setled.outMode = %s \n", __func__, __LINE__, ios_setled->outMode);
    printf("[IOS](%s)%d: ios_setled.outBlinkDelay = %d \n", __func__, __LINE__, ios_setled->outBlinkDelay);
    printf("[IOS](%s)%d: ios_setled.outMode = %d \n", __func__, __LINE__, ios_setled->outOffDelay);
    printf("[IOS](%s)%d: ios_setled = %p \n", __func__, __LINE__, ios_setled);

  if((ios_setled->outPin >= PowerLED) && (ios_setled->outPin <= UserDef1LED)){
    IOSLOG(0, "[IOS](%s): new command is LED_SET \n", __func__);
    if((ios_setled->outPin == UserDef2LED) || (ios_setled->outPin == UserDef1LED)){
      if(!strcasecmp((char *)&ios_setled->outStatus[0], "OFF")) {
        if(ios_setled->outPin == UserDef1LED) {
            iosLED_i2cset(LED3_VB_USER1_R, 0);
            iosLED_i2cset(LED3_VB_USER1_G, 0);
        } else if(ios_setled->outPin == UserDef2LED) {
            iosLED_i2cset(LED4_VB_USER2_R, 0);
            iosLED_i2cset(LED4_VB_USER2_G, 0);
        }
      }else if(!strcasecmp((char *)&ios_setled->outStatus[0], "Red ON")){
        if((ios_setled->outPin == UserDef1LED)) {
            iosLED_i2cset(LED3_VB_USER1_R, 1);
            iosLED_i2cset(LED3_VB_USER1_G, 0);
            iosLED_blink_bit &= ~(1 << LED3_VB_USER1_R);
        } else if((ios_setled->outPin == UserDef2LED)) {
            iosLED_i2cset(LED4_VB_USER2_R, 1);
            iosLED_i2cset(LED4_VB_USER2_G, 0);
            iosLED_blink_bit &= ~(1 << LED4_VB_USER2_R);
        }
      }else if(!strcasecmp((char *)&ios_setled->outStatus[0], "Red OFF")){
        if((ios_setled->outPin == UserDef1LED)) {
            iosLED_i2cset(LED3_VB_USER1_R, 0);
            iosLED_blink_bit &= ~(1 << LED3_VB_USER1_R);
        } else if((ios_setled->outPin == UserDef2LED)) {
            iosLED_i2cset(LED4_VB_USER2_R, 0);
            iosLED_blink_bit &= ~(1 << LED4_VB_USER2_R);
        }
      }else if(!strcasecmp((char *)&ios_setled->outStatus[0], "Red Blink")){
        if((ios_setled->outPin == UserDef1LED)) {
            iosLED_i2cset(LED3_VB_USER1_R, 1);
            iosLED_i2cset(LED3_VB_USER1_G, 0);
            iosLED_blink_bit |= 1 << LED3_VB_USER1_R;
        } else if((ios_setled->outPin == UserDef2LED)) {
            iosLED_i2cset(LED4_VB_USER2_R, 1);
            iosLED_i2cset(LED4_VB_USER2_G, 0);
            iosLED_blink_bit |= 1 << LED4_VB_USER2_R;
        }
      }else if(!strcasecmp((char *)&ios_setled->outStatus[0], "Green ON")){
        if((ios_setled->outPin == UserDef1LED)) {
            iosLED_i2cset(LED3_VB_USER1_R, 0);
            iosLED_i2cset(LED3_VB_USER1_G, 1);
            iosLED_blink_bit &= ~(1 << LED3_VB_USER1_G);
        } else if((ios_setled->outPin == UserDef2LED)) {
            iosLED_i2cset(LED4_VB_USER2_R, 0);
            iosLED_i2cset(LED4_VB_USER2_G, 1);
            iosLED_blink_bit &= ~(1 << LED4_VB_USER2_G);
        }
      }else if(!strcasecmp((char *)&ios_setled->outStatus[0], "Green OFF")){
        if((ios_setled->outPin == UserDef1LED)) {
            iosLED_i2cset(LED3_VB_USER1_G, 0);
            iosLED_blink_bit &= ~(1 << LED3_VB_USER1_G);
        } else if((ios_setled->outPin == UserDef2LED)) {
            iosLED_i2cset(LED4_VB_USER2_G, 0);
            iosLED_blink_bit &= ~(1 << LED4_VB_USER2_G);
        }
      }else if(!strcasecmp((char *)&ios_setled->outStatus[0], "Green Blink")){
        if((ios_setled->outPin == UserDef1LED)) {
            iosLED_blink_bit |= 1 << LED3_VB_USER1_G;
        } else if((ios_setled->outPin == UserDef2LED)) {
            iosLED_blink_bit |= 1 << LED4_VB_USER2_G;
        }
      }else if(!strcasecmp((char *)&ios_setled->outStatus[0], "Orange ON")){
        if((ios_setled->outPin == UserDef1LED)) {
            iosLED_i2cset(LED3_VB_USER1_G, 1);
            iosLED_blink_bit &= ~(1 << LED3_VB_USER1_G);
            iosLED_i2cset(LED3_VB_USER1_R, 1);
            iosLED_blink_bit &= ~(1 << LED3_VB_USER1_R);
        } else if((ios_setled->outPin == UserDef2LED)) {
            iosLED_i2cset(LED4_VB_USER2_G, 1);
            iosLED_blink_bit &= ~(1 << LED4_VB_USER2_G);
            iosLED_i2cset(LED4_VB_USER2_R, 1);
            iosLED_blink_bit &= ~(1 << LED4_VB_USER2_R);
        }
      }else if(!strcasecmp((char *)&ios_setled->outStatus[0], "Orange OFF")){
        if((ios_setled->outPin == UserDef1LED)) {
            iosLED_i2cset(LED3_VB_USER1_G, 0);
            iosLED_blink_bit &= ~(1 << LED3_VB_USER1_G);
            iosLED_i2cset(LED3_VB_USER1_R, 0);
            iosLED_blink_bit &= ~(1 << LED3_VB_USER1_R);
        } else if((ios_setled->outPin == UserDef2LED)) {
            iosLED_i2cset(LED4_VB_USER2_G, 0);
            iosLED_blink_bit &= ~(1 << LED4_VB_USER2_G);
            iosLED_i2cset(LED4_VB_USER2_R, 0);
            iosLED_blink_bit &= ~(1 << LED4_VB_USER2_R);
        }
      }else if(!strcasecmp((char *)&ios_setled->outStatus[0], "Orange Blink")){
        if((ios_setled->outPin == UserDef1LED)) {
            iosLED_blink_bit |= 1 << LED3_VB_USER1_R;
            iosLED_blink_bit |= 1 << LED3_VB_USER1_G;
        } else if((ios_setled->outPin == UserDef2LED)) {
            iosLED_blink_bit |= 1 << LED4_VB_USER2_R;
            iosLED_blink_bit |= 1 << LED4_VB_USER2_G;
        }
      }
    }

    IOSLOG(0, "[IOS](%s):  ret=%d\n", __func__, ret);
    return ret;
  }
  return ret;
}

int ios_doutSetProcess(IOS_DOUT_SET_PROCESS *ios_dout)
{
  int ret = 0;

  if((ios_dout != NULL) && (ios_dout->outPin < OUT_PIN_MAX) && (ios_dout->outPin >= DOUT_PIN_1)){
    IOSLOG(0, "%s()%d: outPin=[%d] outMode=[%s] onoffSetting=[%d] OneShotPeriod=[%d]\n", __func__, __LINE__
        , ios_dout->outPin, ios_dout->outMode, ios_dout->onoffSetting, ios_dout->OneShotPeriod);
        
    if(!strcasecmp((char *)&ios_dout->outMode[0], "High") && ios_dout->onoffSetting == 1) {
        spi_gpio_set_value(ios_dout->outPin - 1, 1);
    } else {
        spi_gpio_set_value(ios_dout->outPin - 1, 0);
    }
    if(ios_dout->outPin == 1) {
        iosDO1_blink_time = ios_dout->OneShotPeriod;
    } else if(ios_dout->outPin == 2) {
        iosDO2_blink_time = ios_dout->OneShotPeriod;
    } else if(ios_dout->outPin == 3) {
        iosDO3_blink_time = ios_dout->OneShotPeriod;
    } else if(ios_dout->outPin == 4) {
        iosDO4_blink_time = ios_dout->OneShotPeriod;
    }
    return ret;
  }
  return ret;
}

int ios_getStatus(IOS_IO_GET_STATUS *ios_getstatus)
{
    int ret = 0;
    IOSLOG(0, "[IOS](%s): new command is DIO_GET_STATUS. \n", __func__);
    unsigned int di1_val = 0, di2_val = 0, di3_val = 0, di4_val = 0, ailed_detect_val = 0;
    unsigned int do1_val = 0, do2_val = 0, do3_val = 0, do4_val = 0;
    #if 1
    spi_gpio_do_get_value(0, &do1_val);
    spi_gpio_do_get_value(1, &do2_val);
    
    spi_gpio_di_get_value(0, &di1_val);
    spi_gpio_di_get_value(1, &di2_val);
    spi_gpio_di_get_value(4, &ailed_detect_val);

    #else
    gpio_get_value(DO1_VB, &do1_val);
    gpio_get_value(DO2_VB, &do2_val);
    gpio_get_value(DO3_VB, &do3_val);
    gpio_get_value(DO4_VB, &do4_val);
    #endif
    
    ios_getstatus->dout1 = do1_val;
    ios_getstatus->dout2 = do2_val;
    ios_getstatus->dout3 = do3_val;
    ios_getstatus->dout4 = do4_val;
    ios_getstatus->din1 = di1_val;
    ios_getstatus->din2 = di2_val;
    ios_getstatus->din3 = di3_val;
    ios_getstatus->din4 = di4_val;
    ios_getstatus->ailed_detect = ailed_detect_val;

    ios_getstatus->trigger1 = trig_trigger1;
    ios_getstatus->trigger2 = trig_trigger2;
    IOSLOG(0, "ios_getstatus.din1=%d\n", ios_getstatus->din1);
    IOSLOG(0, "ios_getstatus.din2=%d\n", ios_getstatus->din2);

    return ret;
}

void ios_modbusSetParameters(IOS_MODBUS_SET_PARAMS *ios_modbusSetParams)
{
  IOSLOG(0, "[IOS](%s): new command is MODBUS_SET_PARAMS. \n", __func__);
  ios_modbus_free();

  RTU_BAUDRATE = ios_modbusSetParams->RTU_BAUDRATE;
  GI_Testing_Register = ios_modbusSetParams->GI_Testing_Register;
  GI_Test_Status_Register = ios_modbusSetParams->GI_Test_Status_Register;
  GI_Testing_Result_Register = ios_modbusSetParams->GI_Testing_Result_Register;

  int result = ios_modbus_init();
  if (result == 0) {
    IOSLOG(0, "[IOS](%s): Modbus Init Finish \r\n", __func__);
  } else {
    IOSLOG(0, "[IOS](%s): Modbus Init Fail \r\n", __func__);
  }
}

void ios_modbusGetParameters(IOS_MODBUS_SET_PARAMS *ios_modbusSetParams)
{
  IOSLOG(0, "[IOS](%s): new command is MODBUS_GET_PARAMS. \n", __func__);

  ios_modbusSetParams->RTU_BAUDRATE = RTU_BAUDRATE;
  ios_modbusSetParams->GI_Testing_Register = GI_Testing_Register;
  ios_modbusSetParams->GI_Test_Status_Register = GI_Test_Status_Register;
  ios_modbusSetParams->GI_Testing_Result_Register = GI_Testing_Result_Register;
}

int check_wifi_status() {
    FILE *fp;
    char path[256];
    char result[256] = "";
    
    // Open the command for reading
    fp = popen("iwconfig 2>&1", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return 1;
    }
    
    // Read the output a line at a time - output it.
    while (fgets(path, sizeof(path), fp) != NULL) {
        strcat(result, path);
    }
    
    // Close the file
    pclose(fp);

    // Check if the result contains "ESSID:off/any" which means no connection
    if (strstr(result, "ESSID:off/any") != NULL) {
        printf("WiFi is not connected.\n");
        return 0;
    } else {
        printf("WiFi is connected.\n");
        return 1;
    }
}

void ios_LED_Status_Handler(void)
{
  if (!UpdateLEDStatus_Flg) {
    return;
  }

  IOSLOG(0, "[IOS](%s)%d: LED Status Handler. [%s] [%s]\n", __func__, __LINE__, Process_Node.UsrDef1Mode, Process_Node.UsrDef2Mode);
  usleep(50000);

  if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"Light status") == 0 ||
      strcmp((char *)Process_Node.UsrDef2Mode, (char *)"Light status") == 0)
  {
    uint16_t  brightness[2];
    IOS_LED_SET_PROCESS  ios_setled_tmp;
    IOS_LIGHT_SET_PWM  ios_setpwm_tmp;

    ios_setpwm_tmp.inLight = LIGHT_SOURCE_1;
    ios_lightGetPWM(&ios_setpwm_tmp);
    brightness[0] = ios_setpwm_tmp.value;
    ios_setpwm_tmp.inLight = LIGHT_SOURCE_2;
    ios_lightGetPWM(&ios_setpwm_tmp);
    brightness[1] = ios_setpwm_tmp.value;
    
    IOSLOG(0, "%s()%d: brightness=[%d][%d] \n", __func__, __LINE__, brightness[0], brightness[1]);
    if ((brightness[0] > 0) && (brightness[1] > 0)) {
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Orange ON");
    } else if (brightness[0] > 0) {
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Red ON");
    } else if (brightness[1] > 0) {
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Green ON");
    } else {
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"OFF");
    }
    
    sprintf((char *)ios_setled_tmp.outMode, "%s", (char *)"Light status");
    ios_setled_tmp.outBlinkDelay = 0;
    ios_setled_tmp.outOffDelay = 0;

    if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"Light status") == 0) {
      ios_setled_tmp.outPin = UserDef1LED;
      ios_ledSetProcess(&ios_setled_tmp);
      usleep(50000); /* delay 50  ms */
    }

    if (strcmp((char *)Process_Node.UsrDef2Mode, (char *)"Light status") == 0) {
      ios_setled_tmp.outPin = UserDef2LED;
      ios_ledSetProcess(&ios_setled_tmp);
      usleep(50000); /* delay 50  ms */
    }
  } else if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"WiFi status") == 0 ||
             strcmp((char *)Process_Node.UsrDef2Mode, (char *)"WiFi status") == 0) 
  {
    IOS_LED_SET_PROCESS  ios_setled_tmp;
    int status = check_wifi_status();
    if (status > 0) {   // WiFi connected
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Green ON");
    } else if (status < 0) {   // WiFi disconnected
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Red ON");
    } else if (status == 0) { // AP mode (TBD)
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Orange ON");
    } else {
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"OFF");
    }

    if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"WiFi status") == 0) {
      ios_setled_tmp.outPin = UserDef1LED;
      ios_ledSetProcess(&ios_setled_tmp);
      usleep(50000); /* delay 50  ms */
    }

    if (strcmp((char *)Process_Node.UsrDef2Mode, (char *)"WiFi status") == 0) {
      ios_setled_tmp.outPin = UserDef2LED;
      ios_ledSetProcess(&ios_setled_tmp);
      usleep(50000); /* delay 50  ms */
    }
  } else if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"Test result") == 0 ||
             strcmp((char *)Process_Node.UsrDef2Mode, (char *)"Test result") == 0) 
  {
    IOS_LED_SET_PROCESS  ios_setled_tmp;
    
    sprintf((char *)ios_setled_tmp.outMode, "%s", (char *)"Test result");
    ios_setled_tmp.outBlinkDelay = 0;
    ios_setled_tmp.outOffDelay = 0;
    
    if (strcmp((char *)Process_Node.TestResult, (char *)"PASS") == 0) {
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Green ON");
    } else if (strcmp((char *)Process_Node.TestResult, (char *)"NG") == 0) {
      sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Red ON");
    } else { /* TBD: AI model loaded and AI/vision working */
      sprintf((char *)ios_setled_tmp.outStatus, "%s", "OFF");
    }
    
    if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"Test result") == 0) {
      ios_setled_tmp.outPin = UserDef1LED;
      ios_ledSetProcess(&ios_setled_tmp);
      usleep(50000); /* delay 50  ms */
    }
    
    if (strcmp((char *)Process_Node.UsrDef2Mode, (char *)"Test result") == 0) {
      ios_setled_tmp.outPin = UserDef2LED;
      ios_ledSetProcess(&ios_setled_tmp);
      usleep(50000); /* delay 50  ms */
    }
  } else if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"Trigger status") == 0 ||
      strcmp((char *)Process_Node.UsrDef2Mode, (char *)"Trigger status") == 0 ||
      strcmp((char *)Process_Node.UsrDef1Mode, (char *)"DI status")      == 0 ||
      strcmp((char *)Process_Node.UsrDef2Mode, (char *)"DI status")      == 0 ||
      strcmp((char *)Process_Node.UsrDef1Mode, (char *)"DO status")      == 0 ||
      strcmp((char *)Process_Node.UsrDef2Mode, (char *)"DO status")      == 0) 
  {
      IOS_IO_GET_STATUS ios_getstatus_tmp;
      IOS_LED_SET_PROCESS  ios_setled_tmp;

      ios_getStatus(&ios_getstatus_tmp);
      IOSLOG(0, "[IOS](%s)%d: dout=[%d][%d][%d][%d] di_val=[%d][%d][%d][%d]. \n", __func__, __LINE__
        , ios_getstatus_tmp.dout1, ios_getstatus_tmp.dout2, ios_getstatus_tmp.dout3, ios_getstatus_tmp.dout4
        , ios_getstatus_tmp.din1, ios_getstatus_tmp.din2, ios_getstatus_tmp.din3, ios_getstatus_tmp.din4);
      ios_setled_tmp.outBlinkDelay = 0;
      ios_setled_tmp.outOffDelay = 0;

      if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"Trigger status") == 0 ||
          strcmp((char *)Process_Node.UsrDef2Mode, (char *)"Trigger status") == 0 ||
          strcmp((char *)Process_Node_Dual.UsrDef1Mode, (char *)"Trigger status") == 0 ||
          strcmp((char *)Process_Node_Dual.UsrDef2Mode, (char *)"Trigger status") == 0
         )
      {IOSLOG(0, "%s()%d: trigger1=[%d] trigger2=[%d] \n", __func__, __LINE__, ios_getstatus_tmp.trigger1, ios_getstatus_tmp.trigger2);
        if (ios_getstatus_tmp.trigger1 && ios_getstatus_tmp.trigger2) {
            sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Orange ON");
        } else if (ios_getstatus_tmp.trigger1) {
            sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Red ON");
        } else if (ios_getstatus_tmp.trigger2) {
            sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Green ON");
        } else {
            sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"OFF");
        }
        sprintf((char *)ios_setled_tmp.outMode, "%s", (char *)"Trigger status");
        trig_trigger1 = trig_trigger2 = 0;
        
        if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"Trigger status") == 0) {
          ios_setled_tmp.outPin = UserDef1LED;
          ios_ledSetProcess(&ios_setled_tmp);
          usleep(50000); /* delay 50  ms */
        }
      
        if (strcmp((char *)Process_Node.UsrDef2Mode, (char *)"Trigger status") == 0) {
          ios_setled_tmp.outPin = UserDef2LED;
          ios_ledSetProcess(&ios_setled_tmp);
          usleep(50000); /* delay 50  ms */
        }
      
        if (strcmp((char *)Process_Node_Dual.UsrDef1Mode, (char *)"Trigger status") == 0) {
          ios_setled_tmp.outPin = UserDef1LED;
          ios_ledSetProcess(&ios_setled_tmp);
          usleep(50000); /* delay 50  ms */
        }
      
        if (strcmp((char *)Process_Node_Dual.UsrDef2Mode, (char *)"Trigger status") == 0) {
          ios_setled_tmp.outPin = UserDef2LED;
          ios_ledSetProcess(&ios_setled_tmp);
          usleep(50000); /* delay 50  ms */
        }
      }

      if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"DI status") == 0 ||
          strcmp((char *)Process_Node.UsrDef2Mode, (char *)"DI status") == 0)
      {
        sprintf((char *)ios_setled_tmp.outMode, "%s", (char *)"DI status");
      
        if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"DI status") == 0) {
          if (ios_getstatus_tmp.din1 && ios_getstatus_tmp.din2) {
            sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Orange ON");
          } else if (ios_getstatus_tmp.din2) {
             sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Red ON");
          } else if (ios_getstatus_tmp.din1) {
             sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Green ON");
          } else {
             sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"OFF");
          }
      
          ios_setled_tmp.outPin = UserDef1LED;
          ios_ledSetProcess(&ios_setled_tmp);
          usleep(50000); /* delay 50  ms */
        }
      
        if (strcmp((char *)Process_Node.UsrDef2Mode, (char *)"DI status") == 0) {
            if (ios_getstatus_tmp.din3 && ios_getstatus_tmp.din4) {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Orange ON");
            } else if (ios_getstatus_tmp.din4) {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Red ON");
            } else if (ios_getstatus_tmp.din3) {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Green ON");
            } else {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"OFF");
            }
      
            ios_setled_tmp.outPin = UserDef2LED;
            ios_ledSetProcess(&ios_setled_tmp);
            usleep(50000); /* delay 50  ms */
        }
      }

      if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"DO status") == 0 ||
          strcmp((char *)Process_Node.UsrDef2Mode, (char *)"DO status") == 0)
      {
         sprintf((char *)ios_setled_tmp.outMode, "%s", (char *)"DO status");
    
         if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"DO status") == 0) {
            if (ios_getstatus_tmp.dout1 && ios_getstatus_tmp.dout2) {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Orange ON");
            } else if (ios_getstatus_tmp.dout2) {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Red ON");
            } else if (ios_getstatus_tmp.dout1) {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Green ON");
            } else {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"OFF");
            }
    
            ios_setled_tmp.outPin = UserDef1LED;
            ios_ledSetProcess(&ios_setled_tmp);
            usleep(50000); /* delay 50  ms */
         }
    
         if (strcmp((char *)Process_Node.UsrDef2Mode, (char *)"DO status") == 0) {
            if (ios_getstatus_tmp.dout3 && ios_getstatus_tmp.dout4) {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Orange ON");
            } else if (ios_getstatus_tmp.dout4) {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Red ON");
            } else if (ios_getstatus_tmp.dout3) {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Green ON");
            } else {
              sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"OFF");
            }
    
            ios_setled_tmp.outPin = UserDef2LED;
            ios_ledSetProcess(&ios_setled_tmp);
            usleep(50000); /* delay 50  ms */
          }
      }
  }
  if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"POE status") == 0 ||
        strcmp((char *)Process_Node.UsrDef2Mode, (char *)"POE status") == 0) {
    /* TBD */
  }

/*
      if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"Test result") == 0 ||
          strcmp((char *)Process_Node.UsrDef2Mode, (char *)"Test result") == 0)
      {
          IOS_LED_SET_PROCESS  ios_setled_tmp;
      
          sprintf((char *)ios_setled_tmp.outMode, "%s", (char *)"Test result");
          ios_setled_tmp.outBlinkDelay = 0;
          ios_setled_tmp.outOffDelay = 0;
      
          if (strcmp((char *)Process_Node.TestResult, (char *)"PASS") == 0) {
            sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Green ON");
          } else if (strcmp((char *)Process_Node.TestResult, (char *)"NG") == 0) {
            sprintf((char *)ios_setled_tmp.outStatus, "%s", (char *)"Red ON");
          } else {
            sprintf((char *)ios_setled_tmp.outStatus, "%s", "OFF");
          }
      
          if (strcmp((char *)Process_Node.UsrDef1Mode, (char *)"Test result") == 0) {
            ios_setled_tmp.outPin = UserDef1LED;
            ios_ledSetProcess(&ios_setled_tmp);
            usleep(50000);
          }
      
          if (strcmp((char *)Process_Node.UsrDef2Mode, (char *)"Test result") == 0) {
            ios_setled_tmp.outPin = UserDef2LED;
            ios_ledSetProcess(&ios_setled_tmp);
            usleep(50000);
          }
      }
*/
  UpdateLEDStatus_Flg = 0;
  IOSLOG(0, "[IOS](%s): LED Status Handler Done.. \n", __func__);
}

void ios_Control_Light_Handler(uint8_t LightPin, uint8_t Enable)
{
  IOSLOG(0, "[IOS](%s): Control Light Handler. \n", __func__);
  usleep(50000); /* delay 50  ms */
  IOS_LIGHT_SET_PWM ios_setpwm_tmp;
  
  snprintf((char*)ios_setpwm_tmp.LightSwitch, sizeof(ios_setpwm_tmp.LightSwitch), "%s", ios_setpwm.LightSwitch);

  ios_setpwm_tmp.inLight = LightPin;
  if (Enable) {
    ios_setpwm_tmp.value = (ios_setpwm_tmp.inLight == LIGHT_SOURCE_1) ? Process_Node.Brightness[0]:
                                                                        Process_Node.Brightness[1];
  } else {
    ios_setpwm_tmp.value = 0;
  }
  IOSLOG(0, "[IOS](%s): [%d][%d][%s]. \n", __func__, ios_setpwm_tmp.inLight, ios_setpwm_tmp.value, ios_setpwm_tmp.LightSwitch);
  ios_lightSetPWM(&ios_setpwm_tmp);

  usleep(50000); /* delay 50  ms */

  IOSLOG(0, "[IOS](%s): Control Light Handler Done.. \n", __func__);
}

int ios_Control_Dout_Handler(const char *selectmode, uint8_t Enable) {
    // IOSLOG(0, "[IOS](%s): Control Dout Handler. \n", __func__);
    int ret = -1;
    IOS_DOUT_SET_PROCESS ios_dout_tmp;
    MAINLOG(0, "[MAINCTL] %s()%d: outPin=[%d] [%s][%s][%s][%s][%s]\n", __FUNCTION__, __LINE__
      , ios_doutMode.outPin
      , Process_Node.Dout1selectMode, Process_Node.Dout2selectMode, Process_Node.Dout3selectMode
      , Process_Node.Dout4selectMode, selectmode);
    if (strcmp((char *)Process_Node.Dout1selectMode, (char *)selectmode) == 0) {
        ios_dout_tmp.outPin = DOUT_PIN_1;
        strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node.Dout1Mode);
        ios_dout_tmp.onoffSetting = Enable;
        ios_doutSetProcess(&ios_dout_tmp);
        ret = 1;
    } 
    if (strcmp((char *)Process_Node.Dout2selectMode, (char *)selectmode) == 0) {
        ios_dout_tmp.outPin = DOUT_PIN_2;
        strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node.Dout2Mode);
        ios_dout_tmp.onoffSetting = Enable;
        ios_doutSetProcess(&ios_dout_tmp);
        ret = 1;
    } 
    if (strcmp((char *)Process_Node.Dout3selectMode, (char *)selectmode) == 0) {
        ios_dout_tmp.outPin = DOUT_PIN_3;
        strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node.Dout3Mode);
        ios_dout_tmp.onoffSetting = Enable;
        ios_doutSetProcess(&ios_dout_tmp);
        ret = 1;
    } 
    if (strcmp((char *)Process_Node.Dout4selectMode, (char *)selectmode) == 0) {
        ios_dout_tmp.outPin = DOUT_PIN_4;
        strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node.Dout4Mode);
        ios_dout_tmp.onoffSetting = Enable;
        ios_doutSetProcess(&ios_dout_tmp);
        ret = 1;
    }

    /*if (ret > 0) {
        ios_doutSetProcess(&ios_dout_tmp);
    }*/

    return ret;
}

int ios_Control_Dout_Handler_Dual(char *selectmode, uint8_t Enable, uint8_t CameraId) {
    // IOSLOG(0, "[IOS](%s): Control Dout Handler. \n", __func__);
    int ret = -1;
    IOS_DOUT_SET_PROCESS ios_dout_tmp;

    if(CameraId == 0) {
        MAINLOG(0, "[MAINCTL] %s()%d: CameraId=[%d] outPin=[%d] [%s][%s][%s][%s][%s]\n", __FUNCTION__, __LINE__
          , CameraId, ios_doutMode.outPin
          , Process_Node.Dout1selectMode, Process_Node.Dout2selectMode, Process_Node.Dout3selectMode
          , Process_Node.Dout4selectMode, selectmode);
        if (strcmp((char *)Process_Node.Dout1selectMode, (char *)selectmode) == 0) {
            ios_dout_tmp.outPin = DOUT_PIN_1;
            strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node.Dout1Mode);
            ios_dout_tmp.onoffSetting = Enable;
            ios_doutSetProcess(&ios_dout_tmp);
            ret = 1;
        } else if (strcmp((char *)Process_Node.Dout2selectMode, (char *)selectmode) == 0) {
            ios_dout_tmp.outPin = DOUT_PIN_2;
            strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node.Dout2Mode);
            ios_dout_tmp.onoffSetting = Enable;
            ios_doutSetProcess(&ios_dout_tmp);
            ret = 1;
        } else if (strcmp((char *)Process_Node.Dout3selectMode, (char *)selectmode) == 0) {
            ios_dout_tmp.outPin = DOUT_PIN_3;
            strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node.Dout3Mode);
            ios_dout_tmp.onoffSetting = Enable;
            ios_doutSetProcess(&ios_dout_tmp);
            ret = 1;
        } else if (strcmp((char *)Process_Node.Dout4selectMode, (char *)selectmode) == 0) {
            ios_dout_tmp.outPin = DOUT_PIN_4;
            strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node.Dout4Mode);
            ios_dout_tmp.onoffSetting = Enable;
            ios_doutSetProcess(&ios_dout_tmp);
            ret = 1;
        }
    } else {
        MAINLOG(0, "[MAINCTL] %s()%d: CameraId=[%d] outPin=[%d] [%s][%s][%s][%s][%s]\n", __FUNCTION__, __LINE__
          , CameraId, ios_doutMode.outPin
          , Process_Node_Dual.Dout1selectMode, Process_Node_Dual.Dout2selectMode, Process_Node_Dual.Dout3selectMode
          , Process_Node_Dual.Dout4selectMode, selectmode);
        if (strcmp((char *)Process_Node_Dual.Dout1selectMode, (char *)selectmode) == 0) {
            ios_dout_tmp.outPin = DOUT_PIN_1;
            strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node_Dual.Dout1Mode);
            ios_dout_tmp.onoffSetting = Enable;
            ios_doutSetProcess(&ios_dout_tmp);
            ret = 1;
        } else if (strcmp((char *)Process_Node_Dual.Dout2selectMode, (char *)selectmode) == 0) {
            ios_dout_tmp.outPin = DOUT_PIN_2;
            strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node_Dual.Dout2Mode);
            ios_dout_tmp.onoffSetting = Enable;
            ios_doutSetProcess(&ios_dout_tmp);
            ret = 1;
        } else if (strcmp((char *)Process_Node_Dual.Dout3selectMode, (char *)selectmode) == 0) {
            ios_dout_tmp.outPin = DOUT_PIN_3;
            strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node_Dual.Dout3Mode);
            ios_dout_tmp.onoffSetting = Enable;
            ios_doutSetProcess(&ios_dout_tmp);
            ret = 1;
        } else if (strcmp((char *)Process_Node_Dual.Dout4selectMode, (char *)selectmode) == 0) {
            ios_dout_tmp.outPin = DOUT_PIN_4;
            strcpy((char *)ios_dout_tmp.outMode, (char *)Process_Node_Dual.Dout4Mode);
            ios_dout_tmp.onoffSetting = Enable;
            ios_doutSetProcess(&ios_dout_tmp);
            ret = 1;
        }
    }
    /*if (ret > 0) {
        ios_doutSetProcess(&ios_dout_tmp);
    }*/

    return ret;
}

void ios_modbusGITesting(uint8_t GI_Test_Status, uint8_t GI_Testing_Result)
{
  /*  by joe bug  
  mb_mapping->tab_registers[GI_Test_Status_Register] = GI_Test_Status;
  mb_mapping->tab_registers[GI_Testing_Result_Register] = GI_Testing_Result;
  printf("[IOS](%s): GI_Test_Status: %d / GI_Testing_Result: %d \n", __func__,
         mb_mapping->tab_registers[GI_Test_Status_Register],
         mb_mapping->tab_registers[GI_Testing_Result_Register]);*/
}

void ios_modbus_TCP_handler(void)
{
  rdset = refset;
    struct timeval timeout;
    timeout.tv_sec = 0;  // timeout.tv_usec = 100000;
    
    if (select(fdmax+1, &rdset, NULL, NULL, &timeout) == -1) {
        printf("[IOS](%s)%d: modbus Server select() failure.\n", __func__, __LINE__);
    // close_sigint();
    ios_modbus_free();
  }

  for (master_socket = 0; master_socket <= fdmax; master_socket++) {
    if (!FD_ISSET(master_socket, &rdset)) {
      continue;
    }

    if (master_socket == server_socket) {
      socklen_t addrlen;
      struct sockaddr_in clientaddr;
      int newfd;

      addrlen = sizeof(clientaddr);
      memset(&clientaddr, 0, sizeof(clientaddr));
      newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);
      if (newfd == -1) {
          printf("[IOS](%s): modbus Server accept() error", __func__);
      } else {
          FD_SET(newfd, &refset);

          if (newfd > fdmax) {
              fdmax = newfd;
          }
          printf("[IOS](%s): New connection from %s:%d on socket %d\n", __func__,
                  inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
      }
    } else {
      modbus_set_socket(ctx, master_socket);
      rc = modbus_receive(ctx, ios_query);
      if (rc > 0) {
        int header_len = modbus_get_header_length(ctx);
        uint8_t function_code = ios_query[header_len];
        uint16_t start_addr = (ios_query[header_len + 1] << 8) | ios_query[header_len + 2];
        //uint16_t lenght = start_addr + ((ios_query[header_len + 3] << 8) | ios_query[header_len + 4]);

        switch (function_code)
        {
          case MODBUS_FC_READ_HOLDING_REGISTERS:          /* 0x03 */
            modbus_reply(ctx, ios_query, rc, mb_mapping);
            printf("[IOS](%s): Read Registers! \n", __func__);
          break;

          case MODBUS_FC_WRITE_SINGLE_REGISTER:           /* 0x06 */
            if (start_addr == GI_Testing_Register) {
              /* Start GI Testing */
              ////msgQ_iosSend("Din_Trigger");
              innerQ_IOS_EnQ((char *)"Din_Trigger");
            }
            modbus_reply(ctx, ios_query, rc, mb_mapping);
            printf("[IOS](%s): Write Single Register! \n", __func__);
          break;

          default:
            printf("[IOS](%s): Unknown Function Code! \n", __func__);
          break;
        }
      } else if (rc == -1) {
        printf("[IOS](%s): Connection closed on socket %d\n", __func__, master_socket);
        close(master_socket);

        FD_CLR(master_socket, &refset);

        if (master_socket == fdmax) {
          fdmax--;
        }
      }
    }
  }
}

void ios_modbus_RTU_handler(void)
{
  rc = modbus_receive(ctx, ios_query);
  if (rc > 0) {
    int header_len = modbus_get_header_length(ctx);
    uint8_t function_code = ios_query[header_len];
    uint16_t start_addr = (ios_query[header_len + 1] << 8) | ios_query[header_len + 2];
    //uint16_t lenght = start_addr + ((ios_query[header_len + 3] << 8) | ios_query[header_len + 4]);

    switch (function_code)
    {
      case MODBUS_FC_READ_HOLDING_REGISTERS:          /* 0x03 */
        modbus_reply(ctx, ios_query, rc, mb_mapping);
        printf("[IOS](%s): Read Registers! \n", __func__);
      break;

      case MODBUS_FC_WRITE_SINGLE_REGISTER:           /* 0x06 */
        if (start_addr == GI_Testing_Register) {
          /* Start GI Testing */
          ////msgQ_iosSend("Din_Trigger");
          innerQ_IOS_EnQ((char *)"Din_Trigger");

        }
        modbus_reply(ctx, ios_query, rc, mb_mapping);
        printf("[IOS](%s): Write Single Register! \n", __func__);
      break;

      default:
        printf("[IOS](%s): Unknown Function Code! \n", __func__);
      break;
    }
  } else if (rc == -1) {
    printf("[IOS](%s): Connection closed by the client or error\n", __func__);
    /* Connection closed by the client or error */
  }
}

void sig_receiveData_handler(int sig, siginfo_t *info, void *unused)
{
    IOSLOG(0, "[IOS](%s): Receive signal from kernel, signal number: %d \n", __func__, sig);

  if (sig == SIGMCU) {
    if (info->si_int & DIN1_trigger) {
      ////msgQ_iosSend("Din_Trigger");
      innerQ_IOS_EnQ("Din_Trigger");
      IOSLOG(0, "[IOS](%s): Response Status is DIN1_trigger \n", __func__);
    }
    if (info->si_int & DIN2_trigger) {
      IOSLOG(0, "[IOS](%s): Response Status is DIN2_trigger \n", __func__);
    }
    if (info->si_int & DIN3_trigger) {
      IOSLOG(0, "[IOS](%s): Response Status is DIN3_trigger \n", __func__);
    }
    if (info->si_int & DIN4_trigger) {
      IOSLOG(0, "[IOS](%s): Response Status is DIN4_trigger \n", __func__);
    }
    if (info->si_int & Trig1_trigger) {
      IOSLOG(0, "[IOS](%s): Response Status is Trig1_trigger \n", __func__);
    }
    if (info->si_int & Trig2_trigger) {
      IOSLOG(0, "[IOS](%s): Response Status is Trig2_trigger \n", __func__);
    }
    if (info->si_int & LED_Status_trigger) {
      UpdateLEDStatus_Flg = 1;
      IOSLOG(0, "[IOS](%s): Response Status is LED_Status_trigger \n", __func__);
        #if 0
        unsigned int rvalue = 0;
        gpio_get_value(DI_TRIG1, &rvalue);
        if(rvalue == 0) {
            char job_t[512];
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
                , trig_pin[0] + 1, &trig_edge[0][0], rvalue);
            // ext_mqtt_publisher_Dual(job_t, ios_cameraid);
            ext_mqtt_publisher_Dual(job_t, 0);
            ext_mqtt_publisher_Dual(job_t, 1);
            IOSLOG(0, "%d: rvalue [%d]=[%d] trig_dout_pin[4]=[%d]11111111111111\r\n", __LINE__, DI_TRIG1, rvalue, trig_dout_pin[4]);
            // PDM_STREAM_0 SAI5_RXD0
            //spi_gpio_set_value(trig_dout_pin[4], 1);
            IOSLOG(0, "[IOS](%s): by joe Need FrontEnd send value\n", __func__);
            Process_Node.Brightness[0] = 101;
            ios_Control_Light_Handler(LIGHT_SOURCE_1, TRUE);
            Process_Node.autoMode = GLUE_INSPECTION;
            Process_Node.giFlow = GI_EXCUTE_AUTO_GI;
            Process_Node.Active = true;
            // Process_Flow_Handler();
        } else {
            //spi_gpio_set_value(trig_dout_pin[4], 0);
            ios_Control_Light_Handler(LIGHT_SOURCE_1, FALSE);
        }
        #endif
        
    }
    if (info->si_int & Unknown_trigger) {
      IOSLOG(0, "[IOS](%s): Unknown Response Status \n", __func__);
    }
  }
}

void *iosCtl(void *argu)
{
  int result = ios_modbus_init();

  if (result == 0) {
    IOSLOG(0, "[IOS](%s): Modbus Init Finish \n", __func__);
  } else {
    IOSLOG(0, "[IOS](%s): Modbus Init Fail \n", __func__);
  }

  memcpy((void *)wbuf, ios_buf_init, IOS_BUF_SIZE);

  while(1) {
    if(bTearDown == true) {
        IOSLOG(0, "[__%s__] : iosCtl bTearDown=[%d]\n", __func__, bTearDown);
        break;
    }
        
    if (modbus_status) {
      if (FrameFormat == TCP) {
        ios_modbus_TCP_handler();
      } else if (FrameFormat == RTU) {
        ios_modbus_RTU_handler();
      }
    } else {
       usleep(100000);
    }
  }
    return 0;
}

void wdtCtl(void *argu)
{
    IOSLOG(0, "[IOS](%s): wdtCtl Init Finish \n", __func__);

    int flags = 120; // timeout (second)
    int ret = 0;
    char cmd[64];
    snprintf(&cmd[0], sizeof(cmd), "keepalive");
        
    int fd = open("/dev/watchdog", O_WRONLY); 
    if (fd == -1) { 
	    fprintf(stderr, "Watchdog device not enabled.\n"); 
	    fflush(stderr); 
	    return;
    } 
    
    flags = 120;    // timeout (second)
    if((ret = ioctl(fd, WDIOC_SETTIMEOUT, &flags)) != 0) {
        fprintf(stderr, "%s()%d: Watchdog set timeout ret=[%d] flags=[%d] fail.\n", __FUNCTION__, __LINE__, ret, flags);
        return;
    }
    
    flags = WDIOS_ENABLECARD; 
    if((ret = ioctl(fd, WDIOC_SETOPTIONS, &flags)) != 0) {
        fprintf(stderr, "%s()%d: Watchdog set timeout ret=[%d] flags=[%d] fail.\n", __FUNCTION__, __LINE__, ret, flags);
        return;
    }

    while(1) {
        if(bTearDown == true) {
            IOSLOG(0, "[__%s__] : wdtCtl bTearDown=[%d]\n", __func__, bTearDown);
            break;
        }

        if((ret = ioctl(fd, WDIOC_KEEPALIVE, &flags)) != 0) {
            fprintf(stderr, "%s()%d: Watchdog keepalive ret=[%d] flags=[%d] fail.\n", __FUNCTION__, __LINE__, ret, flags);
            return;
        }
        //fprintf(stderr, "Watchdog keepalive flags=[%d].\n", flags); 
        //fflush(stderr); 

        sleep(1);
    }

    flags = WDIOS_DISABLECARD; 
    if((ret = ioctl(fd, WDIOC_SETOPTIONS, &flags)) != 0) {
        fprintf(stderr, "%s()%d: Watchdog set timeout ret=[%d] flags=[%d] fail.\n", __FUNCTION__, __LINE__, ret, flags);
        return;
    }
    
    if(fd) {
        close(fd);
    }
    return;
}

void *ioCtl(void *argu)
{
    IOSLOG(0, "[IOS](%s): ioCtl Init Finish \n", __func__);
    //int count = 0;
    
    auto iosDO1_start = std::chrono::high_resolution_clock::now();
    auto iosDO2_start = std::chrono::high_resolution_clock::now();
    auto iosDO3_start = std::chrono::high_resolution_clock::now();
    auto iosDO4_start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
        
    std::chrono::duration<double, std::milli> duration(0);
    std::string strtmp;

    iosDO1_start = std::chrono::high_resolution_clock::now();
        
    // # cycletime_end <<
    end = std::chrono::high_resolution_clock::now();
    duration = end - iosDO1_start;
    strtmp = std::to_string(duration.count());
    IOSLOG(0, RED "[__%s__] CycleTime : %s (ms)\n", "01", strtmp.c_str());

    ios_setStatusLed(LED1_PWR, LED_GREEN);  

    while(1) {
        if(bTearDown == true) {
            break;
        }
    /*int value = 0;
    int level = spi_gpio_di_get_value(0, &value);
    IOSLOG(0, RED "level=[%d]\r\n", level);
    usleep(1000000);*/
        /*{
            int iosLED_blink = 0;
            int iosLED_ret0 = iosLED_val0;
            int iosLED_ret1 = iosLED_val1;
            iosLED_blink = iosLED_ret1 << 8 | iosLED_ret0;

            if((count++ / 5) % 2 == 0) {
                iosLED_ret0 &= ~(iosLED_blink_bit);
                iosLED_ret1 &= ~(iosLED_blink_bit >> 8);
            } else if((count / 5) % 2 == 1) {
                iosLED_ret0 |= (iosLED_blink_bit);
                iosLED_ret1 |= (iosLED_blink_bit >> 8);
            }
            
            iosLED_i2cset_blink(iosLED_ret0 & 0xff, iosLED_ret1);
        }
        {
            if(pter_hdl_GigE == nullptr) {
                //IOSLOG(0, "[IOS](%s)%d: pter_hdl_GigE is null\n", __func__, __LINE__);
                iosLED_i2cset(LED1_VB_CAM1_CON_R, 1);
                iosLED_i2cset(LED1_VB_CAM1_CON_G, 0);
                iosLED_i2cset(LED2_VB_CAM2_CON_R, 1);
                iosLED_i2cset(LED2_VB_CAM2_CON_G, 0);
            } else {
                //IOSLOG(0, "[IOS](%s)%d: pter_hdl_GigE not null\n", __func__, __LINE__);
                iosLED_i2cset(LED1_VB_CAM1_CON_R, 0);
                iosLED_i2cset(LED1_VB_CAM1_CON_G, 1);
                iosLED_i2cset(LED2_VB_CAM2_CON_R, 0);
                iosLED_i2cset(LED2_VB_CAM2_CON_G, 1);
            }
        }*/
        
        if(iosDO1_blink_time > 0) {
            end = std::chrono::high_resolution_clock::now();
            duration = end - iosDO1_start;
            strtmp = std::to_string(duration.count());
            // IOSLOG(0, RED "[__%s__] CycleTime : %s (ms)\n", "01", strtmp.c_str());
            if(duration.count() >= iosDO1_blink_time) {
                iosDO1_blink_time = 0;
                unsigned int di_val = 0;
                spi_gpio_do_get_value(0, &di_val);
                spi_gpio_set_value(0, ~(di_val) & 0x01);
            }
        } else {
            iosDO1_start = std::chrono::high_resolution_clock::now();
        }
        
        if(iosDO2_blink_time > 0) {
            end = std::chrono::high_resolution_clock::now();
            duration = end - iosDO2_start;
            strtmp = std::to_string(duration.count());
            if(duration.count() >= iosDO2_blink_time) {
                iosDO2_blink_time = 0;
                unsigned int di_val = 0;
                spi_gpio_do_get_value(1, &di_val);
                spi_gpio_set_value(1, ~(di_val) & 0x01);
            }
        } else {
            iosDO2_start = std::chrono::high_resolution_clock::now();
        }
        
        
        if(iosDO3_blink_time > 0) {
            end = std::chrono::high_resolution_clock::now();
            duration = end - iosDO3_start;
            strtmp = std::to_string(duration.count());
            if(duration.count() >= iosDO3_blink_time) {
                iosDO3_blink_time = 0;
                unsigned int di_val = 0;
                spi_gpio_do_get_value(2, &di_val);
                spi_gpio_set_value(2, ~(di_val) & 0x01);
            }
        } else {
            iosDO3_start = std::chrono::high_resolution_clock::now();
        }
        
        if(iosDO4_blink_time > 0) {
            end = std::chrono::high_resolution_clock::now();
            duration = end - iosDO4_start;
            strtmp = std::to_string(duration.count());
            if(duration.count() >= iosDO4_blink_time) {
                iosDO4_blink_time = 0;
                unsigned int di_val = 0;
                spi_gpio_do_get_value(3, &di_val);
                spi_gpio_set_value(3, ~(di_val) & 0x01);
            }
        } else {
            iosDO4_start = std::chrono::high_resolution_clock::now();
        }

        usleep(1000);
    }

    ios_setStatusLed(LED1_PWR, LED_OFF);
    ios_setStatusLed(LED2_STAT, LED_OFF);
    ios_setStatusLed(LED3_COM, LED_OFF);
    ios_setStatusLed(LED4_TRIG, LED_OFF);
    ios_setStatusLed(LED5_ERR, LED_OFF);
    
    if(iosLED_file > 0) {
        close(iosLED_file);
    }
    return 0;
}

void sfcCtl_init()
{
    // Open the serial device
    sfcCtl_serial_port = open("/dev/ttyUSB0", O_RDWR);

    // Check whether the serial port is opened successfully
    if (sfcCtl_serial_port < 0) {
        perror("Error opening serial port");
        return;
    }

    // Get the current serial port configuration
    struct termios tty;
    if (tcgetattr(sfcCtl_serial_port, &tty) != 0) {
        perror("Error getting serial port attributes");
        close(sfcCtl_serial_port);
        return;
    }

    // Setting the baud rate
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Set 8-bit characters, no parity, and one stop bit
    tty.c_cflag &= ~PARENB; // No verification
    tty.c_cflag &= ~CSTOPB; // One stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;     // 8-bit characters

    // Set to local connection, no control flow
    tty.c_cflag |= CLOCAL;
    tty.c_cflag |= CREAD;

    // Set the raw input mode
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;   // Disable echo
    tty.c_lflag &= ~ECHOE;  // Disable Echo Erase
    tty.c_lflag &= ~ISIG;   // Prohibition signal

    // Set the raw output mode
    tty.c_oflag &= ~OPOST;

    // Setting the read timeout
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;   // Read timeout, in units of 100ms

    // Application Configuration
    if (tcsetattr(sfcCtl_serial_port, TCSANOW, &tty) != 0) {
        perror("Error setting serial port attributes");
        close(sfcCtl_serial_port);
        return;
    }

}

int ios_sfc_send_msg(IOS_SFC_SET_MODE *ios_sfc)
{
    // Send data to the serial port
    const char *msg = ios_sfc->msg;
    write(sfcCtl_serial_port, msg, strlen(msg));

    // Read data from the serial port
    char read_buf[256];
    memset(read_buf, 0, sizeof(read_buf));
    int num_bytes = read(sfcCtl_serial_port, read_buf, sizeof(read_buf));

    // Check if the read was successful
    if (num_bytes < 0) {
        perror("Error reading from serial port");
        strcpy(ios_sfc->msg, "Error reading from serial port");
        close(sfcCtl_serial_port);
        return 1;
    }

    // Print the read data
    printf("Read %d bytes: %s\n", num_bytes, read_buf);
    strcpy(ios_sfc->msg, read_buf);

    return 0;
}


#define MICRO_WAIT 200000

#define VL53L1_MAX_I2C_XFER_SIZE 256

#define MSG_START "VL53L1X sensor detected\n"
#define MSG_OK "ok\n"
#define MSG_UNKNOWN_CMD "Unknown command\n"
#define MSG_WRONG_VALUE "Warning: Wrong value sent\n"
#define INPUT_BUFFER_SIZE 512

#define ST_TOF_IOCTL_WFI 1

uint16_t tof_Dev;

int tof_init(void)
{
    int status;
	int adapter_nr = 1;
	int file = 0;
	
	uint8_t byteData, sensorState = 0;
	uint16_t wordData;
	
	uint8_t first_range = 1;
	uint8_t I2cDevAddr = 0x29;

	IOSLOG(0, "[IOS](%s): I2C Bus number is %d\n", __func__, adapter_nr);

	file = VL53L1X_UltraLite_Linux_I2C_Init(tof_Dev, adapter_nr, I2cDevAddr);
	if (file == -1) {
	    IOSLOG(0, "[IOS](%s): Error: file is %d fail.\n", __func__, adapter_nr);
		return -1;
	}

	status = VL53L1_RdByte(tof_Dev, 0x010F, &byteData);
	IOSLOG(0, "VL53L1X Model_ID: %X\n", byteData);
	status += VL53L1_RdByte(tof_Dev, 0x0110, &byteData);
	IOSLOG(0, "VL53L1X Module_Type: %X\n", byteData);
	status += VL53L1_RdWord(tof_Dev, 0x010F, &wordData);
	IOSLOG(0, "VL53L1X: %X\n", wordData);
  // ??
	// while (sensorState == 0) {
	// 	status += VL53L1X_BootState(tof_Dev, &sensorState);
	// 	VL53L1_WaitMs(tof_Dev, 2);
	// }
	IOSLOG(0, "Chip booted\n");

  // ??
	// status = VL53L1X_SensorInit(tof_Dev);
	/* status += VL53L1X_SetInterruptPolarity(tof_Dev, 0); */

  // ??
	// status += VL53L1X_SetDistanceMode(tof_Dev, 2); /* 1=short, 2=long */
	// status += VL53L1X_SetTimingBudgetInMs(tof_Dev, 100);
	// status += VL53L1X_SetInterMeasurementInMs(tof_Dev, 100);
	// status += VL53L1X_StartRanging(tof_Dev);


}

int tofReadDistance(void)
{
    uint8_t first_range = 1;
    VL53L1X_Result_t Results;
    int status = 0;

    // ??
    /* Get the data the new way */
    // status += VL53L1X_GetResult(tof_Dev, &Results);

    IOSLOG(0, "Status = %2d, dist = %5d, Ambient = %2d, Signal = %5d, #ofSpads = %5d\n",
			Results.Status, Results.Distance, Results.Ambient, Results.SigPerSPAD, Results.NumSPADs);

    // ??
    /* trigger next ranging */
    // status += VL53L1X_ClearInterrupt(tof_Dev);
    // if (first_range) {
    // 	/* very first measurement shall be ignored
    // 	 * thus requires twice call
    // 	 */
    // 	status += VL53L1X_ClearInterrupt(tof_Dev);
    // 	first_range = 0;
    // }
    
    return Results.Distance;
}

int iosPWM_init(int channel_num)
{
    int ret = 0;

    if ((ret = pwm_export(channel_num)) != 0) {
        //printf("[IOS](%s)%d: pwm_export ret=[%d] fail.\n", __func__, __LINE__, ret);
        xlog("%s:%d, pwm_export fail \n\r", __func__, __LINE__);
        return -1;
    }
    
    if ((ret = pwm_write_enable(channel_num, 0)) != 0) {
        // printf("[IOS](%s)%d: pwm_write_enable ret=[%d] fail.\n", __func__, __LINE__, ret);
        xlog("%s:%d, pwm_write_enable fail \n\r", __func__, __LINE__);
        return -1;
    }
    
    if ((ret = pwm_write_period(channel_num, AILED_MAX_LEVEL)) != 0) {
        // printf("[IOS](%s)%d: pwm_export ret=[%d] fail.\n", __func__, __LINE__, ret);
        xlog("%s:%d, pwm_write_period fail \n\r", __func__, __LINE__);
        return -1;
    }

    if ((ret = pwm_write_duty_cycle(channel_num, 0)) != 0) {
        // printf("[IOS](%s)%d: pwm_write_duty_cycle ret=[%d] fail.\n", __func__, __LINE__, ret);
        xlog("%s:%d, pwm_write_duty_cycle fail \n\r", __func__, __LINE__);
        return -1;
    }

    if ((ret = pwm_write_polarity(channel_num, (char *)"normal")) != 0) {
        // printf("[IOS](%s)%d: pwm_write_polarity ret=[%d] fail.\n", __func__, __LINE__, ret);
        xlog("%s:%d, pwm_write_polarity fail \n\r", __func__, __LINE__);
        return -1;
    }

    if ((ret = pwm_write_enable(channel_num, 1)) != 0) {
        // printf("[IOS](%s)%d: pwm_write_enable ret=[%d] fail.\n", __func__, __LINE__, ret);
        xlog("%s:%d, pwm_write_enable fail \n\r", __func__, __LINE__);
        return -1;
    }

    return ret;
}

int iosGPIO_init() {

  xlog("%s:%d \n\r", __func__, __LINE__);
  {  // DI
    gpio_export(DI1_VB);
    gpio_export(DI1_VB);
    gpio_export(DI3_VB);
    gpio_export(DI4_VB);
    gpio_export(DI_TRIG1);
    gpio_export(DI_TRIG2);

    gpio_set_dir(DI1_VB, 0);
    gpio_set_dir(DI1_VB, 0);
    gpio_set_dir(DI3_VB, 0);
    gpio_set_dir(DI4_VB, 0);
    gpio_set_dir(DI_TRIG1, 1);
    gpio_set_dir(DI_TRIG2, 1);

    gpio_set_edge(DI1_VB, (char *)"rising");
    gpio_set_edge(DI1_VB, (char *)"rising");
    gpio_set_edge(DI3_VB, (char *)"rising");
    gpio_set_edge(DI4_VB, (char *)"rising");
    gpio_set_edge(DI_TRIG1, (char *)"falling");
    gpio_set_edge(DI_TRIG2, (char *)"falling");
  }
  {  // DO
    gpio_export(DO1_VB);
    gpio_export(DO2_VB);
    gpio_export(DO3_VB);
    gpio_export(DO4_VB);

    gpio_set_dir(DO1_VB, 1);
    gpio_set_dir(DO2_VB, 1);
    gpio_set_dir(DO3_VB, 1);
    gpio_set_dir(DO4_VB, 1);
  }
  // AILighting GPIO Enable
  {
    gpio_export(AILED_GPIO1);
    gpio_export(AILED_GPIO2);
    gpio_export(AILED_GPIO3);
    gpio_export(AILED_GPIO4);
    gpio_export(AILED_DETECT);

    gpio_set_dir(AILED_GPIO1, 1);
    gpio_set_dir(AILED_GPIO2, 1);
    gpio_set_dir(AILED_GPIO3, 1);
    gpio_set_dir(AILED_GPIO4, 1);
    gpio_set_dir(AILED_DETECT, 0);

    gpio_set_value(AILED_GPIO1, 1);
    gpio_set_value(AILED_GPIO2, 1);
    gpio_set_value(AILED_GPIO3, 1);
    gpio_set_value(AILED_GPIO4, 1);
  }

  IOSLOG(0, "[IOS](%s): GPIO Init Finish \n", __func__);
  return 0;
}

void *trigCtl(void *argu)
{
    #define POLL_TIMEOUT (1) /* 100 milliseconds */
    #define MAX_BUF 64

    int nfds = 6;
    struct pollfd fdset[nfds];
    int gpio_fd[nfds], rc;
    char *buf[MAX_BUF];
    unsigned int rvalue = 0;
    char trig_pin_old[4];
    char trig_edge_old[6][16];

    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration(0);
    std::string strtmp;

    trig_pin[0] = DI1_VB;
    trig_pin[1] = DI2_VB;
    trig_pin[2] = DI3_VB;
    trig_pin[3] = DI4_VB;
    trig_pin[4] = DI_TRIG1;
    trig_pin[5] = DI_TRIG2;
    snprintf(&trig_edge[0][0], sizeof(&trig_edge[0][0]), "");
    snprintf(&trig_edge[1][0], sizeof(&trig_edge[1][0]), "");
    snprintf(&trig_edge[2][0], sizeof(&trig_edge[2][0]), "");
    snprintf(&trig_edge[3][0], sizeof(&trig_edge[3][0]), "");
    snprintf(&trig_edge[4][0], sizeof(&trig_edge[4][0]), "");
    snprintf(&trig_edge[5][0], sizeof(&trig_edge[5][0]), "");

    trig_dout_pin[0] = DO1_VB;
    trig_dout_pin[1] = DO2_VB;
    trig_dout_pin[2] = DO3_VB;
    trig_dout_pin[3] = DO4_VB;
    trig_dout_pin[4] = DO1_VB;
    trig_dout_pin[5] = DO2_VB;
    snprintf(&trig_dout_active[0][0], (int)sizeof(&trig_dout_active[0][0]), "High");
    snprintf(&trig_dout_active[1][0], (int)sizeof(&trig_dout_active[1][0]), "High");
    snprintf(&trig_dout_active[2][0], (int)sizeof(&trig_dout_active[2][0]), "High");
    snprintf(&trig_dout_active[3][0], (int)sizeof(&trig_dout_active[3][0]), "High");
    snprintf(&trig_dout_active[4][0], (int)sizeof(&trig_dout_active[4][0]), "Low");
    snprintf(&trig_dout_active[5][0], (int)sizeof(&trig_dout_active[5][0]), "Low");

TRIG_RESET:

    memcpy(&trig_pin_old[0], &trig_pin[0], sizeof(sizeof(char) * nfds));
    sprintf(&trig_edge_old[0][0], "%s", &trig_edge[0][0]);
    sprintf(&trig_edge_old[1][0], "%s", &trig_edge[1][0]);
    sprintf(&trig_edge_old[2][0], "%s", &trig_edge[2][0]);
    sprintf(&trig_edge_old[3][0], "%s", &trig_edge[3][0]);
    sprintf(&trig_edge_old[4][0], "%s", &trig_edge[4][0]);
    sprintf(&trig_edge_old[5][0], "%s", &trig_edge[5][0]);
    
    gpio_export(DI1_VB);
    gpio_export(DI2_VB);
    gpio_export(DI3_VB);
    gpio_export(DI4_VB);
    gpio_export(DI_TRIG1);
    gpio_export(DI_TRIG2);

    gpio_set_dir(DI1_VB, 0);
    gpio_set_dir(DI2_VB, 0);
    gpio_set_dir(DI3_VB, 0);
    gpio_set_dir(DI4_VB, 0);
    gpio_set_dir(DI_TRIG1, 1);
    gpio_set_dir(DI_TRIG2, 1);

    gpio_set_edge(DI1_VB, &trig_edge[0][0]);  // rising / falling
    gpio_set_edge(DI2_VB, &trig_edge[1][0]);  // rising / falling
    gpio_set_edge(DI3_VB, &trig_edge[2][0]);  // rising / falling
    gpio_set_edge(DI4_VB, &trig_edge[3][0]);  // rising / falling
    gpio_set_edge(DI_TRIG1, &trig_edge[4][0]);  // rising / falling
    gpio_set_edge(DI_TRIG2, &trig_edge[5][0]);  // rising / falling
    
    gpio_fd[0] = gpio_fd_open(DI1_VB);
    gpio_fd[1] = gpio_fd_open(DI2_VB);
    gpio_fd[2] = gpio_fd_open(DI3_VB);
    gpio_fd[3] = gpio_fd_open(DI4_VB);
    gpio_fd[4] = gpio_fd_open(DI_TRIG1);
    gpio_fd[5] = gpio_fd_open(DI_TRIG2);

    fdset[0].fd = gpio_fd[0];
    fdset[0].events = POLLPRI;
    fdset[1].fd = gpio_fd[1];
    fdset[1].events = POLLPRI;
    fdset[2].fd = gpio_fd[2];
    fdset[2].events = POLLPRI;
    fdset[3].fd = gpio_fd[3];
    fdset[3].events = POLLPRI;
    fdset[4].fd = gpio_fd[4];
    fdset[4].events = POLLPRI;
    fdset[5].fd = gpio_fd[5];
    fdset[5].events = POLLPRI;
    char job_t[1024] = {'\0'};
    int debounce_counter = 0;
    int debounce_pin = 0;

    while(1) {
        if(bTearDown == true) {
            IOSLOG(0, "[__%s__] : iosCtl bTearDown=[%d]\n", __func__, bTearDown);
            break;
        }
        
        if(     strcmp(&trig_edge_old[0][0], &trig_edge[0][0])
            ||  strcmp(&trig_edge_old[1][0], &trig_edge[1][0])
            ||  strcmp(&trig_edge_old[2][0], &trig_edge[2][0])
            ||  strcmp(&trig_edge_old[3][0], &trig_edge[3][0])
            //||  strcmp(&trig_edge_old[4][0], &trig_edge[4][0])  // by joe issue
            //||  strcmp(&trig_edge_old[5][0], &trig_edge[5][0])
          ) {
            //fprintf(stderr, "%d: trig_pin_old=[%d] trig_pin=[%d] \r\n", __LINE__, trig_pin_old, trig_pin);
            gpio_fd_close(gpio_fd[0]); close(gpio_fd[0]);
            gpio_fd_close(gpio_fd[1]); close(gpio_fd[1]);
            gpio_fd_close(gpio_fd[2]); close(gpio_fd[2]);
            gpio_fd_close(gpio_fd[3]); close(gpio_fd[3]);
            gpio_fd_close(gpio_fd[4]); close(gpio_fd[4]);
            gpio_fd_close(gpio_fd[5]); close(gpio_fd[5]);
            goto TRIG_RESET;
        }

        rc = poll(fdset, nfds, POLL_TIMEOUT);
        //printf("rc=%d\r\n", rc);
        if (rc < 0) {
          fprintf(stderr, "poll() failed!\r\n");
          break;
        }
        //if (rc == 0) {
        //  fprintf(stderr, ".");
        //}
#if 0
        // # cycletime_end <<
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        if(duration.count() < 50.0) {
            IOSLOG(0, "Debunce %f (ms)  \n", duration.count());
        } else if (fdset[0].revents & POLLPRI) {
            gpio_get_value(DI1_VB, &rvalue);
            read(fdset[0].fd, buf, MAX_BUF);
            fprintf(stderr, "%d: poll() trig_edge=[%s] rvalue=[%d] \r\n\n", __LINE__, &trig_edge[0][0], rvalue);
            char job_t[1024] = {'\0'};
            
            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[0] + 1, &trig_edge[0][0], rvalue);
            if(!strncasecmp(&trig_edge[0][0], "rising", strlen("rising")) && rvalue == 1) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI1_VB, rvalue);
                doTrigger = true;
            } else if(!strncasecmp(&trig_edge[0][0], "falling", strlen("falling")) && rvalue == 0) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI1_VB, rvalue);
                doTrigger = true;
            }   

            if(doTrigger == true) {
                UpdateLEDStatus_Flg = 1;
                if(trig_DinMode == 1) {
                    trig_trigger1 = 1;
                    ext_mqtt_publisher_Dual(job_t, 0);
                    usleep(10000);
                    innerQ_IOS_EnQ("Din_Trigger");
                }
                
                if(trig_DinMode_Dual == 1) {
                    trig_trigger2 = 1;
                    ext_mqtt_publisher_Dual(job_t, 1);
                    usleep(100000);
                    innerQ_IOS_EnQ("Din_Trigger_Dual");
                }
            }
        } else if (fdset[1].revents & POLLPRI) {
            gpio_get_value(DI2_VB, &rvalue);
            read(fdset[1].fd, buf, MAX_BUF);
            fprintf(stderr, "%d: poll() trig_edge=[%s] rvalue=[%d] \r\n\n", __LINE__, &trig_edge[1][0], rvalue);
            char job_t[1024] = {'\0'};

            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[1] + 1, &trig_edge[1][0], rvalue);
            if(!strncasecmp(&trig_edge[1][0], "rising", strlen("rising")) && rvalue == 1) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI2_VB, rvalue);
                doTrigger = true;
            } else if(!strncasecmp(&trig_edge[1][0], "falling", strlen("falling")) && rvalue == 0) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI2_VB, rvalue);
                doTrigger = true;
            }
            if(doTrigger == true) {
                UpdateLEDStatus_Flg = 1;
                if(trig_DinMode == 2) {
                    trig_trigger1 = 1;
                    ext_mqtt_publisher_Dual(job_t, 0);
                    usleep(10000);
                    innerQ_IOS_EnQ("Din_Trigger");
                }
                
                if(trig_DinMode_Dual == 2) {
                    trig_trigger2 =1;
                    ext_mqtt_publisher_Dual(job_t, 1);
                    usleep(100000);
                    innerQ_IOS_EnQ("Din_Trigger_Dual");
                }
            }
        } else if (fdset[2].revents & POLLPRI) {
            gpio_get_value(DI3_VB, &rvalue);
            read(fdset[2].fd, buf, MAX_BUF);
            fprintf(stderr, "%d: poll() trig_edge=[%s] rvalue=[%d] \r\n\n", __LINE__, &trig_edge[2][0], rvalue);
            char job_t[1024] = {'\0'};

            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[2] + 1, &trig_edge[2][0], rvalue);
            if(!strncasecmp(&trig_edge[2][0], "rising", strlen("rising")) && rvalue == 1) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI3_VB, rvalue);
                doTrigger = true;
            } else if(!strncasecmp(&trig_edge[2][0], "falling", strlen("falling")) && rvalue == 0) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI3_VB, rvalue);
                doTrigger = true;
            }
            if(doTrigger == true) {
                UpdateLEDStatus_Flg = 1;
                if(trig_DinMode == 3) {
                    trig_trigger1 = 1;
                    ext_mqtt_publisher_Dual(job_t, 0);
                    usleep(10000);
                    innerQ_IOS_EnQ("Din_Trigger");
                }
                
                if(trig_DinMode_Dual == 3) {
                    trig_trigger2 = 1;
                    ext_mqtt_publisher_Dual(job_t, 1);
                    usleep(100000);
                    innerQ_IOS_EnQ("Din_Trigger_Dual");
                }
            }
        } else if (fdset[3].revents & POLLPRI) {
            gpio_get_value(DI4_VB, &rvalue);
            read(fdset[3].fd, buf, MAX_BUF);
            fprintf(stderr, "%d: poll() trig_edge=[%s] rvalue=[%d] \r\n\n", __LINE__, &trig_edge[3][0], rvalue);
            char job_t[1024] = {'\0'};

            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[3] + 1, &trig_edge[3][0], rvalue);
            if(!strncasecmp(&trig_edge[3][0], "rising", strlen("rising")) && rvalue == 1) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI4_VB, rvalue);
                doTrigger = true;
            } else if(!strncasecmp(&trig_edge[3][0], "falling", strlen("falling")) && rvalue == 0) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI4_VB, rvalue);
                doTrigger = true;
            }
            if(doTrigger == true) {
                UpdateLEDStatus_Flg = 1;
                if(trig_DinMode == 4) {
                    trig_trigger1 = 1;
                    ext_mqtt_publisher_Dual(job_t, 0);
                    usleep(10000);
                    innerQ_IOS_EnQ("Din_Trigger");
                }
                
                if(trig_DinMode_Dual == 4) {
                    trig_trigger2 = 1;
                    ext_mqtt_publisher_Dual(job_t, 1);
                    usleep(100000);
                    innerQ_IOS_EnQ("Din_Trigger_Dual");
                }
            }
        } else if (fdset[4].revents & POLLPRI) {
            gpio_get_value(DI_TRIG1, &rvalue);
            read(fdset[4].fd, buf, MAX_BUF);
            fprintf(stderr, "%d: poll() trig_edge=[%s] rvalue=[%d] \r\n\n", __LINE__, &trig_edge[4][0], rvalue);
            char job_t[1024] = {'\0'};

            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[4] + 1, &trig_edge[4][0], rvalue);
            if(!strncasecmp(&trig_edge[4][0], "rising", strlen("rising")) && rvalue == 1) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI_TRIG1, rvalue);
                doTrigger = true;
            } else if(!strncasecmp(&trig_edge[4][0], "falling", strlen("falling")) && rvalue == 0) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI_TRIG1, rvalue);
                doTrigger = true;
            }
            if(doTrigger == true) {
                UpdateLEDStatus_Flg = 1;
                if(trig_DinMode == 5) {
                    trig_trigger1 = 1;
                    ext_mqtt_publisher_Dual(job_t, 0);
                    usleep(10000);
                    innerQ_IOS_EnQ("Din_Trigger");
                }
                
                if(trig_DinMode_Dual == 5) {
                    trig_trigger2 = 1;
                    ext_mqtt_publisher_Dual(job_t, 1);
                    usleep(100000);
                    innerQ_IOS_EnQ("Din_Trigger_Dual");
                }
            }
        } else if (fdset[5].revents & POLLPRI) {
            gpio_get_value(DI_TRIG2, &rvalue);
            read(fdset[5].fd, buf, MAX_BUF);
            fprintf(stderr, "%d: poll() trig_edge=[%s] rvalue=[%d] \r\n\n", __LINE__, &trig_edge[5][0], rvalue);
            char job_t[1024] = {'\0'};

            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[5] + 1, &trig_edge[5][0], rvalue);
            if(!strncasecmp(&trig_edge[5][0], "rising", strlen("rising")) && rvalue == 1) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI_TRIG1, rvalue);
                doTrigger = true;
            } else if(!strncasecmp(&trig_edge[5][0], "falling", strlen("falling")) && rvalue == 0) {
                fprintf(stderr, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI_TRIG1, rvalue);
                doTrigger = true;
            }
            if(doTrigger == true) {
                UpdateLEDStatus_Flg = 1;
                if(trig_DinMode == 6) {
                    trig_trigger1 = 1;
                    ext_mqtt_publisher_Dual(job_t, 0);
                    usleep(10000);
                    innerQ_IOS_EnQ("Din_Trigger");
                }
                
                if(trig_DinMode_Dual == 6) {
                    trig_trigger2 = 1;
                    ext_mqtt_publisher_Dual(job_t, 1);
                    usleep(100000);
                    innerQ_IOS_EnQ("Din_Trigger_Dual");
                }
            }
        }

        if(doTrigger == true) {
            doTrigger = false;
            start = std::chrono::high_resolution_clock::now();
        }

        fflush(stdout);
        usleep(1000);
#else
        if (fdset[0].revents & POLLPRI) {
            gpio_get_value(DI1_VB, &rvalue);
            read(fdset[0].fd, buf, MAX_BUF);
            IOSLOG(0, "Debunce %f(ms) count=[%d] \n", duration.count(), debounce_counter);
            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
                  , trig_pin[0] + 1, &trig_edge[0][0], rvalue);
            if(!strncasecmp(&trig_edge[0][0], "rising", strlen("rising")) && rvalue == 1) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI1_VB, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
                debounce_pin = DI1_VB;
            } else if(!strncasecmp(&trig_edge[0][0], "falling", strlen("falling")) && rvalue == 0) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI1_VB, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
                debounce_pin = DI1_VB;
            } else {
                debounce_counter--;
            }
        } else if (fdset[1].revents & POLLPRI) {
            gpio_get_value(DI2_VB, &rvalue);
            read(fdset[1].fd, buf, MAX_BUF);
            IOSLOG(0, "Debunce %f(ms) count=[%d] \n", duration.count(), debounce_counter);
            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[1] + 1, &trig_edge[1][0], rvalue);
            if(!strncasecmp(&trig_edge[1][0], "rising", strlen("rising")) && rvalue == 1) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI2_VB, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
                debounce_pin = DI2_VB;
            } else if(!strncasecmp(&trig_edge[1][0], "falling", strlen("falling")) && rvalue == 0) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI2_VB, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
                debounce_pin = DI2_VB;
            } else {
                debounce_counter--;
            }
        } else if (fdset[2].revents & POLLPRI) {
            gpio_get_value(DI3_VB, &rvalue);
            read(fdset[2].fd, buf, MAX_BUF);
            IOSLOG(0, "Debunce %f(ms) count=[%d] \n", duration.count(), debounce_counter);
            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[2] + 1, &trig_edge[2][0], rvalue);
            if(!strncasecmp(&trig_edge[2][0], "rising", strlen("rising")) && rvalue == 1) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI3_VB, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
                debounce_pin = DI3_VB;
            } else if(!strncasecmp(&trig_edge[2][0], "falling", strlen("falling")) && rvalue == 0) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI3_VB, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
                debounce_pin = DI3_VB;
            } else {
                debounce_counter--;
            }
        } else if (fdset[3].revents & POLLPRI) {
            gpio_get_value(DI4_VB, &rvalue);
            read(fdset[3].fd, buf, MAX_BUF);
            IOSLOG(0, "Debunce %f(ms) count=[%d] \n", duration.count(), debounce_counter);
            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[3] + 1, &trig_edge[3][0], rvalue);
            if(!strncasecmp(&trig_edge[3][0], "rising", strlen("rising")) && rvalue == 1) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI4_VB, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
                debounce_pin = DI4_VB;
            } else if(!strncasecmp(&trig_edge[3][0], "falling", strlen("falling")) && rvalue == 0) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI4_VB, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
                debounce_pin = DI4_VB;
            } else {
                debounce_counter--;
            }
        } else if (fdset[4].revents & POLLPRI) {
            debounce_pin = DI_TRIG1;
            gpio_get_value(DI_TRIG1, &rvalue);
            read(fdset[4].fd, buf, MAX_BUF);
            IOSLOG(0, "Debunce %f(ms) count=[%d] \n", duration.count(), debounce_counter);
            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[4] + 1, &trig_edge[4][0], rvalue);
            if(!strncasecmp(&trig_edge[4][0], "rising", strlen("rising")) && rvalue == 1) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI_TRIG1, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
            } else if(!strncasecmp(&trig_edge[4][0], "falling", strlen("falling")) && rvalue == 0) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI_TRIG1, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
            } else {
                debounce_counter--;
            }
        } else if (fdset[5].revents & POLLPRI) {
            debounce_pin = DI_TRIG2;
            gpio_get_value(DI_TRIG2, &rvalue);
            read(fdset[5].fd, buf, MAX_BUF);
            IOSLOG(0, "Debunce %f(ms) count=[%d] \n", duration.count(), debounce_counter);
            memset(&job_t[0], 0, sizeof(job_t));
            snprintf(&job_t[0], sizeof(job_t), "{\"cmd\":\"IO_TRIG_EVENT_SET_RESP\", \"args\":{ \"din_pin\":%d, \"din_edge\":\"%s\", \"din_value\":%d }}"
              , trig_pin[5] + 1, &trig_edge[5][0], rvalue);
            if(!strncasecmp(&trig_edge[5][0], "rising", strlen("rising")) && rvalue == 1) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI_TRIG2, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
            } else if(!strncasecmp(&trig_edge[5][0], "falling", strlen("falling")) && rvalue == 0) {
                IOSLOG(0, "%d: poll() GPIO %d interrupt rvalue: [%d]\r\n\n", __LINE__, DI_TRIG2, rvalue);
                start = std::chrono::high_resolution_clock::now();
                debounce_counter++;
            } else {
                debounce_counter--;
            }
        }

        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        if(duration.count() < 50.0) {
            if(((int)duration.count() % 10) == 0) {
                //IOSLOG(0, "Debunce %f(ms) count=[%d] \n", duration.count(), debounce_counter);
            }
        } else if(duration.count() >= 51.0 && duration.count() <= 52.0 && debounce_counter >= -2 && debounce_counter <= 2) {
            IOSLOG(0, "Debunce %f(ms) count=[%d] \n", duration.count(), debounce_counter);
            if(debounce_pin == DI1_VB) {
                gpio_get_value(DI1_VB, &rvalue);
                read(fdset[0].fd, buf, MAX_BUF);
                if(!strncasecmp(&trig_edge[0][0], "rising", strlen("rising")) && rvalue == 1) {
                    doTrigger = true;
                } else if(!strncasecmp(&trig_edge[0][0], "falling", strlen("falling")) && rvalue == 0) {
                    doTrigger = true;
                }
            } else if(debounce_pin == DI2_VB) {
                gpio_get_value(DI2_VB, &rvalue);
                read(fdset[1].fd, buf, MAX_BUF);
                if(!strncasecmp(&trig_edge[1][0], "rising", strlen("rising")) && rvalue == 1) {
                    doTrigger = true;
                } else if(!strncasecmp(&trig_edge[1][0], "falling", strlen("falling")) && rvalue == 0) {
                    doTrigger = true;
                }
            } else if(debounce_pin == DI3_VB) {
                gpio_get_value(DI3_VB, &rvalue);
                read(fdset[2].fd, buf, MAX_BUF);
                if(!strncasecmp(&trig_edge[2][0], "rising", strlen("rising")) && rvalue == 1) {
                    doTrigger = true;
                } else if(!strncasecmp(&trig_edge[2][0], "falling", strlen("falling")) && rvalue == 0) {
                    doTrigger = true;
                }
            } else if(debounce_pin == DI4_VB) {
                gpio_get_value(DI4_VB, &rvalue);
                read(fdset[3].fd, buf, MAX_BUF);
                if(!strncasecmp(&trig_edge[3][0], "rising", strlen("rising")) && rvalue == 1) {
                    doTrigger = true;
                } else if(!strncasecmp(&trig_edge[3][0], "falling", strlen("falling")) && rvalue == 0) {
                    doTrigger = true;
                }
            } else if(debounce_pin == DI_TRIG1) {
                gpio_get_value(DI_TRIG1, &rvalue);
                read(fdset[4].fd, buf, MAX_BUF);
                if(!strncasecmp(&trig_edge[4][0], "rising", strlen("rising")) && rvalue == 1) {
                    doTrigger = true;
                } else if(!strncasecmp(&trig_edge[4][0], "falling", strlen("falling")) && rvalue == 0) {
                    doTrigger = true;
                }
            } else if(debounce_pin == DI_TRIG2) {
                gpio_get_value(DI_TRIG1, &rvalue);
                read(fdset[5].fd, buf, MAX_BUF);
                if(!strncasecmp(&trig_edge[5][0], "rising", strlen("rising")) && rvalue == 1) {
                    doTrigger = true;
                } else if(!strncasecmp(&trig_edge[5][0], "falling", strlen("falling")) && rvalue == 0) {
                    doTrigger = true;
                }
            }
        } else if(duration.count() >= 1000.0) {
            debounce_counter = 0;
        }
        
        if(doTrigger == true) {
            doTrigger = false;
            debounce_counter = 0;
            UpdateLEDStatus_Flg = 1;
            if(trig_DinMode == 1) 
            {
                trig_trigger1 = 1;
                ext_mqtt_publisher_Dual(job_t, 0);
                usleep(10000);
                innerQ_IOS_EnQ("Din_Trigger");
            }
            
            if(trig_DinMode_Dual == 1) {
                trig_trigger2 = 1;
                ext_mqtt_publisher_Dual(job_t, 1);
                usleep(100000);
                innerQ_IOS_EnQ("Din_Trigger_Dual");
            }
            IOSLOG(0, "\n\n\n\n\n\n");
        }

        fflush(stdout);
        usleep(1000);
#endif
    }

    gpio_fd_close(gpio_fd[0]);
    gpio_fd_close(gpio_fd[1]);
    gpio_fd_close(gpio_fd[2]);
    gpio_fd_close(gpio_fd[3]);
    gpio_fd_close(gpio_fd[4]);
    gpio_fd_close(gpio_fd[5]);
    
    //gpio_unexport(DI1_VB);
    //gpio_unexport(DI2_VB);
    //gpio_unexport(DI3_VB);
    //gpio_unexport(DI4_VB);

    return 0;
}

int IO_SetLocalTime(bool use_ntp, struct tm *new_tm, char *timezone)
{

    int ret = 0;
    {
        FILE *fp;
        char output[1024];
        if (use_ntp == 0)
        {
            fp = popen("sudo timedatectl set-ntp false", "r");
        }
        else
        {
            fp = popen("sudo timedatectl set-ntp true", "r");
        }

        if (fp == NULL)
        {
            perror("popen failed");
            ret = -1;
        }
        while (fgets(output, sizeof(output), fp) != NULL)
        {
            printf("%s", output);
        }

        pclose(fp);
    }

    if (new_tm)
    {
        time_t seconds = mktime(new_tm);
        struct timeval tv;
        tv.tv_sec = seconds;
        tv.tv_usec = 0;

        if (settimeofday(&tv, NULL) == -1)
        {
            //perror("settimeofday");
            {
                char buf[100];

                snprintf(&buf[0], sizeof(buf), "sudo /usr/bin/date -s \"%d-%d-%d %d:%d:%d\"", 1900 + new_tm->tm_year, new_tm->tm_mon, new_tm->tm_mday, new_tm->tm_hour, new_tm->tm_min, new_tm->tm_sec);
                fprintf(stderr, "%s()%d: buf=[%s]\n", __FUNCTION__, __LINE__, buf);
                FILE *fp;
                char output[1024];
                fp = popen(&buf[0], "r");

                if (fp == NULL)
                {
                    perror("popen failed");
                    ret = -1;
                }
                while (fgets(output, sizeof(output), fp) != NULL)
                {
                    printf("%s()%d: fgets=[%s]", __FUNCTION__, __LINE__, output);
                }

                pclose(fp);
            }
        }
    }
    
    char time_zone[128];
    if(timezone) {
        fprintf(stderr, "%s()%d: timezone=[%s]\n", __FUNCTION__, __LINE__, timezone);
        if(!strcmp(&timezone[0], "+0") || !strcmp(&timezone[0], "-0")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Etc/UTC");
        } else if(!strcmp(&timezone[0], "+1")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Europe/Paris");
        } else if(!strcmp(&timezone[0], "+2")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Europe/Athens");
        } else if(!strcmp(&timezone[0], "+3")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Europe/Moscow");
        } else if(!strcmp(&timezone[0], "+4")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Asia/Dubai");
        } else if(!strcmp(&timezone[0], "+5")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Asia/Ashgabat");
        } else if(!strcmp(&timezone[0], "+6")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Asia/Ashgabat");
        } else if(!strcmp(&timezone[0], "+7")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Asia/Singapore");
        } else if(!strcmp(&timezone[0], "+8")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Asia/Taipei");
        } else if(!strcmp(&timezone[0], "+9")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Asia/Tokyo");
        } else if(!strcmp(&timezone[0], "+10")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Asia/Melbourne");
        } else if(!strcmp(&timezone[0], "+11")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Asia/Kamchatka");
        } else if(!strcmp(&timezone[0], "+12")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Asia/Kamchatka");
        } else if(!strcmp(&timezone[0], "-1")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Atlantic/Azores");
        } else if(!strcmp(&timezone[0], "-2")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Brazil/Acre");
        } else if(!strcmp(&timezone[0], "-3")) {
            snprintf(&time_zone[0], sizeof(time_zone), "America/Argentina/La_Rioja");
        } else if(!strcmp(&timezone[0], "-4")) {
            snprintf(&time_zone[0], sizeof(time_zone), "America/Argentina/San_Juan");
        } else if(!strcmp(&timezone[0], "-5")) {
            snprintf(&time_zone[0], sizeof(time_zone), "America/Detroit");
        } else if(!strcmp(&timezone[0], "-6")) {
            snprintf(&time_zone[0], sizeof(time_zone), "America/Chicago");
        } else if(!strcmp(&timezone[0], "-7")) {
            snprintf(&time_zone[0], sizeof(time_zone), "America/Denver");
        } else if(!strcmp(&timezone[0], "-8")) {
            snprintf(&time_zone[0], sizeof(time_zone), "America/Los_Angeles");
        } else if(!strcmp(&timezone[0], "-9")) {
            snprintf(&time_zone[0], sizeof(time_zone), "America/Juneau");
        } else if(!strcmp(&timezone[0], "-10")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Pacific/Honolulu");
        } else if(!strcmp(&timezone[0], "-11")) {
            snprintf(&time_zone[0], sizeof(time_zone), "Indian/Christmas");
        } else if(!strcmp(&timezone[0], "-12")) {
            snprintf(&time_zone[0], sizeof(time_zone), "US/Samoa");
        } else {
            snprintf(&time_zone[0], sizeof(time_zone), "%s", timezone);
        }
    } else {
        snprintf(&time_zone[0], sizeof(time_zone), "Etc/UTC");
    }
    
    {
        char buf[256];
    
        snprintf(&buf[0], sizeof(buf), "sudo timedatectl set-timezone %s", time_zone);
        fprintf(stderr, "%s()%d: buf=[%s]\n", __FUNCTION__, __LINE__, buf);
        FILE *fp;
        char output[1024];
        fp = popen(&buf[0], "r");
    
        if (fp == NULL)
        {
            perror("popen failed");
            ret = -1;
        }
        while (fgets(output, sizeof(output), fp) != NULL)
        {
            printf("%s()%d: fgets=[%s]", __FUNCTION__, __LINE__, output);
        }
    
        pclose(fp);
    }
    return ret;
}

int ios_readVersionFile(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/etc/primax_version", "r");
    if (file == NULL)
    {
        printf("%s()%d: can't open /etc/primax_version\n", __FUNCTION__, __LINE__);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
    
    return 0;
}

int ios_readEthAddr(char *eth, char *jstring)
{
    struct ifaddrs *ifaddr, *ifa;
    char ip[NI_MAXHOST];
    
    if(eth == NULL) {
        printf("%s()%d: eth is null\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) {
            if (strcmp(ifa->ifa_name, eth) == 0) {

                if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), ip, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
                    printf("IP address of eth0: %s\n", ip);
                    strcpy(jstring, ip);
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return 0;
}

int iosCtl_init()
{
    int ret;

    xlog("%s:%d \n\r", __func__, __LINE__);    
    // IOSLOG(0, "*********************************\n");
    // IOSLOG(0, "*****   IOS Controller Init.    *****\n");
    // IOSLOG(0, "*********************************\n");
    
    //pid_t signal_pid = getpid();
    
    struct sigaction sig;
    sigemptyset(&sig.sa_mask);
    sig.sa_sigaction = sig_receiveData_handler;
    sig.sa_flags = SA_SIGINFO | SA_RESTART;
    ret = sigaction(SIGMCU, &sig, nullptr);
    if (ret == -1) {
      printf("[IOS](%s): Failed to caught SIGMCU signal\n", __func__);
    }
    trig_dout_pin[0] = DOUT_PIN_1;
    trig_dout_pin[1] = DOUT_PIN_2;
    trig_dout_pin[2] = DOUT_PIN_3;
    trig_dout_pin[3] = DOUT_PIN_4;
    trig_dout_pin[4] = DOUT_PIN_1;
    trig_dout_pin[5] = DOUT_PIN_2;

    /*ret = pthread_create(&wdtThread, NULL, (void *)&wdtCtl, NULL);
    if (ret < 0){
        printf("[IOS](%s): Create wdtCtl thread fail! \n", __func__);
        return -1;
    }*/

    /* initial vailable */
    iosPWM_init(LIGHTING_PWM1_NUM);
    iosPWM_init(LIGHTING_PWM2_NUM);
    iosGPIO_init();
    iosLED_init();
    sfcCtl_init();
    SPI_Open();
    tof_init();

    ret = pthread_create(&iosThread, NULL, iosCtl, NULL);  	
    if (ret < 0){
      printf("[IOS](%s): Create iosCtl thread fail! \n", __func__);
      return -1;
    }

    ret = pthread_create(&didoThread, NULL, trigCtl, NULL);  	
    if (ret < 0){
        printf("[IOS](%s): Create trigCtl thread fail! \n", __func__);
        return -1;
    }

    ret = pthread_create(&ledThread, NULL, ioCtl, NULL);
    if (ret < 0){
        printf("[IOS](%s): Create ioCtl thread fail! \n", __func__);
        return -1;
    }
    
    ios_setStatusLed(LED3_COM, LED_GREEN);  // Green
    return ret;
}
