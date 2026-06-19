#include <gtest/gtest.h>

#include "static_vector.h"

TEST(StaticVector, EmptyVector) {
    static_vector<int, 8> vec;

    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 8);
}

TEST(StaticVector, PushBack) {
    static_vector<int, 8> vec;

    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);

    EXPECT_EQ(vec.size(), 3);

    EXPECT_EQ(vec[0], 10);
    EXPECT_EQ(vec[1], 20);
    EXPECT_EQ(vec[2], 30);
}

TEST(StaticVector, RangeForIteration) {
    static_vector<int, 8> vec;

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    int sum = 0;

    for (auto v : vec) {
        sum += v;
    }

    EXPECT_EQ(sum, 6);
}

TEST(StaticVector, ThrowsWhenFull) {
    static_vector<int, 2> vec;

    vec.push_back(1);
    vec.push_back(2);

    EXPECT_THROW(vec.push_back(3), std::length_error);
}

TEST(StaticVector, MovePushBack) {
    static_vector<std::string, 2> vec;

    std::string str = "hello";

    vec.push_back(std::move(str));

    EXPECT_EQ(vec[0], "hello");
}
