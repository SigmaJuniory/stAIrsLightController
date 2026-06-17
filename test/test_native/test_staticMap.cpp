#include <gtest/gtest.h>

#include "staticMap.h"

TEST(StaticMap, EmptyMap) {
    static_map<int, int, 8> map;

    EXPECT_EQ(map.size(), 0);
    EXPECT_EQ(map.capacity(), 8);
}

TEST(StaticMap, InsertAndAt) {
    static_map<int, int, 8> map;

    map.insert(1, 100);
    map.insert(2, 200);

    EXPECT_EQ(map.at(1), 100);
    EXPECT_EQ(map.at(2), 200);
}

TEST(StaticMap, Contains) {
    static_map<int, int, 8> map;

    map.insert(42, 1234);

    EXPECT_TRUE(map.contains(42));
    EXPECT_FALSE(map.contains(100));
}

TEST(StaticMap, OperatorBracketCreatesEntry) {
    static_map<int, int, 8> map;

    map[7] = 777;

    EXPECT_TRUE(map.contains(7));
    EXPECT_EQ(map.at(7), 777);
}

TEST(StaticMap, InsertUpdatesExistingKey) {
    static_map<int, int, 8> map;

    map.insert(1, 10);
    map.insert(1, 20);

    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map.at(1), 20);
}

TEST(StaticMap, Erase) {
    static_map<int, int, 8> map;

    map.insert(1, 10);
    map.insert(2, 20);
    map.insert(3, 30);

    map.erase(2);

    EXPECT_FALSE(map.contains(2));
    EXPECT_EQ(map.size(), 2);
}

TEST(StaticMap, AtThrowsForMissingKey) {
    static_map<int, int, 8> map;

    EXPECT_THROW(map.at(123), std::out_of_range);
}

TEST(StaticMap, ThrowsWhenFull) {
    static_map<int, int, 2> map;

    map.insert(1, 10);
    map.insert(2, 20);

    EXPECT_THROW(map.insert(3, 30), std::length_error);
}

TEST(StaticMap, RangeForIteration) {
    static_map<int, int, 8> map;

    map.insert(1, 10);
    map.insert(2, 20);
    map.insert(3, 30);

    int sum = 0;

    for (const auto &[key, value] : map) {
        sum += value;
    }

    EXPECT_EQ(sum, 60);
}