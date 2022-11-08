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
#include <pthread.h>

const int OVERSAMPLING_FACTOR = 3;
int NUM_CORES;
int ENTRY_SIZE = 100;
char* ERROR_MESSAGE = "An error has occurred";

// #define DEBUGGING
#ifndef DEBUGGING

/**
 * Takes in an input filename and output filename
 * as args. Maps input file to memory, sorts input
 * file by first 4 bytes of each ENTRY_SIZE byte
 * entry. Maps output file to memory then copies
 * sorted file entries to output file. Implements
 * sample sort and parallel processing.
*/
int main(int argc, char** argv) {

    NUM_CORES = get_nprocs();

    int status;

    if(argc != 3) {
        fprintf(stderr, "%s\n", ERROR_MESSAGE);
        return 0;
    }

    void* inputMap;
    int inpFd;
    int numEntries;
    if((status = mapInputFile(&inputMap, &inpFd, &numEntries, argv[1])) != 1) {
        fprintf(stderr, "%s\n", ERROR_MESSAGE);
        return status;
    }

    if((status = sampleSort(inputMap, numEntries)) != 1) {
        fprintf(stderr, "%s\n", ERROR_MESSAGE);
        return status;
    }

    // void* outputMap;
    // int outFd;
    // if((status = mapOutputFile(&outputMap, &outFd, numEntries * ENTRY_SIZE, argv[2])) != 0) {
    //     return status;
    // }

    // Copy sorted records over
    writeEntries(inputMap, numEntries, argv[2]);

    // memcpy(outputMap, inputMap, numEntries * ENTRY_SIZE);
    // msync(outputMap, numEntries * ENTRY_SIZE, MS_SYNC);

    if((status = mapCleanUp(inputMap, inpFd, numEntries * ENTRY_SIZE)) != 1) {
        fprintf(stderr, "%s\n", ERROR_MESSAGE);
        return status;
    }
    // if((status = mapCleanUp(outputMap, outFd, numEntries * ENTRY_SIZE)) != 0) {
    //     return status;
    // }
    
    return 0;
}

#endif

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
        return 0;
    }

    // Get file size
    struct stat fileStats;
    if(fstat(*fd, &fileStats) == -1) {
        return 0;
    }

    if((*map = mmap(NULL, fileStats.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, *fd, 0)) == (void*)-1) {
        return 0;
    }

    *numEntries = fileStats.st_size / ENTRY_SIZE;

    return 1;
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
    if((*fd = open(fileName, O_RDWR | O_CREAT, S_IRWXU)) < 0) {
        return 0;
    }

    if((*map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0)) == (void*)-1) {
        return 0;
    }

    return 1;
}

