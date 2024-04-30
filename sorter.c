#include "headers.h"

int fid;
int sid;
int number_of_families;
int starvation_rate_for_families[50];
int family_max_starvation_rate_index;

int get_num_of_bags(int starve_rate, float required_decrease, int starve_rate_decrease);


int main(int argc, char* argv[]) {

    if (argc < 6) {
        perror("Not Enough Args, sorter.c");
        exit(-1);
    }

    fid = atoi(argv[1]);//the id for the families' message queue
    sid = atoi(argv[2]);//the id for the sorter's message queue
    number_of_families = atoi(argv[3]);


    float required_decrease = ( atoi(argv[4]) / 100.0 );
    int starve_decrease = atoi(argv[5]);

    printf("Hello from sorter with pid %d\n", getpid());
    fflush(NULL);

    for(int i=0; i<50; i++){
        starvation_rate_for_families[i] = 0;
    }

    struct msqid_ds buf;
    familyStruct familia;

    while (1) {

        msgctl(sid, IPC_STAT, &buf);

        if(buf.msg_qnum > 0) {

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
            
            printf("(sorter) calculated that max is family index %d strv %d\n",
                    family_max_starvation_rate_index, starvation_rate_for_families[family_max_starvation_rate_index]);
            fflush(NULL);
 
            familyCritical emptyQueue;
            msgrcv(fid, &emptyQueue, sizeof(familyCritical), SORTER_VALUE, IPC_NOWAIT);

            // write to shared mem
            int bags_required = get_num_of_bags(
                starvation_rate_for_families[family_max_starvation_rate_index],
                required_decrease,
                starve_decrease
            );
            
            familyCritical worst_family;
            worst_family.type = SORTER_VALUE;
            worst_family.family_index = family_max_starvation_rate_index;
            worst_family.num_bags_required = bags_required;

            msgsnd(fid, &worst_family, sizeof(familyCritical), 0);
        }
    }
    return 0;
}

int get_num_of_bags(int starve_rate, float required_decrease, int starve_rate_decrease) {

    int num_bags = (int) round( (starve_rate * required_decrease) / starve_rate_decrease );

    return num_bags;
}