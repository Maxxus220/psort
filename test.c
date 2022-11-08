#include "psort.h"
#include "psort.c"
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

int main() {
    char* testFileName = "testInput_1.txt";
    // srand(time(NULL));
    // FILE *f = fopen(testFileName, "w");
    // for(int i = 0; i < 30; i++) {
    //     fwrite(createRandomEntry(), 100, 1, f);
    // }
    // fclose(f);

    void* map;
    int fd;
    int numEntries;
    mapInputFile(&map, &fd, &numEntries, testFileName);

    for(int i = 0; i < numEntries; i++) {
        printf("Key %d: %d\n", i, getKey(map,i));
    }

    mapCleanUp(map, fd, numEntries * ENTRY_SIZE);


    return 0;
}
