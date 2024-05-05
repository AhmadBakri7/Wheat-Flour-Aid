#include "headers.h"
#include "functions.h"

void got_shot(int sig);

int energy;
int news_queue;
int my_number;
int drawer_queue;


int main(int argc, char *argv[]) {
    if (argc < 8) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    if (signal(SIGUSR2, got_shot) == SIG_ERR) {
        perror("SIGUSR2 Error in worker");
        exit(SIGQUIT);
    }

    key_t sky_key = strtol(argv[1], NULL, 10);
    key_t safe_key = strtol(argv[2], NULL, 10);
    news_queue = atoi(argv[5]);
    drawer_queue = atoi(argv[7]);
    my_number = atoi(argv[6]);

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

    // send info to drawer
    MESSAGE msg = {COLLECTOR, 0, .data.collector = {energy, my_number, 0, false}};

    if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
        perror("Child: msgsend");
        return 4;
    }

    printf("(Collector) with pid (%d) is ready to receive Containers,(ENERGY)=%d\n",  getpid(), energy);

    // Define a buffer to store received messages
    AidPackage received_containers_from_sky;
 
    // Continuously receive and process messages
    while (1) {
    
        // Receive a message from the queue
        if (msgrcv(sky_id, &received_containers_from_sky, sizeof(AidPackage), CONTAINER, 0) == -1) {
            perror("lkfgjdlkfgjdlfg");
            exit(-1);
        }

        // Print the received message
        printf(
            "(COLLECTOR) with pid (%d) Received Container: Type: %ld, Weight: %d\n",
            getpid(),received_containers_from_sky.package_type, received_containers_from_sky.weight
        );
        fflush(stdout);

        // send info to drawer
        MESSAGE msg = {COLLECTOR, 1, .data.collector = {energy, my_number, 1, false}};

        if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
            return 4;
        }

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

        // send info to drawer
        msg.type = COLLECTOR;
        msg.operation = 2;
        msg.data.collector.energy = energy;
        msg.data.collector.containers = 0;
        msg.data.collector.number = my_number;
        msg.data.collector.killed = false;

        if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
            return 4;
        }
    }
       
    printf("Collector exited successfully\n");
    fflush(NULL);

    return 0;
}


void got_shot(int sig) {
    int die_probability = 100 - energy;

    bool die = select_from_range(1, 100) <= die_probability;

    if (die) {

        // send info to drawer
        MESSAGE msg = {COLLECTOR, .data.collector = {energy, my_number, 0, true}};

        if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
        }

        alert_news(news_queue, COLLECTOR, my_number);
        printf("Worker %d is killed\n", getpid());
        exit(-1);
    }
}
