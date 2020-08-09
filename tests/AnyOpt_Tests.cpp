//
// Created by smallville7123 on 9/08/20.
//

#include <gtest/gtest.h>


// if ANYOPT_RUNTIME_ASSERTION is defined
// then AnyOpt uses runtime assertions instead of compile-time assertions

// AnyOpt defaults to compile-time assertions

// use runtime assertions to allow assertion testing using death tests
#define ANYOPT_RUNTIME_ASSERTION

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

TEST(AnyOpt_Flags_Copy, valid_copy) {
    const int z = 5;
    AnyOptCustomFlags<AnyOpt_FLAG_COPY_ONLY> a = z;
}

TEST(AnyOpt_Flags_Copy, invalid_copy) {
    // this will invoke the move constructor
    ASSERT_DEATH(
            {
                 AnyOptCustomFlags<AnyOpt_FLAG_COPY_ONLY> a = 5;
             },
            AnyOpt_Catch_Flag_POSIX_REGEX(AnyOpt_FLAG_MOVE_ONLY)
     );
}

TEST(AnyOpt_Flags_Move, valid_move) {
    // If other is an rvalue expression,
    // move constructor will be selected by overload resolution
    // and called during copy-initialization.
    // There is no such term as move-initialization.
    // THIS only applies if a USER-DEFINED move constructor is present
    AnyOptCustomFlags<AnyOpt_FLAG_MOVE_ONLY> a = 5;
}

//TEST(AnyOpt_Flags_Pointer, valid_pointer) {
//    void * x;
//    AnyOptCustomFlags<AnyOpt_FLAG_ENABLE_POINTERS> a = x;
//}