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

void NetworkTopology::addRouteToBestIfFits(SamePlaceRoutes& bestRoutes, const Route& candidate, const double possibleDiffrenceWithBest)
{
	static double currentBestRouteCapacity;

	// If first iteration
	if (bestRoutes.empty())
	{
		currentBestRouteCapacity = std::numeric_limits<double>::max();
	}

	double routeCapacity = getRouteCurrentCapacity(candidate);
	double routeCapacityAfterAlgorithmVariableApplied = routeCapacity - possibleDiffrenceWithBest;

	if (currentBestRouteCapacity <= routeCapacity && currentBestRouteCapacity >= routeCapacityAfterAlgorithmVariableApplied)
	{
		bestRoutes.push_back(candidate);
	}
	else if (routeCapacity < currentBestRouteCapacity)
	{
		if (!bestRoutes.empty())
		{
			bestRoutes.erase(std::remove_if(bestRoutes.begin(), bestRoutes.end(), [&](Route route) {
				return ((getRouteCurrentCapacity(route) - possibleDiffrenceWithBest) >= routeCapacity);
			}), bestRoutes.end());
		}

		bestRoutes.push_back(candidate);
		currentBestRouteCapacity = routeCapacity;
	}
}

std::tuple<Status, SamePlaceRoutes> NetworkTopology::getBestRoutes(const RouteDescription routeDescription, const short bitRate, const double possibleDiffrenceWithBest)
{
	double currentBestScore = std::numeric_limits<double>::max();

	auto& candidatesRoutes = routes[routeDescription];

	SamePlaceRoutes bestRoutes{};

	for (const auto &route : candidatesRoutes)
	{
		short requiredSlices = getRequiredSlices(route, bitRate);

		Status status;

		std::tie(status, std::ignore) = getFirstFreeChannel(requiredSlices, route);

		if (status == Status::NotOk)
		{
			continue;
		}

		addRouteToBestIfFits(bestRoutes, route, possibleDiffrenceWithBest);
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

	for (const auto& linkDescription : route.links)
	{
		const auto linkIt = links.find(linkDescription);

		const auto& link = linkIt->second;

		takenSlices += link.getCurrentCapacity();
	}
	
	auto linksInRoute = route.links.size();

	double slicesInLinks = static_cast<double>(Link::numOfCores * Link::numOfSlices * linksInRoute);

	return ((static_cast<double>(takenSlices) / (slicesInLinks)) * 100.0);
}

std::tuple<Status, SlicePosition> NetworkTopology::getFirstFreeChannel(Route route, const short bitRate)
{
	short requiredSlices = getRequiredSlices(route, bitRate);

	return getFirstFreeChannel(requiredSlices, route);
}

std::tuple<Status, SlicePosition> NetworkTopology::getFirstFreeChannel(unsigned short requiredSlices, Route route)
{
	auto& link = links[route.links.front()];

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
				
				if (status == Status::NotOk && proposalSlicePosition.slice == 0 && proposalSlicePosition.core == 0)
					break;
			}
		}
		else
		{
			return { Status::Ok, proposalSlicePosition };
		}
	}

	return { Status::NotOk, {} };
}

void NetworkTopology::allocate(const Route route, const SlicePosition slicePosition, unsigned short requiredSlices, unsigned short time)
{
	for (const auto& linkDescription : route.links)
	{
		auto linkIt = links.find(linkDescription);

		auto& link = linkIt->second;

		link.allocate(slicePosition, requiredSlices, time);
	}
}

void NetworkTopology::allocateWithBitrate(const Route route, const SlicePosition slicePosition, const short bitRate, unsigned short time)
{
	short requiredSlices = getRequiredSlices(route, bitRate);

	return allocate(route, slicePosition, requiredSlices, time);
}

void NetworkTopology::tick()
{
	for (auto& linkPair : links)
	{
		linkPair.second.decrementTime();
	}
}

std::tuple<Status, Links::iterator> NetworkTopology::checkIfPositionFitsInEveryLink(const Route route, const SlicePosition position, const unsigned short requiredSlices)
{
	for (const auto& linkDescription : route.links)
	{
		auto linkIt = links.find(linkDescription);
		
		if (linkIt == links.end())
		{
			return { Status::NotOk, links.end() };
		}

		if (linkIt->second.canAllocate(position, requiredSlices) == false)
			return { Status::NotOk, linkIt };
	}

	return { Status::Ok, {} };
}

