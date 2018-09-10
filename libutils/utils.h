#ifndef __UTILITY_H
#define __UTILITY_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

// 字节对齐设置
#define ALIGN_ARG   2
#define ALIGN1(len)  (((len) + (1 << ALIGN_ARG) - 1) & ~((1 << ALIGN_ARG) -1))  // 这里是按 2^ALIGN_ARG字节对齐
#define ALIGN2(len)  ( ((len) + ALIGN_ARG - 1) & ~(ALIGN_ARG - 1))              // ALIGN_ARAG字节对齐

// 数组元素数量
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

// 比较数值大小并返回较大的数
#define MAX(a,b) ((a) > (b)?(a):(b))
// 比较数值大小并返回较小的数
#define MIN(a,b) ((a) > (b)?(b):(a))

#endif
