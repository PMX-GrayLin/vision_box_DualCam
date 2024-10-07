/********** MQTT payload definition ********************
 ______________________________________
|_module__|_command 1_|___command 2___|

******************************************************/

#pragma once

/* MCU define */
#define PWM_PERIOD_COUNT                        1200            /* PLL=24MHz, PWM freq=20KHz, 1200 = 24MHz/20KHz */
#define PWM_ONE_STEP                            4.6875          /* 1200 / 256 step, one step = 4.6875 */
#define SIGMCU                                  44

/* Buff size define */
#define IOS_BUF_SIZE                            202
#define IP_SIZE                                 16

/* Modbus Define */
#define RTU_COM_PORT                            "/dev/ttymxc2"
#define RTU_PARITY                              'N'
#define RTU_DATA_BIT                            8
#define RTU_STOP_BIT                            1
#define RTU_SERVER_ID                           1
#define MODBUS_DEBUG_MODE_ENABLE                1

#define IOS_IO_STATUS_INDEX                     2
#define IOS_CAMERA_STATUS_INDEX                 3
#define IOS_LED_STATUS_INDEX                    4
#define IOS_LED_SET_INDEX                       6
#define IOS_LIGHT1_PWM_INDEX                    10
#define IOS_IO_SET_STATUS_INDEX                 14
#define IOS_IO_DEPENDENCY_INDEX                 18
#define IOS_DIN1_OUT_INDEX                      26
#define IOS_TRIG1_OUT_INDEX                     34
#define IOS_TRIG1_DELAY_INDEX                   38
#define IOS_DIN1_DELAY_INDEX                    46
#define IOS_DOUT1_DELAY_INDEX                   78
#define IOS_DOUT4_DELAY_INDEX                   90
#define IOS_LIGHT1_DELAY_INDEX                  102
#define IOS_LED1_OFF_DELAY_INDEX                126
#define IOS_LED1_Blink_DELAY_INDEX              146
#define IOS_OUTPUT_CLOSE_INDEX                  166
#define IOS_MODE_INDEX                          168
#define IOS_MANUAL_INDEX                        170
#define DIN_MODE_INDEX                          172
#define DOUT_MODE_INDEX                         180

#define IOS_DIN1_NPN                            0x01
#define IOS_TRIG1_NPN                           0x01
#define IOS_DOUT1_NPN                           0x10
#define IOS_CAMERA_STATUS                       0x0C
#define IOS_CAMERA1_STATUS                      0x04
#define IOS_CAMERA2_STATUS                      0x08

#define IOS_DOUT1_BIT                           0x01
#define IOS_DOUT2_BIT                           0x02
#define IOS_DOUT3_BIT                           0x04
#define IOS_DOUT4_BIT                           0x08
#define IOS_DIN1_BIT                            0x10
#define IOS_DIN2_BIT                            0x20
#define IOS_DIN3_BIT                            0x40
#define IOS_DIN4_BIT                            0x80
#define IOS_TRIGGER1_BIT                        0x100
#define IOS_TRIGGER2_BIT                        0x200
#define IOS_CAMERA1_BIT                         0x400
#define IOS_CAMERA2_BIT                         0x800


#define LIGHTING_PWM1_NUM           1
#define LIGHTING_PWM2_NUM           2

// VS DI
#define DI1_GPIO1_IO00 0  // GPIO1_IO00 (1-1)*32+0=0
#define DI2_GPIO1_IO01 1  // GPIO1_IO01                  0*32+1=33
#define DI3_GPIO1_IO07 7  // GPIO1_IO07                  0*32+7=7
#define DI4_GPIO3_IO23 87 // MX8MP_IOMUXC_SAI5_RXD2__GPIO3_IO23   gpio3.IO[23]    3*32+23=87

// VS DO
#define DO1_GPIO4_IO16 112 // MX8MP_IOMUXC_SAI1_TXD4__GPIO4_IO16    3*32+16=112   DO1=9  # GPIO1_IO09                  0*32+9=9
#define DO2_GPIO4_IO17 113 // MX8MP_IOMUXC_SAI1_TXD5__GPIO4_IO17    3*32+17=113
#define DO3_GPIO4_IO18 114 // MX8MP_IOMUXC_SAI1_TXD6__GPIO4_IO18    3*32+18=114
#define DO4_GPIO4_IO19 115 // MX8MP_IOMUXC_SAI1_TXD7__GPIO4_IO19    3*32+19=115

