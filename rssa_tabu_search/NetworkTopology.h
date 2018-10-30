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
		return source < o.source || (source == o.source && destination < o.destination);
	}
};

struct Route
{
	std::list<LinkDescription> links;
	std::array<unsigned, 20> requiredSlices;
};


static unsigned getRequiredSlices(const Route& route, const short bitRate)
{
	if (bitRate == 0)
		return 0;

	if (((bitRate-1)/50)> 20)
		return 0;

	return route.requiredSlices[((bitRate - 1) / 50)];
}


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
	
	double getRouteCurrentCapacity(const Route& route) const;

	std::tuple<Status, SamePlaceRoutes> getBestRoutes(const RouteDescription routeDescription, const short bitRate, const double possibleDiffrenceWithBest = 0);

	unsigned getCurrentCapacity() const;

	std::tuple<Status, Links::iterator> checkIfPositionFitsInEveryLink(const Route route, const SlicePosition position, const unsigned short requiredSlices);
	std::tuple<Status, SlicePosition> getFirstFreeChannel(unsigned short requiredSlices, Route route);
	std::tuple<Status, SlicePosition> getFirstFreeChannel(Route route, const short bitRate);

	void allocate(const Route route, const SlicePosition slicePosition, unsigned short requiredSlices, unsigned short time);
	void allocateWithBitrate(const Route route, const SlicePosition slicePosition, const short bitRate, unsigned short time);
private:

	Links links;
	Routes routes;
};

