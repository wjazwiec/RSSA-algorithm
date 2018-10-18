#pragma once
#include <array>
#include "OperationStatus.h"

/*

Maximal Free-Occupied Block (MFOB)

	-2	-1	3	2	1	-4	-3	-2	-1	1
	-5	-4	-3	-2	-1	5	4	3	2	1

*/
using Index = unsigned int;

struct Slice
{
	Slice() : value(0), remainingTime(0) {}

	short value;
	unsigned short remainingTime;
};

struct SlicePosition 
{
	Index core;
	Index slice;
};

class Link
{
public:
	Link();
	~Link();

	std::tuple<Status, SlicePosition> getFirstFreeSlices(unsigned short requiredSlices);
	bool canAllocate(SlicePosition position, unsigned short requiredSlices);
	Status allocate(SlicePosition position, unsigned short requiredSlices, unsigned short time);
	void decrementTime();

	static const size_t numOfSlices = 360;
	static const size_t numOfCores = 30;

private:
	void changeForwardSlices(SlicePosition startPosition, unsigned short requiredSlices, unsigned short time);
	void changeBackwardSlices(SlicePosition startPosition);
	void initialize();

	std::array<std::array<Slice, numOfSlices>, numOfCores> slices;
};

