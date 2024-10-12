#pragma once 

// #include <cstdint>

#define DEBUGX

#ifndef DEBUGX
#define xlog(...) ((void)0)
#else
// #define xlog(...) printf(__VA_ARGS__)
#define xlog(fmt, ...) printf("%s:%d, " fmt, __func__, __LINE__, ##__VA_ARGS__)
#endif



