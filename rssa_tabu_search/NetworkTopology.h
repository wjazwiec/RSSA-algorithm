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

	LinkDescription(Place src, Place dst) : source(src), destination(dst) {}

	bool const operator==(const LinkDescription &o) const {
		return source == o.source && destination == o.destination;
	}

	bool const operator<(const LinkDescription &o) const {
		return source < o.source || (destination == o.destination && source < o.source);
	}
};

using Route = std::list<LinkDescription>;

using RouteDescription = LinkDescription;

using SamePlaceRoutes = std::vector<Route>;

using Routes = std::map<const RouteDescription, SamePlaceRoutes>;

using Links = std::map<const LinkDescription, Link>;

class NetworkTopology
{
public:
	NetworkTopology();
	~NetworkTopology();

	void addLink(const LinkDescription linkDescription);
	void addRoute(const RouteDescription routeDescription, const Route route);

	unsigned getCurrentCapacity() const;

	double getRouteCurrentCapacity(const Route& route) const;

	std::tuple<Status, Links::iterator> checkIfPositionFitsInEveryLink(const Route route, const SlicePosition position, const unsigned short requiredSlices);
	std::tuple<Status, SlicePosition> getFirstFreeChannel(unsigned short requiredSlices, Route route);

	void allocate(const Route route, const SlicePosition slicePosition, unsigned short requiredSlices, unsigned short time);
private:

	Links links;
	Routes routes;
};

