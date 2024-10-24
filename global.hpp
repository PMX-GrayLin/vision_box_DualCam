#pragma once 

#define USE_TOF
#define DEBUG_TOF

#define DEBUG_SPI

#define DEBUGX

#ifndef DEBUGX
#define xlog(...) ((void)0)
#else
// #define xlog(...) printf(__VA_ARGS__)
#define xlog(fmt, ...) printf("%s:%d, " fmt "\n\r", __func__, __LINE__, ##__VA_ARGS__)
#endif



