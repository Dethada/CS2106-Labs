#include "packer.h"
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

// You can declare global variables here
#define NUM_COLORS 3

typedef struct PackingArea {
    int index; // index of the next available slot in the packing area
    int packed; // number of balls packed in the box
    int *balls; // ids of the balls in the packing area
} PackingArea;

int N = 0;
sem_t mutex[NUM_COLORS];
sem_t sem[NUM_COLORS];
sem_t ready[NUM_COLORS];
PackingArea *packing_areas[NUM_COLORS];

void packer_init(int balls_per_pack) {
    N = balls_per_pack;
    for (int i = 0; i < NUM_COLORS; i++) {
        sem_init(&mutex[i], 0, 1);
        sem_init(&sem[i], 0, N);
        sem_init(&ready[i], 0, 0);
        packing_areas[i] = malloc(sizeof(PackingArea));
        packing_areas[i]->index = 0;
        packing_areas[i]->packed = 0;
        packing_areas[i]->balls = malloc(sizeof(int) * N);
    }
}

void packer_destroy(void) {
    for (int i = 0; i < NUM_COLORS; i++) {
        sem_destroy(&mutex[i]);
        sem_destroy(&sem[i]);
        sem_destroy(&ready[i]);
        free(packing_areas[i]->balls);
        free(packing_areas[i]);
    }
}

void reset_area(PackingArea *color) {
    color->index = 0;
    color->packed = 0;
}

void pack_ball(int colour, int id, int *other_ids) {
    colour--; // make color 0 indexed
    PackingArea *area = packing_areas[colour];

    sem_wait(&sem[colour]); // enter packing area

    sem_wait(&mutex[colour]);

    area->balls[area->index] = id;
    area->index++;

    if (area->index == N) { // if N balls are here, pack them
        for (int i = 0; i < N; i++) {
            sem_post(&ready[colour]);
        }
    }

    sem_post(&mutex[colour]);

    sem_wait(&ready[colour]); // wait for N balls to be in the packing area

    // pack balls
    int j = 0;
    for (int i = 0; i < N; i++) {
        if (area->balls[i] != id) {
            other_ids[j] = area->balls[i];
            j++;
        }
    }

    sem_wait(&mutex[colour]);

    area->packed++;
    if (area->packed == N) {
        // Last ball to be packed does clean up
        reset_area(area);
        // restore semaphore to N
        for (int i = 0; i < N; i++) {
            sem_post(&sem[colour]);
        }
    }

    sem_post(&mutex[colour]);
}
