#ifndef _ERLSRV_GLOBAL_H
#define _ERLSRV_GLOBAL_H
#define APP_NAME L"ErlSrv"
#define ERLANG_MACHINE L"erl.exe"
#define SERVICE_ENV L"ERLSRV_SERVICE_NAME"
#define EXECUTABLE_ENV L"ERLSRV_EXECUTABLE"
#define DEBUG_ENV L"ERLSRV_DEBUG"
#ifdef _DEBUG
#define HARDDEBUG 1
#define DEBUG 1
#else
#define NDEBUG 1
#endif
#endif