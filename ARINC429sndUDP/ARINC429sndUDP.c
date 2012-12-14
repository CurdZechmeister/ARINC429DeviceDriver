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
 * 419  and/or  ARINC 429. For   more   information  on  DO-178x  and  the
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
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "simmsq.h"
#include "arinctransform.h"
#include <time.h>

#define     MSGSZ           9                   // Each message is 9 bytes long
#define     MSQKEY          5678                // Queue key for TAP
#define     METERS_PER_SEC  1.943844924406      // For meters/sec to KT convert

/* UDP Payload Structure */
struct _payload {
    unsigned int  label;
    unsigned int  value;
};

/* IPC Message Structure */
struct _msqstruct {
    long         mtype;
    unsigned int label;
    float        value;
};


int main(int argc, char**argv)
{
    int sockfd;                             // UDP Socket Handle
    struct sockaddr_in servaddr;            // UDP Server Address structure
    struct _payload *payload = NULL;        // UDP Packet Payload Structure
    int key = MSQKEY;                       // IPC Message Queue Key
    int msqid = 0;                          // IPC Message Queue ID
    struct _msqstruct *msq = NULL;          // IPC Message Queue Structure
    char *argumentIP;                       // UDP Interface IP
    unsigned int argumentPort;              // UDP Port
    
    int transmit = 0;                       // Transmit Flag
    
    float value001 = 0.0;       // FMC DISTANCE TO-GO           BCD √
    float value012 = 0.0;       // IRS/FMC GROUND SPEED         BCD √
    float value100 = 0.0;       // VOR SELECTED RADIAL          BNR √
    float value101 = 0.0;       // APFDS HDG SELECT             BNR √
    float value105 = 0.0;       // ILS QFU                      BNR √
    float value114 = 0.0;       // FMC NAV COURSE               BNR √
    float value116 = 0.0;       // FMC CROSS TRACK DISTANCE     BNR
    float value117 = 0.0;       // FMC NAV VERTICAL DEVIATION   BNR
    float value162 = 0.0;       // ADF BEARING                  BNR √
    float value173 = 0.0;       // ILS LOCALIZER                BNR
    float value174 = 0.0;       // GLIDE SLOPE                  BNR
    float value222 = 0.0;       // VOR Omniearing               BNR √
    float value251 = 0.0;       // VOR DME in 4096 increments   BNR 
    float value270 = 0.0;       // FMC ALERT AND TO/FROM        DISCRETE
    float value314 = 0.0;       // FMC TRUE HEADING             BNR
    float value320 = 0.0;       // IRS MAGNETIC HEADING         BNR √
    float value324 = 0.0;       // ADI Pitch Angle              BNR
    float value325 = 0.0;       // ADI Roll Angle               BNR
    float value321 = 0.0;       // IRS DRIFT                    BNR
    
    
    /* Check for input values */
    if ( argc < 2 ) {
        printf("\nInvalid number of arguments.\n");
        printf("format: ARINC429subUDP device-IP udp-port\n");
        exit( EXIT_FAILURE );
    } else {
        printf("+----------------------------------------------------+\n");
        printf("| ARINC429SubUDP Starting in Foreground Mode         |\n");
        printf("+----------------------------------------------------+\n");
        argumentIP = argv[1];
        argumentPort = atoi(argv[2]);
    }
    
    
    /* Allocate UDP payload memory */
    if ( (payload = (struct _payload *)malloc(MSGSZ)) == 0 ) {
        printf("payload memory allocation failed.\n");
        exit( EXIT_FAILURE );
    }
    memset(payload, '\0', MSGSZ);
    
    /* Allocate Message Queue structure memory */
    if ((msq = malloc(MSGSZ)) == 0) {
        printf("TAP message memory allocation failed.\n");
        exit( EXIT_FAILURE );
    }
    memset(msq, '\0', MSGSZ);
    
    /* Attach to IPC message queue */
    if ( (msqid = simMessageQ( key )) == 0 ) {
        printf("TAP message queue not avilable.");
        exit( EXIT_FAILURE );
    } else {
        printf("TAP Queue Ready for Data\n");
    }
    
    /* Create and attach to UDP Socket */
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    
    printf("Connecting to %s on port %i\n", argumentIP, argumentPort);
    
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr(argumentIP);
    servaddr.sin_port=htons(argumentPort);
    
    /*
     * Main  program  processing  loop. Here we  pull updates from the  FIFO IPC
     * message  queue, format  the  message  value  to  ARINC 429  and  send the
     * Label/Data combination on via UDP to a hardware driver running on another 
     * hardare  platform.  Sending  Label 000  to the susystem requests the sub-
     * system to  stop. (Per  ARINC 429  Att 11 System  Label 000 is not used by
     * ARINC 429)
     */
    unsigned int *dataPayload = NULL;
    dataPayload = (unsigned int *)malloc(sizeof(unsigned int));
    
    
    /* Read a message from the IPC queue or wait for one to arrive since this is 
     * a blocking read, messages are stored as a FIFO queue in two 32 bit words,
     * format: LSB |_Lablel_|_Data_| MSB 
     */
    if ( (simRXMessage(msqid, (void *)msq, MSGSZ)) != 0 ) {
        printf("IPC Queue Read Failed.\n");
    }
    
    while ( msq->label )
    {
        
        /* DEBUG only */
        printf("Q-ID %li Label %03o Value %f\n",
               msq->mtype, msq->label, msq->value);
        payload->label = msq->label;
        
        transmit = 0; 
        
        /* Switch label to the right processing method and ready the data to  be 
           transmitted */
        switch ( payload->label ) {
             
            case 01:
                if ( value001 != msq->value ) {
                    *dataPayload = float2BCD4( msq->value, 3, 1 );
                    parityOdd( dataPayload, &payload->label );
                    value001 = msq->value;
                    transmit = 1;
                }
                break;
            case 012:
                if ( value012 != (int) msq->value ) {
                    *dataPayload = float2BCD3( (msq->value*METERS_PER_SEC),
                                             3, 0 );
                    parityOdd( dataPayload, &payload->label );
                    value012 = (int) msq->value;
                    transmit = 1;
                }
                break;
            case 0100:
                if ( value100 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value100 = msq->value;
                    transmit = 1;
                }
                break;
            case 0101:
                if ( value101 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value101 = msq->value;
                    transmit = 1;
                }
                break;
            case 0105:
                if ( value105 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value105 = msq->value;
                    transmit = 1;
                }
                break;
            case 0114:
                if ( value114 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value114 = msq->value;
                    transmit = 1;
                }
                break;
            case 0116:
                if ( value116 != msq->value ) {
                    *dataPayload = 0x600000 |
                        ((unsigned int)(msq->value*250.0))<<7;
                    parityOdd( dataPayload, &payload->label );
                    value116 = msq->value;
                    transmit = 1;
                }
                break;
            case 0117:
                if ( value117 != msq->value ) {
                    *dataPayload = 0x600000 |
                        ((unsigned int)(msq->value*275.0))<<7;
                    parityOdd( dataPayload, &payload->label );
                    value117 = msq->value;
                    transmit = 1;
                }
                break;
            case 0162:
                if ( value162 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value162 = msq->value;
                    transmit = 1;
                }
                break;
            case 0173:
                if ( value173 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value173 = msq->value;
                    transmit = 1;
                }
                break;
            case 0174:
                if ( value174 != msq->value ) {
                    *dataPayload = 0x600000 |
                        ((unsigned int)(msq->value*1000.0))<<7;
                    parityOdd( dataPayload, &payload->label );
                    value174 = msq->value;
                    transmit = 1;
                }
                break;
            case 0222:
                if ( value222 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value222 = msq->value;
                    transmit = 1;
                }
                break;
            case 0251:
                if ( value251 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value251 = msq->value;
                    transmit = 1;
                }
                break;
            case 0270:
                if ( value270 != msq->value ) {
                    if ( msq->value ) {
                        *dataPayload = 0x680000;
                    } else {
                        *dataPayload = 0xE00000;
                    }
                    value270 = msq->value;
                    transmit = 1;
                }
                break;
            case 0314:
                if ( value314 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value314 = msq->value;
                    transmit = 1;
                }
                break;
            case 0320:
                if ( value320 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label);
                    value320 = msq->value;
                    transmit = 1;
                }
                break;
            case 0321:
                if ( value321 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label );
                    value100 = msq->value;
                    transmit = 1;
                }
                break;
            case 0324:
                if ( value324 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label);
                    value324 = msq->value;
                    transmit = 1;
                }
                break;
            case 0325:
                if ( value325 != msq->value ) {
                    *dataPayload = floatDEG2BNR( msq->value );
                    parityOdd( dataPayload, &payload->label);
                    value325 = msq->value;
                    transmit = 1;
                }
                break;
            case 0377:
                    *dataPayload = 0x00;
                    value100 = msq->value;
                    transmit = 1;
                break;
            default:
                printf("No rule found for Label %03o. Label ignored.\n",
                       payload->label);
                break;
                
        }
        
        /* Only transmit if the current label value has changed */
        if ( transmit ) {
            
            // Debug Message
            printf("                 ↳ 24bit ARINC429 data 0x%06X\n",
                   *dataPayload);
            payload->value = *dataPayload;
        
            if ( (sendto(sockfd,payload,
                         sizeof(payload->value)+sizeof(payload->label),0,
                         (struct sockaddr *)&servaddr,sizeof(servaddr))) == 0 )
            {
                printf("Send to UDP server failed.\n");
                exit( EXIT_FAILURE );
            }
            
        }
        
        /* Read next message  from the IPC queue or wait for one to arrive since
         * this is a blocking read,  messages are stored as a FIFO  queue in two
         * 32 bit words, format: LSB |_Lablel_|_Data_| MSB 
         */
        if ( (simRXMessage(msqid, (void *)msq, MSGSZ)) != 0 ) {
            printf("IPC Queue Read Failed.\n");
        }

    } // end main processing loop. ARINC Label o000 exits
    
    /* Cleanup */
    free(payload);
    free(msq);
    free(dataPayload);
    
    printf("ARINC 429 Subsystem for UDP Exiting.\n");
    return( EXIT_SUCCESS );

} // End Main