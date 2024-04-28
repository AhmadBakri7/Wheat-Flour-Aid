#include "headers.h"

int fid;
int sid;
int number_of_families;
int starvation_rate_for_families[50];
int family_max_starvation_rate_index;

int main(int argc, char* argv[]) {

    if (argc < 4) {
        perror("Not Enough Args, sorter.c");
        exit(-1);
    }

    fid = atoi(argv[1]);//the id for the families' message queue
    sid = atoi(argv[2]);//the id for the sorter's message queue
    number_of_families = atoi(argv[3]);

    printf("Hello from sorter with pid %d\n", getpid());

    AidPackage bag;
    familyStruct familia;
    bool msg_received = false;

    while (1) {

        for(int i = 0; i < number_of_families; i++) {

            msgrcv(sid, &familia, sizeof(familyStruct), i + 1, IPC_NOWAIT);
            
            if (errno != ENOMSG) {
                msg_received = true;
                starvation_rate_for_families[i] = familia.starvationRate;
            }
        }

        if (msg_received) {
            // find the maximum
            family_max_starvation_rate_index = 0;

            for(int i = 0; i < number_of_families; i++) {

                if(starvation_rate_for_families[i] > starvation_rate_for_families[family_max_starvation_rate_index]) {
                    family_max_starvation_rate_index = i;
                }
            }

            familyCritical criticalFamily;
            criticalFamily.type = SORTER_VALUE;
            criticalFamily.familyIndex = family_max_starvation_rate_index + 1;

            msgsnd(fid, &criticalFamily, sizeof(familyCritical), 0);

            msg_received = false;
        }
    }
    return 0;
}