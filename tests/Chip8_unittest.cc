#include "../src/Chip8.h"
#include "gtest/gtest.h"
#include <iostream>

namespace {

TEST(chipInit, initPC){
	Chip8 c;
	EXPECT_EQ(c.get_PC(), 0);
}

}