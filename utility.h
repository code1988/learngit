#ifndef __UTILITY_H
#define __UTILITY_H

// 字节对齐设置
#define ALIGN_ARG   2
#define ALIGN(len)  (((len) + (1 << ALIGN_ARG) - 1) & ~((1 << ALIGN_ARG) -1))

// 数组元素数量
#define ARRAY_SIZE(x) (sizeof(x) / (*(x)))


#endif
