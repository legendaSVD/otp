#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "netdb.h"
#include "erl_resolv.h"
struct hostent *erl_gethostbyname(name)
     char *name;
{
    return gethostbyname(name);
}
struct hostent *erl_gethostbyaddr(addr, len, type)
     char *addr;
     int len;
     int type;
{
    return gethostbyaddr(addr, len, type);
}