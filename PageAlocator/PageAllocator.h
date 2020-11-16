#include "NumberDefinitions.h"
#pragma once
using namespace std;

class PageAllocator
{
public:
	PageAllocator();

private:
	void* startfTheMemory;
	void makePagesInEmptyMemory();
	
};

