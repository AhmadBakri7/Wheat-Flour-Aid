#include "headers.h"

int fid;
int sid;
int number_of_families;
int starvation_rate_for_families[50];
int family_max_starvation_rate_index;
familyStruct familia;

int main(int argc, char* argv[]) {

    if (argc < 4) {
        perror("Not Enough Args, sorter.c");
        exit(-1);
    }
    sleep(1);

    fid = atoi(argv[1]);//the id for the families' message queue
    sid = atoi(argv[2]);//the id for the sorter's message queue
    number_of_families = atoi(argv[3]);

    printf("Hello from sorter with pid %d\n", getpid());
    fflush(NULL);

    for(int i=0; i<50; i++){
        starvation_rate_for_families[i] = 0;
    }

    struct msqid_ds buf;

    while (1) {

        msgctl(sid, IPC_STAT, &buf);

        if(buf.msg_qnum>0){
            for(int i = 1; i < (number_of_families+1); i++) {

                if (msgrcv(sid, &familia, sizeof(familyStruct), i, IPC_NOWAIT) != -1) {
                    starvation_rate_for_families[i] = familia.starvationRate;
                    printf("(sorter) received strv from (family) index %ld, strv %d\n", familia.familyIndex, familia.starvationRate);
                    fflush(NULL);         
                } 

            }

            // find the maximum
            family_max_starvation_rate_index = 1;

            for(int i = 1; i < (number_of_families+1); i++) {

                if(starvation_rate_for_families[i] > starvation_rate_for_families[family_max_starvation_rate_index]) {
                        family_max_starvation_rate_index = i;
                }
            }

            familyCritical criticalFamily;
            criticalFamily.type = SORTER_VALUE;
            criticalFamily.familyIndex = family_max_starvation_rate_index;

            printf("sorter calculated that max is family index %d strv %d\n", 
                    family_max_starvation_rate_index, starvation_rate_for_families[family_max_starvation_rate_index]);
            fflush(NULL);
 
            familyCritical emptyQueue;
            msgrcv(fid, &emptyQueue, sizeof(familyCritical), SORTER_VALUE, IPC_NOWAIT);
            msgsnd(fid, &criticalFamily, sizeof(familyCritical), 0);
        }
    }
    return 0;
}