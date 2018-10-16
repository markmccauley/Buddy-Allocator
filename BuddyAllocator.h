/* 
    File: my_allocator.h

    Original Author: R.Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 08/02/08

    Modified: Mark McCauley

*/

#ifndef _BuddyAllocator_h_                   // include file only once
#define _BuddyAllocator_h_
#include <iostream>
#include <math.h>
using namespace std;
typedef unsigned int uint;

class BlockHeader{

public:
	BlockHeader* next; // needed for LinkedList
	uint size; 
	bool free;

	BlockHeader() : next(nullptr), free(false) {}
};

class LinkedList{
	// this is a special linked list that is made out of BlockHeader s. 
private:
	BlockHeader* head;		// you need a head of the list
	int size;

public:
	// Default constructor
	LinkedList() : size(0) {}

	int get_size () { return size; } // access the size

	BlockHeader* get_head () { return head; } // access block header

	void insert (BlockHeader* b);	// adds a block to the list

	void remove (BlockHeader* b);  // removes a block from the list

};


class BuddyAllocator{
private:
	/* declare member variables */
	uint block_size;
	uint memory_size;
	int length_of_FreeList;
	LinkedList* FreeList; 
	char* start;

	uint power_of_two (uint num); 

	int position_of (uint required_size);

	// given a block address, this function returns the address of its buddy 
	BlockHeader* getbuddy (BlockHeader* block);
	
	bool isvalid (char* addr);
	// Is the memory starting at addr is a valid block
	// This is used to verify whether the a computed block address is actually correct 

	bool arebuddies (BlockHeader* block, BlockHeader* buddy) { return buddy->free && buddy->size == block->size; }
	// checks whether the two blocks are buddies are not

	BlockHeader* merge (BlockHeader* block, BlockHeader* buddy);
	// this function merges the two blocks returns the beginning address of the merged block
	// note that either block1 can be to the left of block2, or the other way around

	BlockHeader* split (BlockHeader* block);
	// splits the given block by putting a new header halfway through the block
	// also, the original header needs to be corrected

public:
	BuddyAllocator (uint _basic_block_size, uint _total_memory_length); 
	/* This initializes the memory allocator and makes a portion of 
	   â€™_total_memory_lengthâ€™ bytes available. The allocator uses a â€™_basic_block_sizeâ€™ as 
	   its minimal unit of allocation. The function returns the amount of 
	   memory made available to the allocator. If an error occurred, 
	   it returns 0. 
	*/ 

	~BuddyAllocator(); 
	/* Destructor that returns any allocated memory back to the operating system. 
	   There should not be any memory leakage (i.e., memory staying allocated).
	*/ 

	char* alloc(uint _length); 
	/* Allocate _length number of bytes of free memory and returns the 
		address of the allocated portion. Returns 0 when out of memory. */ 

	int free(char* _a); 
	/* Frees the section of physical memory previously allocated 
	   using â€™my_mallocâ€™. Returns 0 if everything ok. */ 
   
	void debug ();
	/* Mainly used for debugging purposes and running short test cases */
	/* This function should print how many free blocks of each size belong to the allocator
	at that point. The output format should be the following (assuming basic block size = 128 bytes):

	128: 5
	256: 0
	512: 3
	1024: 0
	....
	....
	 which means that at point, the allocator has 5 128 byte blocks, 3 512 byte blocks and so on.*/
};

#endif 