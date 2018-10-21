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
	if (requiredSlices > numOfSlices)
		return { Status::NotOk, {} };

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

bool Link::canAllocate(SlicePosition position, unsigned short requiredSlices) const
{
	if (position.core >= numOfCores || position.slice >= numOfSlices)
		return false;

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

std::tuple<Status, Index> Link::getIndexOfNextPositiveSlice(const Core& core, Index index) const
{
	Index nextPositiveIndex = abs(core[index].value) + index;

	if (nextPositiveIndex < numOfSlices)
		return { Status::Ok, nextPositiveIndex };
	else
		return { Status::NotOk, Index(0) };
}

void Link::decrementTime()
{
	for (auto& core : slices)
	{
		for (Index i = 0; i < core.size(); i++)
		{
			if (core[i].remainingTime > 0)
			{
				core[i].remainingTime--;

				if (core[i].remainingTime == 0)
				{
					handleDecrementTimeToZero(core, i);
				}
			}
		}

	}
}

unsigned Link::getCurrentCapacity() const
{
	unsigned taken = 0;

	for (auto& core : slices)
	{
		for (Index i = 0; i < numOfSlices;)
		{
			if (core[i].value < 0)
			{
				taken += abs(core[i].value);
			}

			i += abs(core[i].value);
		}
	}

	return taken;
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

void Link::handleDecrementTimeToZero(Core & core, Index index)
{
	auto[status, nextPositiveIndex] = getIndexOfNextPositiveSlice(core, index);
	if (status == Status::Ok)
	{
		short nextPositiveValue = core[nextPositiveIndex].value + 1;

		nextPositiveIndex--; //To get one element before

		while (nextPositiveIndex >= 0
			&& nextPositiveIndex < numOfSlices
			&& core[nextPositiveIndex].value < 0
			&& nextPositiveIndex >= index)
		{
			core[nextPositiveIndex].value = nextPositiveValue++;
			core[nextPositiveIndex].remainingTime = 0;

			nextPositiveIndex--;
		}
	}
	else //status not ok - slice is last channel. It's enough to do abs
	{
		while (index < numOfSlices)
		{
			core[index].value = abs(core[index].value);
			core[index].remainingTime = 0;
		}
	}
}
