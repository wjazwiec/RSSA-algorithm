#include "Controller.h"

int main()
{
	Controller controller;

	controller.loadStaticData();
	controller.loadDemands("Euro28/2000_01.dem");

	controller.setAlgVariables(AlgorithmVariables{ 0.0 });

	controller.doAlgorithm();

	controller.loadDemands("Euro28/100_01.dem");
	controller.doAlgorithm();

	system("pause");

	return 0;
}