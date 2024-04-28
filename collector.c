#include "headers.h"
#include "functions.h"

void shoot_worker(int sig);

int energy;


int main(int argc, char *argv[]) {
    if (argc < 5) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    if (signal(SIGUSR2, shoot_worker) == SIG_ERR) {
        perror("SIGUSR2 Error in worker");
        exit(SIGQUIT);
    }

    key_t sky_key = strtol(argv[1], NULL, 10); //convert it to long
    key_t safe_key = strtol(argv[2], NULL, 10); //convert it to long

    int max_energy_decay = atoi( strtok(argv[3], "-") );
    int min_energy_decay = atoi( strtok('\0', "-") );

    int min_start = atoi( strtok(argv[4], "-") );
    int max_start = atoi( strtok('\0', "-") );

    energy = select_from_range(min_start, max_start);

    // Create sky message queue
    int sky_id = msgget(sky_key, IPC_CREAT | 0666);

    if (sky_id == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    int safe_id = msgget(safe_key, IPC_CREAT | 0666);

    if (safe_id == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("(Collector) with pid (%d) is ready to receive Containers,(ENERGY)=%d\n",  getpid(), energy);

    // Define a buffer to store received messages
    AidPackage received_containers_from_sky;
 
    // Continuously receive and process messages
    while (1) {
    
        // Receive a message from the queue
        while (msgrcv(sky_id, &received_containers_from_sky, sizeof(AidPackage), CONTAINER, 0) == -1) {
            printf("(Collector) with pid (%d) is waiting to receive more containers,(ENERGY)=%d\n", getpid(),energy);
            fflush(stdout);
        }

        // Print the received message
        printf(
            "(COLLECTOR) with pid (%d) Received Container: Type: %ld, Weight: %d\n",
            getpid(),received_containers_from_sky.package_type, received_containers_from_sky.weight
        );
        fflush(stdout);

        sleep( get_sleep_duration(energy) );

        // Send message to splitter
        if (msgsnd(safe_id, &received_containers_from_sky, sizeof(AidPackage), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        // Print a message indicating that the container information has been sent to the splitter
        printf("(Container) information sent to splitter by pid (%d)\n", getpid());
        fflush(stdout);

        energy -= select_from_range(min_energy_decay, max_energy_decay);
    }
       
    printf("Collector exited successfully\n");
    fflush(NULL);

    return 0;
}


void shoot_worker(int sig) {
    int die_probability = 100 - energy;

    bool die = select_from_range(1, 100) <= die_probability;

    if (die) {
        printf("Worker %d is killed\n", getpid());
        exit(-1);
    }
}

