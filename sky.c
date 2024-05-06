#include "headers.h"
#include "functions.h"


void missile_attack(int sig);
void update_amps(int sig);

int news_queue, drawer_queue, sky_queue;
int current_drop;
int drop_number;
int threshold;
AidDrop drops[100];


int main(int argc, char* argv[]) {

    if (argc < 5) {
        perror("Not Enough Args, plane.c");
        exit(-1);
    }

    if (signal(SIGUSR1, missile_attack) == SIG_ERR) {
        perror("Signal Sky Error");
        exit(SIGQUIT);
    }

    if (signal(SIGALRM, update_amps) == SIG_ERR) {
        perror("Signal Sky Error");
        exit(SIGQUIT);
    }

    sky_queue = atoi(argv[1]);    /* message queue ID for the sky queue */
    drawer_queue = atoi(argv[4]);
    news_queue = atoi(argv[3]); /* message queue ID for the news queue */
    threshold = atoi(argv[2]);  /* exploded drops above threshold gets totally lost */

    // struct msqid_ds buf;

    current_drop = 0;
    drop_number = 0; /* unique for each drop */

    alarm(2);

    while (1) {

        // msgctl(sky_queue, IPC_STAT, &buf);
        
        // for (int i = 0; i < buf.msg_qnum; i++)
        // {
        //     if ( msgrcv(sky_queue, &drops[current_drop], BUFSIZ, DROP, 0) != -1 ) {
        //         // printf(
        //         //     "current_drop: %d, Message-type: %ld, Weight: %d, amplitude: %d\n",
        //         //     current_drop, drops[current_drop].package_type, drops[current_drop].weight, drops[current_drop].amplitude
        //         // );

        //         // send info to drawer (Drop is in the sky now)
        //         MESSAGE msg = {SKY, 0, .data.sky = {drop_number, drops[i].weight, drops[i].amplitude}};

        //         if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
        //             perror("Child: msgsend");
        //             return 4;
        //         }

        //         current_drop++;
        //         drop_number++;
        //     }
        // }
        if ( msgrcv(sky_queue, &drops[current_drop], BUFSIZ, DROP, 0) != -1 ) {
            // printf(
            //     "current_drop: %d, Message-type: %ld, Weight: %d, amplitude: %d\n",
            //     current_drop, drops[current_drop].package_type, drops[current_drop].weight, drops[current_drop].amplitude
            // );

            drops[current_drop].number = drop_number;

            // send info to drawer (Drop is in the sky now)
            MESSAGE msg = {SKY, 1, .data.sky = {drop_number, drops[current_drop].weight, drops[current_drop].amplitude}};

            if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
                perror("Child: msgsend");
                return 4;
            }

            current_drop++;
            drop_number++;
        }
    }

    return 0;
}

void update_amps(int sig) {

    for (int i = 0; i < current_drop; i++) {

        drops[i].amplitude -= (drops[i].amplitude >= 100)? 100 : drops[i].amplitude;

        // send info to drawer (Drop has reached the ground now)
        MESSAGE msg = {SKY, 0, .data.sky = {drops[i].number, drops[i].weight, drops[i].amplitude}};

        if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
            exit(4);
        }

        if (drops[i].amplitude <= 0) {
            AidPackage package;
            package.package_type = CONTAINER;

            package.weight = drops[i].weight;
            printf("Package (Amp: %d) sent to collector from sky\n", drops[i].amplitude);

            // swap drops[i] with current_drop (last drop) (soft delete).
            drops[i].package_type = drops[current_drop-1].package_type;
            drops[i].weight = drops[current_drop-1].weight;
            drops[i].amplitude = drops[current_drop-1].amplitude;
            drops[i].number = drops[current_drop-1].number;

            current_drop--;

            if (msgsnd(sky_queue, &package, sizeof(package), 0) == -1 ) {
                perror("Child: msgsend");
                exit(4);
            }
        }
    }
    alarm(2);
}


void missile_attack(int sig) {

    if (current_drop <= 0)
        return;

    int random_drop = select_from_range(0, current_drop-1);

    if (drops[random_drop].amplitude > threshold) {
        drops[random_drop].weight = 0;

        // send info to drawer (Drop has reached the ground now)
        MESSAGE msg = {SKY, 2, .data.sky = {drops[random_drop].number, 0, 0}};

        if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
            exit(4);
        }

        // swap drops[i] with current_drop (last drop) (soft delete).
        drops[random_drop].package_type = drops[current_drop-1].package_type;
        drops[random_drop].weight = drops[current_drop-1].weight;
        drops[random_drop].amplitude = drops[current_drop-1].amplitude;

        current_drop--;
        printf("(SKY) Destroyed package (%d) has now weight: %d\n", random_drop, drops[random_drop].weight);

        alert_news(news_queue, SKY, random_drop);

    } else if (drops[random_drop].amplitude > 300 && drops[random_drop].amplitude < threshold) {
        drops[random_drop].weight /= 3;
        drops[random_drop].amplitude -= 100;
        printf("Destroyed package (%d) has now weight: %d\n", random_drop, drops[random_drop].weight);

        alert_news(news_queue, SKY, random_drop);

        // send info to drawer (Drop has reached the ground now)
        MESSAGE msg = {SKY, 3, .data.sky = {drops[random_drop].number, drops[random_drop].weight, drops[random_drop].amplitude}};

        if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
            exit(4);
        }

    } else if (drops[random_drop].amplitude > 100 && drops[random_drop].amplitude < 300) {
        drops[random_drop].weight /= 2;
        drops[random_drop].amplitude -= 100;
        printf("Destroyed package (%d) has now weight: %d\n", random_drop, drops[random_drop].weight);

        alert_news(news_queue, SKY, random_drop);

        // send info to drawer (Drop has reached the ground now)
        MESSAGE msg = {SKY, 3, .data.sky = {drops[random_drop].number, drops[random_drop].weight, drops[random_drop].amplitude}};

        if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
            exit(4);
        }
    }

    printf("(SKY) Missile attack\n");
}

