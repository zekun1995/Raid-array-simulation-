///////////////////////////////////////////////////////////////////////////////
//
//  File           : tagline_driver.c
//  Description    : This is the implementation of the driver interface
//                   between the OS and the low-level hardware.
//
//  Author         : Zekun Yang 	
//  Created        : 10/07/2015

// Include Files
#include <stdlib.h>
#include <string.h>


// Project Includes
#include "raid_bus.h"
#include "tagline_driver.h"
#include "raid_cache.h"
#include "raid_bus.h"
// i got the idea for this funtion from the lecture notes of Dr Mcdaniel. i do the bit operation,which is the shitf to make the actual instruction

RAIDOpCode create_raid_request(uint64_t requestType, uint64_t numberOfBlocks,uint64_t diskNumber, // The function used to create raid request
			uint64_t unused, uint64_t status, uint64_t blockId) { 

			RAIDOpCode requestOps ;
			uint64_t a,b,c,d,e,f; 

			a=requestType<<56;

			b=numberOfBlocks<<48;

			c=diskNumber<<40;

			d=unused<<33;

			e=status<<32;

			f=blockId;

			requestOps=a|b|c|d|e|f;//  i " or " them together and the final operation code will be displayed.
                                   
 return ( requestOps);

}

uint64_t extract_raid_response(RAIDOpCode resp, RAID_OPCODE_FIELDS part ) {// The function used to extract the rai_reponsed

uint64_t temp; 

switch (part)
{
			case 3:
				temp = (resp<<part*8)>>57;
			      
			    break;  
			case 4:
				temp = (resp<<31)>>63;

				break;
			case 5:
			    temp = (resp<<part*4)>> 56;

			    break;                                 
			default:                        
				temp = (resp<<part*8)>>56 ;// we shifted the resp to the left for certain number of bits so the left to be zeros later 
				break;
		                                     // we shift the resp to the right for certain number of bits for big-endian's sake. 
		 
		}

return temp ;  

}

typedef struct m {// the strcut used to store the information of each block ( including the Tagline number , rad disk ID and raid disk block )
	
    TagLineNumber tgl; //The tag line number of the the block

    RAIDDiskID  rdi ;// "rdi is the disk that  a block from the taglinewill be written to
    RAIDDiskID rdi2;// "rdi2" means the backup version of "rdi" in previous line 
    RAIDBlockID rbi ;// "rbi" is the blokID of the target disk that a block from tagline will be written to 
    RAIDBlockID rbi2;// "rbi2" means the backup version of the "rbi" in the previous line.
 
    int usage;// the usage represent the status of a block(used of not)


}loc;// the "loc" is defined  as the memory type of the struct i just made
 
static loc *memory;// the  pointer with the datatype loc
static uint32_t MAX;//a global variable that will be initiated as the number of the maxlines



////////////////////////////////////////////////////////////////////////////////
//
// Function     : tagline_driver_init
// Description  : Initialize the driver with a number of maximum lines to process
//
// Inputs       : maxlines - the maximum number of tag lines in the system
// Outputs      : 0 if successful, -1 if failure

