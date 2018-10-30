#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING

#include "gtest/gtest.h"
#include "../rssa_tabu_search/Controller.h"

class ControllerTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		m_controller.loadStaticData();
		m_controller.loadDemands("Euro28/100_01.dem");
	}

	Controller m_controller;
};

TEST_F(ControllerTest, simple)
{
	while (!m_controller.currentDemands.empty())
	{
		m_controller.processDemand(m_controller.currentDemands.front());
		m_controller.currentDemands.pop();
	}

	auto output = m_controller.outputVariables;
}