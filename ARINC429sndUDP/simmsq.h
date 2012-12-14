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

#ifndef ARINC429SubUDP_simmsq_h
#define ARINC429SubUDP_simmsq_h

int simMessageQ( key_t key );
int simRXMessage( int msqid, char* rxBuffer, int size );


#endif
