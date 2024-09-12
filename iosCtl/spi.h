#ifndef __SPI_TEST_H__
#define __SPI_TEST_H__

/* define the tty color */
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"
static uint32_t DebugLevel = 0;
#define LOG(level, format, b...)     if ( (DebugLevel+1) >= (level+1) )  printf(GREEN format NONE, ##b)

/* related external lighting controller definition */
/* SPI transfer command field*/
enum{
  TYPE            = 0,
  MAUNFACTURER    = 1,
  AI_LIGHT_ENABLE = 1,
  EXT_LIGHT_CMD   = 5,
  EXT_LIGHT_CH    = 6,
  LIGHT_LEVEL     = 8,
  DATA_FIELD      = 8, 
  CHECKSUM        = 9,
  MAX_FORMAT_LEN  = 10
};

/* OPT data field  */
enum{
  ATTRIBUTE =  0,
  MODEL_NUM = 1,
  VERSION_NUM = 2,
};

enum{
  MAX_AI_LIGHT_LEVEL    = 0xFF,
  MAX_MAIN_LIGHT_LEVEL  = 0xFF,
};

enum{
  CH1 = 1,
  CH2,
  CH3,
  CH4
};

typedef enum{
  OPT,  
}__LIGHT_MANUFACTURER__;

/* SPI TYPE */
enum{
  EXT_LIGHT_CONTROL = 0x00,
  EXT_LIGHT_READ    = 0x05
};

/* OPT command */
enum{
  SET_LIGHT_LEVEL       = 0x01,
  READ_LIGHT_ATTRIBUTE  = 0x10,
  READ_LIGHT_LEVEL      = 0x11
};

typedef enum{
  LIGHTING_CONTROL  = 0x00,
  LIGHTING_READ     = 0x05,
  PWM_MAIN_LED      = 0x11,
  PWM_AI_LED        = 0x22,
  GPIO_DO           = 0x23,
  GPIO_DO_READ      = 0x24,
  GPIO_DI_READ      = 0x26,
  GPIO_DIO          = 0x27,
  AILIGHT_DETECT    = 0x28,
  STATUS_LED        = 0x29
  
}__TYPE__;

int SPI_Open(void);
int SPI_Close(void);
int SPI_Transfer(const uint8_t *TxBuf, uint8_t *RxBuf, int len);
int SPI_Write(uint8_t *TxBuf, int len);
int SPI_Read(uint8_t *RxBuf, int len);
int SPI_LookBackTest(void);
int spi_gpio_set_value(unsigned int gpio, unsigned int value);
int spi_gpio_do_get_value(unsigned int gpio, unsigned int *value);
int spi_gpio_di_get_value(unsigned int gpio, unsigned int *value);

/* public function */
int ios_setExtLightLevel(__LIGHT_MANUFACTURER__  manufacturer, uint16_t level, uint8_t channel);
int ios_setAiLightLevel(uint16_t level);
int ios_setAiLightLevel_withEnable(uint16_t levle, uint8_t channel);
uint16_t ios_readExtLightLevel_withChannel(__LIGHT_MANUFACTURER__  manufacturer, uint8_t channel);
uint16_t ios_readExtLightAttribute(__LIGHT_MANUFACTURER__  manufacturer);
int ios_setMainLightLevel(uint16_t level);
int ios_setAiLightLevel_withChannel(uint16_t level, uint8_t channel);
int ios_setExtLightLevel_withChannel(__LIGHT_MANUFACTURER__  manufacturer, uint16_t level, uint8_t channel);
int ios_setStatusLed(uint8_t channel , uint8_t value);
uint16_t readtest(__LIGHT_MANUFACTURER__  manufacturer, uint8_t channel);
#endif