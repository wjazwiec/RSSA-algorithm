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
	Links::iterator link = links.begin();

	std::set<SlicePosition> forbiddenSlices;

	auto[status, proposalSlicePosition] = link->second.getFirstFreeSlices(requiredSlices);

	if (status == Status::NotOk)
	{
		return { Status::NotOk, {} };
	}
	while (true)
	{
		auto[everyPositionStatus, linkNotFillingRequirements] = checkIfPositionFitInEveryLink(route, proposalSlicePosition, requiredSlices);

		if (everyPositionStatus == Status::NotOk)
		{
			if (linkNotFillingRequirements == links.end())
			{
				return { Status::NotOk, {} };
			}
			else
			{
				forbiddenSlices.insert(proposalSlicePosition);
				linkNotFillingRequirements->second.getFirstFreeSlices(requiredSlices);
			}
		}
		else
		{
			return { Status::Ok, proposalSlicePosition };
		}
	}
	return std::tuple<Status, SlicePosition>();
}



std::tuple<Status, Links::iterator> NetworkTopology::checkIfPositionFitInEveryLink(const Route route, const SlicePosition position, const unsigned short requiredSlices)
{
	for (const auto [source, destination] : route)
	{
		auto linkIt = links.begin();//getLink(source, destination);
		
		if (linkIt == links.end())
		{
			return { Status::NotOk, links.end() }; //cannot find link - it schouldn't happend
		}

		if (linkIt->second.canAllocate(position, requiredSlices) == false)
			return { Status::NotOk, linkIt };
	}

	return { Status::Ok, {} };
}

