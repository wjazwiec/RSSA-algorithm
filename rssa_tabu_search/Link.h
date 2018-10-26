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

	std::tuple<Status, SlicePosition> getFirstFreeSlices(unsigned short requiredSlices, SlicePosition startFrom = SlicePosition{});
	bool canAllocate(SlicePosition position, unsigned short requiredSlices) const;
	Status allocate(SlicePosition position, unsigned short requiredSlices, unsigned short time);
	void decrementTime();

	unsigned getCurrentCapacity() const;

	static const size_t numOfSlices = 360;
	static const size_t numOfCores = 7;

	using Core = std::array<Slice, numOfSlices>;
	using Fibre = std::array<Core, numOfCores>;

private:
	void changeForwardSlices(SlicePosition startPosition, unsigned short requiredSlices, unsigned short time);
	void changeBackwardSlices(SlicePosition startPosition);
	std::tuple<Status, Index> getIndexOfNextPositiveSlice(const Core& core, Index index) const;
	void initialize();

	void handleDecrementTimeToZero(Core& core, Index index);

	Fibre slices;
};

