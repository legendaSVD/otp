#include <windows.h>
extern void mainCRTStartup(void);
BOOL WINAPI erl_port_default_handler(DWORD ctrl){
    if(ctrl == CTRL_LOGOFF_EVENT)
	return TRUE;
    if(ctrl == CTRL_SHUTDOWN_EVENT)
	return TRUE;
    return FALSE;
}
void erl_port_entry(void){
    char buffer[2];
    if(GetEnvironmentVariable("ERLSRV_SERVICE_NAME",buffer,(DWORD) 2)){
#ifdef HARDDEBUG
	DWORD dummy;
	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),
		  "Setting handler\r\n",17,&dummy, NULL);
#endif
	SetConsoleCtrlHandler(&erl_port_default_handler, TRUE);
    }
    mainCRTStartup();
}