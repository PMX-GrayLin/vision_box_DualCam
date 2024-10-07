#pragma once 

#include <cstdint>

#define DEBUGX
#ifndef DEBUGX
#define xlog(...) ((void)0)
#else
#define xlog(...) printf(__VA_ARGS__)
#endif

extern int iosCtl_init();
extern int ios_readVersionFile(char *jstring);
