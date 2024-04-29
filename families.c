#include "headers.h"
#include "functions.h"


int sid;
int min_starvation_rate;
int max_starvation_rate;
int starvation_rate;
int starvation_rate_increase_constant;
int starvation_rate_decrease_constant;
int starvation_rate_increase_alarm;
int starvation_survival_threshold;
int family_index;


familyStruct familia;


void increment_starvation_rate(int);

int main(int argc, char* argv[]) {

    if (argc < 9) {
        perror("Not Enough Args, families.c");
        exit(-1);
    }

    if ( signal(SIGALRM, increment_starvation_rate) == SIG_ERR ){
        perror("sigerr family");
        exit(SIGQUIT);
    }

    int fid = atoi(argv[1]); //the id for the families' message queue

    min_starvation_rate = atoi(strtok(argv[2], "-"));
    max_starvation_rate = atoi(strtok('\0', "-"));
    starvation_rate = select_from_range(min_starvation_rate, max_starvation_rate);

    starvation_rate_increase_constant = atoi(argv[3]);
    starvation_rate_decrease_constant = atoi(argv[4]);
    starvation_rate_increase_alarm = atoi(argv[5]);
    starvation_survival_threshold = atoi(argv[6]);
    family_index = atoi(argv[7]);

    sid = atoi(argv[8]);

    printf("Hello from family with index %d, pid %d, stvr: %d\n", family_index, getpid(), starvation_rate);
    fflush(NULL);

    familia.starvationRate = starvation_rate;
    familia.familyIndex = family_index;
    msgsnd(sid, &familia, sizeof(familyStruct), 0);

    alarm(starvation_rate_increase_alarm);

    AidPackage bag;

    while (1) {

        while ( msgrcv(fid, &bag, sizeof(AidPackage), family_index, 0) == -1 );

        starvation_rate -= starvation_rate_decrease_constant;

        printf("(family) index %d Ate, strv %d\n", family_index, starvation_rate);
        fflush(NULL);

        familia.starvationRate = starvation_rate;
        familia.familyIndex = (long)family_index;
        msgsnd(sid, &familia, sizeof(familyStruct), 0);
    }

    return 0;
}

void increment_starvation_rate (int sig) {
    
    if (starvation_rate < starvation_survival_threshold) {
        starvation_rate += starvation_rate_increase_constant;
        familia.starvationRate = starvation_rate;
        familia.familyIndex = (long)family_index;
        printf("(Family) %d is starving (%d)\n", family_index, starvation_rate);
        fflush(NULL);

        msgsnd(sid, &familia, sizeof(familyStruct), 0);
    } else{
        printf("(family) index %d, with starvation rate %d (just died)\n", family_index, starvation_rate);
        exit(-1);
    }
    alarm(starvation_rate_increase_alarm);
}
