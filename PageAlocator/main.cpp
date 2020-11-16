#include <iostream>
#include "NumberDefinitions.h"
#include <vector>
#include <map>
using namespace std;

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


void main() 
{
	
}