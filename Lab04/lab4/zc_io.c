#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "zc_io.h"

// The zc_file struct is analogous to the FILE struct that you get from fopen.
struct zc_file {
    /* Some suggested fields :
        - pointer to the virtual memory space
        - offset from the start of the virtual memory
        - total size of the file
        - file descriptor to the opened file
        - mutex for access to the memory space and number of readers
    */
    char *addr;
    size_t offset;
    size_t size;
    int fd;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_empty;
    int nReaders;
};

/**************
 * Exercise 1 *
 **************/

zc_file* zc_open(const char* path) {
    // Open the file
    // printf("DEBUG: path: %s\n", path);
    int fd = open(path, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("open");
        return NULL;
    }

    // Determine the file size
    struct stat finfo;
    if (fstat(fd, &finfo) == -1) {
        perror("fstat");
        return NULL;
    }
    // printf("DEBUG: file size: %zu\n", finfo.st_size);

    // Map the file to memory
    size_t map_size = (finfo.st_size == 0) ? 1 : finfo.st_size; // mmap() doesn't allow mapping 0 bytes
    char *addr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    // printf("DEBUG: addr: %p\n", addr);

    // Allocate a zc_file struct
    zc_file *file = malloc(sizeof(zc_file));
    if (file == NULL) {
        perror("malloc");
        return NULL;
    }
    file->addr = addr;
    file->offset = 0;
    file->size = finfo.st_size;
    file->fd = fd;
    file->nReaders = 0;
    pthread_mutex_init(&file->mutex, NULL);
    pthread_mutex_init(&file->mutex_empty, NULL);

    return file;
}

int zc_close(zc_file* file) {
    // use msync() flush copy of file in virtual memory into file
    if (msync(file->addr, file->size, MS_SYNC) == -1) {
        perror("msync");
        return -1;
    }

    // Unmap the file from memory
    if (munmap(file->addr, file->size) == -1) {
        perror("munmap");
        return -1;
    }

    // Close the file descriptor
    if (close(file->fd) == -1) {
        perror("close");
        return -1;
    }
    
    pthread_mutex_destroy(&file->mutex);

    // Free the zc_file struct
    free(file);

    return 0;
}

const char* zc_read_start(zc_file* file, size_t* size) {
    pthread_mutex_lock(&file->mutex);
    file->nReaders++;
    if (file->nReaders == 1) {
        pthread_mutex_lock(&file->mutex_empty);
    }
    // prevent over reading
    if (file->offset + *size > file->size) {
        *size = file->size - file->offset;
    }
    // printf("DEBUG: file size: %zu, file offset: %lld, size: %zu\n", file->size, file->offset, *size);

    char *addr = file->addr + file->offset;
    file->offset += *size;
    pthread_mutex_unlock(&file->mutex);

    return addr;
}

void zc_read_end(zc_file* file) {
    // To implement
    pthread_mutex_lock(&file->mutex);
    file->nReaders--;
    if (file->nReaders == 0) {
        pthread_mutex_unlock(&file->mutex_empty);
    }
    pthread_mutex_unlock(&file->mutex);
}

static int zc_resize(zc_file* file, size_t size) {
    if (ftruncate(file->fd, size) == -1) {
        perror("ftruncate");
        return -1;
    }
    file->addr = mremap(file->addr, file->size, size, MREMAP_MAYMOVE);
    file->size = size;
    return 0;
}

char* zc_write_start(zc_file* file, size_t size) {
    pthread_mutex_lock(&file->mutex_empty);

    // check if offset is greater than file size
    if (file->offset > file->size) {
        size_t old_size = file->size;
        if (zc_resize(file, file->offset) == -1) {
            return NULL;
        }
        memset(file->addr + old_size, 0, file->offset - old_size);
    }

    // handle over writing
    if (file->offset + size > file->size) {
        if (zc_resize(file, file->offset + size) == -1) {
            return NULL;
        }
    }
    // printf("DEBUG: file size: %zu, file offset: %lld, size: %zu\n", file->size, file->offset, size);

    char *addr = file->addr + file->offset;
    file->offset += size;


    return addr;

}

void zc_write_end(zc_file* file) {
    msync(file->addr, file->size, MS_SYNC);
    pthread_mutex_unlock(&file->mutex_empty);
}

/**************
 * Exercise 2 *
 **************/

off_t zc_lseek(zc_file* file, long offset, int whence) {
    pthread_mutex_lock(&file->mutex_empty);
    switch(whence) {
        case SEEK_SET:
            if (offset < 0) {
                return -1;
            }
            file->offset = offset;
            break;
        case SEEK_CUR:
            if ((long) file->offset + offset < 0) {
                return -1;
            }
            file->offset += offset;
            break;
        case SEEK_END:
            if ((long) file->size + offset < 0) {
                return -1;
            }
            file->offset = file->size + offset;
            break;
        default:
            return -1;
    }
    pthread_mutex_unlock(&file->mutex_empty);
    return file->offset;
}

/**************
 * Exercise 3 *
 **************/

int zc_copyfile(const char* source, const char* dest) {
    // read source file
    zc_file *src_file = zc_open(source);
    if (src_file == NULL) {
        return -1;
    }
    size_t src_size = src_file->size;
    const char *src_addr = zc_read_start(src_file, &src_size);
    if (src_size != src_file->size) {
        return -1;
    }
    zc_read_end(src_file);

    // write to dest file
    zc_file *dest_file = zc_open(dest);
    if (dest_file == NULL) {
        return -1;
    }
    char *dest_addr = zc_write_start(dest_file, src_size);
    if (dest_addr == NULL) {
        return -1;
    }
    memcpy(dest_addr, src_addr, src_size);
    zc_write_end(dest_file);

    // truncate file if needed
    if (dest_file->size > src_size) {
        if (ftruncate(dest_file->fd, src_size) == -1) {
            perror("ftruncate");
            return -1;
        }
    }

    // close files
    zc_close(src_file);
    zc_close(dest_file);

    return 0;
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
