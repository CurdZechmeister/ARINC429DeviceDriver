/*
 * ARINC 429 Subsystem With UDP
 *
 * This  subsytem  reads  flight  simulator  exported data  from a defined
 * IPC  Message Queue,  performs  ARINC 429  compliant data transformation
 * to the BNR  and BCD formats required by  specific  labels and sends the
 * ARINC 429 payload data to a device driver via UDP.
 *
 * Disclaimer: This software is not DO-178B/C certified and is not for use
 * in actual flight  training  devices. The code and  methos used for data
 * transfer  are not in full compliance with any standards based on  ARINC
 * 419  and/or  ARINC 429. For   more   information  on  DO-178B  and  the
 * ARINC 419/429   standards   please    visit   http://www.rtca.org   and
 * http://www.arinc.com
 *
 * Copyright (C) 2012 Curd S. Zechmeister
 *
 * This  program is   free software: you can redistribute it and/or modify
 * it under  the terms of  the GNU  General Public License as published by
 * the  Free  Software  Foundation, either  version  3 of  the License, or
 * any later version.
 *
 * This   program  is  distributed  in  the  hope  that it will be useful,
 * but  WITHOUT  ANY  WARRANTY;  without  even the  implied  warranty   of
 * MERCHANTABILITY    or  FITNESS  FOR  A  PARTICULAR  PURPOSE.   See  the
 * GNU General Public License for more details.
 *
 * You  should  have received  a  copy  of  the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/*
 * Attach to the message queue run by TAP
 */
int simMessageQ( key_t key ) {
    
    int msqid;
    
    /* Attach to the queue with the IPC key */
    if ( (msqid = msgget( key, IPC_CREAT | 0666 )) < 0) {
        printf("TAP Queue not available.\n");
        return( -1 );
    }
    
    return( msqid );
    
} // end simMessage

/*
 * Read a simulator message from the queue
 */
int simRXMessage( int msqid, void* rxBuffer, int size ) {
    
    if( msgrcv( msqid, rxBuffer, size, 1, 0) < 0 )
        return -1;
    
    return 0;
    
} // end simRXMessage

/*
 * Read a message for the simulator to the queue
 */
int simTXMessage( int msqid, void* txBuffer, int size ) {
    
    if( msgsnd( msqid, txBuffer, size, 0) == -1 )
        return -1;
    
    return 0;
    
} // end simRTMessage