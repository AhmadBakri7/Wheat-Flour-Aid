#include "headers.h"

#define DEFAULT_SETTINGS "settings.txt"

static int NUM_PLANES;
static int NUM_COLLECTORS;
static int NUM_SPLITTERS;
static int NUM_DISTRIBUTORS;
static int NUM_FAMILIES;
static char CARGO_SIZE_RANGE[10];
static char REFILL_RANGE[10];
static char AMPLITUDE_RANGE[10];
static char WORKERS_ENERGY_DECAY[10];
static char WEIGHT_PER_CONTAINER[10];
static char WORKERS_START_ENERGY[10];
static int DROP_PERIOD;
static int DISTRIBUTOR_BAGS_TRIP;
static int OCCUPATION_BRUTALITY;


void readFile(char* filename);
void program_exit(int sig);

int sky_id, safe_id;

int main(int argc, char* argv[]) {

    if (argc < 2) {
        readFile(DEFAULT_SETTINGS);
    } else {
        readFile(argv[1]);
    }

    if ( signal(SIGINT, program_exit) == SIG_ERR ) {
        perror("SIGINT Error in main");
        exit(SIGQUIT);
    }

    printf("Args :%d, %s\n", NUM_PLANES, CARGO_SIZE_RANGE);

    key_t sky_queue_key = ftok(".", 'Q');
    key_t safe_area_key = ftok(".", 'S');

    if ( (sky_id = msgget(sky_queue_key, IPC_CREAT | 0770)) == -1 ) {
        perror("Queue create");
        exit(1);
    }

    if ( (safe_id = msgget(safe_area_key, IPC_CREAT | 0770)) == -1 ) {
        perror("Queue create");
        exit(1);
    }

    pid_t planes[NUM_PLANES];             /* pids for all planes */
    pid_t collectors[NUM_COLLECTORS];     /* pids for all collectors */
    pid_t splitters[NUM_SPLITTERS];       /* pids for all splitters */
    pid_t distributors[NUM_DISTRIBUTORS]; /* pids for all distributors */
    pid_t families[NUM_FAMILIES];         /* pids for all families */

    // fork planes
    for (int i = 0; i < NUM_PLANES; i++) {
        planes[i] = fork();

        // child
        if (planes[i] == 0) {
            char m_id[20];
            char drop_p[20];
            sprintf(m_id, "%d", sky_id);
            sprintf(drop_p, "%d", DROP_PERIOD);
            
            execlp(
                "./plane", "plane", m_id,
                CARGO_SIZE_RANGE, AMPLITUDE_RANGE,
                drop_p, REFILL_RANGE, NULL
            );
            perror("Exec Plane Error");
            exit(SIGQUIT);
        }
    }

    // Fork child processes
    for (int i = 0; i < NUM_COLLECTORS; i++) {
        // Fork a child process
        collectors[i] = fork();

        if (collectors[i] == -1) {
            perror("fork");
            exit(EXIT_FAILURE);


        } else if (collectors[i] == 0) { // Child process
            char sky_key[20];
            char safe_key[20];
            char energy_collector[20];
            
            sprintf(sky_key, "%d", (int)sky_queue_key);
            sprintf(safe_key, "%d", safe_area_key);

            execlp(
                "./collector", "collector",
                sky_key, safe_key,
                WORKERS_ENERGY_DECAY, WORKERS_START_ENERGY, NULL
            );
            perror("execlp");
            exit(EXIT_FAILURE);
        }
    }

    // fork splitters
    for (int i = 0; i < NUM_SPLITTERS; i++) {
        splitters[i] = fork();

        if (splitters[i] == 0) {
            char msgqueue_id[20];

            sprintf(msgqueue_id, "%d", safe_id);
            execlp(
                "./splitter", "splitter",
                msgqueue_id,
                WORKERS_ENERGY_DECAY, WORKERS_START_ENERGY, NULL
            );
            perror("execlp");
            exit(EXIT_FAILURE);
        }
    }

    // fork sky
    pid_t sky_process = fork();

    if (sky_process == 0) {
        char m_id[20];
        sprintf(m_id, "%d", sky_id);

        execlp("./sky", "sky", m_id, NULL);
        perror("Exec Sky Error");
        exit(SIGQUIT);
    }

    // fork occupation
    pid_t occupation = fork();

    if (occupation == 0) {
        char sky_pid[20];
        char workers[1000];
        char num_workers[20];
        char sky_parachutes_id[20];
        char brutality[20];

        strcpy(workers, "");

        for (int i = 0; i < NUM_COLLECTORS; i++) {
            char worker_pid[20];
            sprintf(worker_pid, "%d", collectors[i]);

            strcat(workers, worker_pid);
            strcat(workers, ",");
        }
        for (int i = 0; i < NUM_DISTRIBUTORS; i++) {
            char worker_pid[20];
            sprintf(worker_pid, "%d", distributors[i]);

            strcat(workers, worker_pid);
            strcat(workers, ",");
        }

        sprintf(sky_pid, "%d", sky_process);
        sprintf(num_workers, "%d", NUM_COLLECTORS + NUM_DISTRIBUTORS);
        sprintf(sky_parachutes_id, "%d", sky_queue_key);
        sprintf(brutality, "%d", OCCUPATION_BRUTALITY);


        execlp("./occupation", "occupation", sky_pid, workers, num_workers, sky_parachutes_id, brutality, NULL);

        perror("Occupation Exec Error");
        exit(SIGQUIT);
    }

    for (int i = 0; i < NUM_COLLECTORS; i++) {
        int status;
        pid_t killed_worker = wait(&status);

        printf("Worker %d is Killed (main) status %d\n", killed_worker, WEXITSTATUS(status));

        // killed by occupation
        if ( WEXITSTATUS(status) ) {
            printf("Worker %d shot by occupation\n", i);
            kill(occupation, SIGUSR1);
        }
    }

#ifdef SLEEP
    sleep(SLEEP);
#endif

    for (int i = 0; i < NUM_PLANES; i++) {
        kill(planes[i], SIGINT);
        wait(NULL);
    }
    for (int i = 0; i < NUM_COLLECTORS; i++) {
        kill(collectors[i], SIGINT);
        wait(NULL);
    }
    for (int i = 0; i < NUM_SPLITTERS; i++) {
        kill(splitters[i], SIGINT);
        wait(NULL);
    }

    kill(sky_process, SIGINT);
    waitpid(sky_process, NULL, 0);


#ifdef DELETE
    if ( msgctl(sky_id, IPC_RMID, (struct msqid_ds *) 0)) {
       perror("msgctl");
        exit(EXIT_FAILURE); 
    }

    if (msgctl(safe_id, IPC_RMID, (struct msqid_ds *) 0) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
#endif

    return 0;
}

void readFile(char* filename) {
    char line[200];
    char label[50];

    FILE *file;
    file = fopen(filename, "r");

    if (file == NULL) {
        perror("The file not exist\n");
        exit(-2);
    }

    char separator[] = "=";

    while(fgets(line, sizeof(line), file) != NULL){

        char* str = strtok(line, separator);
        strncpy(label, str, sizeof(label));
        str = strtok(NULL, separator);

        if (strcmp(label, "CARGO_PLANES") == 0){
            NUM_PLANES = atoi(str);
        } else if (strcmp(label, "NUM_COLLECTORS") == 0){
            NUM_COLLECTORS = atoi(str);
        } else if (strcmp(label, "NUM_SPLITTERS") == 0){
            NUM_SPLITTERS = atoi(str);
        } else if (strcmp(label, "CARGO_SIZE_RANGE") == 0){
            strcpy(CARGO_SIZE_RANGE, str);
        } else if (strcmp(label, "DROP_PERIOD") == 0){
            DROP_PERIOD = atoi(str);
        } else if (strcmp(label, "REFILL_RANGE") == 0){
            strcpy(REFILL_RANGE, str);
        } else if (strcmp(label, "AMPLITUDE_RANGE") == 0){
            strcpy(AMPLITUDE_RANGE, str);
        } else if (strcmp(label, "WORKERS_ENERGY_DECAY") == 0){
            strcpy(WORKERS_ENERGY_DECAY, str);
        } else if (strcmp(label, "OCCUPATION_BRUTALITY") == 0){
            OCCUPATION_BRUTALITY = atoi(str);
        } else if (strcmp(label, "WORKERS_START_ENERGY") == 0){
            strcpy(WORKERS_START_ENERGY, str);
        }
    }

    fclose(file);
}

void program_exit(int sig) {

    if ( msgctl(sky_id, IPC_RMID, (struct msqid_ds *) 0)) {
        perror("msgctl");
        exit(EXIT_FAILURE); 
    }

    if (msgctl(safe_id, IPC_RMID, (struct msqid_ds *) 0) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    exit(0);
}
