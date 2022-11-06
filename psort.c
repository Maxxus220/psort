#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "psort.h"

int ENTRY_SIZE = 100;
const int OVERSAMPLING_FACTOR = 3;
const int NUM_CORES;

/**
 * Takes in an input filename and output filename
 * as args. Maps input file to memory, sorts input
 * file by first 4 bytes of each ENTRY_SIZE byte
 * entry. Maps output file to memory then copies
 * sorted file entries to output file. Implements
 * sample sort and parallel processing.
*/
int main(int argc, char** argv) {
    const int NUM_CORES = get_nprocs();

    int status;

    if(argc != 3) {
        printf("Incorrect # of arguments given\n");
        return 1;
    }

    void* inputMap;
    int inpFd;
    int numEntries;
    if((status = mapInputFile(&inputMap, &inpFd, &numEntries, argv[1])) != 0) {
        return status;
    }

    if((status = sampleSort(inputMap, numEntries)) != 0) {
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
int mapInputFile(void** map, int* fd, int* numEntries, char* fileName) {
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

    *numEntries = fileStats.st_size / ENTRY_SIZE;

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
 * @param length    Length of arr
 * @return 0 if successful 1 if an error occurs
*/
int sampleSort(void* arr, int length) {

    int* samples;
    int sampleLength = sampleArray(arr, length, &samples);

    // Temporarily change ENTRY_SIZE to sort samples
    int tmp = ENTRY_SIZE;
    ENTRY_SIZE = 4;
    quickSort(samples, sampleLength);
    ENTRY_SIZE = tmp;

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
*/
void quickSort(void* arr, int length) {

    void* tmp = malloc(ENTRY_SIZE);

    // Select first element as pivot
    int pivot = getKey(arr, 0);
    swap(arr, 0, length-1, tmp);

    // Partition other elements
    int leftPartitionSize = 0;
    for(int i = 0; i < length-1; i++) {
        if(getKey(arr, i) < pivot) {
            if(leftPartitionSize != i) {
                swap(arr, leftPartitionSize, i, tmp);
            }
            leftPartitionSize++;
        }
    }
    swap(arr, leftPartitionSize, length-1, tmp);

    free(tmp);

    // Call quick sort on partitions
    quickSort(arr, leftPartitionSize);
    quickSort((arr + (leftPartitionSize * ENTRY_SIZE)), length-leftPartitionSize);
}

/**
 * Samples (NUM_CORES-1) * OVERSAMPLING_FACTOR keys
 * from arr putting them into an array. Sets samples
 * to that array.
 * 
 * @param arr       Array to sample
 * @param length    Length of arr
 * @param samples   Return parameter for sample array
 * @return Length of samples
*/
int sampleArray(void* arr, int length, int** samples) {
    srand(time(NULL));
    int sampleSize = (NUM_CORES-1) * OVERSAMPLING_FACTOR;
    *samples = malloc(sampleSize * sizeof(int));
    for(int i = 0; i < sampleSize; i++) {
        int sampleIndex = rand() % length;
        *samples[i] = getKey(arr, sampleIndex);
    }
    return sampleSize;
}

/**
 * Gets the pointer to the entry at index
 * 
 * @param arr    Array to search
 * @param index  Index of entry
 * @return Pointer to entry
*/
void* getEntry(void* arr, int index) {
    return arr + (index * ENTRY_SIZE);
}

/**
 * Gets the integer key at the start of an
 * entry at index
 * 
 * @param arr   Array to search
 * @param index Index of entry
 * @return Key of entry
*/
int getKey(void* arr, int index) {
    return *(int*)(arr + (index * ENTRY_SIZE));
}

/**
 * Swaps the entries at firstIndex and secondIndex
 * 
 * @param arr           Array to swap in
 * @param firstIndex
 * @param secondIndex
 * @param tmp           Temporary entry space used to swap values 
 *                      provided by caller must be ENTRY_SIZE
*/
void swap(void* arr, int firstIndex, int secondIndex, void* tmp) {
    void* entryOne = getEntry(arr, firstIndex);
    void* entryTwo = getEntry(arr, secondIndex);

    memcpy(tmp, entryOne, ENTRY_SIZE);
    memcpy(entryOne, entryTwo, ENTRY_SIZE);
    memcpy(entryTwo, tmp, ENTRY_SIZE);
}