#include "Controller.h"

int main()
{
	Controller controller;

	controller.loadStaticData();
	controller.loadDemands("Euro28/1000_01.dem");

	controller.setAlgVariables(AlgorithmVariables{ 0.0 });

	controller.doAlgorithm();

	//controller.loadDemands("Euro28/100_01.dem");
	controller.loadDemands("Euro28/1000_01.dem");
	controller.setAlgVariables(AlgorithmVariables{ 5.0 });
	controller.doAlgorithm();

	controller.loadDemands("Euro28/1000_01.dem");
	controller.setAlgVariables(AlgorithmVariables{ 10.0 });
	controller.doAlgorithm();

	controller.loadDemands("Euro28/1000_01.dem");
	controller.setAlgVariables(AlgorithmVariables{ 15.0 });
	controller.doAlgorithm();

	controller.loadDemands("Euro28/1000_01.dem");
	controller.setAlgVariables(AlgorithmVariables{ 30.0 });
	controller.doAlgorithm();

	system("pause");

	return 0;
}