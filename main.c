#include "headers.h"

#define DEFAULT_SETTINGS "settings.txt"

static int NUM_PLANES;
static int NUM_COLLECTORS;
static int NUM_SPLITTERS;
static int NUM_DISTRIBUTORS;
static int NUM_FAMILIES;
static char CARGO_SIZE_RANGE[10];
static char REFILL_RANGE[20];
static int DROP_PERIOD;


void readFile(char* filename);


int main(int argc, char* argv[]) {

    if (argc < 2) {
        readFile(DEFAULT_SETTINGS);
    } else {
        readFile(argv[1]);
    }

    printf("Args :%d, %s\n", NUM_PLANES, CARGO_SIZE_RANGE);

    key_t sky_queue_key = ftok(".", 'Q');
    int sky_id;
    AidPackage package;

    if ( (sky_id = msgget(sky_queue_key, IPC_CREAT | 0770)) == -1 ) {
        perror("Queue create");
        exit(1);
    }

    pid_t planes[NUM_PLANES];       /* pids for all planes */
    pid_t collectors;   /* pids for all collectors */
    pid_t splitters;    /* pids for all splitters */
    pid_t distributors; /* pids for all distributors */
    pid_t families;     /* pids for all families */

    for (int i = 0; i < NUM_PLANES; i++) {
        planes[i] = fork();

        // child
        if (planes[i] == 0) {
            char m_id[20];
            char drop_p[20];
            sprintf(m_id, "%d", sky_id);
            sprintf(drop_p, "%d", DROP_PERIOD);
            
            execlp("./plane", "plane", m_id, CARGO_SIZE_RANGE, "7", drop_p, REFILL_RANGE, NULL);
            perror("Exec Plane Error");
            exit(SIGQUIT);
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

    // parent process
#ifdef SLEEP
    sleep(SLEEP);
#endif

    for (int i = 0; i < NUM_PLANES; i++) {
        kill(planes[i], SIGINT);
        kill(sky_process, SIGINT);
        wait(NULL);
    }

#ifdef DELETE
    msgctl(sky_id, IPC_RMID, (struct msqid_ds *) 0);
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
        } else if (strcmp(label, "CARGO_SIZE_RANGE") == 0){
            strcpy(CARGO_SIZE_RANGE, str);
        } else if (strcmp(label, "DROP_PERIOD") == 0){
            DROP_PERIOD = atoi(str);
        } else if (strcmp(label, "REFILL_RANGE") == 0){
            strcpy(REFILL_RANGE, str);
        }
    }

    fclose(file);
}
