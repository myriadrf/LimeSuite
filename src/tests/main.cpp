#include "gtest/gtest.h"

int main(int argc, char **argv)
{
    //::testing::GTEST_FLAG(filter) = "*perfTest*";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
