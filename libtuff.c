#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char buf[1024];

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

void libtuff_waitForAck(unsigned int fd,
			unsigned int irfcm) {
  unsigned char nb;
  unsigned char c;
  char *p;
  unsigned int check_irfcm;
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
