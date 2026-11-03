#include <gtest/gtest.h>
#include <unistd.h>

// test de prueba para chequear que ande gtest
TEST(Sanity, OnePlusOneIsTwo) {
    EXPECT_EQ(1 + 1, 2);
}