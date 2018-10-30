#include "Controller.h"

#include <random>
#include <iterator>
#include <algorithm>


void Controller::loadStaticData()
{
	auto loader = DataLoaderFromFile::createLoaderFromFile("Euro28/ff.net", "Euro28/ff30.pat", "Euro28/f30.spec");

	loader->loadNetworkTopology(networkTopology);
	loader->loadPossibleRoutes(networkTopology);
}

void Controller::loadDemands(const FileName demands)
{
	currentDemands = DataLoaderFromFile::loadDemands(demands);
}

Route getRandomRoute(SamePlaceRoutes& routes)
{
	//Route route;
	//std::sample(routes.begin(), routes.end(), std::back_inserter(route),
	//	1, std::mt19937{ std::random_device{}() });

	return routes[0];
}

void Controller::processDemand(Demand demand)
{
	outputVariables.bitrate.allIncoming += demand.bitRate;
	outputVariables.demands.allIncoming++;

	//First, get candidate routes
	auto [getBestRoutesStatus, candidates] = networkTopology.getBestRoutes(RouteDescription(demand.source, demand.target), demand.bitRate, algorithmVariables.rangeOfBestRoutes);

	if (getBestRoutesStatus == Status::NotOk)
	{
		outputVariables.demands.rejected++;
		outputVariables.bitrate.rejected += demand.bitRate;
		return;
	}

	Route randomRoute = getRandomRoute(candidates);

	auto[getFirstFreeChannelStatus, slicePosition] = networkTopology.getFirstFreeChannel(randomRoute, demand.bitRate);
	
	if (getFirstFreeChannelStatus == Status::NotOk)
	{
		outputVariables.demands.rejected++;
		outputVariables.bitrate.rejected += demand.bitRate;
		return;
	}

	networkTopology.allocateWithBitrate(randomRoute, slicePosition, demand.bitRate, demand.time);

	outputVariables.demands.served++;
	outputVariables.bitrate.served += demand.bitRate;
}

void Controller::setAlgVariables(AlgorithmVariables algorithmVariables)
{
	this->algorithmVariables = algorithmVariables;
}

Controller::Controller() : algorithmVariables{ 0 }, outputVariables{}
{
}


Controller::~Controller()
{
}
