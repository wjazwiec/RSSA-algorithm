#include "Controller.h"
#include <string>
int main()
{
	Controller controller;

	controller.loadStaticData();
	controller.setResultsFileName("results.csv");

	for (auto networkLoad = 100; networkLoad <= 100; networkLoad += 100)
	{
		for (auto testcase : { "01", "02", "03", "04", "05", "06", "07", "08", "09", "10" })
		{
			std::string fileName = "Euro28/" + std::to_string(networkLoad) + "_" + testcase + ".dem";

			for (auto algVariable : { 0.0, 5.0, 10.0, 15.0, 30.0 })
			{
				controller.loadDemands(fileName);
				controller.setAlgVariables(AlgorithmVariables{ algVariable });
				controller.doAlgorithm();
			}
		}
	}


	return 0;
}