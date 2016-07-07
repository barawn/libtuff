#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int libtuff_onCommand(unsigned int fd,
		      unsigned int irfcm,
		      unsigned int channel,
		      unsigned int notchMask);

int libtuff_offCommand(unsigned int fd,
		       unsigned int irfcm,
		       unsigned int channel,
		       unsigned int notchMask);

int libtuff_setChannelNotches(unsigned int fd,
			      unsigned int irfcm,
			      unsigned int channel,
			      unsigned int notchMask);

int libtuff_setCap(unsigned int fd,
		   unsigned int irfcm,
		   unsigned int channel,
		   unsigned int notch,
		   unsigned int capValue);

void libtuff_waitForAck(unsigned int fd,
			unsigned int irfcm);

int libtuff_updateCaps(unsigned int fd,
		       unsigned int irfcm,
		       unsigned int channel);
