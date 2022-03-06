/* rfsinit.c - rfsinit */

#include <xinu.h>

struct	rfdata	Rf_data;

/*------------------------------------------------------------------------
 *  rfsinit  -  Initialize the remote file system master device
 *------------------------------------------------------------------------
 */
devcall	rfsinit(
	  struct dentry	*devptr		/* Entry in device switch table	*/
	)
{
	uint32	serverip;

	/* Choose an initial message sequence number */

	Rf_data.rf_seq = 1;

	/* Convert the server name to an IP address */

	serverip = dnslookup(RF_SERVER_NAME);

	if ( (int32)serverip == SYSERR ) {
		panic("Cannot find IP address for %s\n");
	}

	/* Set the server IP address, server port, and local port */

	Rf_data.rf_ser_ip = serverip;
	Rf_data.rf_ser_port = RF_SERVER_PORT;
	Rf_data.rf_loc_port = RF_LOC_PORT;

	/* Create a mutual exclusion semaphore */

	if ( (Rf_data.rf_mutex = semcreate(1)) == SYSERR ) {
		panic("Cannot create remote file system semaphore");
	}

	/* Specify that the server port is not yet registered */

	Rf_data.rf_registered = FALSE;

	return OK;
}
