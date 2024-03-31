
#include <string.h> // for strcmp
#include <stdlib.h>  // for exit
#include <stdio.h>   // For IO
#include <stdbool.h> // For bool type

// Number of memory blocks
#define MEM_SIZE 128
// Each frame/page takes two memory blocks
#define FRAME_SIZE 2

// Array simulates memory
int memory[MEM_SIZE];

/**
 * Fill memory array with zeroes, which represent empty space (available for allocation)
 */
void clearMemory() {
	int i;
	for(i = 0; i < MEM_SIZE; i++) { // Initialize memory
		memory[i] = 0; // 0 indicates free memory
	}
}

// Remember last allocation for next-fit algorithm
int lastAllocationPoint = 0;

/**
 * Fill a specified chunk of memory with a "process" id, which
 * in our simple simulation means that the memory block has been
 * allocated to that process. The function fills size number of
 * slots with the id, starting at the startBlock
 *
 * @param startBlock index in memory array to start allocation from.
 * @param id the process id to place in allocated array locations.
 * @param size number of array locations to place the id into, starting from startBlock (inclusive)
 */
void fillMemory(int startBlock, int id, int size) {
	printf("Allocate %d through %d to %d\n", startBlock, startBlock + size - 1, id);
	int i;
	for(i = 0; i < size; i++) {
		if(memory[startBlock + i] != 0) { // Useful check for debugging: Never fill reserved space
			printf("ERROR! Cell %d not empty. Contains %d\n", startBlock + i, memory[startBlock + i]);
			exit(1); // Exit with error
		}
		if(startBlock + i >= MEM_SIZE) { // Useful check for debugging: Never go outside of bounds
			printf("ERROR! Cell %d out of bounds\n", startBlock + i);
			exit(1); // Exit with error
		}
		memory[startBlock + i] = id; // "allocate" block to id
	}
	lastAllocationPoint = startBlock + size; // Information tracked for next-fit algorithm
}

/**
 * Increment every time vacateProcess is called
 */
int processesVacated = 0;

/**
 * When memory gets full, it will be necessary to vacate "processes."
 * This function scans memory and deallocates (sets to zero) all slots 
 * allocated to the process with the given id.
 *
 * @param id process id in array to replace with 0.
 */
void vacateProcess(int id) {
	printf("vacate %d\n", id);
	processesVacated++; // Incremented processes vacated with each call to vacate process
	
	int maxProcessSize = 0;
    int maxProcessStart = 0;
    int currentProcessSize = 0;
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (memory[i] == id) {
            if (currentProcessSize == 0) {
                maxProcessStart = i; // Update the start index of the current process
            }
            currentProcessSize++;
        } else {
            if (currentProcessSize > maxProcessSize) {
                maxProcessSize = currentProcessSize;
                maxProcessStart = i - currentProcessSize; // Update the start index of the largest process
            }
            currentProcessSize = 0;
        }
    }

	int length = maxProcessSize + maxProcessStart;
    // Set memory slots occupied by the largest process to 0
    for (i = maxProcessStart; i < length; i++) {
        memory[i] = 0;
    }
}

/**
 * This is a pointer to a function. The policy is the memory allocation policy.
 * You will be implementing multiple policies, each with the same signature:
 * a bool is returned indicating the success of the allocation, and the two int
 * inputs are the id number to reserve the memory for, and the size 
 * (number of blocks) to reserve. This value is set in the main function based
 * on command-line parameters.
 */
bool (*policy)(int,int);

/**
 * For each allocation policy below, assign the process id to
 * a contiguous region of memory with "size" number of blocks. Return
 * true on success, and false if no region large enough exists.
 */

/**
 * first-fit allocation according to specification above.
 *
 * @param id process id being placed into memory in the allocated slots.
 * @param size number of blocks in the process being allocated.
 * @return true if allocation succeeds, false if it fails.
 */
bool firstFit(int id, int size) {
	int start = -1; // Variable to store the starting index of the contiguous block
	int count = 0; // Counter for the number of contigous blocks

	// Iterate through the memory array to find the first available slot of memory 
	int i;
	for(i = 0; i < MEM_SIZE; i++){
		if(memory[i] == 0){
			if(start == -1){
				start = i;
			}
			count++;
			if(count == size) {
				fillMemory(start, id, size);
				return true;
			}
		}
		else {
			start = -1; // Reset start if contigous empty slots 
			count = 0;
		}
	}

	return false;
}

/**
 * next-fit allocation according to specification above.
 *
 * @param id process id being placed into memory in the allocated slots.
 * @param size number of blocks in the process being allocated.
 * @return true if allocation succeeds, false if it fails.
 */
