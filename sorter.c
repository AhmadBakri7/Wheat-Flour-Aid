#include "headers.h"

int fid;
int sid;
int number_of_families;
int starvation_rate_for_families[50];
int family_max_starvation_rate_index;
int sem, shmem;

familyStruct familia;

struct sembuf acquire = {0, -1, IPC_NOWAIT};
struct sembuf release = {0, 1, SEM_UNDO};

int get_num_of_bags(int starve_rate, float required_decrease, int starve_rate_decrease);
void write_shmem(int family_index, int num_bags, familyCritical* worst_fam);


int main(int argc, char* argv[]) {

    familyCritical* worst_fam;

    if (argc < 8) {
        perror("Not Enough Args, sorter.c");
        exit(-1);
    }

    fid = atoi(argv[1]);//the id for the families' message queue
    sid = atoi(argv[2]);//the id for the sorter's message queue
    number_of_families = atoi(argv[3]);

    sem = atoi(argv[4]);
    shmem = atoi(argv[5]);

    float required_decrease = ( atoi(argv[6]) / 100.0 );
    int starve_decrease = atoi(argv[7]);

    // attach the shared memory
    if ((worst_fam = (familyCritical*) shmat(shmem, NULL, 0)) == (familyCritical *) -1) {
        perror("shmat");
        exit(1);
    }

    printf("Hello from sorter with pid %d\n", getpid());
    fflush(NULL);

    for(int i=0; i<50; i++){
        starvation_rate_for_families[i] = 0;
    }

    struct msqid_ds buf;

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

            // write to shared mem
            int bags_required = get_num_of_bags(
                starvation_rate_for_families[family_max_starvation_rate_index],
                required_decrease,
                starve_decrease
            );

            write_shmem(family_max_starvation_rate_index, bags_required, worst_fam);
        }
    }
    return 0;
}

int get_num_of_bags(int starve_rate, float required_decrease, int starve_rate_decrease) {

    int num_bags = (int) round( (starve_rate * required_decrease) / starve_rate_decrease );

    return num_bags;
}

void write_shmem(int family_index, int num_bags, familyCritical* worst_fam) {

    acquire.sem_num = SORTER;

    // write the amplitude the shared memory
    if (semop(sem, &acquire, 1) == -1) {

        if (errno == EAGAIN) {
            // Semaphore operation would block (semaphore not available)
            return;
            // Handle the case when semaphore is not available immediately
        } else {
            // Other error occurred
            perror("semop");
            exit(1);
        }
    }

    /* critical section */
    worst_fam->family_index = family_index;
    worst_fam->num_bags_required = num_bags;

    printf("(Sorter--------%d----------------%d)\n", worst_fam->family_index, worst_fam->num_bags_required);

    /* end of critical section */

    release.sem_num = DISTRIBUTOR;

    // Release the semaphore (unlock)
    if (semop(sem, &release, 1) == -1) {
        perror("semop Release");
        exit(1);
    }
}

void sem_release(int sig) {
    
}