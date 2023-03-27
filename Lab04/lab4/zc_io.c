#include "zc_io.h"

// The zc_file struct is analogous to the FILE struct that you get from fopen.
struct zc_file {
    // Insert the fields you need here.


    /* Some suggested fields :
        - pointer to the virtual memory space
        - offset from the start of the virtual memory
        - total size of the file
        - file descriptor to the opened file
        - mutex for access to the memory space and number of readers
    */
};

/**************
 * Exercise 1 *
 **************/

zc_file* zc_open(const char* path) {
    // To implement
    return NULL;
}

int zc_close(zc_file* file) {
    // To implement
    return -1;
}

const char* zc_read_start(zc_file* file, size_t* size) {
    // To implement
    return NULL;
}

void zc_read_end(zc_file* file) {
    // To implement
}

char* zc_write_start(zc_file* file, size_t size) {
    // To implement
    return NULL;
}

void zc_write_end(zc_file* file) {
    // To implement
}

/**************
 * Exercise 2 *
 **************/

off_t zc_lseek(zc_file* file, long offset, int whence) {
    // To implement
    return -1;
}

/**************
 * Exercise 3 *
 **************/

int zc_copyfile(const char* source, const char* dest) {
    // To implement
    return -1;
}

/**************
 * Bonus Exercise *
 **************/

const char* zc_read_offset(zc_file* file, size_t* size, long offset) {
    // To implement
    return NULL;
}

char* zc_write_offset(zc_file* file, size_t size, long offset) {
    // To implement
    return NULL;
}