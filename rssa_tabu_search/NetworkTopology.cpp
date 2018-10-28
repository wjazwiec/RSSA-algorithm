#include "NetworkTopology.h"


NetworkTopology::NetworkTopology()
{
}


NetworkTopology::~NetworkTopology()
{
}

void NetworkTopology::addLink(const LinkDescription linkDescription)
{
	links.try_emplace(linkDescription);
}

void NetworkTopology::addRoute(const RouteDescription routeDescription, const Route route)
{
	if (routes.find(routeDescription) == routes.end())
	{
		routes[routeDescription] = { route };
	}
	else
	{
		routes[routeDescription].push_back(route);
	}
}

std::tuple<Status, SamePlaceRoutes> NetworkTopology::getBestRoutes(const RouteDescription routeDescription)
{
	double currentBestScore = std::numeric_limits<double>::max();

	auto& candidatesRoutes = routes[routeDescription];

	SamePlaceRoutes bestRoutes{};

	for (const auto &route : candidatesRoutes)
	{
		double routeCapacity = getRouteCurrentCapacity(route);

		if (routeCapacity < currentBestScore)
		{
			bestRoutes.clear();
			bestRoutes.push_back(route);
		}
		else if (routeCapacity == currentBestScore)
		{
			bestRoutes.push_back(route);
		}
	}

	if (bestRoutes.empty())
	{
		return { Status::NotOk, {} };
	}
	else
	{
		return { Status::Ok, bestRoutes };
	}
}

unsigned NetworkTopology::getCurrentCapacity() const
{
	return std::accumulate(links.begin(), links.end(), 0, [](unsigned value, const Links::value_type& p)
	{ 
		return value + p.second.getCurrentCapacity(); 
	});
}

double NetworkTopology::getRouteCurrentCapacity(const Route& route) const
{
	unsigned takenSlices = 0;

	for (const auto& linkDescription : route)
	{
		const auto linkIt = links.find(linkDescription);

		const auto& link = linkIt->second;

		takenSlices += link.getCurrentCapacity();
	}
	
	auto linksInRoute = route.size();

	double slicesInLink = Link::numOfCores * Link::numOfSlices;

	return ((static_cast<double>(takenSlices) / (slicesInLink)) * 100.0);
}

std::tuple<Status, SlicePosition> NetworkTopology::getFirstFreeChannel(unsigned short requiredSlices, Route route)
{
	auto& link = links[route.front()];

	auto[status, proposalSlicePosition] = link.getFirstFreeSlices(requiredSlices);

	if (status == Status::NotOk)
	{
		return { Status::NotOk, {} };
	}
	while (true)
	{
		auto[everyPositionStatus, linkNotFillingRequirements] = checkIfPositionFitsInEveryLink(route, proposalSlicePosition, requiredSlices);

		if (everyPositionStatus == Status::NotOk)
		{
			if (linkNotFillingRequirements == links.end())
			{
				return { Status::NotOk, {} };
			}
			else
			{
				std::tie(status, proposalSlicePosition) = linkNotFillingRequirements->second.getFirstFreeSlices(requiredSlices, proposalSlicePosition);
			}
		}
		else
		{
			return { Status::Ok, proposalSlicePosition };
		}
	}
	return std::tuple<Status, SlicePosition>();
}

void NetworkTopology::allocate(const Route route, const SlicePosition slicePosition, unsigned short requiredSlices, unsigned short time)
{
	for (const auto& linkDescription : route)
	{
		auto linkIt = links.find(linkDescription);

		auto& link = linkIt->second;

		link.allocate(slicePosition, requiredSlices, time);
	}
}



std::tuple<Status, Links::iterator> NetworkTopology::checkIfPositionFitsInEveryLink(const Route route, const SlicePosition position, const unsigned short requiredSlices)
{
	for (const auto& linkDescription : route)
	{
		auto linkIt = links.find(linkDescription);
		
		if (linkIt == links.end())
		{
			return { Status::NotOk, links.end() }; //cannot find link - it shouldn't happend
		}

		if (linkIt->second.canAllocate(position, requiredSlices) == false)
			return { Status::NotOk, linkIt };
	}

	return { Status::Ok, {} };
}

