#include "headers.h"
#include "functions.h"

int energy;
int news_queue;

int main(int argc, char *argv[]) {

    if (argc < 7) {
        perror("Not enough arguments\n");
        exit(-1);
    }
    
    int family_queue = atoi(argv[4]);
    int safe_queue = atoi(argv[1]);
    news_queue = atoi(argv[6]);

    int min_energy_decay = atoi( strtok(argv[2], "-") );
    int max_energy_decay = atoi( strtok('\0', "-") );

    int min_start_energy = atoi( strtok(argv[5], "-") );
    int max_start_energy = atoi( strtok('\0', "-") );
    energy = select_from_range(min_start_energy, max_start_energy);

    int DISTRIBUTOR_BAGS_TRIP_hold = atoi(argv[3]);

    printf("(distributor) with pid (%d) is ready to receive bag information ...\n",getpid());
    fflush(NULL);
    AidPackage bags[ DISTRIBUTOR_BAGS_TRIP_hold ];

    // Continuously receive and process container information
    while (1) {
        int count = 0; // Counter to keep track of the number of 10 kg bags stored

        // Receive a message from the queue
        for (int i = 0; i < DISTRIBUTOR_BAGS_TRIP_hold; i++) {
    
            if (msgrcv(safe_queue, &bags[i], sizeof(AidPackage), KG_BAG, 0) == -1) {
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

            // read from msg queue
            familyCritical worst_family;

            if (msgrcv(family_queue, &worst_family, sizeof(familyCritical), SORTER_VALUE, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }

            printf("-----(Distributor) Has Received the most f*** family from (sorter), bags: %d-------, index %d\n", 
                    worst_family.num_bags_required, worst_family.family_index);
            fflush(NULL);

            int sent_bags = (count < worst_family.num_bags_required)? count : worst_family.num_bags_required;

            bags[count-1].package_type = worst_family.family_index;
            bags[count-1].weight = sent_bags;

            if (msgsnd(family_queue, &bags[count-1], sizeof(AidPackage), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            count -= sent_bags;


            printf(
                "(Distributor) feeds family %d\n",
                worst_family.family_index
            );
            fflush(NULL);
        
            energy -= select_from_range(min_energy_decay, max_energy_decay);

            sleep(1);

            if (count == 0)
                break;
        }
    }
    return 0;
}


void shoot_worker(int sig) {
    int die_probability = 100 - energy;

    bool die = select_from_range(1, 100) <= die_probability;

    if (die) {
        alert_news(news_queue, DISTRIBUTOR);
        printf("Worker %d is killed\n", getpid());
        exit(-1);
    }
}

