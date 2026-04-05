#include <windows.h>
#include "erl_driver.h"
#include "sys.h"
#define CMD_GET_CURRENT		0
#define CMD_OPEN_KEY		1
#define CMD_CREATE_KEY 		2
#define CMD_GET_ALL_SUBKEYS	3
#define CMD_GET_VALUE		4
#define CMD_GET_ALL_VALUES	5
#define CMD_SET_VALUE		6
#define CMD_DELETE_KEY		7
#define CMD_DELETE_VALUE	8
extern void _dosmaperr(DWORD);
typedef struct {
    ErlDrvPort port;
    REGSAM sam;
    HKEY hkey;
    HKEY hkey_root;
    char* key;
    DWORD key_size;
    LPSTR name_buf;
    DWORD name_buf_size;
    LPSTR value_buf;
    DWORD value_buf_size;
} RegPort;
static void reply(RegPort* rp, LONG result);
static BOOL fix_value_result(RegPort* rp, LONG result, DWORD type,
			     LPSTR name, DWORD nameSize, LPSTR value,
			     DWORD valueSize);
static int key_reply(RegPort* rp, LPSTR name, DWORD nameSize);
static int value_reply(RegPort* rp, DWORD type, LPSTR name, DWORD nameSize,
		       LPSTR value, DWORD valueSize);
