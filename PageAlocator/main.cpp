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


void main() 
{
	
}