#include "headers.h"
#include "functions.h"

bool check_queue_empty(int queue, long msg_type);
void worker_has_died(int sig);

int size = 0;
pid_t* workers;
int random_worker;

int main(int argc, char* argv[]) {

    if (argc < 6) {
        perror("Not Enough Args, occupation.c");
        exit(-1);
    }

    size = atoi(argv[3]);
    int msg_queue_id = atoi(argv[4]);
    int brutality = atoi(argv[5]);

    pid_t sky_pid = atoi(argv[1]);
    workers = (pid_t*) malloc(sizeof(pid_t) * size);

    char* str = strtok(argv[2], ",");

    workers[0] = atoi(str);

    for (int i = 1; i < size; i++)
        workers[i] = atoi(strtok('\0', ","));

    for (int i = 0; i < size; i++) {
        printf("Worker (%d) has pid: %d\n", i, workers[i]);
        fflush(stdout);
    }

    if ( signal(SIGUSR1, worker_has_died) == SIG_ERR ) {
        perror("Signal USR1 Error (occupation)\n");
        exit(SIGQUIT);
    }

    while (1) {

        sleep(5);

        bool commit_war_crime = ( select_from_range(1, 100) ) <= brutality;

        if (commit_war_crime) {

            kill(sky_pid, SIGUSR1); // missile
            printf("Occupation shooting a missile at packages\n");
            fflush(stdout);

            // shoot a worker
            random_worker = select_from_range(0, size-1);

            kill(workers[random_worker], SIGUSR2);
            printf("Occupation shooting at worker %d\n", workers[random_worker]);
            fflush(stdout);

            sleep(1);
        }
    }
    free(workers);

    return 0;
}

bool check_queue_empty(int queue, long msg_type) {

    if(msgrcv(queue, NULL, 0, msg_type, IPC_NOWAIT) == -1) {
        if(errno == E2BIG)
            return false; // There is data

    }
    return true; // Empty queue
}

void worker_has_died(int sig) {
    workers[random_worker] = workers[size-1];
    size--;
}