int tagline_driver_init(uint32_t maxlines) {//The initialization of the disks
                
			
client_raid_bus_request(create_raid_request(RAID_INIT, 16,RAID_DISKS,0,0,0),  NULL) ;// the initiation of the disk

int i;
for ( i=0;i<RAID_DISKS;i++)// the counter "i" is number of the disks   
client_raid_bus_request(create_raid_request(RAID_FORMAT, 0,i,0,0,0),  NULL) ;// the formattting of the disk 
    
      

logMessage(LOG_INFO_LEVEL, "TAGLINE: initialized storage (maxline=%u)", maxlines);
         
 memory=(loc *)malloc(maxlines * MAX_TAGLINE_BLOCK_NUMBER *sizeof(loc));// the memory allocation of the 2d ARRAY that can represent the simulation of the taglines.

 MAX = maxlines;
      


	return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : tagline_read
// Description  : Read a number of blocks from the tagline driver
//
// Inputs       : tag - the number of the tagline to read from
//                bnum - the starting block to read from
//                blks - the number of blocks to read
//                bug - memory block to read the blocks into
// Outputs      : 0 if successful, -1 if failure

////////////////////////////////////////

static uint8_t j = 0;//the counter " j " is the global variable representing the blockID of the "Backup" disks
static uint64_t bposition=0;// the "bposition" is also a counter representing the current position where blocks wrote to the disks

int tagline_read(TagLineNumber tag, TagLineBlockNumber bnum, uint8_t blks, char *buf) {// The fucntion used to read the blocks from disks
 


	uint8_t i;
	for (i=0;i< blks;i++)// My program reads only one block per time.Thus i made a counter and set it less than the total number of blocks 
	 { 
	   client_raid_bus_request(create_raid_request(RAID_READ,1,(memory+128*tag+bnum+i)->rdi,0,0,(memory+128*tag+bnum+i)->rbi), buf+(1024*i));

	}

            // i use  "(memory+128*tag+bnum+i)" because the "memory" always points the first element of tagline. The tag *128 can lead us to the 
            // follwing taglines because each tagline has 128 blocks and "+bum+i" leads us to the exact position of the block of that  tagline.
     


	// Return successfully
	logMessage(LOG_INFO_LEVEL, "TAGLINE : read %u blocks from tagline %u, starting block %u.",
			blks, tag, bnum);
	return(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : tagline_write
// Description  : Write a number of blocks from the tagline driver
//
// Inputs       : tag - the number of the tagline to write from
//                bnum - the starting block to write from
//                blks - the number of blocks to write
//                bug - the place to write the blocks into
// Outputs      : 0 if successful, -1 if failure
 

int tagline_write(TagLineNumber tag, TagLineBlockNumber bnum, uint8_t blks, char *buf) {

   
uint8_t i;//counter 
for (i=0;i< blks;i++)//every time we only wirte a single 
 {
 	if  ( ((memory+128*tag+bnum+i)->usage) ==1) // the usuage "1" means this block is occupied 
 	{
 		client_raid_bus_request(create_raid_request(RAID_WRITE,1,(memory+128*tag+bnum+i)->rdi,0,0,(memory+128*tag+bnum+i)->rbi),(buf +(1024*i)));
 		client_raid_bus_request(create_raid_request(RAID_WRITE,1,(memory+128*tag+bnum+i)->rdi2,0,0,(memory+128*tag+bnum+i)->rbi2),(buf +(1024*i)));
 	    // if a disk is occupied and we want to write into it, we have to rewrite it 

 	} else {
	 	client_raid_bus_request(create_raid_request(RAID_WRITE,1,j,0,0,bposition),(buf +(1024*i)));    
	    client_raid_bus_request(create_raid_request(RAID_WRITE,1,j+8,0,0,bposition),(buf +(1024*i))); // the counter j is a globally defined counter, we use"j+8" becaus
	                                                                                           //every disk we creat h its backup on the disk 8 unit left
	    (memory+128*tag+bnum+i)->tgl = tag;                                                    
	    (memory+128*tag+bnum+i)->rdi = j; // the raid disk number of the original disk
	    (memory+128*tag+bnum+i)->rdi2 = j+8; // the raid disk number of the back up disk
	     
	    (memory+128*tag+bnum+i)->rbi = bposition;// the block position of the original disk
	    (memory+128*tag+bnum+i)->rbi2 = bposition;// the block position of the back up disk

	    (memory+128*tag+bnum+i)->usage=1;// the 1 means the block is occupied
	    
	    if (bposition < RAID_DISKBLOCKS)
	   		bposition++; // I used the linear method such that the postion increases by 1 unit per time i write to the disk
	   	else {
	   		bposition = 0;
	   		j++;// if a block's position number is lager that the RAID_DISKBLOCKS, we need to move to next disk" j" here is a counter of the disk number.
	   	}

   }
}

// Return successfully
	logMessage(LOG_INFO_LEVEL, "TAGLINE : wrote %u blocks to tagline %u, starting block %u.",
			blks, tag, bnum);
	return(0);
}

//////////////////////////////////////////////////////////////////////////////
//
// Function     : tagline_close
// Description  : Close the tagline interface
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int tagline_close(void) {
    client_raid_bus_request(create_raid_request(RAID_CLOSE, 0,0,0,0,0),  NULL);
	
	// we use it to close program.
	// Return successfully
	logMessage(LOG_INFO_LEVEL, "TAGLINE storage device: closing completed.");
	return(0);
}
////////////////////////////////////////////////////////////////////////////////
// Function     : raid_disk_signal
// Description  : detect the failure of disks and recover them
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int raid_disk_signal(void){ // tthe function used to handle the disk failure.
	

	int i = -1;
	int j = 0;
	int re = 0;
	char *buf[1024];

	while (i < (RAID_DISKS-1) && re != 2) {// "re " is also a global variable that hold the status returned by the function RAID_STATUS
		i++;//is the raid disk number we have to go through to check if it is read yor not. 
		re = client_raid_bus_request(create_raid_request(RAID_STATUS,0,i,0,0,0),NULL);
	}

	if (re == 2)
		client_raid_bus_request(create_raid_request(RAID_FORMAT, 0,i,0,0,0),  NULL);// Once a disk is failed we need to foramt it.

	for (j=0;j<(MAX * MAX_TAGLINE_BLOCK_NUMBER);j++)// copy the data from back up  to failed disk "MAX" means the maxline of a tagline
	{
		if ((memory+j)->rdi == i)
		{
			 client_raid_bus_request(create_raid_request(RAID_READ,1,(memory+j)->rdi2,0,0,(memory+j)->rbi2), buf);// we first read from the backup and then write.
			 client_raid_bus_request(create_raid_request(RAID_WRITE,1,(memory+j)->rdi,0,0,(memory+j)->rbi), buf);

		} else if ((memory+j)->rdi2 == i) {// when the back up is failed , we copy the memory from original to it or vice versa
			client_raid_bus_request(create_raid_request(RAID_READ,1,(memory+j)->rdi,0,0,(memory+j)->rbi), buf);
			client_raid_bus_request(create_raid_request(RAID_WRITE,1,(memory+j)->rdi2,0,0,(memory+j)->rbi2), buf);

		}
	}
	return (0);

}