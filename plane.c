#include "headers.h"
#include "functions.h"


pid_t check_collision(int plane_amplitude, int max_planes, int safe_distance, int plane_number);
void write_shmem(int plane_amplitude, int plane_number);
void cleanup(int sig);


int shmid, semid;
int plane_number;
AirSpace *shared_array;


struct sembuf acquire = {0, -1, SEM_UNDO};
struct sembuf release = {0, 1, SEM_UNDO};

int main(int argc, char* argv[]) {

    if (argc < 12) {
        perror("Not Enough Args, plane.c");
        exit(-1);
    }

    if ( signal(SIGUSR1, cleanup) == SIG_ERR ) {
        perror("Signal Error Plane");
        exit(SIGQUIT);
    }

    int mid = atoi(argv[1]);            /* the id for the message queue */
    semid = atoi(argv[7]);              /* semaphore id */
    shmid = atoi(argv[8]);              /* shared memory id */
    int max_planes = atoi(argv[9]);     /* max number of planes in the sky */
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
    int id = 0;

    for (int i = 0; i < containers; i++) {
        drops[i].package_number = getpid() + id++;
        drops[i].package_type = DROP;
        drops[i].weight = select_from_range(min_weight, max_weight);
        drops[i].amplitude = plane_amplitude;
    }

    for (int i = 0; i < containers; i++) {
        printf("package-type (child): %ld, weight: %d\n", drops[i].package_type, drops[i].weight);
    }

    // attach the shared memory
    if ((shared_array = shmat(shmid, NULL, 0)) == (AirSpace *) -1) {
        perror("shmat");
        exit(1);
    }

    int current_drop = 0;
    // sleep( (unsigned) select_from_range(min_refill, max_refill) ); /* refill time */
    sleep(plane_number);
    write_shmem(plane_amplitude, plane_number);

    while (1) {

        /* collision happened */
        pid_t colliding_plane = check_collision(plane_amplitude, max_planes, safe_distance, plane_number);

        if ( colliding_plane > 0) {
            printf("Plane %d has collided with Plane %d\n", getpid(), colliding_plane);
            fflush(stdout);
            kill(colliding_plane, SIGUSR1);
            kill(getpid(), SIGUSR1);
        }

        if (msgsnd(mid, &drops[current_drop], sizeof(drops[current_drop]), 0) == -1 ) {
            perror("Child: msgsend");
            return 4;
        }
        
        current_drop++;

        sleep(drop_period);

        // refill
        if (current_drop == containers) {

            write_shmem(0, plane_number);

            printf("Plane %d is refilling\n", getpid());

            int new_containers = select_from_range(min_containers, max_containers);
            plane_amplitude = select_from_range(min_amplitude, max_amplitude);

            for (int i = 0; i < new_containers; i++) {
                drops[i].package_number = id++;
                drops[i].package_type = DROP;
                drops[i].weight = select_from_range(10, 20);
                drops[i].amplitude = plane_amplitude;
            }

            sleep( (unsigned) select_from_range(min_refill, max_refill) ); /* refill time */

            containers = new_containers;
            current_drop = 0;
            printf("Plane %d, is flying at %d\n", getpid(), plane_amplitude);

            write_shmem(plane_amplitude, plane_number);
        }
    }

    // Detach from the shared memory segment
    if (shmdt(shared_array) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}

pid_t check_collision(int plane_amplitude, int max_planes, int safe_distance, int plane_number) {
    int count = 0, min = INT_MAX;
    pid_t nearest_plane;

    sighold(SIGUSR1);

    // read the amplitude the shared memory
    if (semop(semid, &acquire, 1) == -1) {
        perror("semop Child");
        exit(1);
    }

    /* critical section */
    for (int i = 0; i < max_planes; i++) {

        if (i == plane_number)
            continue;

        int diff;

        if ( (diff = abs(plane_amplitude - shared_array[i].amplitude)) <= safe_distance ) {
            min = (diff < min)? diff : min;
            nearest_plane = shared_array[i].plane;
            count++;
        }
    }
    /* end of critical section */

    // Release the semaphore (unlock)
    if (semop(semid, &release, 1) == -1) {
        perror("semop Release");
        exit(1);
    }
    sigrelse(SIGUSR1);

    // int collision_probability = 100 - ( (count / max_planes) * 100 )
    // int collision_probability = (count < 5)? (count*10) : 100;
    int collision_probability = count * 5;
    bool collide = select_from_range(1, 100) <= collision_probability;

    if (collide)
        return nearest_plane;
    
    return -1; /* no collision */
}


void write_shmem(int plane_amplitude, int plane_number) {
    sighold(SIGUSR1);

    // write the amplitude the shared memory
    if (semop(semid, &acquire, 1) == -1) {
        perror("semop Child");
        exit(1);
    }

    /* critical section */
    shared_array[plane_number].plane = getpid();
    shared_array[plane_number].amplitude = plane_amplitude;
    /* end of critical section */

    // Release the semaphore (unlock)
    if (semop(semid, &release, 1) == -1) {
        perror("semop Release");
        exit(1);
    }

    sigrelse(SIGUSR1);
}


void cleanup(int sig) {

    write_shmem(0, plane_number);

    // Detach from the shared memory segment
    if (shmdt(shared_array) == -1) {
        perror("shmdt");
        exit(1);
    }
    
    exit(0);
}