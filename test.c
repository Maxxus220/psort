#include "psort.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void* createRandomEntry() {
    int* entry = malloc(ENTRY_SIZE);
    for(int i = 0; i < ENTRY_SIZE/4; i++) {
        entry[i] = rand();
    }
    return (void*)entry;
}

void* createEntryWithKey(int key) {
    int* entry = malloc(ENTRY_SIZE);
    entry[0] = key;
    for(int i = 1; i < ENTRY_SIZE/4; i++) {
        entry[i] = rand();
    }
    return (void*)entry;
}

// #define WRITE

int main() {
    char* testFileName = "testOutput_2.txt";

#ifdef WRITE
    srand(time(NULL));
    FILE *f = fopen(testFileName, "w");
    for(int i = 0; i < 10; i++) {
        fwrite(createRandomEntry(), 100, 1, f);
    }
    fclose(f);
#endif

#ifndef WRITE
    void* map;
    int fd;
    int numEntries;
    mapInputFile(&map, &fd, &numEntries, testFileName);

    int previous = getKey(map, 0);
    int current;
    int lastFailPrevious = -1;
    int lastFailCurrent = -1;
    int pass = 1;
    for(int i = 0; i < numEntries; i++) {
        current = getKey(map,i);
        printf("Key %d: %d\n", i, current);
        if(current < previous) {
            pass = 0;
            lastFailPrevious = previous;
            lastFailCurrent = current;
        }
        previous = current;
    }
    printf("PASS: %d\n", pass);
    if(pass == 0) {
        printf("Previous: %d Current: %d\n", lastFailPrevious, lastFailCurrent);
    }
    mapCleanUp(map, fd, numEntries * ENTRY_SIZE);
#endif


    return 0;
}
