
//
//  File           : raid_cache.c
//  Description    : This is the implementation of the cache for the TAGLINE
//                   driver.
//
//  Author         : ZEKUN YANG


// Includes
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Project includes

#include <raid_cache.h>

typedef uint64_t diskNumber;
typedef uint64_t BlockNum;
typedef int TimeUse;     
typedef char buffer;// Those are type defined , to make the following struc readable

typedef struct m {
	diskNumber dnum; // the disknumber stored in cache 
	
	BlockNum bnum;   // the block number stored in cache 

	TimeUse tus;     // the Timeuse of a  block 

	buffer *buf;     // the buf object 
    
    int   valid;     // The valid is used to indicate if a line of cache is used or not. The "1" means that line is not used , the "0" means that line is used. 

}loc; 
static loc *cache;  // The initialize the cache with the datatype "loc" that is also a type struct
int counter;
// TAGLINE Cache interface

////////////////////////////////////////////////////////////////////////////////
//
// Function     : init_raid_cache
// Description  : Initialize the cache and note maximum blocks
//
// Inputs       : max_items - the maximum number of items your cache can hold
// Outputs      : 0 if successful, -1 if failure

int init_raid_cache(uint32_t max_items) {

	cache=(loc *)malloc(max_items*sizeof(loc));// The memory allocation of the cache 
	

	return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : close_raid_cache
// Description  : Clear all of the contents of the cache, cleanup
//
// Inputs       : none
// Outputs      : o if successful, -1 if failure

int close_raid_cache(void) {
	free(cache);// free the memory allocated to the cache 

	// Return successfully
	return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : put_raid_cache
// Description  : Put an object into the block cache
//
// Inputs       : dsk - this is the disk number of the block to cache
//                blk - this is the block number of the block to cache
//                buf - the buffer to insert into the cache
// Outputs      : 0 if successful, -1 if failure

int put_raid_cache(RAIDDiskID dsk, RAIDBlockID blk, void *buf)  {
//int  full;// variable checking if hte cache is full or not
int j; // The counter for the lines of cache 
int small;// the least recently used the line of this cache 
int success=0;
 for (j=0;j<TAGLINE_CACHE_SIZE;j++)
   

{
		if ( (dsk== cache[j].dnum) && (blk == cache[j].bnum) && (cache[j].valid) == 1){ 
		 
        memcpy((cache[j].buf),buf,1024); 
	    
	    (cache[j].tus) = counter ;

	    success =1 ;
                                                                                      }

	else if (((cache[j].valid) == 0) ){

	        memcpy((cache[j].buf),buf,1024); 										          

            (cache[j].tus) = counter ; 
            
            success = 1 ;

	                                }

}
	
if (success !=1)
 {

int i; // Another counter of the cache line which is used to find the least recently used position 
for (i=0;i<TAGLINE_CACHE_SIZE;j++){

		if (small<cache[i].tus)
			
			{
				small=cache[i].tus;
				memcpy((cache[small].buf),buf,1024); 
				(cache[j].tus)=counter;

	        }

								}
 }
// if the cache has it , update 
// no, cache full eject(add a boollen )
// no find a empty	

	// Return successfully
	return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : get_raid_cache
// Description  : Get an object from the cache (and return it)
//
// Inputs       : dsk - this is the disk number of the block to find
//                blk - this is the block number of the block to find
// Outputs      : pointer to cached object or NULL if not found

void * get_raid_cache(RAIDDiskID dsk, RAIDBlockID blk) {
 int j;
 for (j=0;j<TAGLINE_CACHE_SIZE;j++) 
 {
 	if ( (dsk== (cache[j].dnum)) && (blk == (cache[j].bnum)))
 	 
    return (cache[j].buf);

    else 

	return NULL;	
}

}

