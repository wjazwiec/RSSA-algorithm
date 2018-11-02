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
		m_controller.loadStaticData();
		m_controller.loadDemands("Euro28/2000_01.dem");
	}

	Controller m_controller;
};


TEST_F(ControllerTest, simple)
{
	m_controller.doAlgorithm();
	EXPECT_TRUE(false);
}//3927