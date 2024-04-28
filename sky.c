#include "headers.h"
#include "functions.h"


void missile_attack(int sig);

int current_drop = 0;
AidDrop drops[100];


int main(int argc, char* argv[]) {

    if (argc < 2) {
        perror("Not Enough Args, plane.c");
        exit(-1);
    }

    if (signal(SIGUSR1, missile_attack) == SIG_ERR) {
        perror("Signal Sky Error");
        exit(SIGQUIT);
    }

    int mid = atoi(argv[1]); /* the id for the message queue */


    struct msqid_ds buf;

    while (1) {

        msgctl(mid, IPC_STAT, &buf);
 
        for (int i = 0; i < buf.msg_qnum; i++)
        {
            if ( msgrcv(mid, &drops[current_drop], BUFSIZ, DROP, 0) != -1 ) {
                printf(
                    "current_drop: %d, Message-type: %ld, Weight: %d, amplitude: %d, drop_number: %d\n",
                    current_drop, drops[current_drop].package_type, drops[current_drop].weight, drops[current_drop].amplitude, drops[current_drop].package_number
                );
                current_drop++;
            }
        }

        sleep(2);

        for (int i = 0; i < current_drop; i++) {

            if (drops[i].amplitude > 0) {

                if (drops[i].amplitude < 100)
                    drops[i].amplitude = 0;
                else
                    drops[i].amplitude -= 100;

                printf("Amplitude (%d): %d\n", drops[i].package_number, drops[i].amplitude);
                
            } else {
                AidPackage package;
                package.package_type = CONTAINER;

                package.weight = drops[i].weight;

                // swap drops[i] with current_drop (last drop) (soft delete).
                drops[i].package_type = drops[current_drop-1].package_type;
                drops[i].weight = drops[current_drop-1].weight;
                drops[i].amplitude = drops[current_drop-1].amplitude;

                current_drop--;

                if (msgsnd(mid, &package, sizeof(package), 0) == -1 ) {
                    perror("Child: msgsend");
                    return 4;
                }
                printf("Package (%d) sent to collector from sky\n", drops[i].package_number);
            }
        }
    }

    return 0;
}


void missile_attack(int sig) {
    int random_drop = select_from_range(0, current_drop-1);

    if (drops[random_drop].amplitude > 700) {
        drops[random_drop].weight = 0;

        // swap drops[i] with current_drop (last drop) (soft delete).
        drops[random_drop].package_type = drops[current_drop-1].package_type;
        drops[random_drop].weight = drops[current_drop-1].weight;
        drops[random_drop].amplitude = drops[current_drop-1].amplitude;

        current_drop--;
        printf("Destroyed package (%d) has now weight: %d\n", drops[random_drop].package_number, drops[random_drop].weight);

    } else if (drops[random_drop].amplitude > 300 && drops[random_drop].amplitude < 700) {
        drops[random_drop].weight /= 3;
        drops[random_drop].amplitude = 0;
        printf("Destroyed package (%d) has now weight: %d\n", drops[random_drop].package_number, drops[random_drop].weight);
    } else if (drops[random_drop].amplitude > 100 && drops[random_drop].amplitude < 300) {
        drops[random_drop].weight /= 2;
        drops[random_drop].amplitude = 0;
        printf("Destroyed package (%d) has now weight: %d\n", drops[random_drop].package_number, drops[random_drop].weight);
    }
}

