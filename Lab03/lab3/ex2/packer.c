#include "packer.h"
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

// You can declare global variables here

struct ball {
    int id; // id of the ball
    int flag; // flag to check which balls got selected, 0 = not selected, 1 = selected, N = allocated finish (destroy)
    TAILQ_ENTRY(ball) entries;
};

int N = -1;
TAILQ_HEAD(tailhead, ball);
sem_t mutex;
sem_t mutex_b[3]; // ensure N number of balls reach per colour
sem_t mutex_flags[3]; // ensures that after N balls pass, N balls set their own flag to 1, to determine which balls are selected
sem_t mutex_full[3];
struct tailhead heads[3];
int count_N[3] = {0};

void packer_init(int balls_per_pack) {
    N = balls_per_pack;
    sem_init(&mutex, 1, 1);
    for (int i = 0; i < 3; i++) {
        TAILQ_INIT(&heads[i]);
        struct ball *count = malloc(sizeof(struct ball));
        count->id = 0;
        count->flag = -1;
        TAILQ_INSERT_HEAD(&heads[i], count, entries);
        sem_init(&mutex_flags[i], 1, 0);
        sem_init(&mutex_b[i], 1, 0);
        sem_init(&mutex_full[i], 1, N);
    }
}

void packer_destroy(void) {
    sem_destroy(&mutex);
    for (int i = 0; i < 3; i++) {
        sem_destroy(&mutex_b[i]);
        sem_destroy(&mutex_flags[i]);
        sem_destroy(&mutex_full[i]);
        struct ball *to_delete;
        while (!TAILQ_EMPTY(&heads[i])) {
            to_delete = TAILQ_FIRST(&heads[i]);
            TAILQ_REMOVE(&heads[i], to_delete, entries);
            free(to_delete);
        }
    }
}

void pack_ball(int colour, int id, int *other_ids) {
    // Write your code here.
    struct ball *np, *np_temp;
    sem_wait(&mutex);
        colour--;

        TAILQ_FIRST(&heads[colour])->id++; // counting total number of balls arrived
        struct ball *newBall = malloc(sizeof(struct ball));
        newBall->id = id;
        newBall->flag = 0;
        TAILQ_INSERT_TAIL(&heads[colour], newBall, entries);

        if (TAILQ_FIRST(&heads[colour])->id % N == 0) { // if there are multiples of N balls arrived, unblock the waiting ball
            for (int i = 0; i < N; i++) {
                sem_post(&mutex_b[colour]); // posts N times to allow N balls to pass
            }
        }
    sem_post(&mutex);
    

    sem_wait(&mutex_b[colour]); // block if there are not N balls yet

    sem_wait(&mutex_full[colour]); // waits for N balls to finish packing

    sem_wait(&mutex);
        TAILQ_FOREACH(np, &heads[colour], entries) {
            if (np->id == id) {
                np->flag = 1;
                break;
            }
        }
        count_N[colour]++; // within mutex to ensure that N balls have set their own flags to 1
        if (count_N[colour] == N) {
            count_N[colour] = 0;
            for (int i = 0; i < N; i++) {
                sem_post(&mutex_flags[colour]); // posts N times again to allow the blocked selected balls to pass
            }
        }
    sem_post(&mutex);


    sem_wait(&mutex_flags[colour]); // block if not all N balls have set their own flags (different mutex)


    sem_wait(&mutex);
        int matches = 0;
        np = TAILQ_FIRST(&heads[colour]);
        while (np != NULL) {   
            np_temp = TAILQ_NEXT(np, entries);
            if (matches == N - 1) { // if matched with N - 1 balls, break 
                count_N[colour]++;
                if (count_N[colour] == N) {
                    count_N[colour] = 0;
                    for (int i = 0; i < N; i++) {
                        sem_post(&mutex_full[colour]); // after N balls finish packing, let the next N in
                    }
                }
                break;
            }

            if (np->id != id && np->flag > 0) {
                *(other_ids + matches) = np->id;
                matches++;
                np->flag++;
                if (np->flag == N) { // remove ball from the queue if it has matched with N - 1 balls (flag is 1 after being selected)
                    TAILQ_REMOVE(&heads[colour], np, entries);
                    free(np);
                }
            }
            np = np_temp;
        }
    sem_post(&mutex);
}
