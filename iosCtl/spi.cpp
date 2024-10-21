#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "spi.h"
#include "global.hpp"

static const char *device = "/dev/spidev0.0";
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 100000;
static uint32_t delay = 100000;
static int g_SPI_Fd = 0;

unsigned char SPI_TX_Data[MAX_FORMAT_LEN] = {};

static void pabort(const char *s)
{
    perror(s);
    abort();
}

int SPI_Transfer(const uint8_t *TxBuf, uint8_t *RxBuf, int len)
{
    int ret;
    int fd = g_SPI_Fd;

    struct spi_ioc_transfer tr ={
        .tx_buf = (unsigned long) TxBuf,
        .rx_buf = (unsigned long) RxBuf,
        .len =len,
        .delay_usecs = delay,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
      xlog("ioctl error");
    } else {
#ifdef DEBUG_SPI
      int i;
      // printf("nsend spi message Succeed\n");
      printf("%s()%d: nSPI Send [Len:%d]: ", __FUNCTION__, __LINE__, len);
      for (i = 0; i < len; i++) {
        // if (i % 8 == 0)
        //     printf("nt\n");
        printf("0x%02X ", TxBuf[i]);
      }
      printf("\r\n");

      printf("%s()%d: SPI Receive [len:%d]:", __FUNCTION__, __LINE__, len);
      for (i = 0; i < len; i++) {
        // if (i % 8 == 0)
        //     printf("nt\n");
        printf("0x%02X ", RxBuf[i]);
      }
      printf("\r\n");
#endif
    }
    return ret;
}


int SPI_Write(uint8_t *TxBuf, int len) {
  int ret;
  int fd = g_SPI_Fd;

  ret = write(fd, TxBuf, len);
  if (ret < 0)
    perror("SPI Write error\n");
  else {
#ifdef DEBUG_SPI
    int i;
    printf("SPI Write [Len:%d]: \n", len);
    for (i = 0; i < len; i++) {
      if (i % 8 == 0)
        printf("\n\t");
      printf("0x%02X \n", TxBuf[i]);
    }
    printf("\n");
#endif
  }

  return ret;
}

int SPI_Read(uint8_t *RxBuf, int len)
{
    int ret;
    int fd = g_SPI_Fd;
    ret = read(fd, RxBuf, len);
    if (ret < 0)
        printf("SPI Read error\n");
    else
    {
#ifdef DEBUG_SPI
        int i;
        printf("%s()%d: SPI Read [len:%d]:", __FUNCTION__, __LINE__, len);
        for (i = 0; i < len; i++)
        {
            //if (i % 8 == 0)
            //    printf("\n\t");
            printf("0x%02X ", RxBuf[i]);
        }
        printf("\n");
#endif
    }
    return ret;
}

int SPI_Open(void) {
  int fd;
  int ret = 0;

  if (g_SPI_Fd != 0) {
    xlog("spi already been opened");
    return 0xF1;
  }
  fd = open(device, O_RDWR);
  if (fd < 0) {
    xlog("open fail");
    // pabort("can't open device\n");
  } else {
    xlog("open succeed. Start Init SPI %s", device);
  }

  g_SPI_Fd = fd;
  /*
   * spi mode
   */
  ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
  if (ret == -1) {
    xlog("ioctl fail, can't set spi mode");
    // pabort("can't set spi mode\n");
  }

  ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
  if (ret == -1) {
    xlog("ioctl fail, can't get spi mode");
    pabort("can't get spi mode\n");
  }

  /*
   * bits per word
   */
  ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't set bits per word\n");

  ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't get bits per word\n");

  /*
   * max speed hz
   */
  ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (ret == -1) {
    xlog("can't set max speed");
    // pabort("can't set max speed hz\n");
  }

  ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
  if (ret == -1) {
    xlog("can't get max speed");
    // pabort("can't get max speed hz\n");
  }

  xlog("spi mode:%d", mode);
  xlog("bits per word:%d", bits);
  xlog("max speed:%d KHz", speed/1000);

  return ret;
}

int SPI_Close(void)
{
    int fd = g_SPI_Fd;


    if (fd == 0)
        return 0;
    close(fd);
    g_SPI_Fd = 0;


    return 0;
}

