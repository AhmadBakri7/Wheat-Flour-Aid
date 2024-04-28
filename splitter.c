#include "headers.h"
#include "functions.h"


int main(int argc, char *argv[]) {
     if (argc < 3) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    int safe_area_id = atoi(argv[1]);

    int max_energy_decay = atoi( strtok(argv[2], "-") );
    int min_energy_decay = atoi( strtok('\0', "-") );
    int energy = select_from_range(80, 100);

    printf("(Splitter) with pid (%d) is ready to receive container information ...\n",getpid());

    // Define a buffer to store received messages
    AidPackage received_from_collector;

    // Continuously receive and process container information
    while (1) {
        // Receive a message from the queue
        if (msgrcv(safe_area_id, &received_from_collector, sizeof(AidPackage), CONTAINER, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        // Print the received container information
        printf(
            "(SPLITTER) with pid (%d) Received Container Information: Type: %ld, Weight: %d\n",
            getpid(), received_from_collector.package_type, received_from_collector.weight
        );
        fflush(stdout);

        AidPackage bags[ received_from_collector.weight ];

        for (int i = 0; i < received_from_collector.weight; i++) {
            bags[i].package_type = KG_BAG;
            bags[i].weight = 1;

            // Send message to splitter
            if (msgsnd(safe_area_id, &bags[i], sizeof(AidPackage), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            printf(
                "(SPLITTER) with pid (%d) Sent Bag Information: Type: %ld, Weight: %d to Distributors\n",
                getpid(), bags[i].package_type, bags[i].weight
            );

            sleep( get_sleep_duration(energy) );
        }
        energy -= select_from_range(min_energy_decay, max_energy_decay);
    }

    printf("Splitter exited successfully\n");
    fflush(stdout);

    return 0;
}
