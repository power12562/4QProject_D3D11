#pragma once
#include <iostream>

#ifdef _DEBUG
#define Debug_printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define Debug_wprintf(fmt, ...) wprintf(fmt, ##__VA_ARGS__)
#else
/** DEBUG 빌드에서만 작동되는 매크로 입니다.*/
#define Debug_printf(fmt, ...)
#define Debug_wprintf(fmt, ...)
#endif

