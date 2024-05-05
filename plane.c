#include "headers.h"
#include "functions.h"


bool check_collision(int plane_amplitude, int max_planes, int safe_distance, int plane_number);
void semaphore_acquire(int);
void semaphore_release(int);
void detach_memory(void*);
void write_Plane_info(int plane_amplitude, int plane_number);
void cleanup(int sig);


int shmid, semid;
int plane_number, max_planes;
int news_queue;
AirSpace *planes_air_space;


struct sembuf acquire = {0, -1, SEM_UNDO};
struct sembuf release = {0, 1, SEM_UNDO};

int main(int argc, char* argv[]) {

    if (argc < 13) {
        perror("Not Enough Args, plane.c");
        exit(-1);
    }

    if ( signal(SIGUSR1, cleanup) == SIG_ERR ) {
        perror("Signal Error Plane");
        exit(SIGQUIT);
    }

    int sky_queue = atoi(argv[1]);      /* the id for the sky message queue */
    news_queue = atoi(argv[12]);        /* the id for the news message queue */
    semid = atoi(argv[7]);              /* semaphore id */
    shmid = atoi(argv[8]);              /* shared memory id */
    max_planes = atoi(argv[9]);         /* max number of planes in the sky */
    plane_number = atoi(argv[10]);      /* the number of this plane */
    int safe_distance = atoi(argv[11]); /* minimum safe distance between planes */

    int min_containers = atoi(strtok(argv[2], "-"));
    int max_containers = atoi(strtok('\0', "-"));
    int containers = select_from_range(min_containers, max_containers);

    int min_amplitude = atoi(strtok(argv[3], "-"));
    int max_amplitude = atoi(strtok('\0', "-"));
    int plane_amplitude = select_from_range(min_amplitude, max_amplitude);

    int min_weight = atoi(strtok(argv[4], "-"));
    int max_weight = atoi(strtok('\0', "-"));

    int drop_period = atoi(argv[5]);

    int min_refill = atoi(strtok(argv[6], "-"));
    int max_refill = atoi(strtok('\0', "-"));

    printf("Plane %d, is flying at %d, safe: %d\n", getpid(), plane_amplitude, safe_distance);

    AidDrop drops[max_containers];

    for (int i = 0; i < containers; i++) {
        drops[i].package_type = DROP;
        drops[i].weight = select_from_range(min_weight, max_weight);
        drops[i].amplitude = plane_amplitude;
    }

    for (int i = 0; i < containers; i++) {
        printf("package-type (child): %ld, weight: %d\n", drops[i].package_type, drops[i].weight);
    }

    // attach the shared memory
    if ((planes_air_space = shmat(shmid, NULL, 0)) == (AirSpace *) -1) {
        perror("shmat");
        exit(1);
    }

    int current_drop = 0;
    // sleep( (unsigned) select_from_range(min_refill, max_refill) ); /* refill time */
    sleep(plane_number);
    
    write_Plane_info(plane_amplitude, plane_number);

    while (1) {

        /* collision happened */
        if ( check_collision(plane_amplitude, max_planes, safe_distance, plane_number) ) {

            detach_memory(planes_air_space);
            alert_news(news_queue, PLANE, plane_number);
            exit(1);
        }

        if (msgsnd(sky_queue, &drops[current_drop], sizeof(drops[current_drop]), 0) == -1 ) {
            perror("Child: msgsend");
            return 4;
        }
        
        current_drop++;

        sleep(drop_period);

        // refill
        if (current_drop == containers) {

            write_Plane_info(0, plane_number);

            printf("Plane %d is refilling\n", plane_number);

            int new_containers = select_from_range(min_containers, max_containers);
            plane_amplitude = select_from_range(min_amplitude, max_amplitude);

            for (int i = 0; i < new_containers; i++) {
                drops[i].package_type = DROP;
                drops[i].weight = select_from_range(min_weight, max_weight);
                drops[i].amplitude = plane_amplitude;
            }

            sleep( (unsigned) select_from_range(min_refill, max_refill) ); /* refill time */

            containers = new_containers;
            current_drop = 0;
            printf("Plane %d, is flying at %d\n", plane_number, plane_amplitude);

            write_Plane_info(plane_amplitude, plane_number);
        }
    }

    detach_memory(planes_air_space);

    return 0;
}

bool check_collision(int plane_amplitude, int max_planes, int safe_distance, int plane_number) {
    int count = 0, min = INT_MAX;
    pid_t nearest_plane;
    int nearest_plane_index;

    semaphore_acquire(0);

    /* critical section */
    for (int i = 0; i < max_planes; i++) {

        // printf("(Plane %d) [%d] is flying at: %d\n", plane_number, i, planes_air_space[i].amplitude);

        if (i == plane_number)
            continue;

        int diff;

        if (
            (planes_air_space[i].amplitude > 0)
            && ((diff = abs(plane_amplitude - planes_air_space[i].amplitude)) <= safe_distance) )
        {
            min = (diff < min)? diff : min;
            nearest_plane = planes_air_space[i].plane;
            nearest_plane_index = i;
            count++;
        }
    }

    int collision_probability = (count * 5) * (count > 1);
    bool collide = select_from_range(1, 100) <= collision_probability;

    if (collide) {
        planes_air_space[plane_number].amplitude = 0;
        planes_air_space[nearest_plane_index].amplitude = 0;

        kill(nearest_plane, SIGUSR1);
        printf("(Plane) %d has collided with Plane %d -> %d, count=%d\n", plane_number, nearest_plane_index, nearest_plane, count);
        fflush(stdout);
    }
    /* end of critical section */

    // Release the semaphore (unlock)
    semaphore_release(0);

    // int collision_probability = 100 - ( (count / max_planes) * 100 )
    // int collision_probability = (count < 5)? (count*10) : 100;
    
    return collide;
}

void write_Plane_info(int plane_amplitude, int plane_number) {

    semaphore_acquire(0);

    /* critical section */
    planes_air_space[plane_number].plane = getpid();
    planes_air_space[plane_number].amplitude = plane_amplitude;
    /* end of critical section */

    semaphore_release(0);
}

void cleanup(int sig) {

    printf("I Crashed----%d-----------------------------\n", plane_number);

    write_Plane_info(0, plane_number);

    detach_memory(planes_air_space);

    alert_news(news_queue, PLANE, plane_number);
    
    exit(0);
}

void semaphore_acquire(int sem_num) {

    acquire.sem_num = sem_num;

    // write the amplitude the shared memory
    if (semop(semid, &acquire, 1) == -1) {
        perror("semop Child");
        exit(1);
    }
}

void semaphore_release(int sem_num) {

    release.sem_num = sem_num;

    // Release the semaphore (unlock)
    if (semop(semid, &release, 1) == -1) {
        perror("semop Release");
        exit(1);
    }
}

void detach_memory(void* shared_memory) {

    // Detach from the shared memory segment
    if (shmdt(shared_memory) == -1) {
        perror("shmdt");
        exit(1);
    }
}