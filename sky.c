#include "headers.h"
#include "functions.h"


void missile_attack(int sig);

int news_queue;
int current_drop;
int threshold;
AidDrop drops[100];


int main(int argc, char* argv[]) {

    if (argc < 4) {
        perror("Not Enough Args, plane.c");
        exit(-1);
    }


    if (signal(SIGUSR1, missile_attack) == SIG_ERR) {
        perror("Signal Sky Error");
        exit(SIGQUIT);
    }

    int sky_queue = atoi(argv[1]);    /* message queue ID for the sky queue */
    news_queue = atoi(argv[3]); /* message queue ID for the news queue */
    threshold = atoi(argv[2]);  /* exploded drops above threshold gets totally lost */

    struct msqid_ds buf;

    current_drop = 0;

    while (1) {

        msgctl(sky_queue, IPC_STAT, &buf);
        
        for (int i = 0; i < buf.msg_qnum; i++)
        {
            if ( msgrcv(sky_queue, &drops[current_drop], BUFSIZ, DROP, 0) != -1 ) {
                // printf(
                //     "current_drop: %d, Message-type: %ld, Weight: %d, amplitude: %d\n",
                //     current_drop, drops[current_drop].package_type, drops[current_drop].weight, drops[current_drop].amplitude
                // );
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

                // printf("Amplitude (%d): %d\n", drops[i].amplitude);
                
            } else {
                AidPackage package;
                package.package_type = CONTAINER;

                package.weight = drops[i].weight;

                // swap drops[i] with current_drop (last drop) (soft delete).
                drops[i].package_type = drops[current_drop-1].package_type;
                drops[i].weight = drops[current_drop-1].weight;
                drops[i].amplitude = drops[current_drop-1].amplitude;

                current_drop--;

                if (msgsnd(sky_queue, &package, sizeof(package), 0) == -1 ) {
                    perror("Child: msgsend");
                    return 4;
                }
                printf("Package (Amp: %d) sent to collector from sky\n", drops[i].amplitude);
            }
        }
    }

    return 0;
}


void missile_attack(int sig) {

    if (current_drop <= 0)
        return;

    int random_drop = select_from_range(0, current_drop-1);

    if (drops[random_drop].amplitude > threshold) {
        drops[random_drop].weight = 0;

        // swap drops[i] with current_drop (last drop) (soft delete).
        drops[random_drop].package_type = drops[current_drop-1].package_type;
        drops[random_drop].weight = drops[current_drop-1].weight;
        drops[random_drop].amplitude = drops[current_drop-1].amplitude;

        current_drop--;
        printf("(SKY) Destroyed package (%d) has now weight: %d\n", random_drop, drops[random_drop].weight);

        alert_news(news_queue, SKY);

    } else if (drops[random_drop].amplitude > 300 && drops[random_drop].amplitude < threshold) {
        drops[random_drop].weight /= 3;
        drops[random_drop].amplitude = 0;
        printf("Destroyed package (%d) has now weight: %d\n", random_drop, drops[random_drop].weight);

        alert_news(news_queue, SKY);

    } else if (drops[random_drop].amplitude > 100 && drops[random_drop].amplitude < 300) {
        drops[random_drop].weight /= 2;
        drops[random_drop].amplitude = 0;
        printf("Destroyed package (%d) has now weight: %d\n", random_drop, drops[random_drop].weight);

        alert_news(news_queue, SKY);
    }
}

