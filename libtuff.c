#include "libtuff.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>

static char buf[1024];

int libtuff_reset(unsigned int fd,
		  unsigned int irfcm) {
  sprintf(buf, "{\"reset\":%d}\n\r", irfcm);
  return write(fd, buf, strlen(buf));
}

int libtuff_onCommand(unsigned int fd,
		      unsigned int irfcm,
		      unsigned int channel,
		      unsigned int notchMask) {
  sprintf(buf, "{\"on\":[%d,%d,%d]}\n\r",irfcm, channel, notchMask);
  return write(fd, buf, strlen(buf));
}

int libtuff_offCommand(unsigned int fd,
		       unsigned int irfcm,
		       unsigned int channel,
		       unsigned int notchMask) {
  sprintf(buf, "{\"off\":[%d,%d,%d]}\n\r",irfcm,channel,notchMask);
  return write(fd, buf, strlen(buf));
}

int libtuff_setChannelNotches(unsigned int fd,
			      unsigned int irfcm,
			      unsigned int channel,
			      unsigned int notchMask) {
  unsigned int notch = notchMask & 0x7;
  unsigned int notNotch = (~notchMask) & 0x7;
  
  sprintf(buf,"{\"on\":[%d,%d,%d],\"off\":[%d,%d,%d]}\n\r", irfcm, channel,notch,
	  irfcm,channel,notNotch);
  printf("sending %s\n", buf);
  return write(fd, buf, strlen(buf));
}

int libtuff_updateCaps(unsigned int fd,
		       unsigned int irfcm,
		       unsigned int channel) {
  unsigned int tuffStack;
  unsigned int cmd;

  if (channel > 23) return -1;
  
  if (channel < 12) tuffStack = 0;
  else {
    tuffStack = 1;
    channel -= 12;
  }
  if (channel < 6) {
    cmd = (1<<channel) << 8;
  } else {
    cmd = ((1<<(channel-6)) << 8) | 0x4000;
  }
  cmd |= 0x60;
  sprintf(buf, "{\"raw\":[%d,%d,%d]}\n\r",
	    irfcm,
	    tuffStack,
	    cmd);
  return write(fd, buf, strlen(buf));  
}

int libtuff_setCap(unsigned int fd,
		   unsigned int irfcm,
		   unsigned int channel,
		   unsigned int notch,
		   unsigned int capValue) {
  // build up the raw command, there's no nice function for this
  unsigned int tuffStack;
  unsigned int cmd;

  if (channel > 23 || notch > 2 || capValue > 31) return -1;
  
  if (channel < 12) tuffStack = 0;
  else {
    tuffStack = 1;
    channel -= 12;
  }
  if (channel < 6) {
    cmd = (1<<channel) << 8;
  } else {
    cmd = ((1<<(channel-6)) << 8) | 0x4000;
  }
  // notch cmd (lower 8 bits) is
  // 0 n1 n0 v4 v3 v2 v1 v0
  // where n[1:0] is the notch
  // and v[4:0] is the value
  cmd |= capValue;
  cmd |= (notch << 5);
  sprintf(buf, "{\"raw\":[%d,%d,%d]}\n\r",
	    irfcm,
	    tuffStack,
	    cmd);
  printf("going to send %s\n", buf);
  return write(fd, buf, strlen(buf));
}

int libtuff_setQuietMode(unsigned int fd,
			  bool quiet) {
  sprintf(buf,"{\"quiet\":%d}\n\r", quiet);
  return write(fd, buf, strlen(buf));
}

int libtuff_pingiRFCM(unsigned int fd,
		      unsigned int timeout,
		      unsigned int numPing,
		      unsigned int *irfcmList) {
  unsigned char *p;
  unsigned int pingsSeen;
  int res;
  unsigned int nb;
  struct timeval tv;
  fd_set set;
  int rv;
  unsigned char c;
  unsigned int check_irfcm;

  switch(numPing) {
  case 0: return 0;
  case 1: sprintf(buf, "{\"ping\":[%d]}\n\r", irfcmList[0]); break;
  case 2: sprintf(buf, "{\"ping\":[%d,%d]}\n\r",
		  irfcmList[0],
		  irfcmList[1]); break;
  case 3: sprintf(buf, "{\"ping\":[%d,%d,%d]}\n\r",
		  irfcmList[0],
		  irfcmList[1],
		  irfcmList[2]); break;
  case 4: sprintf(buf, "{\"ping\":[%d,%d,%d,%d]}\n\r",
		  irfcmList[0],
		  irfcmList[1],
		  irfcmList[2],
		  irfcmList[3]); break;
  default: return -1;
  }
  res = write(fd, buf, strlen(buf));
  if (res < 0) return res;
  // sent off the ping, now wait for response
  pingsSeen = 0;
  nb = 0;
  while (pingsSeen != numPing) {
    FD_ZERO(&set);
    FD_SET(fd, &set);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    rv = select(fd+1,&set,NULL,NULL,&tv);
    if (rv == 0) return pingsSeen;
    if (rv == -1) return -1;
    read(fd, &c, 1);
    if (c == '\n') {
      buf[nb] = 0;
      p = strstr(buf, "pong");
      if (p != NULL) {
	p += 2+strlen("pong"); // move past '":'
	check_irfcm = *p - '0';
	for (unsigned int i=0;i<numPing;i++) {
	  if (irfcmList[i] == check_irfcm) {
	    pingsSeen++;
	    irfcmList[i] = 0xFFFFFFFF;
	  }
	}
      }
      nb = 0;
    } else {
      buf[nb] = c;
      nb++;
    }
  }
  return pingsSeen;
}

int libtuff_setNotchRange(unsigned int fd,
			   unsigned int notch,
			   unsigned int phiStart,
			   unsigned int phiEnd) {
  const char *notchStr[3] = { "r0", "r1", "r2" };
  if (notch > 2) return -1;

  sprintf(buf, "{\"%s\":[%d,%d]}\n\r", notchStr[notch], phiStart,phiEnd);
  return write(fd, buf, strlen(buf));
}

int libtuff_setPhiSectors(unsigned int fd,
			  unsigned int irfcm,
			  unsigned int *phiList,
			  unsigned int nb) {
  char *p;
  unsigned int i;
  int cnt;
  p = buf;
  cnt = sprintf(buf, "{\"set\":{\"irfcm\":%d,\"phi\":[", irfcm);
  p += cnt;
  for (i=0;i<nb;i++) {
    if (i != 0) {
      cnt = sprintf(p, ",");
      p += cnt;
    }
    cnt = sprintf(p, "%d", phiList[i]);
    p += cnt;
  }
  sprintf(p, "]}}\n\r");
  return write(fd, buf, strlen(buf));
}

void libtuff_waitForAck(unsigned int fd,
			unsigned int irfcm) {
  unsigned char nb;
  unsigned char c;
  char *p;
  unsigned int check_irfcm;
  nb = 0;
  while (1) {
    read(fd, &c, 1);
    if (c == '\n') {
      buf[nb] = 0;
      p = strstr(buf, "ack");
      if (p != NULL) {
	p += 2+strlen("ack"); // move past '":'
	check_irfcm = *p - '0';
	if (check_irfcm == irfcm) break;
      }
      nb = 0;
    } else {
      buf[nb] = c;
      nb++;
    }
  }  
}
