#pragma once
#include"NetworkTopology.h"
#include"DataLoaderFromFile.h"

struct AlgorithmVariables
{
	// How route can be worse than the best route to be in set of the bests
	double rangeOfBestRoutes;
};

struct Output
{
	unsigned allIncoming;
	unsigned served;
	unsigned rejected;
};

struct OutputVariables
{
	Output bitrate;
	Output demands;
};

class Controller
{
public:
	Controller();
	~Controller();

	void loadStaticData();
	void loadDemands(const FileName demands);
	void processDemand(Demand demand);

	void setAlgVariables(AlgorithmVariables algorithmVariables);

	void doAlgorithm();
	void controlIterations(const short iteration);

	Demands currentDemands;
	OutputVariables outputVariables;
private:
	AlgorithmVariables algorithmVariables;
	NetworkTopology networkTopology, networkTopology_base;
	short iteration;

};

