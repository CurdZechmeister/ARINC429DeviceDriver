#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_msgbuf {
    long mtype;
    unsigned int label;
    unsigned int value;
};

int main(void)
{
    struct my_msgbuf buf;
    int msqid;
    key_t key;
    
    key = 5789;
    
    if ((msqid = msgget(key, 0644)) == -1) { /* connect to the queue */
        perror("msgget");
        exit(1);
    }
    
    printf("Dump the TAP queue elements.\n");
    
    /* Read the message queue until force quit */
    for(;;) {
        
        memset( &buf, '\0', sizeof(struct my_msgbuf));
        if (msgrcv(msqid, &buf, 8, 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        printf("Queue element: Type %li Label %o Value %i\n",
               buf.mtype, buf.label, buf.value);
    }
    
    return 0;
}


