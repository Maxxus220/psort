#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>


int main(int argc, char** argv) {
    const int OVERSAMPLING_FACTOR = 3;
    const int NUM_CORES = get_nprocs();
    const int ENTRY_DATA_SIZE = 96;

    // Check if valid input and get filenames from args

    // Map input file

    // Sample sort file

    // Map output file

    // Copy sorted records over

    // Clean up both maps
    
    return 0;
}

int mapFile(void** map, int* fd, char* fileName, int prot) {
    // Open file

    // Get fd

    // Get file size

    // Map file

    // Set fd

    return 0;
}

int cleanUpMap(void* map, int fd) {
    // Munmap

    // Close file

    return 0;
}

int sampleSort(void* arr, int k, int p, int size) {

    // Select (p-1)*k samples

    // Select p bucket bounds

    // Place arr entries into p buckets

    // Start quicksort for each thread on a bucket

    return 0;
}

int quickSort(void* arr, int size) {

    // Select first element as pivot

    // Partition other elements

    // Call quick sort on partitions

    // Combine

    return 0;
}