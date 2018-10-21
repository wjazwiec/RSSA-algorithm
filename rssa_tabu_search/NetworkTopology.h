#pragma once
#include "Link.h"
#include <map>
#include "OperationStatus.h"
#include <list>
#include <algorithm>
#include <functional>
#include <numeric>
#include <set>

using Place = unsigned short;
using Distance = unsigned short;

struct LinkDescription
{
	Place source;
	Place destination;
	Distance distance;

	LinkDescription(Place src, Place dst) : source(src), destination(dst) {}

	bool const operator==(const LinkDescription &o) const {
		return source == o.source && destination == o.destination;
	}

	bool const operator<(const LinkDescription &o) const {
		return source < o.source || (destination == o.destination && source < o.source);
	}
};

using Route = std::list<std::pair<Place, Place>>;

using Links = std::map<const LinkDescription, Link>;

class NetworkTopology
{
public:
	NetworkTopology();
	~NetworkTopology();

	void addLink(const LinkDescription linkDescription);
	unsigned getCurrentCapacity() const;

	std::tuple<Status, SlicePosition> getFirstFreeChannel(unsigned short requiredSlices, Route route);

	std::tuple<Status, Links::iterator> checkIfPositionFitInEveryLink(const Route route, const SlicePosition position, const unsigned short requiredSlices);
private:

	Links links;
};

