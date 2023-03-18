#include "packer.h"
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>

// You can declare global variables here

struct ball {
    int id;
    TAILQ_ENTRY(ball) entries; 
};

TAILQ_HEAD(tailhead, ball);
sem_t mutex;
sem_t mutex_b[3];
struct tailhead heads[3];

void packer_init(void) {
    // Write initialization code here (called once at the start of the program).
    sem_init(&mutex, 1, 1);
    for (int i = 0; i < 3; i++) {
        TAILQ_INIT(&heads[i]);
        struct ball *count = malloc(sizeof(struct ball));
        count->id = 0;
        TAILQ_INSERT_HEAD(&heads[i], count, entries);
        sem_init(&mutex_b[i], 1, 0);
    }
}

void packer_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    sem_destroy(&mutex);
    for (int i = 0; i < 3; i++) {
        sem_destroy(&mutex_b[i]);
        struct ball *toDelete;
        while (!TAILQ_EMPTY(&heads[i])) {
            toDelete = TAILQ_FIRST(&heads[i]);
            TAILQ_REMOVE(&heads[i], toDelete, entries);
            free(toDelete);
        }
    }
}

int pack_ball(int colour, int id) {
    // Write your code here.
    sem_wait(&mutex);
    colour--;

    TAILQ_FIRST(&heads[colour])->id++; // counting total number of balls arrived
    int count = TAILQ_FIRST(&heads[colour])->id;
    struct ball *newBall = malloc(sizeof(struct ball));
    newBall->id = id;
    TAILQ_INSERT_TAIL(&heads[colour], newBall, entries);

    if (count % 2 == 0) { // if there are multiples of 2 balls arrived, unblock the waiting ball
        sem_post(&mutex_b[colour]);
    }

    sem_post(&mutex);
    
    if (count % 2 != 0) {
        sem_wait(&mutex_b[colour]);
    }

    struct ball *np;
    int returnId = 0;
    TAILQ_FOREACH(np, &heads[colour], entries) {
        if (np == TAILQ_FIRST(&heads[colour])) { // ignoring the head of linked list used for counting
            continue;
        }

        if (np->id != id) {
            returnId = np->id;
            TAILQ_REMOVE(&heads[colour], np, entries);
            free(np);
            return returnId;
        }
    }

    return returnId;
}