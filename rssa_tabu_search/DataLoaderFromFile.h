#pragma once
#include "DataLoader.h"
#include "NetworkTopology.h"

using FileName = std::string;

class DataLoaderFromFile :
	public DataLoader
{
public:
	DataLoaderFromFile();
	virtual ~DataLoaderFromFile();

	void loadNetworkTopology(NetworkTopology& networkTopology);
	void loadPossibleRoutes(NetworkTopology& networkTopology);
	static Demands loadDemands(const FileName demandsFile);
	Demands loadDemands();

	static std::shared_ptr<DataLoaderFromFile> createLoaderFromFile(const FileName networkTopology, const FileName possibleRoutes, const FileName possibleRoutesHelper);

private:

	std::map<unsigned, LinkDescription> savedLinks;

	unsigned networkNodes, links;

	FileName networkTopology;
	FileName possibleRoutes;
	FileName possibleRoutesHelper;
	FileName demands;
};

