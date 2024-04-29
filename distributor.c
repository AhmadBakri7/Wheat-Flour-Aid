#include "headers.h"
#include "functions.h"

int sem, shmem;

struct sembuf acquire = {0, -1, SEM_UNDO};
struct sembuf release = {0, 1, SEM_UNDO};

familyCritical read_shmem(familyCritical* worst_fam);


int main(int argc, char *argv[]) {

    familyCritical* worst_fam;

    if (argc < 8) {
        perror("Not enough arguments\n");
        exit(-1);
    }
    
    int family_id = atoi(argv[4]);
    int safe_id = atoi(argv[1]);

    int min_energy_decay = atoi( strtok(argv[2], "-") );
    int max_energy_decay = atoi( strtok('\0', "-") );

    int min_start_energy = atoi( strtok(argv[5], "-") );
    int max_start_energy = atoi( strtok('\0', "-") );
    int energy = select_from_range(min_start_energy, max_start_energy);

    int DISTRIBUTOR_BAGS_TRIP_hold = atoi(argv[3]);

    sem = atoi(argv[6]);
    shmem = atoi(argv[7]);

    // attach the shared memory
    if ((worst_fam = (familyCritical*) shmat(shmem, NULL, 0)) == (familyCritical *) -1) {
        perror("shmat");
        exit(1);
    }

    printf("(distributor) with pid (%d) is ready to receive bag information ...\n",getpid());
    fflush(NULL);
    AidPackage bags[ DISTRIBUTOR_BAGS_TRIP_hold ];

    // Continuously receive and process container information
    while (1) {
        int count = 0; // Counter to keep track of the number of 10 kg bags stored

        // Receive a message from the queue
        for (int i = 0; i < DISTRIBUTOR_BAGS_TRIP_hold; i++) {
    
            if (msgrcv(safe_id, &bags[i], sizeof(AidPackage), KG_BAG, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }

            printf(
                "(Distributor) have Bag Information: Type: %ld, Weight: %d count = %d\n",
                bags[count].package_type, bags[count].weight, count
            ); // Print the received container information

            fflush(NULL);

            count++;
        }

        sleep( get_sleep_duration(energy) );
        printf("*****************************(Distributor) %d has reached family neighborhood*****************************\n", getpid());
        fflush(NULL);

        for (int i = 0; i < DISTRIBUTOR_BAGS_TRIP_hold; i++) {
            printf("-----(Distributor) Going to families-------\n");
            fflush(NULL);

            // read from shared memory
            familyCritical fam = read_shmem(worst_fam);

            printf("-----(Distributor) Has Received the most f*** family from (sorter), bags: %d-------, index %d\n", 
                    fam.num_bags_required, fam.family_index);
            fflush(NULL);

            int sent_bags = (count < fam.num_bags_required)? count : fam.num_bags_required;

            bags[count-1].package_type = fam.family_index;
            bags[count-1].weight = sent_bags;

            if (msgsnd(family_id, &bags[count-1], sizeof(AidPackage), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            count -= sent_bags;


            printf(
                "(Distributor) feeds family %d\n",
                fam.family_index
            );  
            fflush(NULL);
        
            energy -= select_from_range(min_energy_decay, max_energy_decay);

            sleep(1);

            if (count == 0)
                break;
        }
    }

}

familyCritical read_shmem(familyCritical* worst_fam) {

    familyCritical fam;

    acquire.sem_num = DISTRIBUTOR;

    // write the amplitude the shared memory
    if (semop(sem, &acquire, 1) == -1) {
        perror("semop Child");
        exit(1);
    }

    /* critical section */

    fam.family_index = worst_fam->family_index;
    fam.num_bags_required = worst_fam->num_bags_required;

    printf("(Distrib--------%d----------------%d)\n", worst_fam->family_index, worst_fam->num_bags_required);

    /* end of critical section */

    release.sem_num = SORTER;

    // Release the semaphore (unlock)
    if (semop(sem, &release, 1) == -1) {
        perror("semop Release");
        exit(1);
    }

    return fam;
}