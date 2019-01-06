#include "DataLoaderFromFile.h"
#include <fstream>
#include <sstream>
#include <string>

DataLoaderFromFile::DataLoaderFromFile()
{
}


DataLoaderFromFile::~DataLoaderFromFile()
{
}

void DataLoaderFromFile::loadNetworkTopology(NetworkTopology & networkTopology)
{
	std::ifstream infile(this->networkTopology);

	if (infile.fail())
	{
		return;
	}

	unsigned currentLink;

	infile >> networkNodes >> links;

	unsigned routeId = 0;

	for (unsigned source = 0; source < networkNodes; source++)
	{
		for (unsigned destination = 0; destination < networkNodes; destination++)
		{
			infile >> currentLink;

			if (currentLink != 0)
			{
				LinkDescription linkDescription(source, destination);
				savedLinks.insert(std::pair<unsigned, LinkDescription>(routeId++, linkDescription));
				networkTopology.addLink(linkDescription);
			}
		}
	}

	infile.close();
}

void DataLoaderFromFile::loadPossibleRoutes(NetworkTopology & networkTopology)
{
	std::ifstream infile(this->possibleRoutes);

	std::ifstream helperFile(this->possibleRoutesHelper);

	if (infile.fail())
	{
		return;
	}

	if (helperFile.fail())
	{
		return;
	}

	unsigned currentLink, currentSlices;

	for (Place source = 0; source < networkNodes; source++)
	{
		for (Place destination = 0; destination < networkNodes; destination++)
		{
			if (source == destination)
				continue;
				
			for (auto i = 0; i < 30; i++)
			{
				Route route;

				for (unsigned linkId = 0; linkId < links; linkId++)
				{
					infile >> currentLink;

					if (currentLink != 0)
					{
						auto linkDescription = savedLinks.find(linkId);
					    route.links.push_back(linkDescription->second);
					}
				}

				for (unsigned requiredSlicesIndex = 0; requiredSlicesIndex < 20; requiredSlicesIndex++)
				{
					helperFile >> currentSlices;
					route.requiredSlices[requiredSlicesIndex] = currentSlices;
				}

				networkTopology.addRoute(RouteDescription(source, destination), route);
			}
		}
	}

	infile.close();
	helperFile.close();
}

Demands DataLoaderFromFile::loadDemands() { return {}; }

Demands DataLoaderFromFile::loadDemands(const FileName demandsFile)
{
	Demands demands;

	std::ifstream fileHandle(demandsFile);

	if (fileHandle.fail())
	{
		return {};
	}

	std::string line;

	while (std::getline(fileHandle, line))
	{
		std::istringstream iss(line);
		short iteration, time, bitrate;
		Place source, target;
		if (!(iss >> iteration >> source >> target >> bitrate >> time))
		{
			break; 
		}
		else
		{
			demands.push(Demand{ iteration, source, target, time, bitrate });
		}
	}

	fileHandle.close();

	return demands;
}

std::shared_ptr<DataLoaderFromFile> DataLoaderFromFile::createLoaderFromFile(const FileName networkTopology, const FileName possibleRoutes, const FileName possibleRoutesHelper)
{
	std::shared_ptr<DataLoaderFromFile> dataLoader = std::make_shared<DataLoaderFromFile>();

	dataLoader->networkTopology = networkTopology;
	dataLoader->possibleRoutes = possibleRoutes;
	dataLoader->possibleRoutesHelper = possibleRoutesHelper;

	return dataLoader;
}