// DI VB
#define DI1_VB 0            // GPIO1_IO00                  (1-1)*32+0=0
#define DI2_VB 1            // GPIO1_IO01                  0*32+1=1
#define DI3_VB 7            // GPIO1_IO07                  0*32+7=7
#define DI4_VB 8            // GPIO1_IO08                  0*32+8=8

#define DI_TRIG1  85   // GPIO3_IO21                2*32+21=85
#define DI_TRIG2  86   // GPIO3_IO22                2*32+21=86
        
// DO VB
#define DO1_VB 9            // GPIO1_IO09                0*32+9=9
#define DO2_VB 10           // GPIO1_IO10                0*32+10=10
#define DO3_VB 11           // GPIO1_IO11                0*32+11=11
#define DO4_VB 13           // GPIO1_IO13                0*32+13=13



// AI Lighting GPIO Enable
#define AILED_MAX_LEVEL 49725
#define AILED_STEP_LEVEL 195 // GPIO1_IO13
#define AILED_GPIO1 9        // GPIO1_IO09
#define AILED_GPIO2 10       // VS1=88 SAI5_RXD3 = GPIO3_IO24 // VS2=10     # GPIO1_IO10
#define AILED_GPIO3 11       // GPIO1_IO11
#define AILED_GPIO4 13       // GPIO1_IO13
#define AILED_DETECT 86

#define PCA64_P0 0x02
#define PCA64_P1 0x03

typedef enum {
    LED0_VB_PWR_R,     // PCA64_P0_0   EXT_PWREN1
    LED0_VB_PWR_G,     // PCA64_P0_1   EXT_PWREN2
    LED1_VB_CAM1_CON_R,    // PCA64_P0_2   CAN1/I2C5_SEL
    LED1_VB_CAM1_CON_G,    // PCA64_P0_3   PDM/CAN2_SEL
    LED2_VB_CAM2_CON_R,     // PCA64_P0_4   FAN_EN
    LED2_VB_CAM2_CON_G,     // PCA64_P0_5   PWR_MEAS_IO1
    LED3_VB_USER1_R,    // PCA64_P0_6   EXP_P0_6
    LED3_VB_USER1_G,    // PCA64_P0_7   EXP_P0_7

    LED4_VB_USER2_R,     // PCA64_P1_0   EXP_P0_0
    LED4_VB_USER2_G      // PCA64_P1_1   EXP_P0_1
} LED_PCA64;

typedef enum {
    LED1_PWR = 0,     // PCA64_P0_0   EXT_PWREN1
    LED2_STAT,
    LED3_COM,
    LED4_TRIG,
    LED5_ERR
} AICAMERA_LED_G2;

typedef enum {
    LED_OFF = 0,     // PCA64_P0_0   EXT_PWREN1
    LED_RED,
    LED_GREEN,
    LED_ORG
} AICAMERA_LED_COLOR;

#define IOS_BIT_OPERATION                       0x01

#define IOS_LED1_BIT                            0x0007

enum{
  IOS_NormalMode,             /* = 0, IOS status = Normal Mode */
  IOS_Hardware_Mode,          /* = 1, IOS status = Hardware Trigger Mode*/
  IOS_Auto_Mode,              /* = 2, IOS status = Auto mode */
  IOS_Manual_Mode,            /* = 3, IOS status = Manual Mode */
};

enum{
  DIN1_trigger = 0x0001,
  DIN2_trigger = 0x0002,
  DIN3_trigger = 0x0004,
  DIN4_trigger = 0x0008,
  Trig1_trigger = 0x0010,
  Trig2_trigger = 0x0020,
  LED_Status_trigger = 0x0040,
  Unknown_trigger = 0x8000,
};

volatile char rbuf[IOS_BUF_SIZE];
volatile char wbuf[IOS_BUF_SIZE];

const char ios_buf_init[IOS_BUF_SIZE] = {
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0
};

extern int iosCtl_init();
extern int ios_readVersionFile(char *jstring);
