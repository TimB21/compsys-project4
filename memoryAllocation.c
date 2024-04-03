
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
	
	// create variables to keep track of the size and starting index of the largest process
	int processSize = 0;
    int processStart = 0;

	// initilaize loop variable 
    int i;
	// loop through all positions in memory
    for (i = 0; i < MEM_SIZE; i++) {
		// if the process at the current location in memory is holds the largest process
        if (memory[i] == id) {
			// if this is the first time coming accross the largest process, set the starting index to the current iteration
            if (processSize == 0) {
                processStart = i; 
            }
			// increment the process size each time we encounter the process during the loop
            processSize++;
        }
    }

	// finds the final position from which the process will be vacated
	int length = processSize + processStart;
    // Set memory slots occupied by the largest process to 0
    for (i = processStart; i < length; i++) {
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
	// stores the starting index of the contiguous block
	int start = -1; 
	// counter for the number of contigous blocks 
	int count = 0; 

	// Iterate through the memory array to find the first available slot of memory which will have enough space to hold the process
	int i;
	for(i = 0; i < MEM_SIZE; i++){
		// if the memory at the current loop iteration is empty
		if(memory[i] == 0){
			// if start as not yet been set, set the value of start equal to the current iteration index
			if(start == -1){
				start = i;
			}
			// increment count for each of the memory locations at which the current loop iteration is empty
			count++;
			// if count is eqal to the size of the process, we have enough space to insert the process
			if(count == size) {
				// fill the memory with the process
				fillMemory(start, id, size);
				return true;
			}
		}
		// if we do not find that the current contiguous region has enough space available, we reset the starting index and contigous block count
		else {
			start = -1; 
			count = 0;
		}
	}
	// return false to indicate that firstFit was unsuccessful
	return false;
}

// variable to keep track of the last index of the last checked memory block
static int lastChecked = 0; 

/**
 * next-fit allocation according to specification above.
 *
 * @param id process id being placed into memory in the allocated slots.
 * @param size number of blocks in the process being allocated.
 * @return true if allocation succeeds, false if it fails.
 */
bool nextFit(int id, int size) {
	// variable to keep track of the starting index of the contiguous block
	int start = -1; 
	// counter for the number of contiguous blocks
	int count = 0; 
	// set i equal to last checked because we start at the last index which was checked
	int i = lastChecked;

	// while the count doesn't exceed memory size
	while(count < MEM_SIZE) {
		// memory is empty at index i 
		if(memory[i] == 0){ 
			// if start has not been set, set start to the current value of i
			if(start == -1){
				start = i; // If slot is empty, it could be the spot to allocate memory
			}
			// increment count since the memory at the current index is empty
			count++;
			// if the current count is equal to the size of the process, we fill the memory and set last checked equal to the next position of memory
			if(count == size){
				fillMemory(start, id, size);
				lastChecked = (i + 1) % MEM_SIZE;
				return true;
			}
		}
		else {
			// if contiguous region is not big enough to allocate the process, reset the start and count variables
			start = -1;
			count = 0;
		}
		// moves to the next block of memory and wraps around if needed
		i = (i + 1) % MEM_SIZE; 
	}
	// next fit allocation fails
	return false; 
}

/**
 * best-fit allocation according to specification above.
 *
 * @param id process id being placed into memory in the allocated slots.
 * @param size number of blocks in the process being allocated.
 * @return true if allocation succeeds, false if it fails.
 */
bool bestFit(int id, int size) {
	// starting index and size of best fitting contiguous region
	int bestStart = -1; 
	int bestSize = MEM_SIZE + 1;
	// starting index and size of the current process being accounted for in the loop
	int start = -1; 
	int count = 0; 
	
	// loops through the memory and takes into size the the empty regions of contiguous memory
	int i;
	for(i = 0; i < MEM_SIZE; i++){
		// if the current index of memory is empty
		if(memory[i] == 0){
			// we set the starting index to the current iteration index if we were not already counting contiguous spaces
			if(start == -1) {
				start = i; 
			}
			// increment the count while there are empty contiguous spaces
			count++;
			// if the count is large enough to hold the process
			if(count >= size) {
				// if the current count is less then the best size, the current count is a better fit for the process
				// so we update the best start and best size to the current start and size of ideal contiguous region 
				if(count < bestSize) {
					bestStart = start;
					bestSize = count;
				}
			}
		}
		// if we enounter a process at the next iteration in memory, reset the contiguous block counter to 0 and the starting index to -1 
		// to indicate that we have come accross allocated memory
		else {
			start = -1;
			count = 0;
		}
	}
	// if best start has an index
	if(bestStart != -1){
		fillMemory(bestStart, id, size);
		return true;
	}
	// if the code reaches this point, best fit has failed
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
	// counts the number of current allocated blocks of memory
	int count = 0;
	// loops through the memory blocks
	int i;
	for(i = 0; i < MEM_SIZE; i++){
		// if the memory at the current iteration index is filled, swap the value stored at the current counter of allocated blocks 
		// with current iteration index with the loop
		// this moves the allocated blocks to the front of memory
		if(memory[i] != 0){
			memory[count] = memory[i];
		}
		// if i and count are not equal, set the value at the current index iteration equal to 0 to indicate that it has already been seen
		// this should not happen if i and count are equal, as this means we would be setting the memory location which we just set to 0
		if(i != count) {
			memory[i] = 0;
		}
		count++;
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
	// checks to see if the policy was allocated successfully
	bool success = policy(id, size);
	// checks if the process was successfully allocated
	if(!success) {
		if(!paging){ 
			// checks the amount of free frames
			// if there is not enough free frames, vacate a process
			// perform compaction if it is not paging
			compaction();
			// check to see if allocation was successful
			success = policy(id, size);
		}
		// if allocation was still not successful and paging is not the policy
		if (!success && !paging) {  
			// create variables to keep track of the largest process id and how many spots in memory it takes up
            int largestProcessId = -1;
            int largestSize = -1;
			// creates variables to keep track of the id and size of the current process being counted 
            int currentProcessId = -1;
            int currentSize = 0;
			// loops through the memory and counts the size of the largest process 
			// won't take into account discontinous process 
			// need to create global variables to keep track of the processes sizes as the loop progresses 
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
				// vacate largest process if the one has been found
                vacateProcess(largestProcessId);
                success = policy(id, size);
				// continue to vacate processes until there success
                if (!success) {
                    printf("Cannot allocate memory for process %d\n", id);
                }
            } else {
                printf("No processes to vacate.\n");
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
