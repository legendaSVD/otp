#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif
#include "run_erl.h"
#include "safe_string.h"
#ifdef __clang_analyzer__
#  undef FD_ZERO
#  define FD_ZERO(FD_SET_PTR) memset(FD_SET_PTR, 0, sizeof(fd_set))
#endif
#if defined(O_NONBLOCK)
# define DONT_BLOCK_PLEASE O_NONBLOCK
#else
# define DONT_BLOCK_PLEASE O_NDELAY
# if !defined(EAGAIN)
#  define EAGAIN -3898734
# endif
#endif
#ifdef HAVE_STRERROR
#  define STRERROR(x) strerror(x)
#else
#  define STRERROR(x) ""
#endif
#define noDEBUG_TOERL
#define PIPE_DIR        "/tmp/"
#define PIPE_STUBNAME   "erlang.pipe"
#define PIPE_STUBLEN    strlen(PIPE_STUBNAME)
#ifdef DEBUG_TOERL
#define STATUS(s)  { fprintf(stderr, (s)); fflush(stderr); }
#else
#define STATUS(s)
#endif
#ifndef FILENAME_MAX
#define FILENAME_MAX 250
#endif
static struct termios tty_smode, tty_rmode;
static int tty_eof = 0;
static int recv_sig = 0;
static int protocol_ver = RUN_ERL_LO_VER;
static int write_all(int fd, const char* buf, int len);
static int window_size_seq(char* buf, size_t bufsz);
static int version_handshake(char* buf, int len, int wfd);
#ifdef DEBUG_TOERL
static void show_terminal_settings(struct termios *);
#endif
static void handle_ctrlc(int sig)
{
    signal(SIGINT,handle_ctrlc);
    recv_sig = SIGINT;
}
static void handle_sigwinch(int sig)
{
    recv_sig = SIGWINCH;
}
static void usage(char *pname)
{
    fprintf(stderr, "Usage: %s [-h|-F] [pipe_name|pipe_dir/]\n", pname);
    fprintf(stderr, "\t-h\tThis help text.\n");
    fprintf(stderr, "\t-F\tForce connection even though pipe is locked by other to_erl process.\n");
}
int main(int argc, char **argv)
{
    char  FIFO1[FILENAME_MAX], FIFO2[FILENAME_MAX];
    int i, len, wfd, rfd;
    fd_set readfds;
    char buf[BUFSIZ];
    char pipename[FILENAME_MAX];
    int pipeIx = 1;
    int force_lock = 0;
    int got_some = 0;
    if (argc >= 2 && argv[1][0]=='-') {
	switch (argv[1][1]) {
	case 'h':
	    usage(argv[0]);
	    exit(1);
	case 'F':
	    force_lock = 1;
	    break;
	default:
	    fprintf(stderr,"Invalid option '%s'\n",argv[1]);
	    exit(1);
	}
	pipeIx = 2;
    }
#ifdef DEBUG_TOERL
    fprintf(stderr, "%s: pid is : %d\n", argv[0], (int)getpid());
#endif
    strn_cpy(pipename, sizeof(pipename),
	     (argv[pipeIx] ? argv[pipeIx] : PIPE_DIR));
    if(*pipename && pipename[strlen(pipename)-1] == '/') {
	int highest_pipe_num = 0;
	DIR *dirp;
	struct dirent *direntp;
	dirp = opendir(pipename);
	if(!dirp) {
	    fprintf(stderr, "Can't access pipe directory %s: %s\n", pipename, strerror(errno));
	    exit(1);
	}
	while((direntp=readdir(dirp)) != NULL) {
	    if(strncmp(direntp->d_name,PIPE_STUBNAME,PIPE_STUBLEN)==0) {
		int num = atoi(direntp->d_name+PIPE_STUBLEN+1);
		if(num > highest_pipe_num)
		    highest_pipe_num = num;
	    }
	}
	closedir(dirp);
	strn_catf(pipename, sizeof(pipename), (highest_pipe_num?"%s.%d":"%s"),
		  PIPE_STUBNAME, highest_pipe_num);
    }
    sn_printf(FIFO1,sizeof(FIFO1),"%s.r",pipename);
    sn_printf(FIFO2,sizeof(FIFO2),"%s.w",pipename);
    if ((wfd = open (FIFO1, O_WRONLY|DONT_BLOCK_PLEASE, 0)) >= 0) {
	close(wfd);
	fprintf(stderr, "Another to_erl process already attached to pipe "
			"%s.\n", pipename);
	if (force_lock) {
	    fprintf(stderr, "But we proceed anyway by force (-F).\n");
	}
	else {
	    exit(1);
	}
    }
    if ((rfd = open (FIFO1, O_RDONLY|DONT_BLOCK_PLEASE, 0)) < 0) {
#ifdef DEBUG_TOERL
	fprintf(stderr, "Could not open FIFO %s for reading.\n", FIFO1);
#endif
	fprintf(stderr, "No running Erlang on pipe %s: %s\n", pipename, strerror(errno));
	exit(1);
    }
#ifdef DEBUG_TOERL
    fprintf(stderr, "to_erl: %s opened for reading\n", FIFO1);
#endif
    if ((wfd = open (FIFO2, O_WRONLY|DONT_BLOCK_PLEASE, 0)) < 0) {
#ifdef DEBUG_TOERL
	fprintf(stderr, "Could not open FIFO %s for writing.\n", FIFO2);
#endif
	fprintf(stderr, "No running Erlang on pipe %s: %s\n", pipename, strerror(errno));
	close(rfd);
	exit(1);
    }
#ifdef DEBUG_TOERL
    fprintf(stderr, "to_erl: %s opened for writing\n", FIFO2);
#endif
    fprintf(stderr, "Attaching to %s (^D to exit)\n\n", pipename);
    signal(SIGINT,handle_ctrlc);
    if (tcgetattr(0, &tty_rmode) , 0) {
	fprintf(stderr, "Cannot get terminals current mode\n");
	exit(-1);
    }
    tty_smode = tty_rmode;
    tty_eof = '\004';
#ifdef DEBUG_TOERL
    show_terminal_settings(&tty_rmode);
#endif
    tty_smode.c_iflag =
	1*BRKINT |
        1*IGNPAR |
        0;
#if 0
0*IGNBRK |
0*PARMRK |
0*INPCK  |
0*INLCR  |
0*IGNCR  |
0*ICRNL  |
0*IUCLC  |
0*IXON   |
0*IXANY  |
0*IXOFF  |
0*IMAXBEL|
#endif
    tty_smode.c_oflag =
    OPOST    |
    0*ONLCR  |
#ifdef XTABS
    1*XTABS  |
#endif
#ifdef OXTABS
    1*OXTABS |
#endif
#ifdef NL0
    1*NL0    |
#endif
#ifdef CR0
    1*CR0    |
#endif
#ifdef TAB0
    1*TAB0   |
#endif
#ifdef BS0
    1*BS0    |
#endif
#ifdef VT0
    1*VT0    |
#endif
#ifdef FF0
    1*FF0    |
#endif
											    0;
#if 0
0*OLCUC  |
0*OCRNL  |
0*ONOCR  |
0*ONLRET |
0*OFILL  |
0*OFDEL  |
0*NL1    |
0*CR1    |
0*CR2    |
0*CR3    |
0*TAB1   |
0*TAB2   |
0*TAB3   |
0*BS1    |
0*VT1    |
0*FF1    |
#endif
    tty_smode.c_lflag =
									0;
#if 0
0*ISIG   |
0*ICANON |
0*XCASE  |
0*ECHO   |
0*ECHOE  |
0*ECHOK  |
0*ECHONL |
0*NOFLSH |
0*TOSTOP |
0*ECHOCTL|
0*ECHOPRT|
0*ECHOKE |
0*FLUSHO |
0*PENDIN |
0*IEXTEN |
#endif
    tty_smode.c_cc[VMIN]      =0;
    tty_smode.c_cc[VTIME]     =0;
    tty_smode.c_cc[VINTR]     =3;
    tcsetattr(0, TCSADRAIN, &tty_smode);
#ifdef DEBUG_TOERL
    show_terminal_settings(&tty_smode);
#endif
    if (write(wfd, "\033l", 2) < 0) {
        fprintf(stderr, "Error in writing ^[l to FIFO.\n");
    }
    while (1) {
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	FD_SET(rfd, &readfds);
	if (select(rfd + 1, &readfds, NULL, NULL, NULL) < 0) {
	    if (recv_sig) {
		FD_ZERO(&readfds);
	    }
	    else {
		fprintf(stderr, "Error in select.\n");
		break;
	    }
	}
	len = 0;
	if (recv_sig) {
	    switch (recv_sig) {
	    case SIGINT:
		fprintf(stderr, "[Break]\n\r");
		buf[0] = '\003';
		len = 1;
		break;
	    case SIGWINCH:
		len = window_size_seq(buf,sizeof(buf));
		break;
	    default:
		fprintf(stderr,"Unexpected signal: %u\n",recv_sig);
	    }
	    recv_sig = 0;
	}
	else if (FD_ISSET(0, &readfds)) {
	    len = read(0, buf, sizeof(buf));
	    if (len <= 0) {
		close(rfd);
		close(wfd);
		if (len < 0) {
		    fprintf(stderr, "Error in reading from stdin.\n");
		} else {
		    fprintf(stderr, "[EOF]\n\r");
		}
		break;
	    }
	    for (i = 0; i < len && buf[i] != tty_eof; i++);
	    if (buf[i] == tty_eof) {
		fprintf(stderr, "[Quit]\n\r");
		break;
	    }
	}
	if (len) {
#ifdef DEBUG_TOERL
	    write_all(1, buf, len);
#endif
	    if (write_all(wfd, buf, len) != len) {
		fprintf(stderr, "Error in writing to FIFO.\n");
		close(rfd);
		close(wfd);
		break;
	    }
	    STATUS("\" OK\r\n");
	}
	if (FD_ISSET(rfd, &readfds)) {
	    STATUS("FIFO read: ");
	    len = read(rfd, buf, BUFSIZ);
	    if (len < 0 && errno == EAGAIN) {
		;
	    } else if (len <= 0) {
		close(rfd);
		close(wfd);
		if (len < 0) {
		    fprintf(stderr, "Error in reading from FIFO.\n");
		} else
		    fprintf(stderr, "[End]\n\r");
		break;
	    } else {
		if (!got_some) {
		    if ((len=version_handshake(buf,len,wfd)) < 0) {
			close(rfd);
			close(wfd);
			break;
		    }
		    if (protocol_ver >= 1) {
			signal(SIGWINCH, handle_sigwinch);
			raise(SIGWINCH);
		    }
		    got_some = 1;
		}
		STATUS("Terminal write: \"");
		if (write_all(1, buf, len) != len) {
		    fprintf(stderr, "Error in writing to terminal.\n");
		    close(rfd);
		    close(wfd);
		    break;
		}
		STATUS("\" OK\r\n");
	    }
	}
    }
    tcsetattr(0, TCSADRAIN, &tty_rmode);
    return 0;
}
static int write_all(int fd, const char* buf, int len)
{
    int left = len;
    int written;
    while (left) {
	written = write(fd,buf,left);
	if (written < 0) {
	    return -1;
	}
	left -= written;
	buf += written;
    }
    return len;
}
static int window_size_seq(char* buf, size_t bufsz)
{
#ifdef TIOCGWINSZ
    struct winsize ws;
    static const char prefix[] = "\033_";
    static const char suffix[] = "\033\\";
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0) {
	int len = sn_printf(buf, bufsz, "%swinsize=%u,%u%s",
			    prefix, ws.ws_col, ws.ws_row, suffix);
	return len;
    }
