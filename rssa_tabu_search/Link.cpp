#include "Link.h"


Link::Link()
{
	initialize();
}


Link::~Link()
{
}

std::tuple<Status, SlicePosition> Link::getFirstFreeSlices(unsigned short requiredSlices, SlicePosition startFrom)
{
	if (requiredSlices > numOfSlices)
		return { Status::NotOk, {} };

	Index sliceIndex = startFrom.slice;

	bool firstIteration = true;

	for (Index core = startFrom.core; core < slices.size(); core++)
	{
		if (firstIteration)
		{
			firstIteration = false;
		}
		else
		{
			sliceIndex = 0;
		}

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
	
	if (position.slice == 0)
	{
		allocationOnTheFirstPosition(position, requiredSlices, time);
	}
	else if (position.slice + requiredSlices < Link::numOfSlices)
	{
		allocationInTheMiddle(position, requiredSlices, time);
	}
	else if (position.slice + requiredSlices == Link::numOfSlices)
	{
		allocationToTheEnd(position, requiredSlices, time);
	}
	else
	{
		return Status::NotOk;
	}

	return Status::Ok;
}

std::tuple<Status, Index> Link::getIndexOfNextPositiveSlice(const Core& core, Index index) const
{
	if (index + 1 > numOfSlices)
		return { Status::NotOk, Index(0) };

	if (core[index + 1].value > 0)
		return { Status::Ok, Index(index + 1) };

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
					short distance = 0, startFrom = i;
					bool continueWhile = true;

					while (continueWhile && i < core.size())
					{
						distance++;

						if (core[++i].remainingTime > 0)
						{
							core[i].remainingTime--;
						}
						else
						{
							continueWhile = false;
							continue;
						}
						
						if (core[i].remainingTime != 0)
						{
							continueWhile = false;
						}
					}

					handleDecrementTimeToZero(core, startFrom, distance);
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

void Link::handleDecrementTimeToZero(Core & core, Index index, const short distance)
{
	if (index == 0)
	{
		short naxtValueAfterFinishedPeriod = core[index + distance].value;

		short backwardIterator = index + distance - 1;
		short valueToIncrement = (naxtValueAfterFinishedPeriod > 0) ? naxtValueAfterFinishedPeriod : 0;

		while (backwardIterator >= 0)
		{
			core[backwardIterator].value = ++valueToIncrement;
			backwardIterator--;
		}
	}
	else if ((index + distance) == Link::numOfSlices)
	{
		short backwardIterator = Link::numOfSlices - 1;
		short valueToIncrement = 0;

		while (backwardIterator >= 0)
		{
			if (backwardIterator >= static_cast<short>(index))
			{
				core[backwardIterator].value = ++valueToIncrement;
			}
			else
			{
				if (core[backwardIterator].value < 0)
				{
					break;
				}
				else
				{
					core[backwardIterator].value += valueToIncrement;
				}
			}

			backwardIterator--;
		}
	}
	else 
	{
		Index sliceBeforeIndex = index - 1;
		Slice& sliceBefore = core[sliceBeforeIndex];

		Index sliceAfterIndex = index + distance;
		Slice& sliceAfter = core[sliceAfterIndex];

		short backwardIndex = sliceAfterIndex - 1;
		short valueToIncrement = (sliceAfter.value > 0) ? sliceAfter.value : 0;
		short valueToDecrement = 0;

		bool actionTaken = false;

		while (backwardIndex >= static_cast<short>(index))
		{
			core[backwardIndex--].value = ++valueToIncrement;
		}

		while (backwardIndex >= 0 && core[backwardIndex].value < 0)
		{
			actionTaken = true;
			core[backwardIndex--].value = --valueToDecrement;
		}

		while (backwardIndex >= 0 && core[backwardIndex].value > 0 && actionTaken == false)
		{
			core[backwardIndex--].value = ++valueToIncrement;
		}
	}
}

void Link::allocationOnTheFirstPosition(SlicePosition startPosition, unsigned short requiredSlices, unsigned short time)
{
	/*
	Three possibilities, next slice after just allocated slices is:
		- positive
		- negative
		- allocation takes whole core
	*/

	Index indexAfterAllocatedSlices = startPosition.slice + requiredSlices;

	if (indexAfterAllocatedSlices == Link::numOfSlices 
		|| slices[startPosition.core][indexAfterAllocatedSlices].value > 0)
	{

	/*
	Firstly, we have:
		
		 |
		320 319 318 317 316

	Want to have (fe. requiredSlices = 3):

		-3 -2 -1 317 316 315 

	Occures also when we allocate whole core.
	*/

		while (requiredSlices > 0)
		{
			slices[startPosition.core][startPosition.slice].value = (requiredSlices * -1);
			slices[startPosition.core][startPosition.slice].remainingTime = time;

			startPosition.slice++;
			requiredSlices--;
		}

		return;
	}

	if (slices[startPosition.core][indexAfterAllocatedSlices].value < 0)
	{
		/*
		Firstly, we have:

			|
			3 2 1 -3 -2

		Want to have (fe. requiredSlices = 3):

			-6 -5 -4 -3 -2

		*/

		short valueToDecrement = slices[startPosition.core][indexAfterAllocatedSlices].value;
		short backwardIndex = indexAfterAllocatedSlices - 1;

		const short startSliceToCompare = startPosition.slice;

		while (backwardIndex >= startSliceToCompare)
		{
			slices[startPosition.core][backwardIndex].value = --valueToDecrement;
			slices[startPosition.core][backwardIndex].remainingTime = time;

			backwardIndex--;
		}
	}
}

void Link::allocationInTheMiddle(SlicePosition startPosition, unsigned short requiredSlices, unsigned short time)
{
	/*
	Four possibilities, slice before and next slice after just allocated slices:
		- negative : negative
		- negative : positive
		- positive : negative
		- positive : positive
	*/

	Index sliceBeforeIndex = startPosition.slice - 1;
	Slice& sliceBefore = slices[startPosition.core][sliceBeforeIndex];

	Index sliceAfterIndex = startPosition.slice + requiredSlices;
	Slice& sliceAfter = slices[startPosition.core][sliceAfterIndex];

	if (sliceBefore.value < 0 && sliceAfter.value < 0)
	{
		/*
		Firstly, we have:

			         |
			-3 -2 -1 3 2 1 -3 -2 -1

		Want to have (fe. requiredSlices = 3):

			-9 -8 -7 -6 -5 -4 -3 -2 -1
		*/

		short backwardIndex = sliceAfterIndex - 1;
		short valueToDecrement = sliceAfter.value;

		while (backwardIndex >= 0)
		{
			if (requiredSlices > 0)
			{
				slices[startPosition.core][backwardIndex].remainingTime = time;
				slices[startPosition.core][backwardIndex].value = --valueToDecrement;
				requiredSlices--;
			}
			else
			{
				if (slices[startPosition.core][backwardIndex].value < 0)
				{
					slices[startPosition.core][backwardIndex].value = --valueToDecrement;
				}
				else
				{
					break;
				}
			}

			backwardIndex--;
		}
	}
	else if (sliceBefore.value < 0 && sliceAfter.value > 0)
	{
		/*
		Firstly, we have:

					 |
			-3 -2 -1 6 5 4 3 2 1

		Want to have (fe. requiredSlices = 3):

			-6 -5 -4 -3 -2 -1 3 2 1
		*/

		short backwardIndex = sliceAfterIndex - 1;
		short valueToDecrement = 0;

		while (backwardIndex >= 0)
		{
			if (requiredSlices > 0)
			{
				slices[startPosition.core][backwardIndex].remainingTime = time;
				slices[startPosition.core][backwardIndex].value = --valueToDecrement;
				requiredSlices--;
			}
			else
			{
				if (slices[startPosition.core][backwardIndex].value < 0)
				{
					slices[startPosition.core][backwardIndex].value = --valueToDecrement;
				}
				else
				{
					break;
				}
			}

			backwardIndex--;
		}

	}
	else if (sliceBefore.value > 0 && sliceAfter.value < 0)
	{
		/*
		Firstly, we have:

					 |
			6 5 4 3 2 1 -3 -2 -1

		Want to have (fe. requiredSlices = 3):

			3 2 1 -6 -5 -4 -3 -2 -1
		*/

		short backwardIndex = sliceAfterIndex - 1;
		short valueToDecrement = sliceAfter.value;
		unsigned short copyOfRequiredSlices = requiredSlices;

		while (backwardIndex >= 0)
		{
			if (requiredSlices > 0)
			{
				slices[startPosition.core][backwardIndex].remainingTime = time;
				slices[startPosition.core][backwardIndex].value = --valueToDecrement;
				requiredSlices--;
			}
			else
			{
				if (slices[startPosition.core][backwardIndex].value > 0)
				{
					slices[startPosition.core][backwardIndex].value -= copyOfRequiredSlices;
				}
				else
				{
					break;
				}
			}

			backwardIndex--;
		}
	}
	else if (sliceBefore.value > 0 && sliceAfter.value > 0)
	{
		/*
		Firstly, we have:

				  |
			9 8 7 6 5 4 3 2 1

		Want to have (fe. requiredSlices = 3):

			3 2 1 -3 -2 -1 3 2 1
		*/

		short backwardIndex = sliceAfterIndex - 1;
		short valueToDecrement = 0;
		short valueToIncrement = 0;

		while (backwardIndex >= 0)
		{
			if (requiredSlices > 0)
			{
				slices[startPosition.core][backwardIndex].remainingTime = time;
				slices[startPosition.core][backwardIndex].value = --valueToDecrement;
				requiredSlices--;
			}
			else
			{
				if (slices[startPosition.core][backwardIndex].value > 0)
				{
					slices[startPosition.core][backwardIndex].value = ++valueToIncrement;
				}
				else
				{
					break;
				}
			}

			backwardIndex--;
		}
    }
}

void Link::allocationToTheEnd(SlicePosition startPosition, unsigned short requiredSlices, unsigned short time)
{
	Index sliceBeforeIndex = startPosition.slice - 1;
	Slice& sliceBefore = slices[startPosition.core][sliceBeforeIndex];

	if (sliceBefore.value > 0)
	{
		short backwardIndex = Link::numOfSlices - 1;
		short valueToDecrement = 0;
		short valueToIncrement = 0;

		while (backwardIndex >= 0)
		{
			if (requiredSlices > 0)
			{
				slices[startPosition.core][backwardIndex].remainingTime = time;
				slices[startPosition.core][backwardIndex].value = --valueToDecrement;
				requiredSlices--;
			}
			else
			{
				if (slices[startPosition.core][backwardIndex].value > 0)
				{
					slices[startPosition.core][backwardIndex].value = ++valueToIncrement;
				}
				else
				{
					break;
				}
			}

			backwardIndex--;
		}
	}
	else if (sliceBefore.value < 0)
	{
		short backwardIndex = Link::numOfSlices - 1;
		short valueToDecrement = 0;

		while (backwardIndex >= 0)
		{
			if (requiredSlices > 0)
			{
				slices[startPosition.core][backwardIndex].remainingTime = time;
				slices[startPosition.core][backwardIndex].value = --valueToDecrement;
				requiredSlices--;
			}
			else
			{
				if (slices[startPosition.core][backwardIndex].value < 0)
				{
					slices[startPosition.core][backwardIndex].value = --valueToDecrement;
				}
				else
				{
					break;
				}
			}

			backwardIndex--;
		}
	}
}
