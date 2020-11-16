#pragma once
class NumberDefinitions
{
public:
	static const int PageSize= 4096; // four kilobytes
	static const int WholeMemorySize = 32768; // thirty two kilobytes
	static const int MinBlockClassSize = 16;
	static char WholeMemory[WholeMemorySize];
};

