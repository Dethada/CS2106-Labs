#include "packer.h"
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/queue.h>

// You can declare global variables here
#define NUM_COLORS 4

#define RED 0
#define GREEN 1
#define BLUE 2
#define BLACK 3

typedef struct Ball {
    int id;
    sem_t pack;
    STAILQ_ENTRY(Ball) entries;
} Ball;

// Define the STAILQ head structure
STAILQ_HEAD(QueueHead, Ball);

// Each color has a queue
struct QueueHead heads[NUM_COLORS];

int N = 2;
sem_t color_mutex[NUM_COLORS];
sem_t mutex;

int color_count[NUM_COLORS] = { 0 };

void packer_init(void) {
    sem_init(&mutex, 0, 1);
    for (int i = 0; i < NUM_COLORS; i++) {
        STAILQ_INIT(&heads[i]);
        sem_init(&color_mutex[i], 0, 1);
    }
}

void packer_destroy(void) {
    sem_destroy(&mutex);
    for (int i = 0; i < NUM_COLORS; i++) {
        sem_destroy(&color_mutex[i]);
        Ball *to_delete;
        while (!STAILQ_EMPTY(&heads[i])) {
            to_delete = STAILQ_FIRST(&heads[i]);
            STAILQ_REMOVE_HEAD(&heads[i], entries);
            sem_destroy(&to_delete->pack);
            free(to_delete);
        }
    }
}

int pack_ball(int colour, int id) {
    colour--; // make color 0 indexed

    Ball *new_ball = malloc(sizeof(Ball));
    new_ball->id = id;
    sem_init(&new_ball->pack, 0, 0);

    int pair_color = 0;

    switch (colour) {
        case RED:
            pair_color = GREEN;
            break;
        case GREEN:
            pair_color = RED;
            break;
        case BLUE:
            pair_color = BLACK;
            break;
        case BLACK:
            pair_color = BLUE;
            break;
    }

    sem_wait(&color_mutex[colour]);
    // add ball to its color queue
    sem_wait(&mutex);
    STAILQ_INSERT_TAIL(&heads[colour], new_ball, entries);
    color_count[colour]++;

    if (color_count[pair_color] >= 1) {
        // wake the paired ball
        Ball *tmp_ball = STAILQ_FIRST(&heads[pair_color]);
        sem_post(&tmp_ball->pack);
        int matched = tmp_ball->id;

        // removed the paired ball from queue
        STAILQ_REMOVE_HEAD(&heads[pair_color], entries);
        color_count[pair_color]--;

        // unlock mutexes before returning
        sem_post(&mutex);
        sem_post(&color_mutex[colour]);
        return matched;
    }
    sem_post(&mutex);

    sem_wait(&new_ball->pack);

    // find pair
    sem_wait(&mutex);

    Ball *paired_ball = STAILQ_FIRST(&heads[pair_color]); // Get a pointer to the first item
    int matched = paired_ball->id;

    // removed the paired ball from queue
    STAILQ_REMOVE_HEAD(&heads[pair_color], entries); // Remove it from the queue
    color_count[pair_color]--;

    // first ball to enter will do cleanup after second ball matches
    sem_destroy(&paired_ball->pack);
    sem_destroy(&new_ball->pack);
    free(new_ball);
    free(paired_ball);

    sem_post(&mutex);

    sem_post(&color_mutex[colour]);
    return matched;
}
