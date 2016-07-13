#ifndef _LIBTUFF_H_
#define _LIBTUFF_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/** \brief Issues a reset to a specific iRFCM.
 *
 * Note: resets don't actually affect the iRFCM (the TUFF master)
 * itself. They reset the microcontrollers on the TUFFs.
 */
int libtuff_reset(unsigned int fd,
		  unsigned int irfcm);

/** \brief Turns on notches on a specific iRFCM channel. 
 *
 * This function turns on the notches listed in notchMask.
 * It does not turn off the other notches.
 */
int libtuff_onCommand(unsigned int fd,
		      unsigned int irfcm,
		      unsigned int channel,
		      unsigned int notchMask);

/** \brief Turns off notches on a specific iRFCM channel.
 *
 * This function turns off the notches listed in notchMask.
 * It does not turn on the other notches.
 */
int libtuff_offCommand(unsigned int fd,
		       unsigned int irfcm,
		       unsigned int channel,
		       unsigned int notchMask);

/** \brief Specifically sets the notch values on a given channel.
 *
 * This command gets TWO ACKs, because it sends an on command
 * as well as an off command.
 *
 * Bitmask is what notches you want on.
 */
int libtuff_setChannelNotches(unsigned int fd,
			      unsigned int irfcm,
			      unsigned int channel,
			      unsigned int notchMask);

/** \brief Sets a capacitor value using raw commands.
 *
 * This function sets the tunable capacitor on a notch on a
 * specific channel. I don't expect these to be changed much,
 * if at all, so these just use raw commands rather than
 * anything intelligible.
 *
 * Anyway, the function works, so the readability isn't that
 * important.
 */
int libtuff_setCap(unsigned int fd,
		   unsigned int irfcm,
		   unsigned int channel,
		   unsigned int notch,
		   unsigned int capValue);

/** \brief Makes the TUFFs stop acking all commands.
 *
 * In non-interactive mode, once a pong has been
 * received from all TUFFs, it probably makes sense to
 * go ahead and turn off the ACKs for each command.
 *
 * Quiet mode does this. So call libtuff_setQuietMode(serial_fd, 1);
 *
 * From then on, TUFFs will not acknowledge any commands
 * other than a ping.
 */
int libtuff_setQuietMode(unsigned int fd,
			 bool quiet);

/** \brief Looks for an acknolwedgement from a given iRFCM.
 *
 * After a command, a TUFF acknowledges it with a brief 'ack'
 * as well as its iRFCM number.
 *
 * Note that it acknowledges ALL commands in the JSON message, not just once
 * for the JSON message!
 *
 * This SHOULD NOT BE CALLED if the TUFFs are in quiet mode.
 * It'll hang forever.
 */
void libtuff_waitForAck(unsigned int fd,
			unsigned int irfcm);

/** \brief Saves the current capacitor values to default.
 *
 * Saves all 3 tunable capacitor values into information flash
 * in the TUFF microcontrollers so they are now the new defaults.
 */
int libtuff_updateCaps(unsigned int fd,
		       unsigned int irfcm,
		       unsigned int channel);

/** \brief Pings a list of iRFCMs.
 *
 * To find out if the iRFCMs are present, ping a list of them
 * using this function. It returns the number of pongs received.
 *
 * Timeout is in seconds.
 */
int libtuff_pingiRFCM(unsigned int fd,
		      unsigned int timeout,
		      unsigned int numPing,
		      unsigned int *irfcmList);

/** \brief Ranged notch command.
 * 
 * Turns on notches for phi sectors between phiStart/phiEnd (inclusive).
 * Turns off notches outside of that range.
 *
 * The range works modulo 16, so (0,15) turns on all phi sectors, and
 * (15,0) turns on only phi sectors 15 and 0.
 */
int libtuff_setNotchRange(unsigned int fd,
			    unsigned int notch,
			    unsigned int phiStart,
			    unsigned int phiEnd);

/** \brief Set phi sectors for an iRFCM.
 *
 * Sets the 24 TUFF channels on an iRFCM to the assigned phi sectors.
 *
 * You can pass less than 24 channels, but it always starts with
 * channel 0, and unpassed phi sectors are unchanged.
 *
 * Note that phi sector assignments/notch range commands haven't
 * been extensively tested yet.
 */
int libtuff_setPhiSectors(unsigned int fd,
			  unsigned int irfcm,
			  unsigned int *phiList,
			  unsigned int nb);

#endif
