#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "psort.h"

/**
 * Takes in an input filename and output filename
 * as args. Maps input file to memory, sorts input
 * file by first 4 bytes of each ENTRY_SIZE byte
 * entry. Maps output file to memory then copies
 * sorted file entries to output file. Implements
 * sample sort and parallel processing.
*/
int main(int argc, char** argv) {
    const int OVERSAMPLING_FACTOR = 3;
    const int NUM_CORES = get_nprocs();
    const int ENTRY_DATA_SIZE = 96;
    const int ENTRY_SIZE = 100;

    int status;

    if(argc != 3) {
        printf("Incorrect # of arguments given\n");
        return 1;
    }

    void* inputMap;
    int inpFd;
    int numEntries;
    if((status = mapInputFile(&inputMap, &inpFd, &numEntries, ENTRY_SIZE, argv[1])) != 0) {
        return status;
    }

    if((status = sampleSort(inputMap, OVERSAMPLING_FACTOR, NUM_CORES, numEntries, ENTRY_SIZE)) != 0) {
        return status;
    }

    void* outputMap;
    int outFd;
    if((status = mapOutputFile(&outputMap, &outFd, numEntries * ENTRY_SIZE, argv[2])) != 0) {
        return status;
    }

    // Copy sorted records over
    memcpy(outputMap, inputMap, numEntries * ENTRY_SIZE);

    if((status = mapCleanUp(inputMap, inpFd, numEntries * ENTRY_SIZE)) != 0) {
        return status;
    }
    if((status = mapCleanUp(outputMap, outFd, numEntries * ENTRY_SIZE)) != 0) {
        return status;
    }
    
    return 0;
}

/**
 * Takes an entry size and filename then maps the whole file
 * to memory using mmap. Gives back a pointer to the mapped
 * memory, the file descriptor, and the number of entries.
 * File is read only.
 * 
 * @param map           Return parameter for map pointer
 * @param fd            Return parameter for file descriptor
 * @param numEntries    Return parameter for number of entries
 * @param entrySize     Size of a single entry
 * @param fileName      Name of file to map
 * @return 0 if successful 1 if an error occurs
*/
int mapInputFile(void** map, int* fd, int* numEntries, int entrySize, char* fileName) {
    if((*fd = open(fileName, O_RDONLY)) < 0) {
        return 1;
    }

    // Get file size
    struct stat fileStats;
    if(fstat(*fd, &fileStats) == -1) {
        return 1;
    }

    if((*map = mmap(NULL, fileStats.st_size, PROT_READ, MAP_SHARED, *fd, 0)) == (void*)-1) {
        return 1;
    }

    *numEntries = fileStats.st_size / entrySize;

    return 0;
}

/**
 * Takes a size and filename and creates a new file.
 * Maps the file using mmap. Gives back a pointer to
 * the mapped memory and a file descriptor.
 * File is write only.
 * 
 * @param map       Return parameter for map pointer
 * @param fd        Return parameter for file descriptor
 * @param size      Size of mapped memory for file
 * @param fileName  Name of file to create/map
 * @return 0 if successful 1 if an error occurs
*/
int mapOutputFile(void** map, int* fd, int size, char* fileName) {
    if((*fd = open(fileName, O_WRONLY | O_CREAT | S_IRWXU)) < 0) {
        return 1;
    }

    if((*map = mmap(NULL, size, PROT_WRITE, MAP_SHARED, *fd, 0)) == (void*)-1) {
        return 1;
    }

    return 0;
}

/**
 * Takes a map pointer, file descriptor, and size munmapping
 * the map and closing the file descriptor
 * 
 * @param map   Map to be munmapped
 * @param fd    File descriptor
 * @param size  Size of map
 * @return 0 if successful 1 if an error occurs
*/
int mapCleanUp(void* map, int fd, int size) {
    if(munmap(map, size) == -1) {
        return 1;
    }

    if(close(fd) == -1) {
        return 1;
    }

    return 0;
}

/**
 * Takes an array of entries and sorts them using
 * the sample sort algorithm: https://en.wikipedia.org/wiki/Samplesort
 * Uses parralel processing with threads.
 * 
 * @param arr       The array to sort
 * @param k         Oversampling constant
 * @param p         Number of processors available
 * @param length    Length of arr
 * @param entrySize Size of each entry in arr
 * @return 0 if successful 1 if an error occurs
*/
int sampleSort(void* arr, int k, int p, int length, int entrySize) {

    // Select (p-1)*k samples

    // Select p bucket bounds

    // Place arr entries into p buckets

    // Start quicksort for each thread on a bucket

    return 0;
}

/**
 * Runs the quick sort algorithm on arr.
 * 
 * @param arr       The array to sort
 * @param length    # of elements in arr
 * @param entrySize Size of each entry in arr
 * @return 0 if successful 1 if an error occurs
*/
int quickSort(void* arr, int length, int entrySize) {

    // Select first element as pivot

    // Partition other elements

    // Call quick sort on partitions

    // Combine

    return 0;
}