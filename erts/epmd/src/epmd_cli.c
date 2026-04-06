#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "epmd.h"
#include "epmd_int.h"
#include "erl_printf.h"
static int conn_to_epmd(EpmdVars*);
static int read_fill(int,char*,int);
void kill_epmd(EpmdVars *g)
{
    char buf[5];
    int fd, rval;
    fd = conn_to_epmd(g);
    put_int16(1,buf);
    buf[2] = EPMD_KILL_REQ;
    if (write(fd, buf, 3) != 3) {
	printf("epmd: Can't write to epmd\n");
	epmd_cleanup_exit(g,1);
    }
    if ((rval = read_fill(fd,buf,2)) == 2) {
	if (buf[0] == 'O' && buf[1] == 'K') {
	    printf("Killed\n");
	    epmd_cleanup_exit(g,0);
	} else {
	    printf("Killing not allowed - living nodes in database.\n");
	    epmd_cleanup_exit(g,1);
	}
    } else if (rval < 0) {
	printf("epmd: failed to read answer from local epmd\n");
	epmd_cleanup_exit(g,1);
    } else {
	buf[rval] = '\0';
	printf("epmd: local epmd responded with <%s>\n", buf);
	epmd_cleanup_exit(g,1);
    }
}
void stop_cli(EpmdVars *g, char *name)
{
    char buf[1024];
    int fd, rval, bsize;
    bsize = strlen(name);
    if (bsize > 1000) {
	printf("epmd: Name too long!");
	epmd_cleanup_exit(g, 1);
    }
    fd = conn_to_epmd(g);
    bsize++;
    put_int16(bsize, buf);
    buf[2] = EPMD_STOP_REQ;
    bsize += 2;
    strcpy(buf+3, name);
    if (write(fd, buf, bsize) != bsize) {
	printf("epmd: Can't write to epmd\n");
	epmd_cleanup_exit(g,1);
    }
    if ((rval = read_fill(fd,buf,7)) == 7) {
	buf[7] = '\000';
	printf("%s\n", buf);
	epmd_cleanup_exit(g,0);
    } else if (rval < 0) {
	printf("epmd: failed to read answer from local epmd\n");
	epmd_cleanup_exit(g,1);
    } else {
	buf[rval] = '\0';
	printf("epmd: local epmd responded with <%s>\n", buf);
	epmd_cleanup_exit(g,1);
    }
}
void epmd_call(EpmdVars *g,int what)
{
    char buf[OUTBUF_SIZE];
    int rval,fd,i,j;
    fd = conn_to_epmd(g);
    put_int16(1,buf);
    buf[2] = what;
    if (write(fd, buf, 3) != 3) {
	printf("epmd: Can't write to epmd\n");
	epmd_cleanup_exit(g,1);
    }
    if (read(fd,(char *)&i,4) != 4) {
	if (!g->silent)
	    printf("epmd: no response from local epmd\n");
	epmd_cleanup_exit(g,1);
    }
    j = ntohl(i);
    if (!g->silent) {
	rval = erts_snprintf(buf, OUTBUF_SIZE,
			     "epmd: up and running on port %d with data:\n", j);
	fwrite(buf, 1, rval, stdout);
    }
    while(1) {
	if ((rval = read(fd,buf,OUTBUF_SIZE)) <= 0)  {
	    close(fd);
	    epmd_cleanup_exit(g,0);
	}
	if (!g->silent)
	    fwrite(buf, 1, rval, stdout);
    }
}
static int conn_to_epmd(EpmdVars *g)
{
    struct EPMD_SOCKADDR_IN address;
    size_t salen = 0;
    int connect_sock;
    unsigned short sport = g->port;
#if defined(EPMD6)
    SET_ADDR6(address, in6addr_loopback, sport);
    salen = sizeof(struct sockaddr_in6);
    connect_sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (connect_sock>=0) {
    if (connect(connect_sock, (struct sockaddr*)&address, salen) == 0)
	return connect_sock;
    close(connect_sock);
    }
#endif
    SET_ADDR(address, htonl(INADDR_LOOPBACK), sport);
    salen = sizeof(struct sockaddr_in);
    connect_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_sock<0)
	goto error;
    if (connect(connect_sock, (struct sockaddr*)&address, salen) < 0)
	goto error;
    return connect_sock;
 error:
    if (!g->silent) {
	fprintf(stderr, "epmd: Cannot connect to local epmd\n");
    }
    epmd_cleanup_exit(g,1);
    return -1;
}
static int read_fill(int fd,char *buf,int len)
{
    int i;
    int got = 0;
    do {
	if ((i = read(fd, buf+got, len-got)) <= 0)
	    return (i);
	got += i;
    } while (got < len);
    return (len);
}