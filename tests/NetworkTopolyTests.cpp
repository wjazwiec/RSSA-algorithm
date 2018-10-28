#include "gtest/gtest.h"
#include "../rssa_tabu_search/NetworkTopology.h"

class NetworkTopologyTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		m_networkTopology.addLink(LinkDescription(0, 1));
		m_networkTopology.addLink(LinkDescription(1, 2));
		m_networkTopology.addLink(LinkDescription(2, 4));
		m_networkTopology.addLink(LinkDescription(4, 3));
	}

	NetworkTopology m_networkTopology;
	Status m_status;
};

TEST_F(NetworkTopologyTest, SimpleFitsInFirstPosition) 
{
	Links::iterator iterator;

	std::tie(m_status, iterator) = m_networkTopology.checkIfPositionFitsInEveryLink(Route({ {1,2}, {2,4}, {4,3} }), SlicePosition{ 0,0 }, 100);
	
	EXPECT_EQ(m_status, Status::Ok);
}
TEST_F(NetworkTopologyTest, FirstFreeChannelBasic)
{
	Status status;
	SlicePosition slicePosition;

	Route route({ {1,2}, {2,4}, {4,3} });
	Route route_2({{2,4}, {4,3} });

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(100, route);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 0);
	EXPECT_EQ(slicePosition.slice, 0);

	m_networkTopology.allocate(route, slicePosition, 100, 10);

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(100, route_2);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 0);
	EXPECT_EQ(slicePosition.slice, 100);

	m_networkTopology.allocate(route_2, slicePosition, 100, 10);

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(30, route);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 0);
	EXPECT_EQ(slicePosition.slice, 200);

	m_networkTopology.allocate(route, slicePosition, 30, 10);
}

TEST_F(NetworkTopologyTest, CapacityMeasurements)
{
	Route route{ {0,1} };

	unsigned requiredSlices = 252; // 10%

	SlicePosition slicePosition;

	std::tie(m_status, slicePosition) = m_networkTopology.getFirstFreeChannel(requiredSlices, route);
	m_networkTopology.allocate(route, slicePosition, requiredSlices, 10);

	EXPECT_EQ(m_networkTopology.getRouteCurrentCapacity(route), 10);
}

TEST_F(NetworkTopologyTest, BestRoutes)
{
	RouteDescription routeDescription{ 0, 1 };

	m_networkTopology.addRoute(routeDescription, Route{ {0, 1} });
	m_networkTopology.addRoute(routeDescription, Route{ {0, 2}, {2, 3}, {3, 1} });

	SamePlaceRoutes routes;

	std::tie(m_status, routes) = m_networkTopology.getBestRoutes(routeDescription);

	EXPECT_EQ(m_status, Status::Ok);
	EXPECT_EQ(routes.size(), 2);
}

