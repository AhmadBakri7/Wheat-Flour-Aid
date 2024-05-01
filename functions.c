#include "headers.h"

int select_from_range(int min, int max) {
    srand(time(NULL) + getpid());

    int range = max - min + 1;
    int random = (rand() % range) + min;

    return random;
}

unsigned int get_sleep_duration(int energy) {
    srand(time(NULL));

    int duration;

    if (energy <= 100 && energy > 90)
        duration = select_from_range(3, 5); // 3-5
    else if (energy <= 90 && energy > 80)
        duration = select_from_range(5, 7); // 5-7
    else if (energy <= 80 && energy > 70)
        duration = select_from_range(7, 9); // 7-9
    else if (energy <= 70 && energy > 60)
        duration = select_from_range(9, 11); // 9-11
    else
        duration = select_from_range(11, 13); // 11-13

    return duration;
}


void alert_news(int news_queue, long process_type) {
    NewsReport report;
    report.process_type = process_type;

    // Send message to news channels
    if (msgsnd(news_queue, &report, sizeof(report), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}


NewsReport watch_news(int news_queue, int process_type) {

    NewsReport report;

    if ( msgrcv(news_queue, &report, sizeof(report), process_type, 0) == -1 ) {
        perror("msgrcv Parent (News queue)");
        exit(SIGQUIT);
    }

    return report;
}