#pragma once

void COMMON_API Dout(size_t line, const char *writeFmt, va_list argptr );
void COMMON_API Dout(size_t line, const char *writeFmt, ...);

void COMMON_API Dout(size_t line, size_t startingColumn, const char *writeFmt, va_list argptr);
void COMMON_API Dout(size_t line, size_t startingColumn, const char *writeFmt, ...);

//Disable to test LUA calls (for wind-river this should always be disabled)
#if 1
#define DOUT(x,y,...) Dout(x,y,__VA_ARGS__);
#else
#define DOUT(x,y,...)
#endif
