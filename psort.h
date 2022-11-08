
int ENTRY_SIZE = 100;

struct node {
    int entryIndex;
    struct node* next;
};

struct threadArgs {
    void* arr;
    int length;
};

extern int mapInputFile(void** map, int* fd, int* numEntries, char* fileName);

extern int mapOutputFile(void** map, int* fd, int size, char* filename);

extern int mapCleanUp(void* map, int fd, int size);

extern int sampleSort(void* arr, int length);

extern void quickSort(void* arr, int length);

extern int sampleArray(void* arr, int length, int** samples);

extern int getKey(void* arr, int index);

extern void* getEntry(void* arr, int index);

extern void swap(void* arr, int firstIndex, int secondIndex, void* tmp);

extern int selectSamples(int** selectedSamples, int* samples);

extern void createBuckets(struct node*** buckets, struct node*** tails, int* bucketSizes);

extern void fillBuckets(struct node** buckets, struct node** tails, int* bucketSizes, int* selectedSamples, int selectedSamplesLength, void* arr, int length);

extern void placeBuckets(struct node** buckets, int* bucketSizes, void* arr);

extern void* createQuickSortThread(void* args);