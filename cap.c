 #include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "libtuff.h"

/* baudrate settings are defined in <asm/termbits.h>, which is
   included by <termios.h> */
#define BAUDRATE B115200
/* change this definition for the correct port */
#define MODEMDEVICE "/dev/ttyS3"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE; 

main(int argc, char **argv)
{
  int fd,c, res;
  struct termios oldtio,newtio;
  char buf[255];
  unsigned int irfcm;
  unsigned int channel;
  unsigned int notch;
  unsigned int cap;
  argc--;
  argv++;

  // need port, irfcm, channel, notch, cap
  if (argc < 5) {
    exit(-1);
  }  
  /* 
     Open modem device for reading and writing and not as controlling tty
     because we don't want to get killed if linenoise sends CTRL-C.
  */
  fd = open(argv[0], O_RDWR | O_NOCTTY ); 
  if (fd <0) {perror(argv[0]); exit(-1); }

  irfcm = strtoul(argv[1], NULL, 0);
  channel = strtoul(argv[2], NULL, 0);
  notch = strtoul(argv[3], NULL, 0);
  cap = strtoul(argv[4], NULL, 0);
  
  if (notch > 2) {
    printf("notch must be from 0-2\n");
    exit(-1);
  }
  if (channel > 23) {
    printf("channel must be from 0-23\n");
    exit(-1);
  }
  if (cap > 31) {
    printf("cap must be from 0-31\n");
    exit(-1);
  }
  /*
notNotch = (~notch) & 0x7;
  sprintf(buf,"{\"on\":[%d,%d,%d],\"off\":[%d,%d,%d]}\n\r", irfcm, channel,notch,
	  irfcm,channel,notNotch);

  printf("going to send %s", buf);
  */
  
  tcgetattr(fd,&oldtio); /* save current serial port settings */
  bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
  
  /* 
     BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
     CRTSCTS : output hardware flow control (only used if the cable has
     all necessary lines. See sect. 7 of Serial-HOWTO)
     CS8     : 8n1 (8bit,no parity,1 stopbit)
     CLOCAL  : local connection, no modem contol
     CREAD   : enable receiving characters
  */
  newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
  
  /*
    IGNPAR  : ignore bytes with parity errors
    ICRNL   : map CR to NL (otherwise a CR input on the other computer
    will not terminate input)
    otherwise make device raw (no other input processing)
  */
  newtio.c_iflag = IGNPAR | ICRNL;
  
  /*
    Raw output.
  */
  newtio.c_oflag = 0;
  
  /*
    ICANON  : enable canonical input
    disable all echo functionality, and don't send signals to calling program
  */
  newtio.c_lflag = ICANON;
  
  /* 
     initialize all control characters 
     default values can be found in /usr/include/termios.h, and are given
     in the comments, but we don't need them here
  */
  newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
  newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
  newtio.c_cc[VERASE]   = 0;     /* del */
  newtio.c_cc[VKILL]    = 0;     /* @ */
  newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
  newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
  newtio.c_cc[VSWTC]    = 0;     /* '\0' */
  newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
  newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
  newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
  newtio.c_cc[VEOL]     = 0;     /* '\0' */
  newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
  newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
  newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
  newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
  newtio.c_cc[VEOL2]    = 0;     /* '\0' */
  
  /* 
     now clean the modem line and activate the settings for the port
  */
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);
  
  libtuff_setCap(fd, irfcm, channel, notch, cap);
  // wait for cap ack
  libtuff_waitForAck(fd, irfcm);
  printf("got ack; cap set complete\n");
  /*
  write(fd,buf,strlen(buf));
  nb = 0;
  while (STOP==FALSE) {
    unsigned char c;
    res = read(fd,&c, 1);
    if (c == '\n') {
      buf[nb] = 0;
      STOP = TRUE;
      printf("got %s\n\r", buf);
    } else {
      buf[nb] = c;
      nb++;
    }
  }
  */
/* restore the old port settings */
  tcsetattr(fd,TCSANOW,&oldtio);
}
