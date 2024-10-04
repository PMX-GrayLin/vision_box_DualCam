#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include "../global.hpp"

 /****************************************************************
  *  * Constants
  *   ****************************************************************/

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (100) /* 100 milliseconds */
#define MAX_BUF 64

void help()
{
  printf("usage:\r\n");
  printf("set input:  ./gpio DIN_source -i -r/-f(rising or falling)\r\n");
  printf("set output: ./gpio DOUT_source -o -n -g/-s(get or set) Value\r\n");
}

/****************************************************************
 *  * gpio_export
 *   ****************************************************************/
int gpio_export(unsigned int gpio)
{
  int fd, len;
  char buf[MAX_BUF];

  fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
  if (fd < 0) {
    printf("gpio/export gpio=[%d]\r\n", gpio);
    return fd;
  }

  len = snprintf(buf, sizeof(buf), "%d", gpio);
  write(fd, buf, len);
  close(fd);

  return 0;
}

/****************************************************************
 *  * gpio_unexport
 *   ****************************************************************/
int gpio_unexport(unsigned int gpio)
{
  int fd, len;
  char buf[MAX_BUF];

  fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
  if (fd < 0) {
    printf("gpio/unexport\r\n");
    return fd;
  }

  len = snprintf(buf, sizeof(buf), "%d", gpio);
  write(fd, buf, len);
  close(fd);
  return 0;
}

/****************************************************************
 *  * gpio_set_dir
 *   ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
  int fd;
  char buf[MAX_BUF];

  snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

  fd = open(buf, O_WRONLY);
  if (fd < 0) {
    printf("gpio/direction\r\n");
    return fd;
  }

  if (out_flag)
    write(fd, "out", 4);
  else
    write(fd, "in", 3);

  close(fd);
  return 0;
}

/****************************************************************
 *  * gpio_set_value
 *   ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value)
{
  int fd;
  char buf[MAX_BUF];

  snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

  fd = open(buf, O_WRONLY);
  if (fd < 0) {
    printf("gpio/set-value\r\n");
    xlog("%s:%d, fail \n\r", __func__, __LINE__);
    return fd;
  }

  if (value)
    write(fd, "1", 2);
  else
    write(fd, "0", 2);

  close(fd);
  return 0;
}

/****************************************************************
 *  * gpio_get_value
 *   ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value)
{
  int fd;
  char buf[MAX_BUF];
  char ch;

  snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

  fd = open(buf, O_RDONLY);
  if (fd < 0) {
    printf("gpio/get-value fd=[%d] gpio=[%d] fail.\r\n", fd, gpio);
    return fd;
  }

  read(fd, &ch, 1);
  
  if (ch != '0') {
    *value = 1;
  } else {
    *value = 0;
  }

  close(fd);
  return 0;
}


/****************************************************************
 *  * gpio_set_edge
 *   ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge)
{
  int fd;
  char buf[MAX_BUF];

  snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

  fd = open(buf, O_WRONLY);
  if (fd < 0) {
    printf("gpio/set-edge\r\n");
    return fd;
  }

  write(fd, edge, strlen(edge) + 1);
  close(fd);
  return 0;
}

/****************************************************************
 *  * gpio_fd_open
 *   ****************************************************************/

int gpio_fd_open(unsigned int gpio)
{
  int fd;
  char buf[MAX_BUF];

  snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

  fd = open(buf, O_RDONLY | O_NONBLOCK );
  if (fd < 0) {
    printf("gpio/fd_open\r\n");
  }
  return fd;
}

/****************************************************************
 *  * gpio_fd_close
 *   ****************************************************************/

int gpio_fd_close(int fd)
{
  return close(fd);
}

/****************************************************************
 *  * Main
 *   ****************************************************************/
#if 0
int main(int argc, char **argv, char **envp)
{
  struct pollfd fdset[2];
  int nfds = 2;
  int gpio_fd, timeout, rc;
  char *buf[MAX_BUF];
  unsigned int gpio, gpio_pin, rvalue = 0, gpio_value;
  int len, index=0;

  if (argc < 2) {
    printf("Usage: gpio-int <gpio-pin>\r\n");
    printf("Waits for a change in the GPIO pin voltage level or input on stdin\r\n");
    exit(-1);
  }
  /*
    ./gpio 1/2/3/4 -i -r/-f
    ./gpio 1/2/3/4 -o -n -g
    ./gpio 1/2/3/4 -o -n -s 1
  */
  gpio = atoi(argv[1]);
  char* gpio_dir = argv[2];
  char* gpio_edge = argv[3];

  if (strncmp(gpio_dir, "-i", 2) == 0) {
    if (gpio == 1) {
      gpio_pin = 0;
    } else if (gpio == 2) {
      gpio_pin = 1;
    } else if (gpio == 3) {
      gpio_pin = 7;
    } else if (gpio == 4) {
      gpio_pin = 8;
    } else {
      help();
      return rvalue;
    }

    gpio_export(gpio_pin);
    gpio_set_dir(gpio_pin, 0);

    if (strncmp(gpio_edge, "-r", 2) == 0) {
      gpio_set_edge(gpio_pin, "rising");
    } else if (strncmp(gpio_edge, "-f", 2) == 0) {
      gpio_set_edge(gpio_pin, "falling");
    } else {
      help();
      return rvalue;
    }
    gpio_fd = gpio_fd_open(gpio_pin);
    timeout = POLL_TIMEOUT;

    memset((void*)fdset, 0, sizeof(fdset));
    fdset[0].fd = STDIN_FILENO;
    fdset[0].events = POLLIN;
    fdset[1].fd = gpio_fd;
    fdset[1].events = POLLPRI;

    rc = poll(fdset, nfds, timeout);
    printf("rc=%d\r\n", rc);
    if (rc < 0) {
      printf("poll() failed!\r\n");
      return -1;
    }
    if (rc == 0) {
      printf(".");
    }

    if (fdset[1].revents & POLLPRI) {
      len = read(fdset[1].fd, buf, MAX_BUF);
      gpio_get_value(gpio_pin, &rvalue);
      printf("poll() GPIO %d interrupt occurred\r\n", gpio_pin);
      printf("rvalue:%d\r\n", rvalue);
    }
    if (fdset[0].revents & POLLIN) {
      (void)read(fdset[0].fd, buf, 1);
      printf("poll() stdin read 0x%2.2X\r\n", (unsigned int) buf[0]);
    }
    fflush(stdout);
    gpio_fd_close(gpio_fd);
  } else if (strncmp(gpio_dir, "-o", 2) == 0) {
    if (gpio == 1) {
      gpio_pin = 9;
    } else if (gpio == 2) {
      gpio_pin = 10;
    } else if (gpio == 3) {
      gpio_pin = 11;
    } else if (gpio == 4) {
      gpio_pin = 13;
    } else {
      help();
      return 0;
    }

    gpio_export(gpio_pin);
    gpio_set_dir(gpio_pin, 1);

    char* gpio_op_mod = argv[4];
    if (strncmp(gpio_op_mod, "-g", 2) == 0) {
      gpio_get_value(gpio_pin, &rvalue);
    } else if (strncmp(gpio_op_mod, "-s", 2) == 0) {
      gpio_value = atoi(argv[5]);
      gpio_set_value(gpio_pin, gpio_value);
    } else {
      help();
      return rvalue;
    }
  } else {
    help();
    return rvalue;
  }

  gpio_unexport(gpio_pin);
  return rvalue;
}
#endif
