#include "gtest/gtest.h"
#include "../rssa_tabu_search/NetworkTopology.h"

TEST(NetworkTopology, SimpleAdd) {

	NetworkTopology networkTopology;

	networkTopology.addLink(LinkDescription(1, 2));
	networkTopology.addLink(LinkDescription(2, 4));
	networkTopology.addLink(LinkDescription(4, 3));

	Status status;
	Links::iterator iterator;

	std::tie(status, iterator) = networkTopology.checkIfPositionFitsInEveryLink(Route({ {1,2}, {2,4}, {4,3} }), SlicePosition{ 0,0 }, 100);

	EXPECT_EQ(status, Status::Ok);
}

TEST(NetworkTopology, FirstFreeChannelBasic) {

	NetworkTopology networkTopology;

	networkTopology.addLink(LinkDescription(1, 2));
	networkTopology.addLink(LinkDescription(2, 4));
	networkTopology.addLink(LinkDescription(4, 3));

	Status status;
	SlicePosition slicePosition;

	Route route({ {1,2}, {2,4}, {4,3} });
	Route route_2({{2,4}, {4,3} });

	std::tie(status, slicePosition) = networkTopology.getFirstFreeChannel(100, route);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 0);
	EXPECT_EQ(slicePosition.slice, 0);

	networkTopology.allocate(route, slicePosition, 100, 10);

	std::tie(status, slicePosition) = networkTopology.getFirstFreeChannel(100, route_2);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 0);
	EXPECT_EQ(slicePosition.slice, 100);

	networkTopology.allocate(route_2, slicePosition, 100, 10);

	std::tie(status, slicePosition) = networkTopology.getFirstFreeChannel(30, route);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 0);
	EXPECT_EQ(slicePosition.slice, 200);

	networkTopology.allocate(route, slicePosition, 30, 10);
}