int writeEntries(void* map, int numEntries, char* fileName) {
    FILE* file;
    if((file = fopen(fileName, "w")) < 0) {
        return 0;
    }

    for(int i = 0; i < numEntries; i++) {
        fwrite(getEntry(map, i), ENTRY_SIZE, 1, file);
    } 

    fclose(file);
    return 1;
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
    if(close(fd) == -1) {
        return 0;
    }

    if(munmap(map, size) == -1) {
        return 0;
    }

    return 1;
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

    int* selectedSamples;
    int selectedSamplesLength = selectSamples(&selectedSamples, samples);

    // Buckets are linked lists
    int bucketSizes[NUM_CORES];
    struct node** buckets;
    struct node** tails;
    struct node** nodeHashTable = malloc(length * sizeof(struct node*));
    createBuckets(&buckets, &tails, bucketSizes);

    fillBuckets(buckets, tails, bucketSizes, nodeHashTable, selectedSamples, selectedSamplesLength, arr, length);

    placeBuckets(buckets, bucketSizes, nodeHashTable, arr);

    int bucketStartIndex = 0;
    pthread_t thread_ids[NUM_CORES];
    for(int i = 0; i < NUM_CORES; i++) {
        struct threadArgs* args = malloc(sizeof(struct threadArgs));
        args->arr = getEntry(arr, bucketStartIndex);
        args->length = bucketSizes[i];
        pthread_create(&(thread_ids[i]), NULL, createQuickSortThread, (void*)(args));
        bucketStartIndex += bucketSizes[i];
    }
    for(int i = 0; i < NUM_CORES; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(samples);
    free(selectedSamples);
    free(buckets);
    free(tails);
    return 1;
}

/**
 * Runs the quick sort algorithm on arr.
 * 
 * @param arr    Array to sort
 * @param length Length of arr
*/
void quickSort(void* arr, int length) {

    if(length <= 1) {
        return;
    }

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
    quickSort((arr + ((leftPartitionSize + 1) * ENTRY_SIZE)), length-leftPartitionSize-1);
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
    // srand(time(NULL));
    srand(202);
    int sampleSize = (NUM_CORES-1) * OVERSAMPLING_FACTOR;
    *samples = malloc(sampleSize * sizeof(int));
    for(int i = 0; i < sampleSize; i++) {
        int sampleIndex = rand() % length;
        (*samples)[i] = getKey(arr, sampleIndex);
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

/**
 * Selects samples from samples at k, 2k, 3k, ..., (p-1)k
 * 
 * @param selectedSamples   Return parameter for array of samples
 * @param samples           Sample array
 * @return Length of selectedSamples
*/
int selectSamples(int** selectedSamples, int* samples) {
    *selectedSamples = malloc((NUM_CORES-1) * sizeof(int));
    for(int i = 1; i < NUM_CORES + 1; i++) {
        (*selectedSamples)[i-1] = samples[i * OVERSAMPLING_FACTOR];
    }
    return NUM_CORES-1;
}

/**
 * Initiates a linked list for each bucket
 * 
 * @param buckets       Return parameter for array of linked list heads
 * @param tails         Return parameter for array of linked list tails
 * @param bucketSizes   Return parameter for sizes of linked lists
*/
void createBuckets(struct node*** buckets, struct node*** tails, int* bucketSizes) {
    for(int i = 0; i < NUM_CORES; i++) {
        bucketSizes[i] = 0;
    }
    *buckets = malloc(NUM_CORES * sizeof(struct node*));
    *tails = malloc(NUM_CORES * sizeof(struct node*));
    for(int i = 0; i < NUM_CORES; i++) {
        struct node* head = malloc(sizeof(struct node));
        head->entryIndex = -1;
        head->next = NULL;
        (*buckets)[i] = head;
        (*tails)[i] = head;
    }
}

/**
 * Enters every element of arr into its corresponding bucket
 * 
 * @param buckets               Array of head nodes for buckets
 * @param tails                 Array of tail nodes for buckets
 * @param bucketSizes           Array of bucket sizes
 * @param selectedSamples       Array containing sample values
 * @param selectedSamplesLength Size of selected samples array
 * @param arr                   Array to take elements from
 * @param length                Length of arr
*/
void fillBuckets(struct node** buckets, struct node** tails, int* bucketSizes, struct node** hashTable, int* selectedSamples, int selectedSamplesLength, void* arr, int length) {
    for(int i = 0; i < length; i++) {

        int key = getKey(arr, i);

        struct node* element = malloc(sizeof(struct node));
        element->entryIndex = i;
        element->next = NULL;
        hashTable[i] = element;

        for(int j = 0; j < selectedSamplesLength; j++) {
            if(key <= selectedSamples[j]) {
                tails[j]->next = element;
                tails[j] = element;
                bucketSizes[j]++;
                goto NEXT;
            }
        }
        tails[selectedSamplesLength]->next = element;
        tails[selectedSamplesLength] = element;
        bucketSizes[selectedSamplesLength]++;

        NEXT:
    }
}

/**
 * Places bucket elements into corresponding positions in arr
 * 
 * @param buckets       Array of bucket head nodes
 * @param bucketSizes   Array of bucket sizes
 * @param arr           Array with entries
*/
void placeBuckets(struct node** buckets, int* bucketSizes, struct node** hashTable, void* arr) {
    int arrCounter = 0;
    void* tmp = malloc(ENTRY_SIZE);
    struct node* tmpNode = malloc(sizeof(struct node*));
    for(int i = 0; i < NUM_CORES; i++) {
        if(bucketSizes[i] == 0) {
            continue;
        }
        struct node* curNode = buckets[i]->next;
        for(int j = 0; j < bucketSizes[i]; j++) {
            if(arrCounter != curNode->entryIndex) {
                swap(arr, arrCounter, curNode->entryIndex, tmp);
                hashTable[arrCounter]->entryIndex = curNode->entryIndex;
                tmpNode = hashTable[arrCounter];
                hashTable[arrCounter] = curNode;
                hashTable[curNode->entryIndex] = tmpNode;
            }
            arrCounter++;
            curNode = curNode->next;
        }
    }
    free(tmp);
}

/**
 * Wrapper function for quicksort when called
 * by a newly created thread.
 * 
 * @param args A pointer to a threadArgs struct
 * @return NULL
*/
void* createQuickSortThread(void* args) {
    struct threadArgs arr = *((struct threadArgs*)args);
    quickSort(arr.arr, arr.length);
    return NULL;
}