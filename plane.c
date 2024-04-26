#include "headers.h"


int select_from_range(int min, int max);


int main(int argc, char* argv[]) {

    if (argc < 6) {
        perror("Not Enough Args, plane.c");
        exit(-1);
    }

    int mid = atoi(argv[1]); /* the id for the message queue */
    int min_containers = atoi(strtok(argv[2], "-"));
    int max_containers = atoi(strtok('\0', "-"));
    int plane_amplitude = atoi(argv[3]); /* of the plane */
    int drop_period = atoi(argv[4]);
    int min_refill = atoi(strtok(argv[5], "-"));
    int max_refill = atoi(strtok('\0', "-"));

    int containers = select_from_range(min_containers, max_containers);

    printf("Hello from Child %d, mid: %d, max: %d, min: %d\n", getpid(), mid, max_refill, min_refill);

    AidDrop drops[max_containers];

    for (int i = 0; i < containers; i++) {
        drops[i].package_type = DROP;
        drops[i].weight = 50;
        drops[i].amplitude = plane_amplitude;
    }

    for (int i = 0; i < containers; i++) {
        printf("package-type (chile): %ld, weight: %d\n", drops[i].package_type, drops[i].weight);
    }

    int current_drop = 0;

    while (1) {

        if (msgsnd(mid, &drops[current_drop], sizeof(drops[current_drop]), 0) == -1 ) {
            perror("Child: msgsend");
            return 4;
        }
        
        current_drop++;

        sleep(drop_period);

        // refill
        if (current_drop == containers) {
            return 0;

            int new_containers = select_from_range(min_containers, max_containers);

            for (int i = 0; i < new_containers; i++) {
                drops[i].package_type = DROP;
                drops[i].weight = 50;
                drops[i].amplitude = plane_amplitude;
            }

            sleep( (unsigned) select_from_range(min_refill, max_refill) );

            containers = new_containers;
            current_drop = 0;
        }
    }

    return 0;
}

int select_from_range(int min, int max) {
    srand(time(NULL));

    int range = max - min + 1;
    int random = (rand() % range) + min;

    return random;
}