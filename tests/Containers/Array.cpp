#include<gtest/gtest.h>
#include"Container/Array.hpp"

namespace Terminal
{
  u32 Print(const char*, ...)
  {
    return 0;
  }
}

TEST(ArrayTest, PushBack) {
  Array<int> test;

  EXPECT_EQ(test.Size(), 0);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3);
}

TEST(ArrayTest, Clear) {
  Array<int> test;

  EXPECT_EQ(test.Size(), 0);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3);

  test.Clear();

  EXPECT_EQ(test.Size(), 0);
}