int SPI_LookBackTest(void)
{
    int ret, i;
    const int BufSize = 16;
    uint8_t tx[BufSize], rx[BufSize];

    bzero(rx, sizeof(rx));
    for (i = 0; i < BufSize; i++)
        tx[i] = i;

    printf("nSPI - LookBack Mode Test...\n");
    ret = SPI_Transfer(tx, rx, BufSize);
    if (ret > 1)
    {
        ret = memcmp(tx, rx, BufSize);
        if (ret != 0)
        {
            printf("tx:\n");
            for (i = 0; i < BufSize; i++)
            {
                printf("%d ", tx[i]);
            }
            printf("\n");
            printf("rx:\n");
            for (i = 0; i < BufSize; i++)
            {
                printf("%d ", rx[i]);
            }
            printf("\n");
            perror("LookBack Mode Test error\n");
        }
        else
            printf("SPI - LookBack Mode OK\n");
    }

    return ret;
}

unsigned char xor_checksum(unsigned char *data, int length) {
    unsigned char checksum = 0;
    for (int i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

/*******************************************************************
*   Function : ios_setExtLightLevel_withChannel()
*   Description : set external controller lighting level 
*   Arrgument : uint16_t level
*   Return : OK 0, Error -1
*********************************************************************/
int ios_setExtLightLevel_withChannel(__LIGHT_MANUFACTURER__ manufacturer, uint16_t level, uint8_t channel) {
  static uint8_t PWM_TX_Data[MAX_FORMAT_LEN + 2] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x64, 0x00, 0x0D, 0x0A};
  unsigned char RxBuf[100];

  if (level > MAX_MAIN_LIGHT_LEVEL) return -1;
  PWM_TX_Data[TYPE] = LIGHTING_CONTROL;

  // switch(manufacturer){
  //   case OPT:
  //     strncpy(&PWM_TX_Data[MAUNFACTURER],"OPT", 3);
  //   break;
  // }

  // OPT >> 0x4F, 0x50, 0x54
  if (manufacturer == OPT) {
    PWM_TX_Data[MAUNFACTURER] = 0x4F;      // O
    PWM_TX_Data[MAUNFACTURER + 1] = 0x50;  // P
    PWM_TX_Data[MAUNFACTURER + 2] = 0x54;  // T
  }

  PWM_TX_Data[EXT_LIGHT_CH] = channel;
  PWM_TX_Data[LIGHT_LEVEL] = level;
  PWM_TX_Data[CHECKSUM] = xor_checksum(PWM_TX_Data, MAX_FORMAT_LEN - 1);

  printf("%s()%d: PWM_TX_Data=", __FUNCTION__, __LINE__);
  for (int i = 0; i < MAX_FORMAT_LEN; i++) {
    printf("[0x%x] ", PWM_TX_Data[i]);
  }
  printf("\r\n");

  SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN + 2);
  return 0;
}

/*******************************************************************
*   Function : ios_readExtLightLevel_withChannel()
*   Description : read external controller lighting level 
*   Arrgument : manufacturer, uint18_t channel
*   Return : OK=uint16_t level, Error=-1
*********************************************************************/
uint16_t ios_readExtLightLevel_withChannel(__LIGHT_MANUFACTURER__ manufacturer, uint8_t channel) {
  static uint8_t PWM_TX_Data[MAX_FORMAT_LEN] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x64, 0x00};
  unsigned char checksum, RxBuf[10], count = 0;
  uint16_t level;

  // switch(manufacturer){
  //   case OPT:
  //     strncpy(&PWM_TX_Data[MAUNFACTURER],"OPT", 3);
  //   break;
  // }

  // OPT >> 0x4F, 0x50, 0x54
  if (manufacturer == OPT) {
    PWM_TX_Data[MAUNFACTURER] = 0x4F;      // O
    PWM_TX_Data[MAUNFACTURER + 1] = 0x50;  // P
    PWM_TX_Data[MAUNFACTURER + 2] = 0x54;  // T
  }

  PWM_TX_Data[TYPE] = EXT_LIGHT_READ;
  PWM_TX_Data[EXT_LIGHT_CH] = channel;
  PWM_TX_Data[EXT_LIGHT_CMD] = READ_LIGHT_LEVEL;
  checksum = xor_checksum(PWM_TX_Data, MAX_FORMAT_LEN - 1);
  printf("%s()%d: checksum=%x\r\n", __FUNCTION__, __LINE__, checksum);
  PWM_TX_Data[CHECKSUM] = checksum;
  SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN + 2);
  usleep(100000);
  // SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
  while ((RxBuf[TYPE] != EXT_LIGHT_READ) || (RxBuf[EXT_LIGHT_CMD] != READ_LIGHT_LEVEL)) {
    memset(RxBuf, 0xff, sizeof(RxBuf));
    SPI_Read(RxBuf, MAX_FORMAT_LEN);

    printf("%s()%d: RxBuf=", __FUNCTION__, __LINE__);
    for (int i = 0; i < MAX_FORMAT_LEN; i++)
      printf("0x%x ", RxBuf[i]);
    printf("\r\n");

    usleep(100000);
    if (count++ > 100) {
      printf("%s()%d: SPI reading timeout\r\n", __FUNCTION__, __LINE__);
      return -1;
    }
  }
  level = RxBuf[LIGHT_LEVEL - 1] << 8 | RxBuf[LIGHT_LEVEL];
  printf("READ ext lighting channel %d, level=%d\r\n", channel, level);
  return level;
}

