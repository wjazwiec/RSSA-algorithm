#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING

#include "gtest/gtest.h"
#include "../rssa_tabu_search/Controller.h"
#include <iostream>
#include <Windows.h>


class ControllerTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		controller.loadStaticData();
		controller.loadDemands("Euro28/2000_01.dem");
	}

	Controller controller;
};


TEST_F(ControllerTest, simple)
{
	controller.loadDemands("Euro28/2000_01.dem");

	controller.setAlgVariables(AlgorithmVariables{ 0.0 });

	controller.doAlgorithm();

	//controller.loadDemands("Euro28/100_01.dem");
	controller.loadDemands("Euro28/2000_01.dem");
	controller.setAlgVariables(AlgorithmVariables{ 10.0 });
	controller.doAlgorithm();

	EXPECT_TRUE(true);
}//3927