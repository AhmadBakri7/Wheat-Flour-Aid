#include "headers.h"

#define DEFAULT_SETTINGS "settings.txt"

static int NUM_PLANES;
static int NUM_COLLECTORS;
static int NUM_FAMILIES;
static int NUM_SPLITTERS;
static int NUM_DISTRIBUTORS;
static int FAMILIES_STARVATION_RATE_INCREASE;
static int FAMILIES_STARVATION_RATE_DECREASE;
static int FAMILIES_INCREASE_ALARM;
static int FAMILIES__STARVATION_SURVIVAL_THRESHOLD;
static int DROP_LOST_THRESHOLD;
static char CARGO_SIZE_RANGE[10];
static char REFILL_RANGE[10];
static char AMPLITUDE_RANGE[10];
static char WORKERS_ENERGY_DECAY[10];
static char WEIGHT_PER_CONTAINER[10];
static char FAMILIES_STARVATION_RATE_RANGE[10];
static char WORKERS_START_ENERGY[10];
static int DROP_PERIOD;
static int PLANE_SAFE_DISTANCE;
static int DISTRIBUTOR_BAGS_TRIP;
static int OCCUPATION_BRUTALITY;


void readFile(char* filename);
void create_message_queues(key_t, key_t, key_t);
void program_exit(int sig);