uint16_t readtest(__LIGHT_MANUFACTURER__  manufacturer, uint8_t channel){
  // static uint8_t PWM_TX_Data[MAX_FORMAT_LEN]={0x00,0x00,0x00,0x00,0xFF,0x01,0x00,0x00,0x64,0x00};
  unsigned char RxBuf[10], count=0;
  uint16_t level;


  //SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
  {
    memset(RxBuf, 0xff, sizeof(RxBuf));
    SPI_Read(RxBuf,MAX_FORMAT_LEN);
    printf("RxBuf=");
    for(int i=0;i<MAX_FORMAT_LEN;i++)
      printf("0x%x", RxBuf[i]);
    printf("\r\n");
    usleep(100000);  
    if(count++ > 100){
      printf("SPI reading timeout\r\n");
      return -1;
    }
  }
  level = RxBuf[LIGHT_LEVEL-1] << 8 | RxBuf[LIGHT_LEVEL];
  printf("READ ext lighting channel %d, level=%d\r\n", channel, level);
  return level;
  
}

/*******************************************************************
*   Function : ios_setStatusLed()
*   Description : set Status Led
*   Arrgument : uint16_t level
*   Return : OK 0, Error -1
*********************************************************************/
int ios_setStatusLed(uint8_t channel , uint8_t value){
  unsigned char TX_Data[MAX_FORMAT_LEN]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00};
  unsigned char checksum;

  uint8_t RxBuf[10]={0};
  // TX_Data[LIGHT_LEVEL] = i;
  memset(&TX_Data[0], 0x00, sizeof(TX_Data));
  TX_Data[TYPE] = STATUS_LED;
  TX_Data[EXT_LIGHT_CH] = channel;
  TX_Data[LIGHT_LEVEL] = value;
  checksum = xor_checksum(TX_Data, MAX_FORMAT_LEN-1);
  xlog("checksum:0x%X", checksum);

  TX_Data[CHECKSUM] = checksum;
  SPI_Transfer(TX_Data, RxBuf, MAX_FORMAT_LEN+2);

  // printf("%s()%d: TX_Data=", __FUNCTION__, __LINE__);
  // for(int i=0;i<MAX_FORMAT_LEN;i++) {
  //   printf("[0x%x] ", TX_Data[i]);
  // }
  // printf("\r\n");

  return 0;
}

