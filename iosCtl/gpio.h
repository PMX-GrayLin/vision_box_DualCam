
int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
int gpio_set_value(unsigned int gpio, unsigned int value);
int gpio_get_value(unsigned int gpio, unsigned int *value);
int gpio_set_edge(unsigned int gpio, char *edge);
int gpio_fd_open(unsigned int gpio);
int gpio_fd_close(int fd);

    	
#define PCA64_P0  0x02
#define PCA64_P1  0x03

#define PCA64_P1_4  (1 << 4)
#define PCA64_P1_5  (1 << 5)
#define PCA64_P1_6  (1 << 6)
#define PCA64_P1_7  (1 << 7)

// DI
#define DI1_GPIO1_IO00  0  // GPIO1_IO00 (1-1)*32+0=0
#define DI2_GPIO1_IO01  1  // GPIO1_IO01                  0*32+1=33
#define DI3_GPIO1_IO07  7  // GPIO1_IO07                  0*32+7=7
#define DI4_GPIO1_IO08  8 // MX8MP_IOMUXC_SAI5_RXD2__GPIO3_IO23   gpio3.IO[23]    3*32+23=87
#define DI_TRIG1_GPIO3_IO21  85   // GPIO3_IO21                2*32+21=85
#define DI_TRIG2_GPIO3_IO22  86   // GPIO3_IO22                2*32+21=86

#define DO1_GPIO1_IO09  9    // GPIO1_IO09                0*32+9=9
#define DO2_GPIO1_IO10  10   // GPIO1_IO10                0*32+10=10
#define DO3_GPIO1_IO11  11   // GPIO1_IO11                0*32+11=11
#define DO4_GPIO1_IO13  13   // GPIO1_IO13                0*32+13=13

