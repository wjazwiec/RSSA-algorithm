#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING

#include "gtest/gtest.h"
#include "../rssa_tabu_search/Link.h"

TEST(LinkTest, GetFreeSlicesAfterInitializaion)
{
	Link link;

	Status status;
	SlicePosition position;

	std::tie(status, position) = link.getFirstFreeSlices(10);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(position.core, 0);
	EXPECT_EQ(position.slice, 0);
}

TEST(LinkTest, MaximumSliceAllocation)
{
	Link link;

	Status status;
	SlicePosition position;

	std::tie(status, position) = link.getFirstFreeSlices(Link::numOfSlices);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(position.core, 0);
	EXPECT_EQ(position.slice, 0);

	EXPECT_EQ(link.allocate(position, Link::numOfSlices, 10), Status::Ok);
}

TEST(LinkTest, SliceAllocationAtFirstPosition)
{
	Link link;

	Status status;
	SlicePosition position;

	std::tie(status, position) = link.getFirstFreeSlices(3);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(position.core, 0);
	EXPECT_EQ(position.slice, 0);

	EXPECT_EQ(link.allocate(position, 3, 1), Status::Ok);
}

TEST(LinkTest, SliceAllocationAtFirstPositionWithNextNegative)
{
	Link link;

	Status status;
	SlicePosition position;

	SlicePosition position_after_first;
	position_after_first.core = 0;
	position_after_first.slice = 3;

	link.allocate(position_after_first, 3, 10);

	std::tie(status, position) = link.getFirstFreeSlices(3);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(position.core, 0);
	EXPECT_EQ(position.slice, 0);

	EXPECT_EQ(link.allocate(position, 3, 1), Status::Ok);

}

TEST(LinkTest, SliceAllocationInTheMiddle_11)
{
	Link link;

	Status status;
	SlicePosition position;

	position.core = 0;
	position.slice = 3;

	EXPECT_EQ(link.allocate(position, 3, 1), Status::Ok);

}

TEST(LinkTest, SliceAllocationInTheMiddle_00)
{
	Link link;

	SlicePosition position_0, position_1, position_2;

	position_0.core = 0;
	position_0.slice = 0;

	link.allocate(position_0, 3, 10);

	position_2.core = 0;
	position_2.slice = 6;

	link.allocate(position_2, 3, 10);

	position_1.core = 0;
	position_1.slice = 3;

	EXPECT_EQ(link.allocate(position_1, 3, 1), Status::Ok);


}

TEST(LinkTest, SliceAllocationInTheMiddle_01)
{
	Link link;

	SlicePosition position_0, position_1;

	position_0.core = 0;
	position_0.slice = 0;

	link.allocate(position_0, 3, 10);


	position_1.core = 0;
	position_1.slice = 3;

	EXPECT_EQ(link.allocate(position_1, 3, 1), Status::Ok);
}

TEST(LinkTest, SliceAllocationInTheMiddle_10)
{
	Link link;

	SlicePosition position_1, position_2;

	position_2.core = 0;
	position_2.slice = 6;

	link.allocate(position_2, 3, 10);

	position_1.core = 0;
	position_1.slice = 3;

	EXPECT_EQ(link.allocate(position_1, 3, 1), Status::Ok);
	link.decrementTime();
}

TEST(LinkTest, SliceAllocationAtTheEnd_0)
{
	Link link;

	SlicePosition position_1, position_2;

	position_2.core = 0;
	position_2.slice = Link::numOfSlices - 6;

	link.allocate(position_2, 3, 10);

	position_1.core = 0;
	position_1.slice = Link::numOfSlices - 3;

	EXPECT_EQ(link.allocate(position_1, 3, 10), Status::Ok);
}

TEST(LinkTest, SliceAllocationAtTheEnd_1)
{
	Link link;

	SlicePosition position_1;

	position_1.core = 0;
	position_1.slice = Link::numOfSlices - 3;

	EXPECT_EQ(link.allocate(position_1, 3, 10), Status::Ok);
}

TEST(LinkTest, IncorrectSize)
{
	Link link;

	Status status;
	SlicePosition position;

	std::tie(status, position) = link.getFirstFreeSlices(Link::numOfSlices + 1);

	EXPECT_EQ(status, Status::NotOk);

	EXPECT_FALSE(link.canAllocate({ Link::numOfCores, 1 }, 10));
	EXPECT_FALSE(link.canAllocate({ 1, Link::numOfSlices }, 10));
}

TEST(LinkTest, MaximumCoresTaken)
{
	Link link;

	Status status;
	SlicePosition position;

	unsigned slicesToAlocate = Link::numOfSlices - 10;

	for (Index i = 0; i < Link::numOfCores; i++)
	{
		std::tie(status, position) = link.getFirstFreeSlices(slicesToAlocate);

		EXPECT_EQ(status, Status::Ok);
		EXPECT_EQ(position.core, i);
		EXPECT_EQ(position.slice, 0);

		EXPECT_EQ(link.allocate(position, slicesToAlocate, 10), Status::Ok);
	}

	std::tie(status, position) = link.getFirstFreeSlices(11);

	EXPECT_EQ(status, Status::NotOk);
}

TEST(LinkTest, NextFreeSlicesAfterAllocation)
{
	Link link;

	Status status;
	SlicePosition position;

	std::tie(status, position) = link.getFirstFreeSlices(100);

	EXPECT_EQ(link.allocate(position, 100, 10), Status::Ok);

	std::tie(status, position) = link.getFirstFreeSlices(100);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(position.core, 0);
	EXPECT_EQ(position.slice, 100);

	EXPECT_EQ(link.allocate(position, 100, 10), Status::Ok);

	std::tie(status, position) = link.getFirstFreeSlices(Link::numOfSlices - 1);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(position.core, 1);
	EXPECT_EQ(position.slice, 0);
}

TEST(LinkTest, TimeDecrementing)
{
	Link link;

	Status status;
	SlicePosition position_0, position_10;

	std::tie(status, position_0) = link.getFirstFreeSlices(10);

	EXPECT_EQ(link.allocate(position_0, 10, 2), Status::Ok);

	std::tie(status, position_10) = link.getFirstFreeSlices(10);

	EXPECT_EQ(link.allocate(position_10, 10, 1), Status::Ok);

	EXPECT_FALSE(link.canAllocate(position_0, 10));
	EXPECT_FALSE(link.canAllocate(position_10, 10));

	link.decrementTime();

	EXPECT_FALSE(link.canAllocate(position_0, 10));
	EXPECT_TRUE(link.canAllocate(position_10, 10));
}

TEST(LinkTest, Capacity)
{
	Link link;

	Status status;
	SlicePosition position;

	std::tie(status, position) = link.getFirstFreeSlices(100);

	EXPECT_EQ(link.allocate(position, 100, 2), Status::Ok);

	EXPECT_EQ(link.getCurrentCapacity(), 100);

	std::tie(status, position) = link.getFirstFreeSlices(Link::numOfSlices);

	EXPECT_EQ(link.allocate(position, Link::numOfSlices, 2), Status::Ok);

	EXPECT_EQ(link.getCurrentCapacity(), 100 + Link::numOfSlices);
}