int sky_id, safe_id, family_id, sorter_id, plane_sem, plane_shmem;

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
    key_t families_key = ftok(".", 'F');
    key_t sorter_key = ftok(".", 'R');
    key_t psem_key = ftok(".", ('P' + 'S'));
    key_t pshmem_key = ftok(".", ('P' + 'M'));


    create_message_queues(sky_queue_key, safe_area_key, families_key);

    printf("Fam_Key: %d, Safe_area_key: %d\n", family_id, safe_id);
    
    if ( (sorter_id = msgget(sorter_key, IPC_CREAT | 0770)) == -1 ) {
        perror("Queue create");
        exit(1);
    }

    // create or retrieve the semaphore
    if ( (plane_sem = semget(pshmem_key, 1, IPC_CREAT | 0660)) == -1 ) {
        perror("semget: IPC_CREAT | 0660");
    }

    // initialize semaphore
    union semun su;
    su.val = 1;
    if (semctl(plane_sem, 0, SETVAL, su) < 0) {
        perror("semctl");
        exit(1);
    }

    // Create or retrieve the shared memory segment
    if ((plane_shmem = shmget(psem_key, sizeof(AirSpace) * NUM_PLANES, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
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
            char m_id[20], drop_p[20], max_planes[20], num[20];
            char sem[20], shmem[20];
            char safe_distance[20];

            sprintf(m_id, "%d", sky_id);
            sprintf(drop_p, "%d", DROP_PERIOD);
            sprintf(sem, "%d", plane_sem);
            sprintf(shmem, "%d", plane_shmem);
            sprintf(max_planes, "%d", NUM_PLANES);
            sprintf(num, "%d", i);
            sprintf(safe_distance, "%d", PLANE_SAFE_DISTANCE);
            
            execlp(
                "./plane", "plane", m_id,
                CARGO_SIZE_RANGE, AMPLITUDE_RANGE, WEIGHT_PER_CONTAINER,
                drop_p, REFILL_RANGE,
                sem, shmem, max_planes, num, safe_distance, NULL
            );
            perror("Exec Plane Error");
            exit(SIGQUIT);
        }
    }

    // Fork collectors
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

    // fork distributors
    for (int i = 0; i < NUM_DISTRIBUTORS; i++) {
        distributors[i] = fork();

        if (distributors[i] == 0) {
            char msgqueue_id_safe[20];
            char num_of_can_hold[20];
            char msgqueue_family[20];

            sprintf(msgqueue_id_safe, "%d", safe_id);
            sprintf(num_of_can_hold, "%d", DISTRIBUTOR_BAGS_TRIP);
            sprintf(msgqueue_family, "%d", family_id);

            execlp("./distributor", "distributor", msgqueue_id_safe, WORKERS_ENERGY_DECAY, num_of_can_hold, msgqueue_family, WORKERS_START_ENERGY, NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        }
    }

    // fork families
    for (int i = 0; i < NUM_FAMILIES; i++) {
        families[i] = fork();

        if (families[i] == 0) {
            char f_id[20];
            char STARVATION_RATE_INCREASE[20], STARVATION_RATE_DECREASE[20],
                 INCREASE_ALARM[20], SURVIVAL_THRESHOLD[20], i_char[20], sorter[20];

            sprintf(f_id, "%d", family_id);
            sprintf(STARVATION_RATE_INCREASE, "%d", FAMILIES_STARVATION_RATE_INCREASE);
            sprintf(STARVATION_RATE_DECREASE, "%d", FAMILIES_STARVATION_RATE_DECREASE);

            sprintf(INCREASE_ALARM, "%d", FAMILIES_INCREASE_ALARM);
            sprintf(SURVIVAL_THRESHOLD, "%d", FAMILIES__STARVATION_SURVIVAL_THRESHOLD);
            sprintf(i_char, "%d", i);
            sprintf(sorter, "%d", sorter_id);
            
            execlp(
                "./families", "families", f_id, FAMILIES_STARVATION_RATE_RANGE, 
                STARVATION_RATE_INCREASE, STARVATION_RATE_DECREASE, 
                INCREASE_ALARM, SURVIVAL_THRESHOLD, i_char, sorter, NULL
            );
            perror("Exec families Error");
            exit(SIGQUIT);
        }
    }

    // fork sky
    pid_t sky_process = fork();

    if (sky_process == 0) {
        char m_id[20];
        char drop[20];
        sprintf(m_id, "%d", sky_id);
        sprintf(drop, "%d", DROP_LOST_THRESHOLD);

        execlp("./sky", "sky", m_id, drop, NULL);
        perror("Exec Sky Error");
        exit(SIGQUIT);
    }

    // fork sorter
    pid_t sorter_process = fork();

    if (sorter_process == 0) {
        char f_id[20];
        char s_id[20];
        char number[20];
        sprintf(f_id, "%d", family_id);
        sprintf(s_id, "%d", sorter_id);
        sprintf(number, "%d", NUM_FAMILIES);

        execlp("./sorter", "sorter", f_id, s_id, number, NULL);
        perror("Exec Sorter Error");
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

        // killed by occupation
        if ( WEXITSTATUS(status) ) {
            printf("Worker %d shot by occupation\n", i);
            printf("-----Worker %d is Killed (main) status %d\n", killed_worker, WEXITSTATUS(status));
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
    for (int i = 0; i < NUM_DISTRIBUTORS; i++) {
        kill(distributors[i], SIGINT);
        wait(NULL);
    }
    for (int i = 0; i < NUM_FAMILIES; i++) {
        kill(families[i], SIGINT);
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

    if (msgctl(family_id, IPC_RMID, (struct msqid_ds *) 0) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    if (msgctl(sorter_id, IPC_RMID, (struct msqid_ds *) 0) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    if ( semctl(plane_sem, IPC_RMID, 0) == -1 ) {
        perror("semctl: IPC_RMID");	/* remove semaphore */
        exit(5);
    }

    if ( shmctl(plane_shmem, IPC_RMID, (struct shmid_ds *) 0) == -1 ) {
        perror("shmid: IPC_RMID");	/* remove semaphore */
        exit(5);
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

        } else if (strcmp(label, "NUM_FAMILIES") == 0){
            NUM_FAMILIES = atoi(str);

        } else if (strcmp(label, "NUM_COLLECTORS") == 0){
            NUM_COLLECTORS = atoi(str);
            
        } else if (strcmp(label, "NUM_SPLITTERS") == 0){
            NUM_SPLITTERS = atoi(str);

        } else if (strcmp(label, "CARGO_SIZE_RANGE") == 0){
            strcpy(CARGO_SIZE_RANGE, str);

        } else if (strcmp(label, "WEIGHT_PER_CONTAINER") == 0){
            strcpy(WEIGHT_PER_CONTAINER, str);

        } else if (strcmp(label, "DROP_PERIOD") == 0){
            DROP_PERIOD = atoi(str);

        } else if (strcmp(label, "PLANE_SAFE_DISTANCE") == 0){
            PLANE_SAFE_DISTANCE = atoi(str);

        } else if (strcmp(label, "DROP_LOST_THRESHOLD") == 0){
            DROP_LOST_THRESHOLD = atoi(str);

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

        } else if (strcmp(label, "DISTRIBUTOR_BAGS_TRIP") == 0){
            DISTRIBUTOR_BAGS_TRIP = atoi(str);

        } else if (strcmp(label, "NUM_DISTRIBUTORS") == 0){
            NUM_DISTRIBUTORS = atoi(str);

        } else if (strcmp(label, "FAMILIES_STARVATION_RATE_RANGE") == 0){
            strcpy(FAMILIES_STARVATION_RATE_RANGE, str);

        } else if (strcmp(label, "FAMILIES_STARVATION_RATE_INCREASE") == 0){
            FAMILIES_STARVATION_RATE_INCREASE = atoi(str);
            
        } else if (strcmp(label, "FAMILIES_STARVATION_RATE_DECREASE") == 0){
            FAMILIES_STARVATION_RATE_DECREASE = atoi(str);

        } else if (strcmp(label, "FAMILIES_INCREASE_ALARM") == 0){
            FAMILIES_INCREASE_ALARM = atoi(str);

        } else if (strcmp(label, "FAMILIES__STARVATION_SURVIVAL_THRESHOLD") == 0){
            FAMILIES__STARVATION_SURVIVAL_THRESHOLD = atoi(str);
        }
    }

    fclose(file);
}


void create_message_queues(key_t sky_queue_key, key_t safe_area_key, key_t families_key) {
    
    if ( (sky_id = msgget(sky_queue_key, IPC_CREAT | 0770)) == -1 ) {
        perror("Queue create");
        exit(1);
    }
    
    if ( (safe_id = msgget(safe_area_key, IPC_CREAT | 0770)) == -1 ) {
        perror("Queue create");
        exit(1);
    }

    if ( (family_id = msgget(families_key, IPC_CREAT | 0770)) == -1 ) {
        perror("Queue create");
        exit(1);
    }
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

    if (msgctl(family_id, IPC_RMID, (struct msqid_ds *) 0) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    if (msgctl(sorter_id, IPC_RMID, (struct msqid_ds *) 0) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    if ( semctl(plane_sem, IPC_RMID, 0) == -1 ) {
        perror("semctl: IPC_RMID");	/* remove semaphore */
        exit(5);
    }

    if ( shmctl(plane_shmem, IPC_RMID, (struct shmid_ds *) 0) == -1 ) {
        perror("shmid: IPC_RMID");	/* remove semaphore */
        exit(5);
    }
    exit(0);
}
