#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <assert.h>
wchar_t *progname;
#ifndef CASE_SENSITIVE_OPTIONS
#define strnicmp strncmp
#else
#define strnicmp _strnicmp
#endif
#define RELEASE_SUBDIR L"\\releases"
#define ERTS_SUBDIR_PREFIX L"\\erts-"
#define BIN_SUBDIR L"\\bin"
#define REGISTRY_BASE L"Software\\Ericsson\\Erlang\\"
#define DEFAULT_DATAFILE L"start_erl.data"
wchar_t *CommandLineStart = NULL;
wchar_t *ErlCommandLine = NULL;
wchar_t *MyCommandLine = NULL;
wchar_t *DataFileName = NULL;
wchar_t *RelDir = NULL;
wchar_t *BootFlagsFile = NULL;
wchar_t *BootFlags = NULL;
wchar_t *RegistryKey = NULL;
wchar_t *BinDir = NULL;
wchar_t *RootDir = NULL;
wchar_t *VsnDir = NULL;
wchar_t *Version = NULL;
wchar_t *Release = NULL;
BOOL NoConfig=FALSE;
PROCESS_INFORMATION ErlProcessInfo;
void ShowLastError(void)
{
    LPVOID	lpMsgBuf;
    DWORD	dwErr;
    dwErr = GetLastError();
    if( dwErr == ERROR_SUCCESS )
	return;
    FormatMessage(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		  NULL,
		  dwErr,
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (LPTSTR) &lpMsgBuf,
		  0,
		  NULL
		  );
    fprintf(stderr, lpMsgBuf);
    LocalFree( lpMsgBuf );
}
void exit_help(char *err)
{
    ShowLastError();
    fprintf(stderr, "** Error: %s\n", err);
    printf("Usage:\n%S\n"
	   "      [<erlang options>] ++\n"
	   "      [-data <datafile>]\n"
	   "      {-rootdir <erlang root directory> | \n"
	   "       -reldir <releasedir>}\n"
	   "      [-bootflags <bootflagsfile>]\n"
	   "      [-noconfig]\n", progname);
    exit(0);
}
void split_commandline(void)
{
    wchar_t	*cmdline = CommandLineStart;
    progname=cmdline;
    if(*cmdline == L'"') {
	cmdline++;
	while( (*cmdline != L'\0') && (*cmdline++) != L'"' )
	    ;
    } else {
	while( (*cmdline != L'\0') && (*cmdline++) != L' ' )
	    ;
    }
    while( (*cmdline) == L' ' )
	cmdline++;
    if( *cmdline == L'\0') {
	ErlCommandLine = L"";
	MyCommandLine = L"";
	return;
    }
    cmdline[-1] = L'\0';
    ErlCommandLine = cmdline;
    if(wcsncmp(cmdline,L"++ ",3))
	cmdline = wcsstr(cmdline,L" ++ ");
    if( cmdline == NULL ) {
	MyCommandLine = L"";
	return;
    }
    *cmdline++ = '\0';
    while( (*cmdline) == L' ' )
	cmdline++;
    while( (*cmdline) == L'+' )
	cmdline++;
    while( (*cmdline) == L' ' )
	cmdline++;
    MyCommandLine = cmdline;
#ifdef _DEBUG
    fprintf(stderr, "ErlCommandLine: '%S'\n", ErlCommandLine);
    fprintf(stderr, "MyCommandLine:  '%S'\n", MyCommandLine);
#endif
}
wchar_t * unquote_optionarg(wchar_t *str, wchar_t **strp)
{
    wchar_t	*newstr = (wchar_t *)malloc((wcslen(str)+1)*sizeof(wchar_t));
    int		i = 0, inquote = 0;
    assert(newstr);
    assert(str);
    while( *str == L' ' )
	str++;
    while( (inquote==1)  || ( (*str!=0) && (*str!=L' ') )  ) {
	switch( *str ) {
	case L'\\':
	    if( inquote  && str[1] == L'"' )
		str++;
	    newstr[i++]=*str++;
	    break;
	case L'"':
	    inquote = 1-inquote;
	    *str++;
	    break;
	default:
	    newstr[i++]=*str++;
	    break;
	}
	if( (*str == 0) && (inquote==1) ) {
	    exit_help("Unterminated quote.");
	}
    }
    newstr[i++] = 0x00;
    *strp = str;
    newstr = (wchar_t *)realloc(newstr, i*sizeof(wchar_t));
    assert(newstr);
    return(newstr);
}
void parse_commandline(void)
{
    wchar_t *cmdline = MyCommandLine;
    while( *cmdline != L'\0' ) {
	switch( *cmdline ) {
	case '-':
	case '/':
	    *cmdline++;
	    if( _wcsnicmp(cmdline, L"data", 4) == 0) {
		DataFileName = unquote_optionarg(cmdline+4, &cmdline);
	    } else if( _wcsnicmp(cmdline, L"rootdir", 7) == 0) {
		RootDir = unquote_optionarg(cmdline+7, &cmdline);
#ifdef _DEBUG
		fprintf(stderr, "RootDir: '%S'\n", RootDir);
#endif
	    } else if( _wcsnicmp(cmdline, L"reldir", 6) == 0) {
		RelDir = unquote_optionarg(cmdline+6, &cmdline);
#ifdef _DEBUG
		fprintf(stderr, "RelDir: '%S'\n", RelDir);
#endif
	    } else if( _wcsnicmp(cmdline, L"bootflags", 9) == 0) {
		BootFlagsFile = unquote_optionarg(cmdline+9, &cmdline);
	    } else if( _wcsnicmp(cmdline, L"noconfig", 8) == 0) {
		NoConfig=TRUE;
#ifdef _DEBUG
		fprintf(stderr, "NoConfig=TRUE\n");
#endif
	    } else {
		fprintf(stderr, "Unknown option: '%S'\n", cmdline);
		exit_help("Unknown command line option");
	    }
	    break;
	default:
	    cmdline++;
	    break;
	}
    }
}
void read_datafile(void)
{
    FILE	*fp;
    wchar_t	*newname;
    long	size;
    char        *ver;
    char        *rel;
    if(!DataFileName){
	DataFileName = malloc((wcslen(DEFAULT_DATAFILE) + 1)*sizeof(wchar_t));
	wcscpy(DataFileName,DEFAULT_DATAFILE);
    }
    if( (DataFileName[0] != L'\\') && (wcsncmp(DataFileName+1, L":\\", 2)!=0) ) {
	if( !RelDir ) {
	    exit_help("Need -reldir when -data filename has relative path.");
	} else {
	    size = (wcslen(DataFileName)+wcslen(RelDir)+2);
	    newname = (wchar_t *)malloc(size*sizeof(wchar_t));
	    assert(newname);
	    swprintf(newname, size, L"%s\\%s", RelDir, DataFileName);
	    free(DataFileName);
	    DataFileName=newname;
	}
    }
#ifdef _DEBUG
    fprintf(stderr, "DataFileName: '%S'\n", DataFileName);
#endif
    if( (fp=_wfopen(DataFileName, L"rb")) == NULL) {
	exit_help("Cannot find the datafile.");
    }
    fseek(fp, 0, SEEK_END);
    size=ftell(fp);
    fseek(fp, 0, SEEK_SET);
    ver = (char *)malloc(size+1);
    rel = (char *)malloc(size+1);
    assert(ver);
    assert(rel);
    if( (fscanf(fp, "%s %s", ver, rel)) == 0) {
	fclose(fp);
	exit_help("Format error in datafile.");
    }
    fclose(fp);
    size = MultiByteToWideChar(CP_UTF8, 0, ver, -1, NULL, 0);
    Version = malloc(size*sizeof(wchar_t));
    assert(Version);
    MultiByteToWideChar(CP_UTF8, 0, ver, -1, Version, size);
    free(ver);
    size = MultiByteToWideChar(CP_UTF8, 0, rel, -1, NULL, 0);
    Release = malloc(size*sizeof(wchar_t));
    assert(Release);
    MultiByteToWideChar(CP_UTF8, 0, rel, -1, Release, size);
    free(rel);
#ifdef _DEBUG
    fprintf(stderr, "DataFile version: '%S'\n", Version);
    fprintf(stderr, "DataFile release: '%S'\n", Release);
#endif
}
void read_bootflags(void)
{
    FILE	*fp;
    long	fsize;
    wchar_t	*newname;
    char        *bootf;
    if(BootFlagsFile) {
	if( (BootFlagsFile[0] != L'\\') &&
	    (wcsncmp(BootFlagsFile+1, L":\\", 2)!=0) ) {
	    if( !RelDir ) {
		exit_help("Need -reldir when -bootflags "
			  "filename has relative path.");
	    } else {
		int len = wcslen(BootFlagsFile)+
		    wcslen(RelDir)+wcslen(Release)+3;
		newname = (wchar_t *)malloc(len*sizeof(wchar_t));
		assert(newname);
		swprintf(newname, len, L"%s\\%s\\%s", RelDir, Release, BootFlagsFile);
		free(BootFlagsFile);
		BootFlagsFile=newname;
	    }
	}
#ifdef _DEBUG
	fprintf(stderr, "BootFlagsFile: '%S'\n", BootFlagsFile);
#endif
	if( (fp=_wfopen(BootFlagsFile, L"rb")) == NULL) {
	    exit_help("Could not open BootFlags file.");
	}
	fseek(fp, 0, SEEK_END);
	fsize=ftell(fp);
	fseek(fp, 0, SEEK_SET);
	bootf = (char *)malloc(fsize+1);
	assert(bootf);
	if( (fgets(bootf, fsize+1, fp)) == NULL) {
	    exit_help("Error while reading BootFlags file");
	}
	fclose(fp);
	bootf = (char *)realloc(bootf, strlen(bootf)+1);
	assert(bootf);
	fsize = strlen(bootf);
	while( fsize > 0 &&
	       ( (bootf[fsize-1] == '\r') ||
		(bootf[fsize-1] == '\n') ) ) {
	    bootf[--fsize]=0;
	}
	fsize = MultiByteToWideChar(CP_UTF8, 0, bootf, -1, NULL, 0);
	BootFlags = malloc(fsize*sizeof(wchar_t));
	assert(BootFlags);
	MultiByteToWideChar(CP_UTF8, 0, bootf, -1, BootFlags, fsize);
	free(bootf);
    } else {
	BootFlags = L"";
    }
#ifdef _DEBUG
    fprintf(stderr, "BootFlags: '%S'\n", BootFlags);
#endif
}
long start_new_node(void)
{
    wchar_t			*CommandLine;
    unsigned long		i;
    STARTUPINFOW		si;
    DWORD			dwExitCode;
    i = wcslen(RelDir) + wcslen(Release) + 4;
    VsnDir = (wchar_t *)malloc(i*sizeof(wchar_t));
    assert(VsnDir);
    swprintf(VsnDir, i, L"%s\\%s", RelDir, Release);
    if( NoConfig ) {
	i = wcslen(BinDir) + wcslen(ErlCommandLine) +
	    wcslen(BootFlags) + 64;
	CommandLine = (wchar_t *)malloc(i*sizeof(wchar_t));
	assert(CommandLine);
	swprintf(CommandLine,
		 i,
		 L"\"%s\\erl.exe\" -boot \"%s\\start\" %s %s",
		 BinDir,
		 VsnDir,
		 ErlCommandLine,
		 BootFlags);
    } else {
	i = wcslen(BinDir) + wcslen(ErlCommandLine)
	    + wcslen(BootFlags) + wcslen(VsnDir)*2 + 64;
	CommandLine = (wchar_t *)malloc(i*sizeof(wchar_t));
	assert(CommandLine);
	swprintf(CommandLine,
		 i,
		 L"\"%s\\erl.exe\" -boot \"%s\\start\" -config \"%s\\sys\" %s %s",
		 BinDir,
		 VsnDir,
		 VsnDir,
		 ErlCommandLine,
		 BootFlags);
    }
#ifdef _DEBUG
    fprintf(stderr, "CommandLine: '%S'\n", CommandLine);
#endif
    memset(&si, 0, sizeof(STARTUPINFOW));
    si.cb = sizeof(STARTUPINFOW);
    si.lpTitle = NULL;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    if( (CreateProcessW(
			NULL,
			CommandLine,
			NULL,
			NULL,
			TRUE,
			GetPriorityClass(GetCurrentProcess()),
			NULL,
			BinDir,
			&si,
			&ErlProcessInfo
			)) == FALSE) {
	ShowLastError();
	exit_help("Failed to start new node");
    }
#ifdef _DEBUG
    fprintf(stderr, "Waiting for Erlang to terminate.\n");
#endif
    if(MsgWaitForMultipleObjects(1,&ErlProcessInfo.hProcess, FALSE,
				 INFINITE, QS_POSTMESSAGE) == WAIT_OBJECT_0+1){
	if(PostThreadMessage(ErlProcessInfo.dwThreadId,
			     WM_USER,
			     (WPARAM) 0,
			     (LPARAM) 0)){
	    if(WaitForSingleObject(ErlProcessInfo.hProcess, 10000)
	       != WAIT_OBJECT_0){
		TerminateProcess(ErlProcessInfo.hProcess,0);
	    }
	} else {
	   TerminateProcess(ErlProcessInfo.hProcess,0);
	}
    }
    GetExitCodeProcess(ErlProcessInfo.hProcess, &dwExitCode);
#ifdef _DEBUG
    fprintf(stderr, "Erlang terminated.\n");
#endif
    free(CommandLine);
    return(dwExitCode);
}
void complete_options(void)
{
    int len;
    if( !RelDir ) {
	DWORD sz = 32;
	while (1) {
	    DWORD nsz;
	    if (RelDir)
		free(RelDir);
	    RelDir = malloc(sz*sizeof(wchar_t));
	    if (!RelDir) {
		fprintf(stderr, "** Error : failed to allocate memory\n");
		exit(1);
	    }
	    SetLastError(0);
	    nsz = GetEnvironmentVariableW(L"RELDIR", RelDir, sz);
	    if (nsz == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
		free(RelDir);
		RelDir = NULL;
		break;
	    }
	    else if (nsz <= sz)
		break;
	    else
		sz = nsz;
	}
	if (RelDir == NULL) {
	  if (!RootDir) {
	    exit_help("Need either Root directory nor Release directory.");
	  }
          sz = wcslen(RootDir)+wcslen(RELEASE_SUBDIR)+1;
	  RelDir = (wchar_t *) malloc(sz * sizeof(wchar_t));
	  assert(RelDir);
	  swprintf(RelDir, sz, L"%s" RELEASE_SUBDIR, RootDir);
	  read_datafile();
	} else {
	    read_datafile();
	}
    } else {
	read_datafile();
    }
    if( !RootDir ) {
	wchar_t *p;
	RootDir = malloc((wcslen(RelDir)+1)*sizeof(wchar_t));
	wcscpy(RootDir,RelDir);
	p = RootDir+wcslen(RootDir)-1;
	if (p >= RootDir && (*p == L'/' || *p == L'\\'))
	    --p;
	while (p >= RootDir && *p != L'/' &&  *p != L'\\')
	    --p;
	if (p <= RootDir) {
	    exit_help("Cannot determine Root directory from "
		      "Release directory.");
	}
	*p = L'\0';
    }
    len = wcslen(RootDir)+wcslen(ERTS_SUBDIR_PREFIX)+
	wcslen(Version)+wcslen(BIN_SUBDIR)+1;
    BinDir = (wchar_t *) malloc(len * sizeof(wchar_t));
    assert(BinDir);
    swprintf(BinDir, len, L"%s" ERTS_SUBDIR_PREFIX L"%s" BIN_SUBDIR, RootDir, Version);
    read_bootflags();
#ifdef _DEBUG
    fprintf(stderr, "RelDir: '%S'\n", RelDir);
    fprintf(stderr, "BinDir: '%S'\n", BinDir);
#endif
}
BOOL WINAPI LogoffHandlerRoutine( DWORD dwCtrlType )
{
    if(dwCtrlType == CTRL_LOGOFF_EVENT) {
	return TRUE;
    }
    if(dwCtrlType == CTRL_SHUTDOWN_EVENT) {
	return TRUE;
    }
    return FALSE;
}
int main(void)
{
    DWORD	dwExitCode;
    wchar_t	*cmdline;
    SetConsoleCtrlHandler(LogoffHandlerRoutine, TRUE);
    cmdline = GetCommandLineW();
    assert(cmdline);
    CommandLineStart = (wchar_t *) malloc((wcslen(cmdline) + 1)*sizeof(wchar_t));
    assert(CommandLineStart);
    wcscpy(CommandLineStart,cmdline);
    split_commandline();
    parse_commandline();
    complete_options();
    dwExitCode = start_new_node();
    return( (int) dwExitCode );
}