static int state_reply(RegPort* rp, HKEY root, LPSTR name, DWORD nameSize);
static int maperror(DWORD error);
static int reg_init(void);
static ErlDrvData reg_start(ErlDrvPort, char*);
static void reg_stop(ErlDrvData);
static void reg_from_erlang(ErlDrvData, char*, ErlDrvSizeT);
struct erl_drv_entry registry_driver_entry = {
    reg_init,
    reg_start,
    reg_stop,
    reg_from_erlang,
    NULL,
    NULL,
    "registry__drv__",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    0,
    NULL,
    NULL,
    NULL,
};
static int
reg_init(void)
{
    DEBUGF(("reg_init()\n"));
    return 0;
}
static ErlDrvData
reg_start(ErlDrvPort port, char* buf)
{
    RegPort* rp;
    char* s;
    REGSAM sam = KEY_READ;
    if ((s = strchr(buf, ' ')) != NULL) {
	while (isspace(*s))
	    s++;
	while (*s != '\0') {
	    if (*s == 'r') {
		sam |= KEY_READ;
	    } else if (*s == 'w') {
		sam |= KEY_WRITE;
	    }
	    s++;
	}
    }
    rp = driver_alloc(sizeof(RegPort));
    if (rp == NULL) {
	return ERL_DRV_ERROR_GENERAL;
    }
    rp->port = port;
    rp->hkey = rp->hkey_root = HKEY_CLASSES_ROOT;
    rp->sam = sam;
    rp->key = driver_alloc(1);
    rp->key_size = 0;
    rp->name_buf_size = 64;
    rp->name_buf = driver_alloc(rp->name_buf_size);
    rp->value_buf_size = 64;
    rp->value_buf = driver_alloc(rp->value_buf_size);
    return (ErlDrvData) rp;
}
static void
reg_stop(ErlDrvData clientData)
{
    RegPort* rp = (RegPort *) clientData;
    (void) RegCloseKey(rp->hkey);
    driver_free(rp->name_buf);
    driver_free(rp->value_buf);
    driver_free(rp->key);
    driver_free(rp);
}
static void
reg_from_erlang(ErlDrvData clientData, char* buf, ErlDrvSizeT count)
{
    RegPort* rp = (RegPort *) clientData;
    int cmd;
    HKEY hkey;
    LONG result;
    DWORD nameSize;
    DWORD type;
    DWORD valueSize;
    cmd = buf[0];
    buf++, count--;
    switch (cmd) {
    case CMD_GET_CURRENT:
	state_reply(rp, rp->hkey_root, rp->key, rp->key_size);
	break;
    case CMD_OPEN_KEY:
	{
	    char* key;
	    HKEY newKey;
	    hkey = (HKEY)(SWord) get_int32(buf+0);
	    rp->hkey_root = hkey;
	    key = buf+4;
	    result = RegOpenKeyEx(hkey, key, 0, rp->sam, &newKey);
	    if (result == ERROR_SUCCESS) {
		RegCloseKey(rp->hkey);
		rp->hkey = newKey;
		driver_free(rp->key);
		rp->key_size = strlen(key);
		rp->key = driver_alloc(rp->key_size+1);
		strcpy(rp->key, key);
	    }
	    reply(rp, result);
	    return;
	}
	break;
    case CMD_CREATE_KEY:
	{
	    char* key;
	    HKEY newKey;
	    DWORD disposition;
	    hkey = (HKEY)(SWord) get_int32(buf+0);
	    rp->hkey_root = hkey;
	    key = buf+4;
	    result = RegCreateKeyEx(hkey, key, 0, "", 0, rp->sam, NULL,
				    &newKey, &disposition);
	    if (result == ERROR_SUCCESS) {
		RegCloseKey(rp->hkey);
		rp->hkey = newKey;
		driver_free(rp->key);
		rp->key_size = strlen(key);
		rp->key = driver_alloc(rp->key_size+1);
		strcpy(rp->key, key);
	    }
	    reply(rp, result);
	    return;
	}
	break;
    case CMD_GET_ALL_SUBKEYS:
	{
	    int i;
	    i = 0;
	    for (;;) {
		nameSize = rp->name_buf_size;
		result = RegEnumKeyEx(rp->hkey, i, rp->name_buf, &nameSize,
				      NULL, NULL, NULL, NULL);
		if (result == ERROR_MORE_DATA) {
		    rp->name_buf_size *= 2;
		    rp->name_buf = driver_realloc(rp->name_buf,
						  rp->name_buf_size);
		    continue;
		} else if (result == ERROR_NO_MORE_ITEMS) {
		    reply(rp, ERROR_SUCCESS);
		    return;
		} else if (result != ERROR_SUCCESS) {
		    reply(rp, result);
		    return;
		}
		key_reply(rp, rp->name_buf, nameSize);
		i++;
	    }
	}
	break;
    case CMD_GET_VALUE:
	do {
	    valueSize = rp->value_buf_size;
	    result = RegQueryValueEx(rp->hkey, buf, NULL, &type,
				     rp->value_buf, &valueSize);
	} while (!fix_value_result(rp, result, type, buf, strlen(buf),
				   rp->value_buf, valueSize));
	break;
    case CMD_GET_ALL_VALUES:
	{
	    int i;
	    i = 0;
	    for (;;) {
		nameSize = rp->name_buf_size;
		valueSize = rp->value_buf_size;
		result = RegEnumValue(rp->hkey, i, rp->name_buf, &nameSize,
				      NULL, &type, rp->value_buf, &valueSize);
		if (result == ERROR_NO_MORE_ITEMS) {
		    reply(rp, ERROR_SUCCESS);
		    return;
		}
		if (fix_value_result(rp, result, type, rp->name_buf, nameSize,
				     rp->value_buf, valueSize)) {
		    i++;
		}
	    }
	}
	break;
    case CMD_SET_VALUE:
	{
	    LPSTR name;
	    DWORD dword;
	    type = get_int32(buf);
	    buf += 4;
	    count -= 4;
	    name = buf;
	    nameSize = strlen(buf) + 1;
	    buf += nameSize;
	    count -= nameSize;
	    if (type == REG_DWORD) {
		dword = get_int32(buf);
		buf = (char *) &dword;
		ASSERT(count == 4);
	    }
	    result = RegSetValueEx(rp->hkey, name, 0, type, buf, (DWORD)count);
	    reply(rp, result);
	}
	break;
    case CMD_DELETE_KEY:
	{
	    char* key = buf;
	    result = RegDeleteKey(rp->hkey, key);
	    reply(rp, result);
	}
	break;
    case CMD_DELETE_VALUE:
	result = RegDeleteValue(rp->hkey, buf);
	reply(rp, result);
	break;
    }
}
static BOOL
fix_value_result(RegPort* rp, LONG result, DWORD type,
		 LPSTR name, DWORD nameSize, LPSTR value, DWORD valueSize)
{
    if (result == ERROR_MORE_DATA) {
	DWORD max_name1;
	DWORD max_name2;
	DWORD max_value;
	int ok;
	ok = RegQueryInfoKey(rp->hkey, NULL, NULL, NULL,
			     NULL, &max_name1, NULL, NULL, &max_name2,
			     &max_value, NULL, NULL);
#ifdef DEBUG
	if (ok != ERROR_SUCCESS) {
	    char buff[256];
	    erts_snprintf(buff, sizeof(buff), "Failure in registry_drv line %d, error = %d",
		    __LINE__, GetLastError());
	    MessageBox(NULL, buff, "Internal error", MB_OK);
	    ASSERT(ok == ERROR_SUCCESS);
	}
#endif
	rp->name_buf_size = (max_name1 > max_name2 ? max_name1 : max_name2)
	    + 1;
	rp->value_buf_size = max_value + 1;
	rp->name_buf = driver_realloc(rp->name_buf, rp->name_buf_size);
	rp->value_buf = driver_realloc(rp->value_buf, rp->value_buf_size);
	return FALSE;
    } else if (result != ERROR_SUCCESS) {
	reply(rp, result);
	return TRUE;
    }
    switch (type) {
    case REG_SZ:
    case REG_EXPAND_SZ:
        if (valueSize > 0 && value[valueSize - 1] == '\0') {
            valueSize--;
        }
        break;
    case REG_DWORD_LITTLE_ENDIAN:
    case REG_DWORD_BIG_ENDIAN:
	{
	    DWORD dword = * ((DWORD *) value);
	    put_int32(dword, value);
	    type = REG_DWORD;
	    break;
	}
    }
    return value_reply(rp, type, name, nameSize, value, valueSize);
}
static void
reply(RegPort* rp, LONG result)
{
    char sbuf[256];
    if (result == ERROR_SUCCESS) {
	sbuf[0] = 'o';
	driver_output(rp->port, sbuf, 1);
    } else {
	char* s;
	char* t;
	int err;
	sbuf[0] = 'e';
	err = maperror(result);
	for (s = erl_errno_id(err), t = sbuf+1; *s; s++, t++) {
	    *t = tolower(*s);
	}
	driver_output(rp->port, sbuf, t-sbuf);
    }
}
static int
key_reply(RegPort* rp,
	  LPSTR name,
	  DWORD nameSize)
{
    char sbuf[512];
    char* s = sbuf;
    int needed = 1+nameSize;
    if (sizeof sbuf < needed) {
	s = driver_alloc(needed);
    }
    s[0] = 'k';
    memcpy(s+1, name, nameSize);
    driver_output(rp->port, s, needed);
    if (s != sbuf) {
	driver_free(s);
    }
    return 1;
}
static int
value_reply(RegPort* rp,
	    DWORD type,
	    LPSTR name,
	    DWORD nameSize,
	    LPSTR value,
	    DWORD valueSize)
{
    char sbuf[512];
    char* s = sbuf;
    int needed = 1+4+nameSize+1+valueSize;
    int i;
    if (sizeof sbuf < needed) {
	s = driver_alloc(needed);
    }
    s[0] = 'v';
    i = 1;
    put_int32(type, s+i);
    i += 4;
    memcpy(s+i, name, nameSize);
    i += nameSize;
    s[i++] = '\0';
    memcpy(s+i, value, valueSize);
    ASSERT(i+valueSize == needed);
    driver_output(rp->port, s, needed);
    if (s != sbuf) {
	driver_free(s);
    }
    return 1;
}
static int
state_reply(RegPort* rp,
	    HKEY root,
	    LPSTR name,
	    DWORD nameSize)
{
    char sbuf[512];
    char* s = sbuf;
    int needed = 1+4+nameSize;
    int i;
    if (sizeof sbuf < needed) {
	s = driver_alloc(needed);
    }
    s[0] = 's';
    i = 1;
    put_int32((DWORD)(SWord) root, s+i);
    i += 4;
    memcpy(s+i, name, nameSize);
    ASSERT(i+nameSize == needed);
    driver_output(rp->port, s, needed);
    if (s != sbuf) {
	driver_free(s);
    }
    return 1;
}
static int
maperror(DWORD error)
{
    DEBUGF(("Mapping %d\n", error));
    switch (error) {
    case ERROR_BADDB:
    case ERROR_BADKEY:
    case ERROR_CANTOPEN:
    case ERROR_CANTWRITE:
    case ERROR_REGISTRY_RECOVERED:
    case ERROR_REGISTRY_CORRUPT:
    case ERROR_REGISTRY_IO_FAILED:
    case ERROR_NOT_REGISTRY_FILE:
	return EIO;
    case ERROR_KEY_DELETED:
	return EINVAL;
    default:
	_dosmaperr(error);
	return errno;
    }
}