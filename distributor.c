#include "headers.h"
#include "functions.h"


int main(int argc, char *argv[]) {
    if (argc < 6) {
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

        for (int i = 0; i < DISTRIBUTOR_BAGS_TRIP_hold; i++) {
            printf("-----(Distributor) Going to families-------\n");

            familyCritical fam;

            if (msgrcv(family_id, &fam, sizeof(familyCritical), SORTER_VALUE, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
            printf("-----(Distributor) Has Received the most f*** family from (sorter)-------\n");

            bags[count-1].package_type = fam.familyIndex;

            if (msgsnd(family_id, &bags[count-1], sizeof(AidPackage), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            count--;

            printf(
                "(Distributor) feeds family %d\n",
                fam.familyIndex
            );  
            fflush(NULL);
        
            energy -= select_from_range(min_energy_decay, max_energy_decay);

            sleep(1);
        }
    }

}