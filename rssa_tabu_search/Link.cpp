#include "Link.h"


Link::Link()
{
	initialize();
}


Link::~Link()
{
}

std::tuple<Status, SlicePosition> Link::getFirstFreeSlices(unsigned short requiredSlices)
{
	for (Index core = 0; core < slices.size(); core++)
	{
		Index sliceIndex = 0;

		while (sliceIndex < slices[core].size())
		{
			const auto sliceValue = slices[core][sliceIndex].value;

			if (sliceValue >= requiredSlices)
				return std::make_tuple(Status::Ok, SlicePosition{ core, sliceIndex });
			else
				sliceIndex += abs(sliceValue);
		}
	}

	return std::make_tuple(Status::NotOk, SlicePosition{});
}

bool Link::canAllocate(SlicePosition position, unsigned short requiredSlices)
{
	return (slices[position.core][position.slice].value >= requiredSlices);
}

Status Link::allocate(SlicePosition position, unsigned short requiredSlices, unsigned short time)
{
	if (canAllocate(position, requiredSlices) == false)
		return Status::NotOk;
	
	changeForwardSlices(position, requiredSlices, time);
	changeBackwardSlices(position);

	return Status::Ok;
}

void Link::decrementTime()
{
	for (auto& core : slices)
	{
		std::for_each(core.begin(), core.end(), [](Slice& slice) 
		{ 
			if (slice.remainingTime > 0)
				slice.remainingTime--;
		});
	}
}

void Link::changeForwardSlices(SlicePosition startPosition, unsigned short requiredSlices, unsigned short time)
{
	while (requiredSlices > 0 && startPosition.slice < numOfSlices)
	{
		auto& slice = slices[startPosition.core][startPosition.slice++];
		slice.value = (requiredSlices * -1);
		slice.remainingTime = time;

		requiredSlices--;
	}
}

void Link::changeBackwardSlices(SlicePosition startPosition)
{
	short backwardCounter = 1;

	while (startPosition.slice != 0)
	{
		auto& slice = slices[startPosition.core][startPosition.slice--];

		if (slice.value < 0)
		{
			break;
		}

		slice.value = backwardCounter++;
		slice.remainingTime = 0;
	}
}

void Link::initialize()
{
	for (auto& core : slices)
	{
		short value = numOfSlices;

		for(Index i = 0; i < core.size(); i++)
		{
			core[i].value = value--;
		}
	}
}
