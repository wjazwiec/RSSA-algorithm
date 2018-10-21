#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING

#include "gtest/gtest.h"
#include "../rssa_tabu_search/NetworkTopology.h"

TEST(NetworkTopology, SimpleAdd) {
	NetworkTopology net;
	net.addLink({ 1, 2, 10 });

	EXPECT_TRUE(true);
}