/*******************************************************************
*   Function : ios_readExtLightAttribute()
*   Description : read external lighting controller attribute
*   Arrgument : manufacturer
*   Return : OK = version number, Error = -1
*********************************************************************/
uint16_t ios_readExtLightAttribute(__LIGHT_MANUFACTURER__ manufacturer) {
  static uint8_t PWM_TX_Data[MAX_FORMAT_LEN] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x64, 0x00};
  unsigned char checksum, RxBuf[10], count = 0;
  uint16_t model_num;

  // switch(manufacturer){
  //   case OPT:
  //     strncpy(&PWM_TX_Data[MAUNFACTURER],"OPT", 3);
  //   break;
  // }

  // OPT >> 0x4F, 0x50, 0x54
  if (manufacturer == OPT) {
    PWM_TX_Data[MAUNFACTURER] = 0x4F;      // O
    PWM_TX_Data[MAUNFACTURER + 1] = 0x50;  // P
    PWM_TX_Data[MAUNFACTURER + 2] = 0x54;  // T
  }

  PWM_TX_Data[TYPE] = EXT_LIGHT_READ;
  PWM_TX_Data[DATA_FIELD] = MODEL_NUM;
  PWM_TX_Data[EXT_LIGHT_CMD] = READ_LIGHT_ATTRIBUTE;
  checksum = xor_checksum(PWM_TX_Data, MAX_FORMAT_LEN - 1);
  printf("checksum=%x\r\n", checksum);
  PWM_TX_Data[CHECKSUM] = checksum;
  SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN + 2);
  usleep(100000);
  // SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
  // memset(RxBuf, 0xff, sizeof(RxBuf));
  while ((RxBuf[TYPE] != EXT_LIGHT_READ) || (RxBuf[EXT_LIGHT_CMD] != READ_LIGHT_ATTRIBUTE)) {
    SPI_Read(RxBuf, MAX_FORMAT_LEN);

    printf("RxBuf=");
    for (int i = 0; i < MAX_FORMAT_LEN; i++)
      printf("0x%x", RxBuf[i]);
    printf("\r\n");

    usleep(500000);
    if (count++ > 100) {
      printf("SPI reading timeout\r\n");
      return -1;
    }
  }
  model_num = RxBuf[DATA_FIELD - 1] << 8 | RxBuf[DATA_FIELD];
  printf("READ ext lighting model number: %x\r\n", model_num);
  return model_num;
}

/*******************************************************************
*   Function : ios_setAiLightLevel()
*   Description : set AI LED lighting level 
*   Arrgument : uint16_t level
*   Return : OK 0, Error -1
*********************************************************************/
int ios_setAiLightLevel(uint16_t level){
  unsigned char PWM_TX_Data[MAX_FORMAT_LEN]={0x00,0x4F,0x50,0x54,0xFF,0x01,0x01,0x00,0x11,0x00};
  // unsigned char PWM_MAIN_TX_Data[MAX_FORMAT_LEN]={0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00};
  unsigned char PWM_AI_TX_Data[MAX_FORMAT_LEN]={0x22,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x64,0x00};

  if(level > MAX_AI_LIGHT_LEVEL) return -1;
  PWM_AI_TX_Data[LIGHT_LEVEL] = level;
  PWM_AI_TX_Data[CHECKSUM] = xor_checksum(&PWM_TX_Data[4], 5);
  SPI_Write(PWM_AI_TX_Data, MAX_FORMAT_LEN+1);
  return 0;
}

