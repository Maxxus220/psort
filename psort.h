extern int ENTRY_SIZE;

struct node {
    int entryIndex;
    struct node* next;
};

struct threadArgs {
    void* arr;
    int length;
};

int mapInputFile(void** map, int* fd, int* numEntries, char* fileName);

int mapOutputFile(void** map, int* fd, int size, char* filename);

int mapCleanUp(void* map, int fd, int size);

int sampleSort(void* arr, int length);

void quickSort(void* arr, int length);

int sampleArray(void* arr, int length, int** samples);

int getKey(void* arr, int index);

void* getEntry(void* arr, int index);

void swap(void* arr, int firstIndex, int secondIndex, void* tmp);

int selectSamples(int** selectedSamples, int* samples);

void createBuckets(struct node*** buckets, struct node*** tails, int* bucketSizes);

void fillBuckets(struct node** buckets, struct node** tails, int* bucketSizes, int* selectedSamples, int selectedSamplesLength, void* arr, int length);

void placeBuckets(struct node** buckets, int* bucketSizes, void* arr);

void* createQuickSortThread(void* args);

int writeEntries(void* map, int numEntries, char* fileName);