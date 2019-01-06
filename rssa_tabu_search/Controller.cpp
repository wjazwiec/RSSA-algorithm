#include "Controller.h"
#include <random>
#include <iterator>
#include <algorithm>
#include <Windows.h>
#include <iostream>
#include <string>
#include <time.h>
#include <chrono>
#include <fstream>

void Controller::loadStaticData()
{
	auto loader = DataLoaderFromFile::createLoaderFromFile("Euro28/ff.net", "Euro28/ff30.pat", "Euro28/f30.spec");

	loader->loadNetworkTopology(networkTopology_base);
	loader->loadPossibleRoutes(networkTopology_base);

	networkTopology = NetworkTopology(networkTopology_base);
}

void Controller::loadDemands(const FileName demands)
{
	std::cout << "Loading file " << demands << std::endl;
	currentDemands = DataLoaderFromFile::loadDemands(demands);

	outputVariables = {};
	networkTopology = NetworkTopology(networkTopology_base);

	std::size_t start_sign = demands.find("/");

	networkLoad = demands.substr(start_sign + 1);

	std::size_t end_sign = networkLoad.find("_");

	networkLoad = networkLoad.substr(0, end_sign);
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
	std::cout << "Alg start" << std::endl;
	auto start = std::chrono::high_resolution_clock::now();

	iteration = 0;

	while (!currentDemands.empty())
	{
#if 0
		std::string s = std::to_string(iteration) + "\n";
		char const *pchar = s.c_str();  //use char const* as target type

		OutputDebugString(pchar);
		if (iteration == 20)
			break;
#endif
		processDemand(currentDemands.front());
		currentDemands.pop();
	}

	auto finish = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> elapsed = finish - start;

	std::cout << std::endl << "Elapsed time: " << elapsed.count() << " s\n";
	std::cout << "Alg variable: " << this->algorithmVariables.rangeOfBestRoutes << " Demands all:" << outputVariables.demands.allIncoming << " served:" << outputVariables.demands.served << " rejected:" << outputVariables.demands.rejected << std::endl;
	std::cout << "Bitrate all:" << outputVariables.bitrate.allIncoming << " served:" << outputVariables.bitrate.served << " rejected:" << outputVariables.bitrate.rejected << std::endl;

	saveToFile(resultFile);
}

void Controller::controlIterations(const short newIteration)
{
	if (newIteration > iteration)
	{
		networkTopology.tick();
		iteration = newIteration;

		if (iteration % 200 == 0)
		{
			std::cout << "|";
		}
	}
}

void Controller::saveToFile(const FileName file)
{
	std::ofstream fileHandle(file, std::fstream::out | std::ios_base::app);

	const std::string delimeter = ",";

	if (fileHandle.is_open())
	{
		fileHandle << networkLoad << delimeter << outputVariables.bitrate.allIncoming << delimeter << outputVariables.bitrate.served << delimeter << outputVariables.bitrate.rejected
			<< delimeter << outputVariables.demands.allIncoming << delimeter << outputVariables.demands.served << delimeter << outputVariables.demands.rejected
			<< delimeter << this->algorithmVariables.rangeOfBestRoutes << std::endl;
	}

	fileHandle.close();
}

void Controller::setResultsFileName(const FileName file)
{
	this->resultFile = file;
}

Controller::Controller() : algorithmVariables{ 0 }, outputVariables{}
{
}

Controller::~Controller()
{
	srand(static_cast<unsigned int>(time(NULL)));
}
