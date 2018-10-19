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

TEST(LinkTest, MaximumSlice)
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

TEST(LinkTest, IncorrectValues)
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

	for (Index i = 0; i < Link::numOfCores; i++)
	{
		std::tie(status, position) = link.getFirstFreeSlices(Link::numOfSlices);

		EXPECT_EQ(status, Status::Ok);
		EXPECT_EQ(position.core, i);
		EXPECT_EQ(position.slice, 0);

		EXPECT_EQ(link.allocate(position, Link::numOfSlices, 10), Status::Ok);
	}

	std::tie(status, position) = link.getFirstFreeSlices(Link::numOfSlices);

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
	EXPECT_TRUE(link.canAllocate(position_10, 10)); //TODO COVER THIS
}