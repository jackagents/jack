#include "bitmaskops.h"

#include <gtest/gtest.h>

enum class MyFlags {
    Empty = 0x0,
    A  = 0x1,
    B  = 0x2,
};

ENABLE_BITMASK_OPERATORS(MyFlags);

TEST(BITMASKOPS, TEST1)
{
    MyFlags f = (MyFlags::A | MyFlags::B);
    EXPECT_EQ(f, (MyFlags::A | MyFlags::B));
    f = MyFlags::Empty;
    EXPECT_EQ(f, MyFlags::Empty);
    f = MyFlags::A;
    EXPECT_EQ(f, MyFlags::A);
    f |= MyFlags::B;
    EXPECT_EQ(f, (MyFlags::A | MyFlags::B));

    // remove A
    f &= ~MyFlags::A;
    EXPECT_EQ(f, MyFlags::B);
    EXPECT_NE(f, MyFlags::A);
    EXPECT_NE(f, (MyFlags::A | MyFlags::B));

    EXPECT_EQ((f & MyFlags::B), MyFlags::B);

}
