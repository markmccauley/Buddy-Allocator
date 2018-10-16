#include "Ackerman.h"
#include "BuddyAllocator.h"
#include <vector>
#include <stdexcept>
#include <unistd.h>

int main(int argc, char ** argv) {
  int basic_block_size = 128; int memory_length = 512 * 1024;
  int c;
  try {
    while ((c = getopt (argc, argv, "b:s:")) != -1) // used this to learn how to 
    	switch(c) {									// use getopt http://www.cplusplus.com/forum/general/141573/
        	case 'b':
          		basic_block_size = atoi(optarg);  
          		break;
        	case 's':
          		memory_length = atoi(optarg);
         		break;
      	}
    if ((memory_length < 0) || (memory_length < basic_block_size) || (basic_block_size < 0)) {
    	throw exception();
    }
    // create memory manager
    BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);

    // test memory manager
   	Ackerman* am = new Ackerman ();
    am->test(allocator); // this is the full-fledged test. 
    
    // destroy memory manager
    delete allocator;
}
	catch (exception){
    cerr << "Exit due to error." << endl;
  	}
}