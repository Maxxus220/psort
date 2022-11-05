
int mapInputFile(void** map, int* fd, int* numEntries, int entrySize, char* fileName);

int mapOutputFile(void** map, int* fd, int size, char* filename);

int mapCleanUp(void* map, int fd, int size);

int sampleSort(void* arr, int k, int p, int length, int entrySize);

int quickSort(void* arr, int length, int entrySize);