bool nextFit(int id, int size) {
	static int lastChecked = 0; // Static variable to keep track of the index of last checked memory block
	int start = -1; // Variable to store the starting index of the contiguous block
	int count = 0; // Counter for the number of contigous blocks
	int i = lastChecked;

	while(count < MEM_SIZE) {
		if(memory[i] == 0){ // Memory is empty at index i
			if(start == -1){
				start = i; // If slot is empty, it could be the spot to allocate memory
			}
			count++;
			if(count == size){
				fillMemory(start, id, size);
				lastChecked = (i + 1) % MEM_SIZE;
				return true;
			}
		}
		else {
			start = -1;
			count = 0;
		}
		i = (i + 1) % MEM_SIZE; // Move to the next memory block, using modulo to wrap around
	}
	return false; // Allocation failed
}

/**
 * best-fit allocation according to specification above.
 *
 * @param id process id being placed into memory in the allocated slots.
 * @param size number of blocks in the process being allocated.
 * @return true if allocation succeeds, false if it fails.
 */
bool bestFit(int id, int size) {
	int bestStart = -1; // Starting index of the best fit contigous region
	int bestSize = MEM_SIZE + 1; // Stores the size of the region which fits the process size best
	int start = -1; // Variable to store the starting index of the contiguous block
	int count = 0; // Counter for the number of contigous blocks
	
	int i;
	for(i = 0; i < MEM_SIZE; i++){
		if(memory[i] == 0){
			if(start == -1) {
				start = i; 
			}
			count++;
			if(count >= size) {
				if(count < bestSize) {
					bestStart = start;
					bestSize = count;
				}
			}
		}
		else {
			start = -1;
			count = 0;
		}
	}

	if(bestStart != -1){
		fillMemory(bestStart, id, size);
		return true;
	}

	return false;
}

/**
 * worst-fit allocation according to specification above.
 *
 * @param id process id being placed into memory in the allocated slots.
 * @param size number of blocks in the process being allocated.
 * @return true if allocation succeeds, false if it fails.
 */
bool worstFit(int id, int size) {
	int worstStart = -1; // Starting index of the best fit contigous region
	int worstSize = -1; // Stores the size of the region which fits the process size best
	int start = -1; // Variable to store the starting index of the contiguous block
	int count = 0; // Counter for the number of contigous blocks
	
	int i;
	for(i = 0; i < MEM_SIZE; i++){
		if(memory[i] == 0){
			if(start == -1) {
				start = i; 
			}
			count++;
			if(count >= size) {
				if(count > worstSize) {
					worstStart = start;
					worstSize = count;
				}
			}
		}
		else {
			start = -1;
			count = 0;
		}
	}

	if(worstStart != -1){
		fillMemory(worstStart, id, size);
		return true;
	}

	return false;
}

/**
 * Implements simple paging memory allocation.
 * The pages policy is special in that it may allocate
 * non-contiguous regions of memory to the process id.
 * However, a full frame must be reserved for each chunk
 * of allocation, even if it is not used by the process.
 *
 * @param id process id being placed into memory in the allocated slots.
 * @param size number of blocks in the process being allocated.
 * @return true if allocation succeeds, false if it fails.
 */
bool pages(int id, int size) {
    int requiredFrames = (size + FRAME_SIZE - 1) / FRAME_SIZE; // Calculate number of frames required
	
    // Iterate through memory to find available frames
    int start = -1; // Variable to store the starting index of the contiguous block
    int count = 0; // Counter for the number of contiguous blocks
    for (int i = 0; i < MEM_SIZE; i++) {
        if (memory[i] == 0) { // Found a free frame
            if (start == -1) {
                start = i; // Set the start index of the contiguous block
            }
            count++; // Increment the count of contiguous blocks
            if (count == requiredFrames * FRAME_SIZE) { // If enough contiguous frames are found
                fillMemory(start, id, requiredFrames * FRAME_SIZE); // Allocate frames to the process
                return true; // Allocation successful
            }
        } else { // Reset start if contiguous empty slots
            start = -1;
            count = 0;
        }
    }
    return false;
} 

// Track the number of compaction events
int compactionEvents = 0;

/**
 * Compact memory: This function should never be used in
 * conjunction with the paging policy, but all of the others
 * need it.
 */
void compaction() {
	int free = 0;
	int i;
	for(i = 0; i < MEM_SIZE; i++){
		if(memory[i] != 0){
			memory[free] = memory[i];
		if(i != free) {
			memory[i] = 0;
		}
		free++;
		}
	}
	// Clear remaining memory blocks
    for (int i = free; i < MEM_SIZE; i++) {
        memory[i] = 0;
    }
	printf("Memory compacted \n");
}

// Used to track whether the paging policy was chosen, so compaction can be avoided
bool paging = false;

