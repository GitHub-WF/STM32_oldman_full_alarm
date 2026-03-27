#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__

// 使用串口重定向实现日志输出打印
#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"

// 日志输出打印开关（方便替换到生产环境）
#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE
// 使用字符串分割函数，去掉 __FILE__ 中的目录名称
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FILENAME (strrchr(__FILENAME__, '\\') ? strrchr(__FILENAME__, '\\') + 1 : __FILENAME__)
// 扩展加上文件名和行号
#define debug_printf(format, ...) printf("[%s:%d]" format "\r\n", __FILENAME, __LINE__, ##__VA_ARGS__);
#else
// 定义一个空文本替换宏定义
#define debug_printf(format, ...)
#endif // DEBUG

#endif /* __COM_DEBUG_H__ */
