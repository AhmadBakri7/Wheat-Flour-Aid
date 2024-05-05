#include "headers.h"
#include "functions.h"

void change_to_collector(int sig);
void change_to_distributor(int sig);

int safe_area_id;
int drawer_queue;
int energy;
int my_number;


int main(int argc, char *argv[]) {

    if (argc < 6) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    if (signal(SIGRTMIN, change_to_collector) == SIG_ERR) {
        perror("Signal min error (Splitter)");
        exit(SIGQUIT);
    }

    if (signal(SIGRTMAX, change_to_distributor) == SIG_ERR) {
        perror("Signal max error (Splitter)");
        exit(SIGQUIT);
    }

    safe_area_id = atoi(argv[1]);

    int max_energy_decay = atoi( strtok(argv[2], "-") );
    int min_energy_decay = atoi( strtok('\0', "-") );

    int min_energy_start = atoi( strtok(argv[3], "-") );
    int max_energy_start = atoi( strtok('\0', "-") );
    energy = select_from_range(min_energy_start, max_energy_start);

    drawer_queue = atoi(argv[4]);
    my_number = atoi(argv[5]);

    printf("(Splitter) with pid (%d) is ready to receive container information ...\n",getpid());

    // send info to drawer
    MESSAGE msg = {SPLITTER, 0, .data.splitter = {energy, my_number, 0, false}};

    if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
        perror("Child: msgsend");
        exit(-1);
    }

    AidPackage received_from_collector;

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

            sleep( get_sleep_duration(energy) );

            msg.type = SPLITTER;
            msg.operation = 1;
            msg.data.splitter.energy = energy;
            msg.data.splitter.weight = received_from_collector.weight - i;
            msg.data.splitter.number = my_number;
            msg.data.splitter.swapped = false;

            if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
                perror("Child: msgsend");
                exit(-1);
            }

            bags[i].package_type = KG_BAG;
            bags[i].weight = 1;

            if (msgsnd(safe_area_id, &bags[i], sizeof(AidPackage), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            printf(
                "(SPLITTER) with pid (%d) Sent Bag Information: Type: %ld, Weight: %d to Distributors\n",
                getpid(), bags[i].package_type, bags[i].weight
            );
        }

        energy -= select_from_range(min_energy_decay, max_energy_decay);

        msg.type = SPLITTER;
        msg.operation = 2;
        msg.data.splitter.energy = energy;
        msg.data.splitter.weight = 0;
        msg.data.splitter.number = my_number;
        msg.data.splitter.swapped = false;

        if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
            exit(-1);
        }
        
        sleep( get_sleep_duration(energy) );
    }

    printf("Splitter exited successfully\n");
    fflush(stdout);

    return 0;
}

void change_to_collector(int sig) {

    SwapInfo info;

    // Receive a message from the queue
    if (msgrcv(safe_area_id, &info, sizeof(info), EMERGENCY, 0) == -1) {
        perror("msgrcv ccccccc");
        exit(EXIT_FAILURE);
    }

    printf("(SPLITTER) Args From MAin%s\n", info.arguments);

    // int argc = atoi( strtok(buffer, ",") );

    char* arg1 = strtok(info.arguments, ",");
    char* arg2 = strtok('\0', ",");
    char* arg3 = strtok('\0', ",");
    char* arg5 = strtok('\0', ",");
    char* arg6 = strtok('\0', ",");

    char arg7[20];
    sprintf(arg7, "%d", drawer_queue);

    char arg4[20];
    sprintf(arg4, "%d-%d", energy, energy);

    printf("(SPLITTER) %d is changing to Collector\n", getpid());
    fflush(stdout);

    execlp("./collector", "collector", arg1, arg2, arg3, arg4, arg5, arg6, arg7, NULL);

    // send info to drawer
    MESSAGE msg = {SPLITTER, .data.splitter = {energy, my_number, true}};

    if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
        perror("Child: msgsend");
    }

    perror("Exec to collector error in (SPLITTER)");
    exit(SIGQUIT);
}

void change_to_distributor(int sig) {

    SwapInfo info;

    // Receive a message from the queue
    if (msgrcv(safe_area_id, &info, sizeof(info), EMERGENCY, 0) == -1) {
        perror("msgrcv Ssssss");
        exit(EXIT_FAILURE);
    }

    printf("(SPLITTER) Args From MAin%s\n", info.arguments);

    // int argc = atoi( strtok(buffer, ",") );

    char* arg1 = strtok(info.arguments, ",");
    char* arg2 = strtok('\0', ",");
    char* arg3 = strtok('\0', ",");
    char* arg4 = strtok('\0', ",");
    char* arg6 = strtok('\0', ",");
    char* arg7 = strtok('\0', ",");

    char arg8[20];
    sprintf(arg8, "%d", drawer_queue);

    char arg5[20];
    sprintf(arg5, "%d-%d", energy, energy);

    printf("(SPLITTER) %d is changing to Distributor\n", getpid());
    fflush(stdout);

    // send info to drawer
    MESSAGE msg = {SPLITTER, .data.splitter = {energy, my_number, true}};

    if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
        perror("Child: msgsend");
    }

    execlp("./distributor", "distributor", arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, NULL);

    perror("Exec to distributor error in (SPLITTER)");
    exit(SIGQUIT);
}