#endif
    return 0;
}
static int version_handshake(char* buf, int len, int wfd)
{
    unsigned re_high=0, re_low;
    char *end = find_str(buf,len,"]\n");
    if (end && sscanf(buf,"[run_erl v%u-%u",&re_high,&re_low)==2) {
	char wbuf[30];
	int wlen;
	if (re_low > RUN_ERL_HI_VER || re_high < RUN_ERL_LO_VER) {
	    fprintf(stderr,"Incompatible versions: to_erl=v%u-%u run_erl=v%u-%u\n",
		    RUN_ERL_HI_VER, RUN_ERL_LO_VER, re_high, re_low);
	    return -1;
	}
	protocol_ver = re_high < RUN_ERL_HI_VER ? re_high : RUN_ERL_HI_VER;
	wlen = sn_printf(wbuf, sizeof(wbuf), "\033_version=%u\033\\",
			 protocol_ver);
	if (write_all(wfd, wbuf, wlen) < 0) {
	    fprintf(stderr,"Failed to send version handshake\n");
	    return -1;
	}
	end += 2;
	len -= (end-buf);
	memmove(buf,end,len);
    }
    else {
	protocol_ver = 0;
    }
    if (re_high != RUN_ERL_HI_VER) {
	fprintf(stderr,"run_erl has different version, "
		"using common protocol level %u\n", protocol_ver);
    }
    return len;
}
#ifdef DEBUG_TOERL
#define S(x)  ((x) > 0 ? 1 : 0)
static void show_terminal_settings(struct termios *t)
{
  fprintf(stderr,"c_iflag:\n");
  fprintf(stderr,"Signal interrupt on break:   BRKINT  %d\n", S(t->c_iflag & BRKINT));
  fprintf(stderr,"Map CR to NL on input:       ICRNL   %d\n", S(t->c_iflag & ICRNL));
  fprintf(stderr,"Ignore break condition:      IGNBRK  %d\n", S(t->c_iflag & IGNBRK));
  fprintf(stderr,"Ignore CR:                   IGNCR   %d\n", S(t->c_iflag & IGNCR));
  fprintf(stderr,"Ignore char with par. err's: IGNPAR  %d\n", S(t->c_iflag & IGNPAR));
  fprintf(stderr,"Map NL to CR on input:       INLCR   %d\n", S(t->c_iflag & INLCR));
  fprintf(stderr,"Enable input parity check:   INPCK   %d\n", S(t->c_iflag & INPCK));
  fprintf(stderr,"Strip character              ISTRIP  %d\n", S(t->c_iflag & ISTRIP));
  fprintf(stderr,"Enable start/stop input ctrl IXOFF   %d\n", S(t->c_iflag & IXOFF));
  fprintf(stderr,"ditto output ctrl            IXON    %d\n", S(t->c_iflag & IXON));
  fprintf(stderr,"Mark parity errors           PARMRK  %d\n", S(t->c_iflag & PARMRK));
  fprintf(stderr,"\n");
  fprintf(stderr,"c_oflag:\n");
  fprintf(stderr,"Perform output processing    OPOST   %d\n", S(t->c_oflag & OPOST));
  fprintf(stderr,"\n");
  fprintf(stderr,"c_cflag:\n");
  fprintf(stderr,"Ignore modem status lines    CLOCAL  %d\n", S(t->c_cflag & CLOCAL));
  fprintf(stderr,"\n");
  fprintf(stderr,"c_local:\n");
  fprintf(stderr,"Enable echo                  ECHO    %d\n", S(t->c_lflag & ECHO));
  fprintf(stderr,"\n");
  fprintf(stderr,"c_cc:\n");
  fprintf(stderr,"c_cc[VEOF]                           %d\n", t->c_cc[VEOF]);
}
#endif