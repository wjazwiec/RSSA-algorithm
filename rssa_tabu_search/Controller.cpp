#include "Controller.h"

#include <random>
#include <iterator>
#include <algorithm>
#include <Windows.h>
#include <iostream>
#include <string>
#include <time.h>

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
	Index randIndex = (rand() % routes.size() + 1) - 1;

	return routes[randIndex];
}

void Controller::processDemand(Demand demand)
{
	controlIterations(demand.iteration);

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

void Controller::doAlgorithm()
{
	iteration = 0;

	while (!currentDemands.empty())
	{
		std::string s = std::to_string(iteration) + "\n";
		char const *pchar = s.c_str();  //use char const* as target type

		OutputDebugString(pchar);

		processDemand(currentDemands.front());
		currentDemands.pop();
	}
}

void Controller::controlIterations(const short newIteration)
{
	if (newIteration > iteration)
	{
		networkTopology.tick();
		iteration = newIteration;
	}
}

Controller::Controller() : algorithmVariables{ 0 }, outputVariables{}
{
}


Controller::~Controller()
{
	srand(time(NULL));
}
