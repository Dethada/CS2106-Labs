#include "packer.h"
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/queue.h>

// You can declare global variables here
#define NUM_COLORS 4
#define TYPE_PAIRS 2

#define RG_PAIR 0
#define BB_PAIR 1

#define RED 0
#define GREEN 1
#define BLUE 2
#define BLACK 3

// typedef struct PackingArea {
//     int count;
//     int packed; // number of balls packed in the box
//     int balls[2]; // ids of the balls in the packing area
// } PackingArea;

// Define a sample data structure for the list items
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
    for (int i = 0; i < NUM_COLORS; i++) {
        sem_destroy(&color_mutex[i]);
        Ball *to_delete;
        while (!STAILQ_EMPTY(&heads[i])) {
            to_delete = STAILQ_FIRST(&heads[i]);
            // STAILQ_FOREACH(to_delete, &heads[i], entries) {
            //     STAILQ_REMOVE(&heads[i], to_delete, Ball, entries);
            //     free(to_delete);
            // }
            STAILQ_REMOVE_HEAD(&heads[i], entries);
            sem_destroy(&to_delete->pack);
            free(to_delete);
        }
    }
}

void print_queue(int colour) {
    Ball *tmp_ball;
    printf("Color %d Queue: ", colour);
    STAILQ_FOREACH(tmp_ball, &heads[colour], entries) {
        printf("%d, ", tmp_ball->id);
    }
    printf("\n");
}

int find_pair(int pair_color) {
    sem_wait(&color_mutex[pair_color]);
    // sem_wait(&mutex);
    Ball *tmp_ball = STAILQ_FIRST(&heads[pair_color]);
    int matched = tmp_ball->id;

    STAILQ_REMOVE_HEAD(&heads[pair_color], entries);
    sem_post(&tmp_ball->pack);
    // tmp_ball = STAILQ_FIRST(&heads[colour]);
    // sem_post(&tmp_ball->pack);
    // color_count[colour]--;
    // sem_destroy(&tmp_ball->pack);
    free(tmp_ball);
    color_count[pair_color]--;

    sem_post(&color_mutex[pair_color]);
    // sem_post(&mutex);

    return matched;
}

int pack_ball(int colour, int id) {
    colour--; // make color 0 indexed

    Ball *new_ball = malloc(sizeof(Ball));
    new_ball->id = id;
    // printf("New ball %d of color %d\n", new_ball->id, colour);
    sem_init(&new_ball->pack, 0, 0);

    // add ball to its color queue
    // sem_wait(&color_mutex[colour]);
    sem_wait(&mutex);
    STAILQ_INSERT_TAIL(&heads[colour], new_ball, entries);
    color_count[colour]++;
    print_queue(colour);
    sem_post(&mutex);
    // sem_post(&color_mutex[colour]);

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
    sem_wait(&color_mutex[pair_color]);
    // sem_wait(&mutex);
    printf("Ball: %d Red: %d, Green: %d, Blue: %d, Black: %d\n",
           id, color_count[RED], color_count[GREEN], color_count[BLUE], color_count[BLACK]);
    if (color_count[pair_color] >= 1) {
        // wake the paired ball
        // Ball *tmp_ball = STAILQ_FIRST(&heads[pair_color]);
        // int matched = tmp_ball->id;
        //
        // STAILQ_REMOVE_HEAD(&heads[pair_color], entries);
        // sem_post(&tmp_ball->pack);
        // // tmp_ball = STAILQ_FIRST(&heads[colour]);
        // // sem_post(&tmp_ball->pack);
        // // color_count[colour]--;
        // sem_destroy(&tmp_ball->pack);
        // free(tmp_ball);
        // color_count[pair_color]--;
        //
        // printf("Early pack: %d\n", id);
        return find_pair(pair_color);
    }
    sem_post(&color_mutex[colour]);
    sem_post(&color_mutex[pair_color]);
    // sem_post(&mutex);

    // wait matching color ball to arrive
    sem_wait(&new_ball->pack);
    printf("Wait pack: %d\n", id);

    // printf("Packed ball %d of\n", new_ball->id);

    // find pair
    // sem_wait(&color_mutex[pair_color]);
    // sem_wait(&mutex);

    // Ball *paired_ball = STAILQ_FIRST(&heads[pair_color]); // Get a pointer to the first item
    // int matched = paired_ball->id;
    // // printf("Id: %d, Matched: %d\n", id, matched);
    //
    // // clean up
    // STAILQ_REMOVE_HEAD(&heads[pair_color], entries); // Remove it from the queue
    // sem_destroy(&paired_ball->pack);
    // free(paired_ball);
    // color_count[pair_color]--;
    //
    // // sem_post(&color_mutex[pair_color]);
    // sem_post(&mutex);

    return find_pair(pair_color);
}