/*******************************************************************
*   Function : ios_setAiLightLevel()
*   Description : set AI LED lighting level 
*   Arrgument : uint16_t level
*   Return : OK 0, Error -1
*********************************************************************/
int ios_setMainLightLevel(uint16_t level){
  unsigned char PWM_MAIN_TX_Data[MAX_FORMAT_LEN]={0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00};
  unsigned char checksum;

  uint8_t RxBuf[10]={0};
  // PWM_MAIN_TX_Data[LIGHT_LEVEL] = i;
  memset(&PWM_MAIN_TX_Data[0], 0x00, sizeof(PWM_MAIN_TX_Data));
  PWM_MAIN_TX_Data[TYPE] = PWM_MAIN_LED;
  PWM_MAIN_TX_Data[LIGHT_LEVEL] = level;
  checksum = xor_checksum(PWM_MAIN_TX_Data, MAX_FORMAT_LEN-1);
  printf("%s()%d: checksum=%x\r\n", __FUNCTION__, __LINE__,checksum);
  PWM_MAIN_TX_Data[CHECKSUM] = checksum;
  SPI_Transfer(PWM_MAIN_TX_Data, RxBuf, MAX_FORMAT_LEN+2);
  printf("%s()%d: PWM_MAIN_TX_Data=", __FUNCTION__, __LINE__);
  for(int i=0;i<MAX_FORMAT_LEN;i++) {
    printf("[0x%x] ", PWM_MAIN_TX_Data[i]);
  }
  printf("\r\n");

  return 0;
}
/*******************************************************************
*   Function : ios_setAiLightLevel_withEnable()
*   Description : set AI LED lighting level with which channel is 
*                 enable/disable
*   Arrgument : uint16_t level
*               uint8_t channel (combined bit, channel 4 : 0x08, 
*               channel 3 : 0x04, channel 2 : 0x02, channel 1 : 0x01)
*   Return : OK 0, Error -1
*********************************************************************/
int ios_setAiLightLevel_withChannel(uint16_t level, uint8_t channel){
  unsigned char PWM_AI_TX_Data[MAX_FORMAT_LEN]={PWM_AI_LED,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x64,0x00};
  unsigned char checksum;
  uint8_t RxBuf[10]={0};

  if(level > MAX_AI_LIGHT_LEVEL) return -1;
  PWM_AI_TX_Data[LIGHT_LEVEL] = level;
  PWM_AI_TX_Data[CHECKSUM] = xor_checksum(PWM_AI_TX_Data, MAX_FORMAT_LEN-1);
  PWM_AI_TX_Data[AI_LIGHT_ENABLE]   = ((channel & 0x08) > 0) ? 1 : 0;   // Led4
  PWM_AI_TX_Data[AI_LIGHT_ENABLE+1] = ((channel & 0x04) > 0) ? 1 : 0;   // Led3
  PWM_AI_TX_Data[AI_LIGHT_ENABLE+2] = ((channel & 0x02) > 0) ? 1 : 0;   // Led2
  PWM_AI_TX_Data[AI_LIGHT_ENABLE+3] = ((channel & 0x01) > 0) ? 1 : 0;   // Led1


  checksum = xor_checksum(PWM_AI_TX_Data, MAX_FORMAT_LEN-1);
  printf("%s()%d: checksum=%x\r\n", __FUNCTION__, __LINE__,checksum);
  PWM_AI_TX_Data[CHECKSUM] = checksum;
  SPI_Transfer(PWM_AI_TX_Data, RxBuf, MAX_FORMAT_LEN+2);
  return 0;
}

int spi_gpio_set_value(unsigned int gpio, unsigned int value){
    unsigned char TX_Data[MAX_FORMAT_LEN]={0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00};
    unsigned char checksum;
    uint8_t RxBuf[10]={0};

    /* GPIO DO testing */
    // TX_Data[LIGHT_LEVEL] = i;
    memset(&TX_Data[0], 0x00, sizeof(TX_Data));
    TX_Data[TYPE] = GPIO_DO;
    TX_Data[1] = gpio;
    TX_Data[2] = value % 2;
    checksum = xor_checksum(TX_Data, MAX_FORMAT_LEN-1);
    printf("%s()%d: checksum=%x\r\n", __FUNCTION__, __LINE__,checksum);
    TX_Data[CHECKSUM] = checksum;
    SPI_Transfer(TX_Data, RxBuf, MAX_FORMAT_LEN+2);
    printf("%s()%d: ", __FUNCTION__, __LINE__);
    for(int i=0;i<MAX_FORMAT_LEN;i++) {
      printf("[0x%x] ", TX_Data[i]);
    }
    printf("\r\n");

    return 0;
}

