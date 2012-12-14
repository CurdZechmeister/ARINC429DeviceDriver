
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "xplanetransform.h"
#include "simmsq.h"

#define     NPACK           10000
#define     PORT            25002
#define     PAYLOAD_SIZE    9
#define     MSQSZ           8
#define     MSQKEY          5789                // Queue key for TAP
#define     COM_RADIO       0
#define     NAV_RADIO       1

/* UDP Packet Payload */
struct _payload {
    unsigned int label;
    unsigned int value;
};


/* Message Queue Format */
struct _msqstruct {
    long         mtype;
    unsigned int label;
    unsigned int value;
};

void diep(char *s)
{
    perror(s);
    printf("Error: %s", s);
    exit(1);

}


/* 
 * Main Function
 */
int main(void)
{
    
    /* UDP Variable Setup */
    struct sockaddr_in si_me, si_other;
    socklen_t slen;
    int s;
    struct _payload *payload;
    
    /* MSQ Varible Setup */
    int key = MSQKEY;
    int msqid = 0;
    struct _msqstruct *msq = NULL;
    
    
    /* Assign Memory to the UDP payload */
    if ( (payload = (struct _payload *)malloc(sizeof(*payload))) == 0 ) {
        printf("payload memory allocation failed.\n");
        exit( -1 );
    }
    memset(payload,'\0',sizeof(*payload));
    
    /* Allocate Message Queue structure memory */
    if ((msq = malloc(sizeof(struct _msqstruct))) == 0) {
        printf("TAP message memory allocation failed.\n");
        exit( EXIT_FAILURE );
    }
    memset(msq, '\0', sizeof(struct _msqstruct));
    
    /* Attach to IPC message queue */
    if ( (msqid = simMessageQ( key )) == 0 ) {
        printf("TAP message queue not avilable.");
        exit( EXIT_FAILURE );
    } else {
        printf("TAP Queue Ready for Data\n");
    }
    
    /* Create the UDP Socket */
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");
        
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (const struct sockaddr *) &si_me, sizeof(si_me))==-1)
        diep("bind");
    
    
    /*
     * Main program executio nloop. UDP packets will be received until an ARINC
     * label of 000 is sent, which will end the execution loop and close down 
     * the device driver.
     */
    printf("ARINC429rcvUDP ready for traffic.\n");
    do {
        
        /* Clear UDP Payload */
        memset(payload,'\0',sizeof(*payload));
        
        if (recvfrom(s, payload, sizeof(*payload), 0,
                     (struct sockaddr *)&si_other, &slen)==-1)
                diep("recvfrom()");
        printf("Received packet from %s:%d - Label %03o Value 0x%06X\n",
                inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port),
               payload->label, payload->value);
        
        msq->label = payload->label;
        
        /* Label Switchboard */
        switch (msq->label) {
                
            case 030:
                msq->mtype = getARINCsdi(payload->value);
                msq->value = vhf030ToSim(payload->value)/10;
                printf("      ↳ VHF Radio %li: %i\n",
                       msq->mtype,
                       msq->value);
                break;
            case 0205:
                if ( getARINChfWord(payload->value) == 0 ) {
                    msq->mtype = getARINCsdi(payload->value);
                    msq->value = hf205ToSim(payload->value);
                    printf("      ↳ HF Radio %i Word %i: %i \n",
                           getARINCsdi(payload->value),
                           getARINChfWord(payload->value),
                           msq->value );
                }
                break;
            default:
                printf("No rule found for Label %3o - Label Ignored\n",
                       msq->label);
                break;
        }
        
        /* Write the new label and value into the queue to the simulator */
        if ( (simTXMessage(msqid, (void *)msq,
                           sizeof(struct _msqstruct)-8)) != 0 ) {
            printf("IPC Queue Write Failed.\n");
        }
        
    } while ( payload->label != 0 );
    
    /* Close the UDP Socket Server down */
    close(s);
    
    /* Free memory allocations */
    free( payload );
    free( msq );
    
    printf("ARINC429rcv429 driver is exiting.\n");
    return 0;
    
} // End ARINC429rcvUDP
