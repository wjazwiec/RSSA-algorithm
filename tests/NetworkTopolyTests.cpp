#include "gtest/gtest.h"
#include "../rssa_tabu_search/NetworkTopology.h"
#include "../rssa_tabu_search/DataLoaderFromFile.h"

class NetworkTopologyTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		m_networkTopology.addLink(LinkDescription(0, 1));
		m_networkTopology.addLink(LinkDescription(0, 2));
		m_networkTopology.addLink(LinkDescription(1, 2));
		m_networkTopology.addLink(LinkDescription(2, 1));
		m_networkTopology.addLink(LinkDescription(2, 4));
		m_networkTopology.addLink(LinkDescription(2, 3));
		m_networkTopology.addLink(LinkDescription(4, 3));
		m_networkTopology.addLink(LinkDescription(3, 1));
	}

	NetworkTopology m_networkTopology;
	Status m_status;
};

TEST_F(NetworkTopologyTest, FitsInFirstPosition) 
{
	Route route;
	route.links = { {1,2}, {2,4}, {4,3} };
	route.requiredSlices.fill(5);

	SlicePosition slicePosition { 0, 0 };

	std::tie(m_status, std::ignore) = m_networkTopology.checkIfPositionFitsInEveryLink(route, slicePosition, 100);
	
	EXPECT_EQ(m_status, Status::Ok);

	Route shortRoute;
	shortRoute.links = { {1,2} };
	shortRoute.requiredSlices.fill(5);

	m_networkTopology.allocateWithBitrate(shortRoute, slicePosition, 10, 10);

	std::tie(m_status, std::ignore) = m_networkTopology.checkIfPositionFitsInEveryLink(route, slicePosition, 100);

	EXPECT_EQ(m_status, Status::NotOk);
}

TEST_F(NetworkTopologyTest, NotFitsInFirstShoot)
{
	Route route;
	route.links = { {1,2}, {2,4}, {4,3} };
	route.requiredSlices.fill(5);

	SlicePosition slicePosition{ 0, 0 };

	Route shortRoute;
	shortRoute.links = { {1,2} };
	shortRoute.requiredSlices.fill(5);

	m_networkTopology.allocate(shortRoute, slicePosition, Link::numOfSlices - 1, 10);

	Links::iterator notFittingLink;
	std::tie(m_status, notFittingLink) = m_networkTopology.checkIfPositionFitsInEveryLink(route, slicePosition, 2);

	EXPECT_EQ(m_status, Status::NotOk);
	EXPECT_EQ(notFittingLink->first.source, 1);
	EXPECT_EQ(notFittingLink->first.destination, 2);
}

TEST_F(NetworkTopologyTest, FirstFreeChannelBasic)
{
	Status status;
	SlicePosition slicePosition;

	Route route({ { {1,2}, {2,4}, {4,3} }, {} });
	Route route_2({ {{2,4}, {4,3} }, {} });

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(100, route);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 0);
	EXPECT_EQ(slicePosition.slice, 0);
}

TEST_F(NetworkTopologyTest, FirstFreeChannelInTheMiddle)
{
	Status status;
	SlicePosition slicePosition;

	Route route({ { {1,2}, {2,4}, {4,3} }, {} });

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(100, route);
	m_networkTopology.allocate(route, slicePosition, 100, 100);

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(100, route);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 0);
	EXPECT_EQ(slicePosition.slice, 100);
}

TEST_F(NetworkTopologyTest, FirstFreeChannelInTheEnd)
{
	Status status;
	SlicePosition slicePosition;

	Route route({ { {1,2}, {2,4}, {4,3} }, {} });

	unsigned slicesToAlocate = Link::numOfSlices - 10;

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(slicesToAlocate, route);
	m_networkTopology.allocate(route, slicePosition, slicesToAlocate, 100);

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(10, route);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 0);
	EXPECT_EQ(slicePosition.slice, 350);
}

TEST_F(NetworkTopologyTest, FirstFreeChanneOnTheSecondCore)
{
	Status status;
	SlicePosition slicePosition;

	Route route({ { {1,2}, {2,4}, {4,3} }, {} });

	unsigned slicesToAlocate = Link::numOfSlices - 10;

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(slicesToAlocate, route);
	m_networkTopology.allocate(route, slicePosition, slicesToAlocate, 100);

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(11, route);

	EXPECT_EQ(status, Status::Ok);
	EXPECT_EQ(slicePosition.core, 1);
	EXPECT_EQ(slicePosition.slice, 0);
}

TEST_F(NetworkTopologyTest, NoFreeChannel)
{
	Status status;
	SlicePosition slicePosition;

	Route route({ { {1,2}, {2,4}, {4,3} }, {} });

	unsigned slicesToAlocate = Link::numOfSlices - 10;

	for (Index i = 0; i < Link::numOfCores; i++)
	{
		std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(slicesToAlocate, route);
		m_networkTopology.allocate(route, slicePosition, slicesToAlocate, 100);
	}

	std::tie(status, slicePosition) = m_networkTopology.getFirstFreeChannel(11, route);

	EXPECT_EQ(status, Status::NotOk);
}

TEST_F(NetworkTopologyTest, CapacityMeasurements)
{
	Route route{ { {0,1} }, {} };

	unsigned requiredSlices = 252; // 10%

	SlicePosition slicePosition;

	std::tie(m_status, slicePosition) = m_networkTopology.getFirstFreeChannel(requiredSlices, route);
	m_networkTopology.allocate(route, slicePosition, requiredSlices, 10);

	EXPECT_EQ(m_networkTopology.getRouteCurrentCapacity(route), 10);
}

TEST_F(NetworkTopologyTest, BestRoutes)
{
	Status status;
	SlicePosition slicePosition;

	RouteDescription routeDescription{ 0, 1 };

	Route route_1;
	route_1.links = { {0,2}, {2,1} };
	route_1.requiredSlices.fill(300);

	Route route_2;
	route_2.links = { {0, 2}, {2, 3}, {3, 1} };
	route_2.requiredSlices.fill(300);

	m_networkTopology.addRoute(routeDescription, route_1);
	m_networkTopology.addRoute(routeDescription, route_2);

	SamePlaceRoutes routes;

	std::tie(m_status, routes) = m_networkTopology.getBestRoutes(routeDescription, 100);

	EXPECT_EQ(m_status, Status::Ok);
	EXPECT_EQ(routes.size(), 2);

	for (Index i = 0; i < Link::numOfCores; i++)
	{
		std::tie(m_status, slicePosition) = m_networkTopology.getFirstFreeChannel(300, routes[0]);
		m_networkTopology.allocateWithBitrate(routes[0], slicePosition, 100, 10);
	}

	std::tie(m_status, std::ignore) = m_networkTopology.getBestRoutes(routeDescription, 100);

	EXPECT_EQ(m_status, Status::NotOk);
}
