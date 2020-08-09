//
// Created by smallville7123 on 9/08/20.
//

#include <gtest/gtest.h>

#include <AnyOpt.h>

/*
class AnyOpt_Core : public ::testing::Test {
protected:
    AnyOpt a;
};

*/

TEST(AnyOpt_Core, initialization_no_data_checking) {
    AnyOpt a;
}

TEST(AnyOpt_Core, initialization_data_checking) {
    AnyOpt a;
    ASSERT_EQ(a.data, nullptr);
    ASSERT_EQ(a.data_is_allocated, false);
    ASSERT_EQ(a.isAnyNullOpt, false);
}

TEST(AnyOpt_Core, initialization_AnyNullOpt_no_data_checking) {
    AnyOpt a = AnyNullOpt;
}

TEST(AnyOpt_Core, initialization_AnyNullOpt_data_checking) {
    AnyOpt a = AnyNullOpt;
    ASSERT_EQ(a.data, nullptr);
    ASSERT_EQ(a.data_is_allocated, false);
    ASSERT_EQ(a.isAnyNullOpt, true);
}