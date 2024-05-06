#include "headers.h"
#include "functions.h"


int sorter_queue, news_queue, drawer_queue;
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

    if (argc < 11) {
        perror("Not Enough Args, families.c");
        exit(-1);
    }

    if ( signal(SIGALRM, increment_starvation_rate) == SIG_ERR ){
        perror("sigerr family");
        exit(SIGQUIT);
    }

    int fid = atoi(argv[1]);    /* the id for the families' message queue */

    min_starvation_rate = atoi(strtok(argv[2], "-"));
    max_starvation_rate = atoi(strtok('\0', "-"));
    starvation_rate = select_from_range(min_starvation_rate, max_starvation_rate);

    starvation_rate_increase_constant = atoi(argv[3]);
    starvation_rate_decrease_constant = atoi(argv[4]);
    starvation_rate_increase_alarm = atoi(argv[5]);
    starvation_survival_threshold = atoi(argv[6]);
    family_index = atoi(argv[7]);

    sorter_queue = atoi(argv[8]);
    news_queue = atoi(argv[9]); /* The MSG queue id for news */
    drawer_queue = atoi(argv[10]);

    // printf("Hello from family with index %d, pid %d, stvr: %d\n", family_index, getpid(), starvation_rate);
    // fflush(NULL);

    // send info to drawer (initial values)
    MESSAGE msg = {FAMILY, 0, .data.families = {starvation_rate, family_index-1, true}};

    if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
        perror("Child: msgsend");
        exit(-1);
    }

    familia.starvationRate = starvation_rate;
    familia.familyIndex = family_index;
    msgsnd(sorter_queue, &familia, sizeof(familyStruct), 0);

    alarm(starvation_rate_increase_alarm);

    AidPackage bag;

    while (1) {

        while ( msgrcv(fid, &bag, sizeof(AidPackage), family_index, 0) == -1 );

        starvation_rate -= (starvation_rate_decrease_constant * bag.weight);

        // printf("(family) index %d Ate, strv %d\n", family_index, starvation_rate);
        // fflush(NULL);

        familia.starvationRate = starvation_rate;
        familia.familyIndex = (long)family_index;
        msgsnd(sorter_queue, &familia, sizeof(familyStruct), 0);

        // send info to drawer (starvation rate after eating)
        MESSAGE msg = {FAMILY, 0, .data.families = {starvation_rate, family_index-1, true}};

        if (msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
            exit(-1);
        }
    }

    return 0;
}

void increment_starvation_rate (int sig) {
    
    if (starvation_rate < starvation_survival_threshold) {
        starvation_rate += starvation_rate_increase_constant;
        familia.starvationRate = starvation_rate;
        familia.familyIndex = (long)family_index;
        // printf("(Family) %d is starving (%d)\n", family_index, starvation_rate);
        // fflush(NULL);

        msgsnd(sorter_queue, &familia, sizeof(familyStruct), 0);

        // send info to drawer (starvation rate update)
        MESSAGE msg = {FAMILY, 0, .data.families = {starvation_rate, family_index-1, true}};

        if ( msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 ) {
            perror("Child: msgsend");
            exit(-1);
        }
        
    } else{
        printf("(family) index %d, with starvation rate %d (just died)\n", family_index, starvation_rate);

        alert_news(news_queue, FAMILY, family_index);

        // send info to drawer (death)
        MESSAGE msg = {FAMILY, 0, .data.families = {starvation_rate, family_index-1, false}};

        if ( msgsnd(drawer_queue, &msg, sizeof(msg), 0) == -1 )
            perror("Child: msgsend");
        
        exit(-1);
    }
    alarm(starvation_rate_increase_alarm);
}
