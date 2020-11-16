#include <iostream>
#include "NumberDefinitions.h"
#include <vector>
#include <map>
using namespace std;

#define PAGE_SIZE 4096 // 4 kb
#define MEMORY_SIZE 16384 //16 kb
#define PAGE_AMOUNT ceil(MEMORY_SIZE / PAGE_SIZE)
#define MIN_CLASS_SIZE 16 // 2^x where x >= 4
#define BLOCK_HEADER_SIZE 1 //bool busy or not

static void* startPointer;

enum class pageStatus {
	Free,
	Divided,
	MultipageBlock
};

#pragma pack(push, 1)
struct memoryPageHeader {
	pageStatus status;
	size_t classSize;
	uint8_t* availableBlock;

};

static char memory[MEMORY_SIZE];
static std::vector<void*> freePages;
static std::map<void*, memoryPageHeader> headers;
static std::map <int, std::vector<void*>> classifiedPages;

void initializePages() {
	// Set pointer to the start of the memory
	startPointer = memory;

	for (int i = 0; i < PAGE_AMOUNT; i++) {
		// calculate a pointer for a next page
		void* pagePointer = (uint8_t*)startPointer + PAGE_SIZE * i;
		// put it in the list of free pages
		freePages.push_back(pagePointer);
		// create a header for the page in that position and put it into dictonary
		memoryPageHeader header = { pageStatus::Free, 0, nullptr };
		headers.insert({ pagePointer, header });
	}

	// create entry for all the classes in the hash table
	for (int classSize = MIN_CLASS_SIZE; classSize <= PAGE_SIZE / 2; classSize *= 2) {
		classifiedPages.insert({ classSize, {} });
	}
}

// sets the info for a particular header that works for the page with address
void setPageHeader(void* pagePointer, pageStatus status, void* blockPointer, size_t classSize) {
	headers[pagePointer].status = status;
	headers[pagePointer].availableBlock = (uint8_t*)blockPointer;
	headers[pagePointer].classSize = classSize;
}

// sets status of a block with a particular addres
void setBlockHeader(void* blockPointer, bool status) {
	uint8_t* pointer = (uint8_t*)blockPointer;
	*pointer = status;
}


void* anyFreeBlock(void* pagePointer, size_t classSize) {
	// starts at the begining of the page
	// moves forvard on size of a block class
	// stops when end of the page is reached
	for (uint8_t* cursor = (uint8_t*)pagePointer; cursor != (uint8_t*)pagePointer + PAGE_SIZE; cursor += classSize)
	{
		// if the current block is free
		// returns pointer to this block
		if ((bool)*cursor == true)
		{
			return cursor + BLOCK_HEADER_SIZE;
		}
	}
	return nullptr;
}

void main() 
{
	
}