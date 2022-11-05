
int mapInputFile(void** map, int* fd, int* numEntries, char* fileName);

int mapOutputFile(void** map, int* fd, int size, char* filename);

int mapCleanUp(void* map, int fd, int size);

int sampleSort(void* arr, int length);

int quickSort(void* arr, int length);

int sampleArray(void* arr, int length, int** samples);

int getKey(void* arr, int index);