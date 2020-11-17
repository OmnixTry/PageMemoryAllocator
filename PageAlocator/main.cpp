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


size_t powerOfTwoAligment(size_t number) {
	size_t minValue = 1;

	// stops on the first pwer of two number after the required number
	while (minValue <= number)
	{
		minValue *= 2;
	}

	return minValue;
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

bool isEveryBlockFree(void* pagePointer, size_t classSize) {
	for (uint8_t* cursor = (uint8_t*)pagePointer; cursor != (uint8_t*)pagePointer + PAGE_SIZE; cursor += classSize)
	{
		if ((bool)*cursor == false)
		{
			return false;
		}
	}
	return true;
}

bool setBlocksSize(size_t classSize) {
	// can't do if there are no empty pages
	if (freePages.empty()) {
		return false;
	}

	// take first free page
	uint8_t* freePage = (uint8_t*)freePages[0];

	// untill cursor reaches end of the page
	// move forward on the size of the block
	// set first byte to true to mrk block as free
	for (uint8_t* cursor = freePage; cursor != freePage + PAGE_SIZE; cursor += classSize) {
		*cursor = true;
	}

	// remove page from free pages ('cause it's divided into blocks allredy)
	// make it's header
	// add it to pages that are divided into classes
	freePages.erase(freePages.begin());
	setPageHeader(freePage, pageStatus::Divided, freePage + BLOCK_HEADER_SIZE, classSize);
	classifiedPages[classSize].push_back(freePage);

	return true;
}

void* mem_alloc(size_t size) {
	// if operation fails return nullpointer
	void* pointer = nullptr;

	// if we need space less than half of the page wee need a block of power of 2
	if (size <= PAGE_SIZE / 2) {
		// find apropreate block size
		size_t classSize = powerOfTwoAligment(size + BLOCK_HEADER_SIZE);

		// if there's no page for this class
		// and if we can't make new page for this class 
		// operation fails
		if (classifiedPages[classSize].empty() && !setBlocksSize(classSize)) {
			return nullptr;
		}

		// take classified page
		// set header of the first available block to occupied
		// save this block to pointer
		uint8_t* page = (uint8_t*)classifiedPages[classSize].at(0); // take first popavshuyusia
		setBlockHeader(headers[page].availableBlock - BLOCK_HEADER_SIZE, false);
		pointer = headers[page].availableBlock;

		// check if there are any free blocks in the page
		uint8_t* freeBlock = (uint8_t*)anyFreeBlock(page, classSize);
		// if there are save it as the first free bolck
		if (freeBlock != nullptr) {
			headers[page].availableBlock = freeBlock;
		}

		// if there's no more free blocks in page
		// delete page from classified
		// 'cause there must be only non full pages (that have some empty blocks)
		else
		{
			classifiedPages[classSize].erase(std::find(classifiedPages[classSize].begin(),
				classifiedPages[classSize].end(), page));
		}
	}
	// else we have multipage block
	else
	{
		// find number of needed pages
		int pagesNeeded = ceil((double)size / PAGE_SIZE);

		// if not enough free pages exist 
		// then fail
		if (freePages.size() < pagesNeeded)
		{
			return nullptr;
		}

		pointer = freePages[0];

		// going through free pages for as many times as many pages wee require
		for (int i = 1; i <= pagesNeeded; i++)
		{
			// take first page and. if we'll need one more take second one
			uint8_t* page = (uint8_t*)freePages[0];
			uint8_t* nextPage = pagesNeeded == i ? nullptr : (uint8_t*)freePages[1];

			// set a header for first page, make it occupied, set addres  of the next block, 
			// set how much more space this multipage block takes
			setPageHeader(page, pageStatus::MultipageBlock, i == pagesNeeded ? nullptr : nextPage + BLOCK_HEADER_SIZE, pagesNeeded * PAGE_SIZE);

			freePages.erase(freePages.begin());
		}
	}
	return pointer;
}

void mem_free(void* addr) {
	if (addr == nullptr)
	{
		return;
	}

	// if the pointer is outside our memory - than can't perform operation
	if ((uint8_t*)addr < (uint8_t*)startPointer || (uint8_t*)addr >(uint8_t*)startPointer + MEMORY_SIZE) {
		return;
	}

	// get the beginig of the page and find how many pages belong to the block
	size_t pageIndex = ((uint8_t*)addr - (uint8_t*)startPointer) / PAGE_SIZE;
	uint8_t* page = (uint8_t*)startPointer + pageIndex * PAGE_SIZE;

	if (headers[page].status == pageStatus::Divided)
	{
		// mark the requested block as free 
		uint8_t* blockToFree = (uint8_t*)addr - BLOCK_HEADER_SIZE;
		*blockToFree = true;

		if (isEveryBlockFree(page, headers[page].classSize))
		{
			// if all of the blocks are free 
			// no need to store page in classified pages
			// it's free now Hurray!
			classifiedPages[headers[page].classSize].erase(std::find(classifiedPages[headers[page].classSize].begin(),
				classifiedPages[headers[page].classSize].end(), page));
			// mark header as free
			setPageHeader(page, pageStatus::Free, nullptr, 0);
			freePages.push_back(page);
		}
		// when the page is absolutely full it's deleted from classified pages
		// when we free a block we need to add it to classified pages again
		// if this page is not in the clasified pages add it to the classified pages
		if (std::find(classifiedPages[headers[page].classSize].begin(),
			classifiedPages[headers[page].classSize].end(), page) == classifiedPages[headers[page].classSize].end())
		{
			classifiedPages[headers[page].classSize].push_back(page);
		}
	}

	// if it's block of many pages than free this one and all the following 
	if (headers[page].status == pageStatus::MultipageBlock)
	{
		double amountOfPages = ceil(headers[page].classSize / PAGE_SIZE);

		for (int i = 0; i < amountOfPages; i++)
		{
			uint8_t* nextPage = headers[page].availableBlock - BLOCK_HEADER_SIZE;
			setPageHeader(page, pageStatus::Free, nullptr, 0);
			freePages.push_back(page);
			page = nextPage;
		}
	}
}

void* mem_realloc(void* addr, size_t size) {
	// can't realloc nothind
	// or something outside our memory
	if (addr == nullptr)
	{
		return mem_alloc(size);
	}

	if ((uint8_t*)addr < (uint8_t*)startPointer || (uint8_t*)addr >(uint8_t*)startPointer + MEMORY_SIZE) {
		return nullptr;
	}

	// get the beginig of the page and find how many pages belong to the block
	void* pointer = addr;
	size_t pageIndex = ((uint8_t*)addr - (uint8_t*)startPointer) / PAGE_SIZE;
	uint8_t* page = (uint8_t*)startPointer + pageIndex * PAGE_SIZE;

	if (headers[page].status == pageStatus::Divided)
	{
		size_t classSize = powerOfTwoAligment(size);
		// if the class is the same don't doooo a thing 
		if (headers[page].classSize == classSize)
		{
			return addr;
		}

		// allocate new memory		
		pointer = mem_alloc(size);
		// free old memory
		mem_free(addr);
		return pointer;
	}
	if (headers[page].status == pageStatus::MultipageBlock)
	{
		// get info about old version
		size_t sizeOld = headers[page].classSize;
		double amountOfPagesOld = ceil(headers[page].classSize / PAGE_SIZE);
		double pagesNeeded = powerOfTwoAligment(size) / PAGE_SIZE;

		// if the same number of pages required
		// do NOTHING
		if (amountOfPagesOld == pagesNeeded)
		{
			return addr;
		}

		// if we now need a smal block
		// allocate new memory
		// free old memory
		if (size <= PAGE_SIZE / 2)
		{
			mem_free(addr);
			pointer = mem_alloc(size);
			return pointer;
		}

		// if more pages
		if (amountOfPagesOld < pagesNeeded)
		{
			// if not enough free pages - fail
			if (pagesNeeded - amountOfPagesOld > freePages.size())
			{
				return nullptr;
			}


			uint8_t* nextPage;
			for (int i = 1; i <= pagesNeeded; i++)
			{
				// available block contains addres of the next page in squence
				// contains NOTHINGNESS if we're outside the preexisting pages
				nextPage = headers[page].availableBlock - BLOCK_HEADER_SIZE;

				// if we are outside pages that were already allocated before, 
				// we allocate a new page, put it after the previous one
				// and remove it from free
				if (i >= amountOfPagesOld)
				{
					page = (uint8_t*)freePages[0];
					nextPage = pagesNeeded == i ? nullptr : (uint8_t*)freePages[1];
					setPageHeader(page, pageStatus::MultipageBlock, i == pagesNeeded ? nullptr : nextPage + BLOCK_HEADER_SIZE, pagesNeeded * PAGE_SIZE);
					freePages.erase(freePages.begin());
				}
				page = nextPage;
			}
			return addr;
		}
		// if wee need less pages than before
		if (amountOfPagesOld > pagesNeeded)
		{

			uint8_t* nextPage;
			for (int i = 0; i < amountOfPagesOld; i++)
			{
				nextPage = headers[page].availableBlock - BLOCK_HEADER_SIZE;
				// if it's outside the required number of pages
				// mark this page as free
				if (i >= pagesNeeded)
				{
					setPageHeader(page, pageStatus::Free, nullptr, 0);

					freePages.push_back(page);
				}
				else
				{
					setPageHeader(page, pageStatus::MultipageBlock, i == pagesNeeded ? nullptr : nextPage + BLOCK_HEADER_SIZE, PAGE_SIZE * pagesNeeded);
				}
				page = nextPage;
			}
			return addr;
		}

	}
}

void mem_dump() {
	std::cout << "-----------------------------------" << std::endl;
	uint8_t* page = (uint8_t*)startPointer;
	for (int i = 0; i < PAGE_AMOUNT; i++) {
		memoryPageHeader header = headers[page];

		std::string state;

		switch (header.status)
		{
		case pageStatus::Free:
			state = "Free";
			break;
		case pageStatus::Divided:
			state = "Divided";
			break;
		case pageStatus::MultipageBlock:
			state = "MultiPageBlock";
			break;
		}

		std::cout << "PAGE " << i << std::endl;
		std::cout << "Address: " << (uint16_t*)page << std::endl;
		std::cout << "Status: " << state << std::endl;
		std::cout << "Page size: " << PAGE_SIZE << std::endl;

		if (header.status == pageStatus::Divided) {
			std::cout << "Class size: " << header.classSize << std::endl;

			for (int j = 0; j < PAGE_SIZE / header.classSize; j++) {
				uint8_t* blockAddress = page + header.classSize * j + BLOCK_HEADER_SIZE;
				uint8_t* isOccupied = blockAddress - BLOCK_HEADER_SIZE;
				std::cout << "BLOCK " << j << std::endl;
				std::cout << "Address " << (uint16_t*)blockAddress << std::endl;
				std::cout << "Free " << (bool)*isOccupied << std::endl;
			}
		}
		if (header.status == pageStatus::MultipageBlock)
		{
			std::cout << "Block size: " << header.classSize << std::endl;
			std::cout << "Next block: " << (uint16_t*)header.availableBlock << std::endl;
		}
		page += PAGE_SIZE;
		std::cout << "-----------------------------------" << std::endl;
	}

}
int main() {
	initializePages();
	void* x1 = mem_alloc(5000);
	void* x2 = mem_alloc(400);
	void* x3 = mem_alloc(400);
	std::cout << (uint16_t*)x1 << std::endl;
	mem_dump();
	mem_free(x1);
	mem_free(x2);
	mem_dump();
	void* x5 = mem_realloc(x3, 50);
	mem_dump();
	return 0;
}