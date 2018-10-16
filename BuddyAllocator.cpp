/* 
    File: my_allocator.cpp
*/
#include "BuddyAllocator.h"
#include <iostream>
#include <math.h>
using namespace std;

//-----------------------------------------------------------------------------------------------------

void LinkedList::insert(BlockHeader* b) {
    if (head) { // check for non-empty
        b->next = head; 
    } else { // if empty
        b->next = nullptr;
    }
    head = b; // inserted block is new head
    b->free = true;
    size++;
}

//-----------------------------------------------------------------------------------------------------

void LinkedList::remove(BlockHeader* b) {
    BlockHeader* current = head; // create current node
    if (b == nullptr) return; // do nothing if no blocks are in the list

    if (current == b) { // check if it is the block header
        head = current->next; // move onto next block
    } else {
        while (current->next != b) { // move until we get back to NULL
            current = current->next; // move onto next block
        }
        current->next = current->next->next;  // no delete just move to next
    }
    b->free = false; // set free back to false
    size--;        
}

//-----------------------------------------------------------------------------------------------------

BuddyAllocator::BuddyAllocator (uint _basic_block_size, uint _total_memory_length) { // constructor
    block_size = power_of_two(_basic_block_size);
    memory_size = power_of_two(_total_memory_length);  
    length_of_FreeList = int(log2(memory_size / block_size)) + 1;

    // initialize FreeList
    FreeList = new LinkedList[length_of_FreeList];

    start = new char[memory_size]; // create first block header
    BlockHeader* front_block = (BlockHeader*) (start);
    front_block->size = memory_size; 
    front_block->free = true;

    // insert block header
    FreeList[length_of_FreeList-1].insert(front_block);
}

//-----------------------------------------------------------------------------------------------------

BuddyAllocator::~BuddyAllocator () { // destructor
    delete [] FreeList;
}

//-----------------------------------------------------------------------------------------------------

// Optimized from while loop to a branched architecture
uint BuddyAllocator::power_of_two (uint num) {
    uint power;
    if ((num & (num - 1)) == 0) { // check if the input size is already good
      power = num;
    } else {

        // idea from: https://codereview.stackexchange.com/questions/141210/return-the-next-power-of-two
        // this is likely faster since there is no while loop and 
        // this function is called oftern called
        power = num;
        
        power--;
        power |= power >> 1;
        power |= power >> 2;
        power |= power >> 4;
        power |= power >> 8;
        power |= power >> 16;
        power++;

    }
    return power;
}

//-----------------------------------------------------------------------------------------------------

int BuddyAllocator::position_of (uint required_size) {
    return length_of_FreeList - ((int(log2(memory_size / required_size)) + 1));
}

//-----------------------------------------------------------------------------------------------------

char* BuddyAllocator::alloc (uint _length) {
    int min_level_needed = 0; // location of nearest available blocks
    BlockHeader* block = nullptr; // set a new header to null

    // add the size of the block header to the request for accurate allocation
    uint needed_block_size = power_of_two(_length + sizeof(BlockHeader));
    if (needed_block_size < block_size) { // check if request is smaller than bbs
        needed_block_size = block_size;
    }
    min_level_needed = int(log2(needed_block_size / block_size)); // find min level of block size needed
    if (FreeList[min_level_needed].get_size() > 0) {  // check for smallest block available = bbs
        block = FreeList[min_level_needed].get_head();
        FreeList[min_level_needed].remove(block); // remove from free mem
        return (char*) (block + 1);

    } else {  // blocks > bbs so split
        while (FreeList[min_level_needed].get_size() == 0) {
            min_level_needed++; // find nearest available block
            if (min_level_needed >= length_of_FreeList) { // check bounds
                return nullptr;
            }
        }
        uint available_size = FreeList[min_level_needed].get_head()->size;
        
        BlockHeader* split_block; // get block ready for split
        while (needed_block_size != available_size){ // do while the available size is still bigger
            block = FreeList[position_of(available_size)].get_head();
            split_block = split(block); // split block in half
            available_size /= 2;
        }
        FreeList[position_of(available_size)].remove(split_block);
        return (char*) (split_block + 1);
    }
} 

//-----------------------------------------------------------------------------------------------------

int BuddyAllocator::free (char* _a) {
    if (_a == nullptr) { // check if null address
        return 1;
    }
    char* addr = _a - sizeof(BlockHeader); // get the address needed
    BlockHeader* block = (BlockHeader*) addr;
    FreeList[position_of(block->size)].insert(block);
    bool check = true;
    while(check) { 
        BlockHeader* buddy = getbuddy(block);
        if (arebuddies(block, buddy)) { // check if blocks are buddys
            block = merge(block, buddy); 
        } else {
            check = false;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------------------------------

BlockHeader* BuddyAllocator::getbuddy (BlockHeader* block) { // find buddy address
    return (BlockHeader*) ((((char*)block - start) ^ block->size) + start);
}

//-----------------------------------------------------------------------------------------------------

BlockHeader* BuddyAllocator::merge (BlockHeader* block, BlockHeader* buddy) {
    BlockHeader* left; BlockHeader* right; // place holders
    if (block > buddy) { // check if block is larger than buddy
        left = buddy;
        right = block;
    } else {
        left = block;
        right = buddy;
    }
    int unmerged = length_of_FreeList - (log2(memory_size / left->size) + 1);
    left->size *= 2;
    int merged = length_of_FreeList - (log2(memory_size / left->size) + 1);
    FreeList[unmerged].remove(left); // remove blocks
    FreeList[unmerged].remove(right);
    FreeList[merged].insert(left); // insert merged block

    return left;
}

//-----------------------------------------------------------------------------------------------------

BlockHeader* BuddyAllocator::split (BlockHeader* block) {
    uint prev_size = block->size; 
    FreeList[position_of(prev_size)].remove(block); // remove block to split
    uint next_size = prev_size / 2; // get split size
    block->size = next_size; // split block
    BlockHeader* buddy = getbuddy(block); // split buddy
    buddy->size = next_size;

    FreeList[position_of(next_size)].insert(block); // insert to free list
    FreeList[position_of(next_size)].insert(buddy);
    return block;
}

//-----------------------------------------------------------------------------------------------------

void BuddyAllocator::debug () {
    cout << "Basic block lock size: " << block_size << "\t Total Memory: " << memory_size << endl;
    for (uint i = 0; i < length_of_FreeList; i++) {
        cout << i << " - " << block_size*pow(2,i);  
        BlockHeader* curr = FreeList[i].get_head();
        while (curr != nullptr) {
            cout << " | (" << curr << ")";
            curr = curr->next;
        }
    cout << endl;
    }
}

//-----------------------------------------------------------------------------------------------------