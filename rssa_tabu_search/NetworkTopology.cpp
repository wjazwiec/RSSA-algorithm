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

unsigned NetworkTopology::getCurrentCapacity() const
{
	return std::accumulate(links.begin(), links.end(), 0, [](unsigned value, const Links::value_type& p)
	{ 
		return value + p.second.getCurrentCapacity(); 
	});
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

