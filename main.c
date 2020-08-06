/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define BUFSIZE 1000

char algorithm[BUFSIZE];
int npages;
int ncurrFrames;
unsigned char *virtmem;

unsigned char *physmem;
struct disk *disk;
int fifoStep = 0;
int now = 1;

int numFaults = 0;
int numDiskReads = 0;
int numDiskWrites = 0;

typedef struct currFrames Frame;

struct currFrames
{
	int page;
	int currBits;
	int loadIn;
	int used;
};
Frame* currFrameTable;

void page_fault_handler( struct page_table *pt, int page )
{
	int currFrame;
	int currBits;

	page_table_get_entry(pt, page, &currFrame, &currBits);


	if(PROT_READ & currBits)
	{
		page_table_set_entry(pt, page, currFrame, currBits|PROT_WRITE);
		currFrameTable[currFrame].currBits = currBits|PROT_WRITE;
		currFrameTable[currFrame].used += 1;
		return;
	}
	numFaults++;
	//printf("page fault on page #%d\n",page);
	if(!strcmp(algorithm,"rand"))
	{
		//I wasn't sure if it was completely random or random from the free spaces, so hopefully this is correct
		currFrame = rand() % ncurrFrames;
	}
	else if(!strcmp(algorithm,"fifo"))
	{
		int old = currFrameTable[0].loadIn;

		int x;
		//gets oldest frame in the page table base don the given time it was placed in
		for(x = 1; x < ncurrFrames; x++)
		{
			if(currFrameTable[x].loadIn < old)
			{
				currFrame = x;
				old = currFrameTable[x].loadIn;
			}
		}


	}
	//attempt to replace the frame that has been used the least
	else if(!strcmp(algorithm,"custom"))
	{
		int min = currFrameTable[0].used;

		int x;
		//gets frame that has been written to the least
		for(x = 1; x < ncurrFrames; x++)
		{
			if(currFrameTable[x].used < min)
			{
				currFrame = x;
				min = currFrameTable[x].used;
			}
		}

	}
	int prevPage = currFrameTable[currFrame].page;
	int prevBits = currFrameTable[currFrame].currBits;

	//write changes to disk if the current currFrame has been changed/written to
	if(prevBits&PROT_WRITE)
	{
		numDiskWrites++;
		disk_write(disk, prevPage, &physmem[currFrame*PAGE_SIZE]);
	}
	if(prevBits != 0)
	{
		page_table_set_entry(pt, prevPage, 0, 0);
	}
	numDiskReads++;
	disk_read(disk, page, &physmem[currFrame*PAGE_SIZE]);
	page_table_set_entry(pt, page, currFrame, PROT_READ);

	currFrameTable[currFrame].page = page;
	currFrameTable[currFrame].currBits = PROT_READ;
	currFrameTable[currFrame].loadIn = now++;
	//page_table_print(pt);
	return;
	//exit(1);
}

int main( int argc, char *argv[] )
{
	srand(time(0));
	if(argc!=5) {
		printf("use: virtmem <npages> <ncurrFrames> <rand|fifo|custom> <alpha|beta|gamma|delta>\n");
		return 1;
	}

	npages = atoi(argv[1]);
	ncurrFrames = atoi(argv[2]);
	strncpy(algorithm, argv[3], sizeof(algorithm));
	const char *program = argv[4];

	currFrameTable = malloc( sizeof(struct currFrames)*ncurrFrames);

	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}


	struct page_table *pt = page_table_create( npages, ncurrFrames, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	virtmem = page_table_get_virtmem(pt);

	physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"alpha")) {
		alpha_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"beta")) {
		beta_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"gamma")) {
		gamma_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"delta")) {
		delta_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[4]);
		return 1;
	}
	printf("Page Faults: %d\nDisk Reads: %d\nDisk Writes: %d\n", numFaults, numDiskReads, numDiskWrites);
	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