int spi_gpio_do_get_value(unsigned int gpio, unsigned int *value){
    unsigned char TX_Data[MAX_FORMAT_LEN]={0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00};
    unsigned char checksum, count=0;
    uint16_t level;

    uint8_t RxBuf[10]={0};

    /* GPIO DO testing */
    // TX_Data[LIGHT_LEVEL] = i;
    memset(&TX_Data[0], 0x00, sizeof(TX_Data));
    TX_Data[TYPE] = GPIO_DO_READ;
    TX_Data[1] = gpio;
    checksum = xor_checksum(TX_Data, MAX_FORMAT_LEN-1);
    printf("%s()%d: checksum=%x\r\n", __FUNCTION__, __LINE__,checksum);
    TX_Data[CHECKSUM] = checksum;
    SPI_Transfer(TX_Data, RxBuf, MAX_FORMAT_LEN+2);
    printf("%s()%d: spi_gpio_do_get_value=", __FUNCTION__, __LINE__);
    for(int i=0;i<MAX_FORMAT_LEN;i++) {
      printf("[0x%x] ", TX_Data[i]);
    }
    printf("\r\n");
    usleep(100000);
    //SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
    while(RxBuf[TYPE] != GPIO_DO_READ) {
      memset(RxBuf, 0xff, sizeof(RxBuf));
      SPI_Read(RxBuf,MAX_FORMAT_LEN+2);
      printf("%s()%d: RxBuf=", __FUNCTION__, __LINE__);
      for(int i=0;i<MAX_FORMAT_LEN;i++) {
        printf("[%x] ", RxBuf[i]);
      }
      printf("\r\n");
      usleep(100000);  
      if(count++ > 100){
        printf("%s()%d: SPI reading timeout\r\n", __FUNCTION__, __LINE__);
        return -1;
      }
    }
    *value = level = RxBuf[DATA_FIELD]; // RxBuf[LIGHT_LEVEL-1] << 8 | RxBuf[LIGHT_LEVEL];
    printf("READ gpio %d, level=%d\r\n", gpio, level);
    return level;
  
}

int spi_gpio_di_get_value(unsigned int gpio, unsigned int *value){
    unsigned char TX_Data[MAX_FORMAT_LEN]={0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00};
    unsigned char checksum, count=0;
    uint16_t level;

    uint8_t RxBuf[10]={0};

    /* GPIO DI testing */
    // TX_Data[LIGHT_LEVEL] = i;
    memset(&TX_Data[0], 0x00, sizeof(TX_Data));
    TX_Data[TYPE] = GPIO_DI_READ;
    TX_Data[1] = gpio;
    checksum = xor_checksum(TX_Data, MAX_FORMAT_LEN-1);
    printf("%s()%d: checksum=%x\r\n", __FUNCTION__, __LINE__,checksum);
    TX_Data[CHECKSUM] = checksum;
    SPI_Transfer(TX_Data, RxBuf, MAX_FORMAT_LEN+2);
    printf("%s()%d: spi_gpio_di_get_value=", __FUNCTION__, __LINE__);
    for(int i=0;i<MAX_FORMAT_LEN;i++) {
      printf("[0x%x] ", TX_Data[i]);
    }
    printf("\r\n");
    usleep(100000);
    //SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
    do {
      memset(RxBuf, 0xff, sizeof(RxBuf));
      SPI_Read(RxBuf,MAX_FORMAT_LEN+2);
      printf("%s()%d: RxBuf=", __FUNCTION__, __LINE__);
      for(int i=0;i<MAX_FORMAT_LEN;i++) {
        printf("[%x] ", RxBuf[i]);
      }
      printf("\r\n");
      usleep(100000);  
      if(count++ > 100){
        printf("%s()%d: SPI reading timeout\r\n", __FUNCTION__, __LINE__);
        return -1;
      }
    } while(RxBuf[TYPE] != GPIO_DI_READ);
    *value = level = RxBuf[LIGHT_LEVEL]; // RxBuf[LIGHT_LEVEL-1] << 8 | RxBuf[LIGHT_LEVEL];
    printf("%s()%d: READ gpio %d, level=%d\r\n", __FUNCTION__, __LINE__, gpio, level);
    return level;
  
}

