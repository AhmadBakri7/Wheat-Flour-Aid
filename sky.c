#include "headers.h"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        perror("Not Enough Args, plane.c");
        exit(-1);
    }

    int mid = atoi(argv[1]); /* the id for the message queue */

    AidDrop drops[100];

    int current_drop = 0;
    struct msqid_ds buf;

    while (1) {

        msgctl(mid, IPC_STAT, &buf);
 
        for (int i = 0; i < buf.msg_qnum; i++)
        {
            if ( msgrcv(mid, &drops[current_drop], BUFSIZ, DROP, 0) != -1 ) {
                printf(
                    "current_drop: %d, Message-type: %ld, Weight: %d, amplitude: %d\n",
                    current_drop, drops[current_drop].package_type, drops[current_drop].weight, drops[current_drop].amplitude
                );
                current_drop++;
            }
        }

        sleep(2);

        for (int i = 0; i < current_drop; i++) {

            if (drops[i].amplitude > 0) {
                drops[i].amplitude--;
                printf("Amplitude (%d): %d\n", i, drops[i].amplitude);
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
                printf("Package sent to collector from sky (%d) (Amp:%d) \n", i, drops[i].amplitude);
            }
        }
    }

    return 0;
}

