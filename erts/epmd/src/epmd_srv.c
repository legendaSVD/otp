#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "epmd.h"
#include "epmd_int.h"
#include "erl_printf.h"
#ifdef __clang_analyzer__
#  undef FD_ZERO
#  define FD_ZERO(FD_SET_PTR) memset(FD_SET_PTR, 0, sizeof(fd_set))
#endif
#ifndef INADDR_NONE
#  define INADDR_NONE 0xffffffff
#endif
static void do_request(EpmdVars*,int,Connection*,char*,int);
static int do_accept(EpmdVars*,int);
static void do_read(EpmdVars*,Connection*);
static time_t current_time(EpmdVars*);
static Connection *conn_init(EpmdVars*);
static int conn_open(EpmdVars*,int);
static int conn_local_peer_check(EpmdVars*, int);
static int conn_close_fd(EpmdVars*,int);
static void node_init(EpmdVars*);
static Node *node_reg2(EpmdVars*, int, char*, int, int, unsigned char, unsigned char, int, int, int, char*);
static int node_unreg(EpmdVars*,char*);
static int node_unreg_sock(EpmdVars*,int);
static int reply(EpmdVars*,int,char *,int);
static void dbg_print_buf(EpmdVars*,char *,int);
static void print_names(EpmdVars*);
static int is_same_str(char *x, char *y)
{
    int i = 0;
    while (x[i] == y[i]) {
	if (x[i] == '\0')
	    return 1;
	i++;
    }
    return 0;
}
static int copy_str(char *x, char *y)
{
    int i = 0;
    while (1) {
	x[i] = y[i];
	if (y[i] == '\0')
	    return i;
	i++;
    }
}
static int length_str(char *x)
{
    int i = 0;
    while (x[i])
	i++;
    return i;
}
static int verify_utf8(const char *src, int sz, int null_term)
{
    unsigned char *source = (unsigned char *) src;
    int size = sz;
    int num_chars = 0;
    while (size) {
	if (null_term && (*source) == 0)
	    return num_chars;
	if (((*source) & ((unsigned char) 0x80)) == 0) {
	    source++;
	    --size;
	} else if (((*source) & ((unsigned char) 0xE0)) == 0xC0) {
	    if (size < 2)
		return -1;
	    if (((source[1] & ((unsigned char) 0xC0)) != 0x80) ||
		((*source) < 0xC2) ) {
		return -1;
	    }
	    source += 2;
	    size -= 2;
	} else if (((*source) & ((unsigned char) 0xF0)) == 0xE0) {
	    if (size < 3)
		return -1;
	    if (((source[1] & ((unsigned char) 0xC0)) != 0x80) ||
		((source[2] & ((unsigned char) 0xC0)) != 0x80) ||
		(((*source) == 0xE0) && (source[1] < 0xA0))  ) {
		return -1;
	    }
	    if ((((*source) & ((unsigned char) 0xF)) == 0xD) &&
		((source[1] & 0x20) != 0)) {
		return -1;
	    }
	    source += 3;
	    size -= 3;
	} else if (((*source) & ((unsigned char) 0xF8)) == 0xF0) {
	    if (size < 4)
		return -1;
	    if (((source[1] & ((unsigned char) 0xC0)) != 0x80) ||
		((source[2] & ((unsigned char) 0xC0)) != 0x80) ||
		((source[3] & ((unsigned char) 0xC0)) != 0x80) ||
		(((*source) == 0xF0) && (source[1] < 0x90)) ) {
		return -1;
	    }
	    if ((((*source) & ((unsigned char)0x7)) > 0x4U) ||
		((((*source) & ((unsigned char)0x7)) == 0x4U) &&
		 ((source[1] & ((unsigned char)0x3F)) > 0xFU))) {
		return -1;
	    }
	    source += 4;
	    size -= 4;
	} else {
	    return -1;
	}
	++num_chars;
    }
    return num_chars;
}
static EPMD_INLINE void select_fd_set(EpmdVars* g, int fd)
{
    FD_SET(fd, &g->orig_read_mask);
    if (fd >= g->select_fd_top) {
	g->select_fd_top = fd + 1;
    }
}
#ifdef HAVE_SOCKLEN_T
static const char *epmd_ntop(struct sockaddr_storage *sa, char *buff, socklen_t len) {
#else
static const char *epmd_ntop(struct sockaddr_storage *sa, char *buff, size_t len) {
#endif
    int myerrno = errno;
    const char *res;
#if defined(EPMD6)
    if (sa->ss_family == AF_INET6) {
        struct sockaddr_in6 *addr = (struct sockaddr_in6 *)sa;
        res = inet_ntop(
            addr->sin6_family,
            (void*)&addr->sin6_addr,
            buff, len);
    } else {
        struct sockaddr_in *addr = (struct sockaddr_in *)sa;
        res = inet_ntop(
            addr->sin_family, &addr->sin_addr.s_addr,
            buff, len);
    }
#else
    struct sockaddr_in *addr = (struct sockaddr_in *)sa;
    res = inet_ntoa(addr->sin_addr);
    erts_snprintf(buff,len,"%s", res);
    res = buff;
#endif
    errno = myerrno;
    return res;
}
void run(EpmdVars *g)
{
  struct EPMD_SOCKADDR_IN iserv_addr[MAX_LISTEN_SOCKETS];
  int listensock[MAX_LISTEN_SOCKETS];
#if defined(EPMD6)
  char socknamebuf[INET6_ADDRSTRLEN];
#else
  char socknamebuf[INET_ADDRSTRLEN];
#endif
  int num_sockets = 0;
  int nonfatal_sockets = 0;
  int i;
  int opt;
  unsigned short sport = g->port;
  int bound = 0;
  node_init(g);
  g->conn = conn_init(g);
#ifdef HAVE_SYSTEMD_DAEMON
  if (g->is_systemd)
    {
      int n;
      dbg_printf(g,2,"try to obtain sockets from systemd");
      n = sd_listen_fds(0);
      if (n < 0)
        {
          dbg_perror(g,"cannot obtain sockets from systemd");
          epmd_cleanup_exit(g,1);
        }
      else if (n == 0)
        {
          dbg_tty_printf(g,0,"systemd provides no sockets");
          epmd_cleanup_exit(g,1);
      }
      else if (n > MAX_LISTEN_SOCKETS)
      {
          dbg_tty_printf(g,0,"cannot listen on more than %d IP addresses", MAX_LISTEN_SOCKETS);
          epmd_cleanup_exit(g,1);
      }
      num_sockets = n;
      for (i = 0; i < num_sockets; i++)
        {
          g->listenfd[i] = listensock[i] = SD_LISTEN_FDS_START + i;
        }
    }
  else
    {
#endif
  dbg_printf(g,2,"try to initiate listening port %d", g->port);
  if (g->addresses != NULL &&
      g->addresses[strspn(g->addresses," ,")] != '\000')
    {
      char *tmp = NULL;
      char *token = NULL;
      SET_ADDR(iserv_addr[num_sockets],htonl(INADDR_LOOPBACK),sport);
      num_sockets++;
#if defined(EPMD6)
      SET_ADDR6(iserv_addr[num_sockets],in6addr_loopback,sport);
      num_sockets++;
#endif
      nonfatal_sockets = num_sockets;
	  if ((tmp = strdup(g->addresses)) == NULL)
	{
	  dbg_perror(g,"cannot allocate memory");
	  epmd_cleanup_exit(g,1);
	}
      for(token = strtok(tmp,", ");
	  token != NULL;
	  token = strtok(NULL,", "))
	{
	  struct in_addr addr;
#if defined(EPMD6)
	  struct in6_addr addr6;
	  struct sockaddr_storage *sa = &iserv_addr[num_sockets];
	  if (inet_pton(AF_INET6,token,&addr6) == 1)
	    {
	      SET_ADDR6(iserv_addr[num_sockets],addr6,sport);
	    }
	  else if (inet_pton(AF_INET,token,&addr) == 1)
	    {
	      SET_ADDR(iserv_addr[num_sockets],addr.s_addr,sport);
	    }
	  else
#else
	  if ((addr.s_addr = inet_addr(token)) != INADDR_NONE)
	    {
	      SET_ADDR(iserv_addr[num_sockets],addr.s_addr,sport);
	    }
	  else
#endif
	    {
	      dbg_tty_printf(g,0,"cannot parse IP address \"%s\"",token);
	      epmd_cleanup_exit(g,1);
	    }
#if defined(EPMD6)
	  if (sa->ss_family == AF_INET6 && IN6_IS_ADDR_LOOPBACK(&addr6))
	      continue;
	  if (sa->ss_family == AF_INET)
#endif
	  if (IS_ADDR_LOOPBACK(addr))
	    continue;
	  num_sockets++;
	  if (num_sockets >= MAX_LISTEN_SOCKETS)
	    {
	      dbg_tty_printf(g,0,"cannot listen on more than %d IP addresses",
			     MAX_LISTEN_SOCKETS);
	      epmd_cleanup_exit(g,1);
	    }
	}
      free(tmp);
    }
  else
    {
      SET_ADDR(iserv_addr[num_sockets],htonl(INADDR_ANY),sport);
      num_sockets++;
#if defined(EPMD6)
      SET_ADDR6(iserv_addr[num_sockets],in6addr_any,sport);
      num_sockets++;
#endif
    }
#ifdef HAVE_SYSTEMD_DAEMON
    }
#endif
#if !defined(__WIN32__)
  signal(SIGPIPE, SIG_IGN);
#endif
  g->active_conn = 3 + num_sockets;
  g->max_conn -= num_sockets;
  FD_ZERO(&g->orig_read_mask);
  g->select_fd_top = 0;
#ifdef HAVE_SYSTEMD_DAEMON
  if (g->is_systemd)
      for (i = 0; i < num_sockets; i++)
          select_fd_set(g, listensock[i]);
  else
    {
#endif
  for (i = 0; i < num_sockets; i++)
    {
      struct sockaddr *sa = (struct sockaddr *)&iserv_addr[i];
#if defined(EPMD6)
      size_t salen = (sa->sa_family == AF_INET6 ?
              sizeof(struct sockaddr_in6) :
              sizeof(struct sockaddr_in));
#else
      size_t salen = sizeof(struct sockaddr_in);
#endif
      if ((listensock[i] = socket(sa->sa_family,SOCK_STREAM,0)) < 0)
	{
	  switch (errno) {
	      case EAFNOSUPPORT:
	      case EPROTONOSUPPORT:
	          continue;
	      default:
	          dbg_perror(g,"error opening stream socket");
	          epmd_cleanup_exit(g,1);
	  }
	}
#if HAVE_DECL_IPV6_V6ONLY
      opt = 1;
      if (sa->sa_family == AF_INET6 &&
          setsockopt(listensock[i],IPPROTO_IPV6,IPV6_V6ONLY,&opt,
              sizeof(opt)) <0)
	{
	  dbg_perror(g,"can't set IPv6 only socket option");
	  epmd_cleanup_exit(g,1);
	}
#endif
#if !defined(__WIN32__)
      opt = 1;
      if (setsockopt(listensock[i],SOL_SOCKET,SO_REUSEADDR,(char* ) &opt,
		     sizeof(opt)) <0)
	{
	  dbg_perror(g,"can't set sockopt");
	  epmd_cleanup_exit(g,1);
	}
#endif
#if (defined(__WIN32__) || defined(NO_FCNTL))
      opt = 1;
      if (ioctl(listensock[i], FIONBIO, &opt) != 0)
#else
      opt = fcntl(listensock[i], F_GETFL, 0);
      if (fcntl(listensock[i], F_SETFL, opt | O_NONBLOCK) == -1)
#endif
	dbg_perror(g,"failed to set non-blocking mode of listening socket %d on ipaddr %s",
		   listensock[i], epmd_ntop(&iserv_addr[i],
                                            socknamebuf, sizeof(socknamebuf)));
      if (bind(listensock[i], sa, salen) < 0)
	{
	  if (errno == EADDRINUSE)
	    {
	      dbg_tty_printf(g,1,"there is already a epmd running at port %d on ipaddr %s",
			     g->port, epmd_ntop(&iserv_addr[i],
                                                socknamebuf, sizeof(socknamebuf)));
	      epmd_cleanup_exit(g,0);
	    }
	  else
	    {
              dbg_perror(g,"failed to bind on ipaddr %s",
                         epmd_ntop(&iserv_addr[i],
                                   socknamebuf, sizeof(socknamebuf)));
              if (i >= nonfatal_sockets)
                  epmd_cleanup_exit(g,1);
              else
              {
                  close(listensock[i]);
                  continue;
              }
	    }
	}
      if(listen(listensock[i], SOMAXCONN) < 0) {
          dbg_perror(g,"failed to listen on ipaddr %s",
                     epmd_ntop(&iserv_addr[i],
                               socknamebuf, sizeof(socknamebuf)));
          epmd_cleanup_exit(g,1);
      }
      g->listenfd[bound++] = listensock[i];
      select_fd_set(g, listensock[i]);
    }
  if (bound == 0) {
      dbg_perror(g,"unable to bind any address");
      epmd_cleanup_exit(g,1);
  }
  num_sockets = bound;
#ifdef HAVE_SYSTEMD_DAEMON
    }
    if (g->is_systemd) {
      sd_notifyf(0, "READY=1\n"
                    "STATUS=Processing port mapping requests...\n"
                    "MAINPID=%lu", (unsigned long) getpid());
    }
#endif
  dbg_tty_printf(g,2,"entering the main select() loop");
 select_again:
  while(1)
    {
      fd_set read_mask = g->orig_read_mask;
      struct timeval timeout;
      int ret;
      timeout.tv_sec = (g->packet_timeout < IDLE_TIMEOUT) ? 1 : IDLE_TIMEOUT;
      timeout.tv_usec = 0;
      if ((ret = select(g->select_fd_top,
			&read_mask, (fd_set *)0,(fd_set *)0,&timeout)) < 0) {
	dbg_perror(g,"error in select ");
        switch (errno) {
          case EAGAIN:
          case EINTR:
            break;
          default:
            epmd_cleanup_exit(g,1);
        }
      }
      else {
	time_t now;
	if (ret == 0) {
	  FD_ZERO(&read_mask);
	}
	if (g->delay_accept) {
	  sleep(g->delay_accept);
	}
	for (i = 0; i < num_sockets; i++)
	  if (FD_ISSET(g->listenfd[i],&read_mask)) {
	    if (do_accept(g, g->listenfd[i]) && g->active_conn < g->max_conn) {
	      goto select_again;
	    }
	  }
	now = current_time(g);
	for (i = 0; i < g->max_conn; i++) {
	  if (g->conn[i].open == EPMD_TRUE) {
	    if (FD_ISSET(g->conn[i].fd,&read_mask))
	      do_read(g,&g->conn[i]);
	    else if ((g->conn[i].keep == EPMD_FALSE) &&
		     ((g->conn[i].mod_time + g->packet_timeout) < now)) {
	      dbg_tty_printf(g,1,"closing because timed out on receive");
	      epmd_conn_close(g,&g->conn[i]);
	    }
	  }
	}
      }
    }
}
static void do_read(EpmdVars *g,Connection *s)
{
  int val, pack_size;
  if (s->open == EPMD_FALSE)
    {
      dbg_printf(g,0,"read on unknown socket");
      return;
    }
  if (s->keep == EPMD_TRUE)
    {
      val = read(s->fd, s->buf, INBUF_SIZE);
      if (val == 0)
	{
	  node_unreg_sock(g,s->fd);
	  epmd_conn_close(g,s);
	}
      else if (val < 0)
	{
	    dbg_tty_printf(g,1,"error on ALIVE socket %d (%d; errno=0x%x)",
			   s->fd, val, errno);
	  node_unreg_sock(g,s->fd);
	  epmd_conn_close(g,s);
	}
      else
	{
	  dbg_tty_printf(g,1,"got more than expected on ALIVE socket %d (%d)",
		 s->fd,val);
	  dbg_print_buf(g,s->buf,val);
	  node_unreg_sock(g,s->fd);
	  epmd_conn_close(g,s);
	}
      return;
    }
  pack_size = s->want ? s->want : INBUF_SIZE - 1;
  val = read(s->fd, s->buf + s->got, pack_size - s->got);
  if (val == 0)
    {
      dbg_printf(g,0,"got partial packet only on file descriptor %d (%d)",
		 s->fd,s->got);
      epmd_conn_close(g,s);
      return;
    }
  if (val < 0)
    {
      dbg_perror(g,"error in read");
      epmd_conn_close(g,s);
      return;
    }
  dbg_print_buf(g,s->buf,val);
  s->got += val;
  if ((s->want == 0) && (s->got >= 2))
    {
      s->want = get_int16(s->buf) + 2;
      if ((s->want < 3) || (s->want >= INBUF_SIZE))
	{
	  dbg_printf(g,0,"invalid packet size (%d)",s->want - 2);
	  epmd_conn_close(g,s);
	  return;
	}
      if (s->got > s->want)
	{
	  dbg_printf(g,0,"got %d bytes in packet, expected %d",
		     s->got - 2, s->want - 2);
	  epmd_conn_close(g,s);
	  return;
	}
    }
  s->mod_time = current_time(g);
  if (s->want == s->got)
    {
      do_request(g, s->fd, s, s->buf + 2, s->got - 2);
      if (!s->keep)
	epmd_conn_close(g,s);
    }
}
static int do_accept(EpmdVars *g,int listensock)
{
    int msgsock;
    struct EPMD_SOCKADDR_IN icli_addr;
    int icli_addr_len;
    icli_addr_len = sizeof(icli_addr);
    msgsock = accept(listensock,(struct sockaddr*) &icli_addr,
		     (unsigned int*) &icli_addr_len);
    if (msgsock < 0) {
        dbg_perror(g,"error in accept");
        switch (errno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
	            return EPMD_FALSE;
            default:
	            epmd_cleanup_exit(g,1);
        }
    }
    return conn_open(g,msgsock);
}
static void bump_creation(Node* node)
{
    if (++node->cr_counter < 4)
        node->cr_counter = 4;
}
static unsigned int get_creation(Node* node)
{
    if (node->highvsn >= 6) {
        return node->cr_counter;
    }
    else {
        return node->cr_counter % 3 + 1;
    }
}
static void do_request(EpmdVars *g, int fd, Connection *s, char *buf, int bsize)
{
  char wbuf[OUTBUF_SIZE];
  int i;
  buf[bsize] = '\0';
  switch (*buf)
    {
    case EPMD_ALIVE2_REQ:
      dbg_printf(g,1,"** got ALIVE2_REQ");
      if (!s->local_peer) {
	   dbg_printf(g,0,"ALIVE2_REQ from non local address");
	   return;
      }
      if (bsize <= 13)
	{
	  dbg_printf(g,0,"packet to small for request ALIVE2_REQ (%d)",bsize);
	  return;
	}
      {
	Node *node;
	int eport;
	unsigned char nodetype;
	unsigned char protocol;
	unsigned short highvsn;
	unsigned short lowvsn;
        unsigned int creation;
	int namelen;
	int extralen;
        int replylen;
	char *name;
	char *extra;
	eport = get_int16(&buf[1]);
	nodetype = buf[3];
	protocol = buf[4];
	highvsn = get_int16(&buf[5]);
	lowvsn = get_int16(&buf[7]);
	namelen = get_int16(&buf[9]);
	if (namelen + 13 > bsize) {
	    dbg_printf(g,0,"Node name size error in ALIVE2_REQ");
	    return;
	}
	extralen = get_int16(&buf[11+namelen]);
	if (extralen + namelen + 13 > bsize) {
	    dbg_printf(g,0,"Extra info size error in ALIVE2_REQ");
	    return;
	}
	for (i = 11 ; i < 11 + namelen; i ++)
	    if (buf[i] == '\000')  {
		dbg_printf(g,0,"node name contains ascii 0 in ALIVE2_REQ");
		return;
	    }
	name = &buf[11];
	name[namelen]='\000';
	extra = &buf[11+namelen+2];
	extra[extralen]='\000';
        node = node_reg2(g, namelen, name, fd, eport, nodetype, protocol,
                         highvsn, lowvsn, extralen, extra);
        creation = node ? get_creation(node) :  99;
        wbuf[1] = node ? 0 : 1;
        if (highvsn >= 6) {
            wbuf[0] = EPMD_ALIVE2_X_RESP;
            put_int32(creation, wbuf+2);
            replylen = 6;
        }
        else {
            wbuf[0] = EPMD_ALIVE2_RESP;
            put_int16(creation, wbuf+2);
            replylen = 4;
        }
	if (!reply(g, fd, wbuf, replylen))
	  {
            node_unreg(g, name);
	    dbg_tty_printf(g,1,"** failed to send EPMD_ALIVE2_RESP for \"%s\"",
			   name);
	    return;
	  }
	dbg_tty_printf(g,1,"** sent EPMD_ALIVE2_RESP for \"%s\"",name);
	s->keep = EPMD_TRUE;
      }
      break;
    case EPMD_PORT2_REQ:
      dbg_printf(g,1,"** got PORT2_REQ");
      if (buf[bsize - 1] == '\000')
	bsize--;
      if (bsize <= 1)
	{
	  dbg_printf(g,0,"packet too small for request PORT2_REQ (%d)", bsize);
	  return;
	}
      for (i = 1; i < bsize; i++)
	if (buf[i] == '\000')
	  {
	    dbg_printf(g,0,"node name contains ascii 0 in PORT2_REQ");
	    return;
	  }
      {
	char *name = &buf[1];
	int nsz;
	Node *node;
	nsz = verify_utf8(name, bsize, 0);
	if (nsz < 1 || 255 < nsz) {
	    dbg_printf(g,0,"invalid node name in PORT2_REQ");
	    return;
	}
	wbuf[0] = EPMD_PORT2_RESP;
	for (node = g->nodes.reg; node; node = node->next) {
	    int offset;
	    if (is_same_str(node->symname, name)) {
		wbuf[1] = 0;
		put_int16(node->port,wbuf+2);
		wbuf[4] = node->nodetype;
		wbuf[5] = node->protocol;
		put_int16(node->highvsn,wbuf+6);
		put_int16(node->lowvsn,wbuf+8);
		put_int16(length_str(node->symname),wbuf+10);
		offset = 12;
		offset += copy_str(wbuf + offset,node->symname);
		put_int16(node->extralen,wbuf + offset);
		offset += 2;
		memcpy(wbuf + offset,node->extra,node->extralen);
		offset += node->extralen;
		if (!reply(g, fd, wbuf, offset))
		  {
		    dbg_tty_printf(g,1,"** failed to send EPMD_PORT2_RESP (ok) for \"%s\"",name);
		    return;
		  }
		dbg_tty_printf(g,1,"** sent EPMD_PORT2_RESP (ok) for \"%s\"",name);
		return;
	    }
	}
	wbuf[1] = 1;
	if (!reply(g, fd, wbuf, 2))
	  {
	    dbg_tty_printf(g,1,"** failed to send EPMD_PORT2_RESP (error) for \"%s\"",name);
	    return;
	  }
	dbg_tty_printf(g,1,"** sent EPMD_PORT2_RESP (error) for \"%s\"",name);
	return;
      }
      break;
    case EPMD_NAMES_REQ:
      dbg_printf(g,1,"** got EPMD_NAMES_REQ");
      {
	Node *node;
	i = htonl(g->port);
	memcpy(wbuf,&i,4);
	if (!reply(g, fd,wbuf,4))
	  {
	    dbg_tty_printf(g,1,"failed to send NAMES_RESP");
	    return;
	  }
	for (node = g->nodes.reg; node; node = node->next)
	  {
	    int len = 0;
	    int r;
	    len += copy_str(&wbuf[len], "name ");
	    len += copy_str(&wbuf[len], node->symname);
	    r = erts_snprintf(&wbuf[len], sizeof(wbuf)-len,
			      " at port %d\n", node->port);
	    if (r < 0)
		goto failed_names_resp;
	    len += r;
	    if (!reply(g, fd, wbuf, len))
	      {
	      failed_names_resp:
		dbg_tty_printf(g,1,"failed to send NAMES_RESP");
		return;
	      }
	  }
      }
      dbg_tty_printf(g,1,"** sent NAMES_RESP");
      break;
    case EPMD_DUMP_REQ:
      dbg_printf(g,1,"** got EPMD_DUMP_REQ");
      if (!s->local_peer) {
	   dbg_printf(g,0,"EPMD_DUMP_REQ from non local address");
	   return;
      }
      {
	Node *node;
	i = htonl(g->port);
	memcpy(wbuf,&i,4);
	if (!reply(g, fd,wbuf,4))
	  {
	    dbg_tty_printf(g,1,"failed to send DUMP_RESP");
	    return;
	  }
	for (node = g->nodes.reg; node; node = node->next)
	  {
	      int len = 0, r;
	      len += copy_str(&wbuf[len], "active name     <");
	      len += copy_str(&wbuf[len], node->symname);
	      r = erts_snprintf(&wbuf[len], sizeof(wbuf)-len,
				"> at port %d, fd = %d\n",
				node->port, node->fd);
	      if (r < 0)
		  goto failed_dump_resp;
	      len += r + 1;
	      if (!reply(g, fd,wbuf,len))
	      {
	      failed_dump_resp:
		dbg_tty_printf(g,1,"failed to send DUMP_RESP");
		return;
	      }
	  }
	for (node = g->nodes.unreg; node; node = node->next)
	  {
	      int len = 0, r;
	      len += copy_str(&wbuf[len], "old/unused name <");
	      len += copy_str(&wbuf[len], node->symname);
	      r = erts_snprintf(&wbuf[len], sizeof(wbuf)-len,
				">, port = %d, fd = %d \n",
				node->port, node->fd);
	      if (r < 0)
		  goto failed_dump_resp2;
	      len += r + 1;
	      if (!reply(g, fd,wbuf,len))
	      {
	      failed_dump_resp2:
		dbg_tty_printf(g,1,"failed to send DUMP_RESP");
		return;
	      }
	  }
      }
      dbg_tty_printf(g,1,"** sent DUMP_RESP");
      break;
    case EPMD_KILL_REQ:
      if (!s->local_peer) {
	   dbg_printf(g,0,"EPMD_KILL_REQ from non local address");
	   return;
      }
      dbg_printf(g,1,"** got EPMD_KILL_REQ");
      if (!g->brutal_kill && (g->nodes.reg != NULL)) {
	  dbg_printf(g,0,"Disallowed EPMD_KILL_REQ, live nodes");
	  if (!reply(g, fd,"NO",2))
	      dbg_printf(g,0,"failed to send reply to EPMD_KILL_REQ");
	  return;
      }
      if (!reply(g, fd,"OK",2))
	dbg_printf(g,0,"failed to send reply to EPMD_KILL_REQ");
      dbg_tty_printf(g,1,"epmd killed");
      conn_close_fd(g,fd);
      dbg_printf(g,0,"got EPMD_KILL_REQ - terminates normal");
      epmd_cleanup_exit(g,0);
    case EPMD_STOP_REQ:
      dbg_printf(g,1,"** got EPMD_STOP_REQ");
      if (!s->local_peer) {
	   dbg_printf(g,0,"EPMD_STOP_REQ from non local address");
	   return;
      }
      if (!g->brutal_kill) {
	  dbg_printf(g,0,"Disallowed EPMD_STOP_REQ, no relaxed_command_check");
	  return;
      }
      if (bsize <= 1 )
	{
	  dbg_printf(g,0,"packet too small for request EPMD_STOP_REQ (%d)",bsize);
	  return;
	}
      {
	char *name = &buf[1];
	int node_fd;
	if ((node_fd = node_unreg(g,name)) < 0)
	  {
	    if (!reply(g, fd,"NOEXIST",7))
	      {
		dbg_tty_printf(g,1,"failed to send STOP_RESP NOEXIST");
		return;
	      }
	    dbg_tty_printf(g,1,"** sent STOP_RESP NOEXIST");
	  }
        else
          {
            conn_close_fd(g,node_fd);
            dbg_tty_printf(g,1,"epmd connection stopped");
          }
	if (!reply(g, fd,"STOPPED",7))
	  {
	    dbg_tty_printf(g,1,"failed to send STOP_RESP STOPPED");
	    return;
	  }
	dbg_tty_printf(g,1,"** sent STOP_RESP STOPPED");
      }
      break;
    default:
      dbg_printf(g,0,"got garbage ");
    }
}
static Connection *conn_init(EpmdVars *g)
{
  int nbytes = g->max_conn * sizeof(Connection);
  Connection *connections = (Connection *)malloc(nbytes);
  if (connections == NULL)
    {
      dbg_printf(g,0,"epmd: Insufficient memory");
#ifdef DONT_USE_MAIN
      free(g->argv);
#endif
      exit(1);
    }
  memzero(connections, nbytes);
  return connections;
}
static int conn_open(EpmdVars *g,int fd)
{
  int i;
  Connection *s;
#if !defined(__WIN32__)
  if (fd >= FD_SETSIZE) {
      dbg_tty_printf(g,0,"fd does not fit in fd_set fd=%d, FD_SETSIZE=%d",fd, FD_SETSIZE);
      close(fd);
      return EPMD_FALSE;
  }
#endif
  for (i = 0; i < g->max_conn; i++) {
    if (g->conn[i].open == EPMD_FALSE) {
      g->active_conn++;
      s = &g->conn[i];
      select_fd_set(g, fd);
      s->fd   = fd;
      s->open = EPMD_TRUE;
      s->keep = EPMD_FALSE;
      s->local_peer = conn_local_peer_check(g, s->fd);
      dbg_tty_printf(g,2,(s->local_peer) ? "Local peer connected" :
		     "Non-local peer connected");
      s->want = 0;
      s->got  = 0;
      s->mod_time = current_time(g);
      s->buf = malloc(INBUF_SIZE);
      if (s->buf == NULL) {
	dbg_printf(g,0,"epmd: Insufficient memory");
	close(fd);
	return EPMD_FALSE;
      }
      dbg_tty_printf(g,2,"opening connection on file descriptor %d",fd);
      return EPMD_TRUE;
    }
  }
  dbg_tty_printf(g,0,"failed opening connection on file descriptor %d",fd);
  close(fd);
  return EPMD_FALSE;
}
static int conn_local_peer_check(EpmdVars *g, int fd)
{
  struct EPMD_SOCKADDR_IN si;
  struct EPMD_SOCKADDR_IN di;
  struct sockaddr_in *si4 = (struct sockaddr_in *)&si;
  struct sockaddr_in *di4 = (struct sockaddr_in *)&di;
#if defined(EPMD6)
  struct sockaddr_in6 *si6 = (struct sockaddr_in6 *)&si;
  struct sockaddr_in6 *di6 = (struct sockaddr_in6 *)&di;
#endif
#ifdef HAVE_SOCKLEN_T
  socklen_t st;
#else
  int st;
#endif
  st = sizeof(si);
#ifdef __clang_analyzer__
  memset(&si, 0, sizeof(si));
#endif
  if (getpeername(fd,(struct sockaddr*) &si,&st) ||
	  st > sizeof(si)) {
	  return EPMD_FALSE;
  }
#if defined(EPMD6)
  if (si.ss_family == AF_INET6 && IN6_IS_ADDR_LOOPBACK(&(si6->sin6_addr)))
	  return EPMD_TRUE;
  if (si.ss_family == AF_INET)
#endif
  if ((((unsigned) ntohl(si4->sin_addr.s_addr)) & 0xFF000000U) ==
	  0x7F000000U)
	  return EPMD_TRUE;
  if (getsockname(fd,(struct sockaddr*) &di,&st))
	  return EPMD_FALSE;
#if defined(EPMD6)
  if (si.ss_family == AF_INET6)
      return IN6_ARE_ADDR_EQUAL( &(si6->sin6_addr), &(di6->sin6_addr));
  if (si.ss_family == AF_INET)
#endif
  return si4->sin_addr.s_addr == di4->sin_addr.s_addr;
#if defined(EPMD6)
  return EPMD_FALSE;
#endif
}
static int conn_close_fd(EpmdVars *g,int fd)
{
  int i;
  for (i = 0; i < g->max_conn; i++)
    if (g->conn[i].fd == fd)
      {
	epmd_conn_close(g,&g->conn[i]);
	return EPMD_TRUE;
      }
  return EPMD_FALSE;
}
int epmd_conn_close(EpmdVars *g,Connection *s)
{
  dbg_tty_printf(g,2,"closing connection on file descriptor %d",s->fd);
  FD_CLR(s->fd,&g->orig_read_mask);
  close(s->fd);
  s->open = EPMD_FALSE;
  if (s->buf != NULL) {
    free(s->buf);
  }
  g->active_conn--;
  return EPMD_TRUE;
}
static void node_init(EpmdVars *g)
{
  g->nodes.reg         = NULL;
  g->nodes.unreg       = NULL;
  g->nodes.unreg_tail  = NULL;
  g->nodes.unreg_count = 0;
}
static int node_unreg(EpmdVars *g,char *name)
{
  Node **prev = &g->nodes.reg;
  Node *node  = g->nodes.reg;
  for (; node; prev = &node->next, node = node->next)
    if (is_same_str(node->symname, name))
      {
	dbg_tty_printf(g,1,"unregistering '%s:%u', port %d",
		       node->symname, get_creation(node), node->port);
	*prev = node->next;
	if (g->nodes.unreg == NULL)
	  g->nodes.unreg = g->nodes.unreg_tail = node;
	else
	  {
	    g->nodes.unreg_tail->next = node;
	    g->nodes.unreg_tail = node;
	  }
	g->nodes.unreg_count++;
	node->next = NULL;
	print_names(g);
	return node->fd;
      }
  dbg_tty_printf(g,1,"trying to unregister node with unknown name %s", name);
  return -1;
}
static int node_unreg_sock(EpmdVars *g,int fd)
{
  Node **prev = &g->nodes.reg;
  Node *node  = g->nodes.reg;
  for (; node; prev = &node->next, node = node->next)
    if (node->fd == fd)
      {
	dbg_tty_printf(g,1,"unregistering '%s:%u', port %d",
		       node->symname, get_creation(node), node->port);
	*prev = node->next;
	if (g->nodes.unreg == NULL)
	  g->nodes.unreg = g->nodes.unreg_tail = node;
	else
	  {
	    g->nodes.unreg_tail->next = node;
	    g->nodes.unreg_tail = node;
	  }
	g->nodes.unreg_count++;
	node->next = NULL;
	print_names(g);
	return node->fd;
      }
  dbg_tty_printf(g,1,
		 "trying to unregister node with unknown file descriptor %d",
		 fd);
  return -1;
}
static Node *node_reg2(EpmdVars *g,
		       int namelen,
		       char* name,
		       int fd,
		       int port,
		       unsigned char nodetype,
		       unsigned char protocol,
		       int highvsn,
		       int lowvsn,
		       int extralen,
		       char* extra)
{
  Node *prev;
  Node *node;
  int sz;
  if (extra == NULL)
     extra = "";
  if (namelen > MAXSYMLEN)
    {
    too_long_name:
      dbg_printf(g,0,"node name is too long (%d) %s", namelen, name);
      return NULL;
    }
  sz = verify_utf8(name, namelen, 0);
  if (sz > 255)
      goto too_long_name;
  if (sz < 0) {
      dbg_printf(g,0,"invalid node name encoding");
      return NULL;
  }
  if (extralen > MAXSYMLEN)
    {
#if 0
    too_long_extra:
#endif
      dbg_printf(g,0,"extra data is too long (%d) %s", extralen, extra);
      return NULL;
    }
#if 0
  sz = verify_utf8(extra, extralen, 0);
  if (sz > 255)
      goto too_long_extra;
  if (sz < 0) {
      dbg_printf(g,0,"invalid extra data encoding");
      return NULL;
  }
#endif
  for (node = g->nodes.reg; node; node = node->next)
    if (is_same_str(node->symname, name))
      {
	dbg_printf(g,0,"node name already occupied %s", name);
	return NULL;
      }
  prev = NULL;
  for (node = g->nodes.unreg; node; prev = node, node = node->next)
    if (is_same_str(node->symname, name))
      {
	dbg_tty_printf(g,1,"reusing slot with same name '%s'", node->symname);
	if (prev == NULL)
	  {
	    if (node->next == NULL)
	      g->nodes.unreg = g->nodes.unreg_tail = NULL;
	    else
	      g->nodes.unreg = node->next;
	  }
	else
	  {
	    if (node->next == NULL)
	      {
		g->nodes.unreg_tail = prev;
		prev->next = NULL;
	      }
	    else
	      prev->next = node->next;
	  }
	g->nodes.unreg_count--;
	bump_creation(node);
	break;
      }
  if (node == NULL)
    {
      if ((g->nodes.unreg_count > MAX_UNREG_COUNT) ||
	  (g->debug && (g->nodes.unreg_count > DEBUG_MAX_UNREG_COUNT)))
	{
          ASSERT(g->nodes.unreg != NULL);
	  node = g->nodes.unreg;
	  g->nodes.unreg = node->next;
	  g->nodes.unreg_count--;
	}
      else
	{
	  if ((node = (Node *)malloc(sizeof(Node))) == NULL)
	    {
	      dbg_printf(g,0,"epmd: Insufficient memory");
	      exit(1);
	    }
	  node->cr_counter = current_time(g);
          bump_creation(node);
	}
    }
  node->next = g->nodes.reg;
  g->nodes.reg  = node;
  node->fd       = fd;
  node->port     = port;
  node->nodetype = nodetype;
  node->protocol = protocol;
  node->highvsn  = highvsn;
  node->lowvsn   = lowvsn;
  node->extralen = extralen;
  memcpy(node->extra,extra,extralen);
  copy_str(node->symname,name);
  select_fd_set(g, fd);
  if (highvsn == 0) {
    dbg_tty_printf(g,1,"registering '%s:%u', port %d",
		   node->symname, get_creation(node), node->port);
  } else {
    dbg_tty_printf(g,1,"registering '%s:%u', port %d",
		   node->symname, get_creation(node), node->port);
    dbg_tty_printf(g,1,"type %d proto %d highvsn %d lowvsn %d",
		   nodetype, protocol, highvsn, lowvsn);
  }
  print_names(g);
  return node;
}
static time_t current_time(EpmdVars *g)
{
  time_t t = time((time_t *)0);
  dbg_printf(g,3,"time in seconds: %d",t);
  return t;
}
static int reply(EpmdVars *g,int fd,char *buf,int len)
{
  char* p = buf;
  int nbytes = len;
  int val, ret;
  if (len < 0)
    {
      dbg_printf(g,0,"Invalid length in write %d",len);
      return -1;
    }
  if (g->delay_write)
    sleep(g->delay_write);
  for (;;) {
      val = write(fd, p, nbytes);
      if (val == nbytes) {
          ret = 1;
          break;
      }
      if (val < 0) {
          if (errno == EINTR)
              continue;
          dbg_perror(g,"error in write, errno=%d", errno);
          ret = 0;
          break;
      }
      dbg_printf(g,0,"could only send %d bytes out of %d to fd %d",val,nbytes,fd);
      p += val;
      nbytes -= val;
  }
  dbg_print_buf(g,buf,len);
  return ret;
}
#define LINEBYTECOUNT 16
static void print_buf_hex(unsigned char *buf,int len,char *prefix)
{
  int rows, row;
  rows = len / LINEBYTECOUNT;
  if (len % LINEBYTECOUNT)
    rows++;
  for (row = 0; row < rows; row++)
    {
      int rowstart = row * LINEBYTECOUNT;
      int rowend   = rowstart + LINEBYTECOUNT;
      int i;
      fprintf(stderr,"%s%.8x",prefix,rowstart);
      for (i = rowstart; i < rowend; i++)
	{
	  if ((i % (LINEBYTECOUNT/2)) == 0)
	    fprintf(stderr," ");
	  if (i < len)
	    fprintf(stderr," %.2x",buf[i]);
	  else
	    fprintf(stderr,"   ");
	}
      fprintf(stderr,"  |");
      for (i = rowstart; (i < rowend) && (i < len); i++)
	{
	  int c = buf[i];
	  if ((c >= 32) && (c <= 126))
	    fprintf(stderr,"%c",c);
	  else
	    fprintf(stderr,".");
	}
      fprintf(stderr,"|\r\n");
    }
}
static void dbg_print_buf(EpmdVars *g,char *buf,int len)
{
  int plen;
  if ((g->is_daemon) ||
      (g->debug < 2))
    return;
  dbg_tty_printf(g,1,"got %d bytes",len);
  plen = len > 1024 ? 1024 : len;
  print_buf_hex((unsigned char*)buf,plen,"***** ");
  if (len != plen)
    fprintf(stderr,"***** ......and more\r\n");
}
static void print_names(EpmdVars *g)
{
  int count = 0;
  Node *node;
  if ((g->is_daemon) ||
      (g->debug < 3))
    return;
  for (node = g->nodes.reg; node; node = node->next)
    {
      fprintf(stderr,"*****     active name     \"%s#%u\" at port %d, fd = %d\r\n",
	      node->symname, get_creation(node), node->port, node->fd);
      count ++;
    }
  fprintf(stderr, "*****     reg calculated count  : %d\r\n", count);
  count = 0;
  for (node = g->nodes.unreg; node; node = node->next)
    {
      fprintf(stderr,"*****     old/unused name \"%s#%u\"\r\n",
	      node->symname, get_creation(node));
      count ++;
    }
  fprintf(stderr, "*****     unreg counter         : %d\r\n",
	  g->nodes.unreg_count);
  fprintf(stderr, "*****     unreg calculated count: %d\r\n", count);
}