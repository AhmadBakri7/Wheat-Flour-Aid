#include "headers.h"

int fid;
int min_starvation_rate;
int max_starvation_rate;
int starvation_rate;
int starvation_rate_increase_constant;
int starvation_rate_decrease_constant;
int starvation_rate_increase_alarm;
int starvation_survival_threshold;
int family_index;
char eat[20];

void increment_starvation_rate(int);

int main(int argc, char* argv[]) {

    if (argc < 8) {
        perror("Not Enough Args, families.c");
        exit(-1);
    }

    fid = atoi(argv[1]);//the id for the families' message queue

    min_starvation_rate = atoi(strtok(argv[2], "-"));
    max_starvation_rate = atoi(strtok('\0', "-"));
    starvation_rate = select_from_range(min_starvation_rate, max_starvation_rate);

    starvation_rate_increase_constant = atoi(argv[3]);
    starvation_rate_decrease_constant = atoi(argv[4]);
    starvation_rate_increase_alarm = atoi(argv[5]);
    starvation_survival_threshold = atoi(argv[6]);

    family_index = atoi(argv[7]);

    printf("Hello from family with index %d, pid %d, stvr: %d\n", family_index, getpid(), starvation_rate);

    alarm(starvation_rate_increase_alarm);

    while (1) {

        if ( msgrcv(fid, &eat, BUFSIZ, family_index, 0) == -1 ) {
            perror("Child: msgsend");
            return 4;
        }
        starvation_rate -= starvation_rate_decrease_constant;
        printf("family with pid %d, index %d received a bag, stvr %d\n", getpid(), family_index, starvation_rate);

    }
    return 0;
}

void increment_starvation_rate (int sig) {
    
    if (starvation_rate < starvation_survival_threshold)
        starvation_rate += starvation_rate_increase_constant;
    else
        printf("family with pid %d, index %d, with starvation rate %d just died", getpid(), family_index, starvation_rate);
        kill(getpid(), SIGKILL);

    alarm(starvation_rate_increase_alarm);
}