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

TEST(LinkTest, CheckNextFreeSlicesAfterAllocation)
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