/**
 * Allocate memory of appropriate size to the process with id using the
 * chosen policy. For paging, allocation should only fail if there are not
 * enough free frames, in which case the process occupying the most memory
 * should be vacated before trying again. 
 *
 * For policies that allocate in contiguous space,
 * failure to allocate should result first in a check on the number of
 * remaining free blocks in memory. If there is enough space to hold the
 * process, but the space is fragmented, then compaction should occur.
 * Otherwise, the policy occupying the most space in memory should be
 * vacated before repeating the attempt to allocate.
 *
 * @param id process id being placed into memory in the allocated slots.
 * @param size number of blocks in the process being allocated.
 */
void allocate(int id, int size) { 
	bool success = policy(id, size);
	if(!success) {
		if(!paging){ 
			compaction();
			success = policy(id, size);
		}
		if (!success) {  
            int largestProcessId = -1;
            int largestSize = -1;
            int currentProcessId = -1;
            int currentSize = 0;
            for (int i = 0; i < MEM_SIZE; ++i) {
                if (memory[i] != currentProcessId) {
                    if (currentSize > largestSize) {
                        largestSize = currentSize;
                        largestProcessId = currentProcessId;
                    }
                    currentProcessId = memory[i];
                    currentSize = 1;
                } else {
                    currentSize++;
                }
            }
            if (currentSize > largestSize) {
                largestSize = currentSize;
                largestProcessId = currentProcessId;
            }
            if (largestProcessId != -1) {
                vacateProcess(largestProcessId);
                success = policy(id, size);
                if (!success) {
                    printf("Error: Cannot allocate memory for process %d\n", id);
                }
            } else {
                printf("Error: No processes to vacate.\n");
            }
		} 
	}
}

/**
 * Main function runs a memory management simulation based on an input file,
 * and outputs the final state of memory to a specified output file. The command
 * line arguments dictate how to set up the simulation.
 *
 * @param argc number of command line parameters.
 * @param argv elements of this array are string representations of each command line parameter.
 * @return Success returns 0, crash/failure returns -1.
 */
int main(int argc, char *argv[]) {
	// Proper usage consists of 4 arguments:
	// 0: C file: name of program being run
	// 1: input fiename: file with sequence of memory requests (one int per line)
	// 2: output filename: file that final memory contents will be (over)written to
	// 3: memory allocation policy: ff=first-fit, bf=best-fit, nf=next-fit, wf=worst-fit, pages=paging 
	if(argc != 4) {
		printf("Incorrect arguments. Expected:\n");
		printf(" 0: C file: name of program being run\n");
		printf(" 1: input fiename: file with sequence of memory requests (one int per line)\n");
		printf(" 2: output filename: file that final memory contents will be (over)written to\n");
		printf(" 3: memory allocation policy: ff=first-fit, bf=best-fit, nf=next-fit, wf=worst-fit, pages=paging\n");
		return 1; // Error
	}

	// Check for valid memory allocation policy, using strcmp, then assign policy variable
	if(strcmp(argv[3], "ff") == 0) { 
		printf("first-fit allocation\n");
		policy = &firstFit;
	} else if(strcmp(argv[3], "nf") == 0) { 
		printf("next-fit allocation\n");
		policy = &nextFit;
	} else if(strcmp(argv[3], "bf") == 0) { 
		printf("best-fit allocation\n");
		policy = &bestFit;
	} else if(strcmp(argv[3], "wf") == 0) { 
		printf("worst-fit allocation\n");
		policy = &worstFit;
	} else if(strcmp(argv[3], "pages") == 0) { 
		printf("simple paging\n");
		policy = &pages;
		paging = true; // indicates that compaction should not occur
	} else {
		printf("Invalid memory allocation policy\n");
		printf(" ff=first-fit, bf=best-fit, nf=next-fit, wf=worst-fit, pages=paging\n");
		return 1; // Error
	}

	printf("Reading from file: %s\n", argv[1]); // Second argument is input filename from user
	FILE *input, *output;
	input = fopen(argv[1], "r"); // Open file in read mode
	if(input == NULL) { // Failed to read file
		printf("Problem reading file %s\n", argv[1]);
		return 1; // Error
	}

	clearMemory(); // Clear memory before allocating it for processes
	int requestID = 1; // Start IDs at 1 because 0 indicates empty memory
	int requestSize; // Holds values read from file
	while (fscanf(input, "%d", &requestSize) != EOF) { // Scan numbers into requestSize until end of file
		printf("%d requested %d blocks\n", requestID, requestSize); // Announce the request
		allocate(requestID, requestSize); // Claim space for "process"
		requestID++; // For simplicity, each request is from a new "process"
	}
	fclose(input); // Close the file

	printf("%d processes vacated\n", processesVacated);
	printf("%d compaction events\n", compactionEvents);

	// Output the state of memory at the end of the simulation
	printf("Writing to file: %s\n", argv[2]); // Third argument is output filename from user
	output = fopen(argv[2], "w"); // Open file in write mode
	int i;
	for(i = 0; i < MEM_SIZE; i++) {
		fprintf(output, "%d\n", memory[i]);
	}
	fclose(output); // Close the file

	return 0;
}