/*******************************************************************
*   Function : main()
*   Description : main function
*   Arrgument : 
*   Return : OK 0, Error -1
*********************************************************************/
int spi_main(int argc, char **argv){
  // unsigned char PWM_TX_Data[MAX_FORMAT_LEN]={0x00,0x4F,0x50,0x54,0xFF,0x01,0x01,0x00,0x11,0x00};
  // unsigned char PWM_MAIN_TX_Data[MAX_FORMAT_LEN]={0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00};
  // unsigned char PWM_AI_TX_Data[MAX_FORMAT_LEN]={0x22,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x64,0x00};
  // uint8_t RxBuf[10]={0}, checksum;
  // uint16_t level, model_num;
  return 0;

  // int i;
  SPI_Open();
  for(;;){
    /* SPI testing */
   /* transfer */    
#if 0    
    i = rand() % 255;
    PWM_TX_Data[TYPE] = EXT_LIGHT_CONTROL;
    PWM_TX_Data[EXT_LIGHT_CMD] = SET_LIGHT_LEVEL;
    PWM_TX_Data[LIGHT_LEVEL-1] = 0x00;
    PWM_TX_Data[LIGHT_LEVEL] = i;
    checksum = xor_checksum(PWM_TX_Data, MAX_FORMAT_LEN-1);
    printf("checksum=%x\r\n",checksum);
    PWM_TX_Data[CHECKSUM] = checksum;
    SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
    //for(int i=0;i<MAX_FORMAT_LEN;i++)
    //  printf("RxBuf[%d]=%x .....\r\n", i,RxBuf[i]);
    usleep(1000000);
   
    /* read */
/*
#if 1
    level = ios_readExtLightLevel_withChannel(OPT, CH1);
    model_num = ios_readExtLightAttribute(OPT);
#else
    PWM_TX_Data[TYPE] = EXT_LIGHT_READ;
    PWM_TX_Data[EXT_LIGHT_CH] = 0x01;
    PWM_TX_Data[EXT_LIGHT_CMD] = READ_LIGHT_LEVEL;
    checksum = xor_checksum(PWM_TX_Data, MAX_FORMAT_LEN-1);
    printf("checksum=%x\r\n",checksum);
    PWM_TX_Data[CHECKSUM] = checksum;
    SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
    usleep(100000);
    //SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
    //memset(RxBuf, 0xff, sizeof(RxBuf));
    while(RxBuf[TYPE] != EXT_LIGHT_READ){
      SPI_Read(RxBuf,MAX_FORMAT_LEN);
      for(int i=0;i<MAX_FORMAT_LEN;i++)
        printf("RxBuf[%d]=%x .....\r\n", i,RxBuf[i]);
      usleep(100000);  
    }
#endif
*/
    usleep(1000000);

    for(i=0;i<255;i++){
        PWM_TX_Data[LIGHT_LEVEL] = i;
        checksum = xor_checksum(PWM_TX_Data, MAX_FORMAT_LEN-1);
        printf("checksum=%x\r\n",checksum);
        PWM_TX_Data[CHECKSUM] = checksum;
        SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
        for(int i=0;i<MAX_FORMAT_LEN;i++)
          printf("PWM_TX_Data[%d]=%x .....\r\n", i,PWM_TX_Data[i]);
        usleep(1000);
    }
    for(i=255;i>0;i--){
        PWM_TX_Data[LIGHT_LEVEL] = i;
        checksum = xor_checksum(PWM_TX_Data, MAX_FORMAT_LEN-1);
        printf("checksum=%x\r\n",checksum);
        PWM_TX_Data[CHECKSUM] = checksum;
        SPI_Transfer(PWM_TX_Data, RxBuf, MAX_FORMAT_LEN);
        for(int i=0;i<MAX_FORMAT_LEN;i++)
          printf("PWM_TX_Data[%d]=%x .....\r\n", i,PWM_TX_Data[i]);
        usleep(1000);
    }
   
    /* PWM MAIN testing */
    for(i=0;i<=100;i++){
      PWM_MAIN_TX_Data[LIGHT_LEVEL] = i;
      checksum = xor_checksum(PWM_MAIN_TX_Data, MAX_FORMAT_LEN-1);
      printf("checksum=%x\r\n",checksum);
      PWM_MAIN_TX_Data[CHECKSUM] = checksum;
      SPI_Transfer(PWM_MAIN_TX_Data, RxBuf, MAX_FORMAT_LEN);
      for(int i=0;i<MAX_FORMAT_LEN;i++)
        printf("PWM_MAIN_TX_Data[%d]=%x .....\r\n", i,PWM_MAIN_TX_Data[i]);
      usleep(10000);
      ios_setExtLightLevel_withChannel(OPT, i, CH1);
      usleep(10000);
    }
#endif 

  }  
  //SPI_Transfer(PWM_TX_Data, RxBuf,9);
  //SPI_Write(&PWM_TX_Data[0], 9);
    //sleep(1);
  //SPI_LookBackTest();
  //}
  SPI_Close();

}