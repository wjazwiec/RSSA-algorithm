#pragma once
#include "NetworkTopology.h"
#include <queue>

struct Demand
{
	const short iteration;
	const Place source;
	const Place target;
	const short time;
	const short bitRate;
};

using Demands = std::queue<Demand>;

class DataLoader
{
public:
	DataLoader();
	virtual ~DataLoader();
	virtual void loadNetworkTopology(NetworkTopology& networkTopology) = 0;
	virtual void loadPossibleRoutes(NetworkTopology& networkTopology) = 0;
	virtual Demands loadDemands() = 0;
};

