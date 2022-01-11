#include<gtest/gtest.h>
#include"Container/Array.hpp"

#define ContainerType Array
#include"IContainer.hpp"

// TEST(TestSuite, OperatorArray) {
//   TestedContainer<int> test;

//   test.PushBack(1);
//   test.PushBack(5);
//   test.PushBack(9);
//   test.PushBack(2);

//   EXPECT_EQ(test.Size(), 4u);
//   EXPECT_EQ(test[0], 1);
//   EXPECT_EQ(test[1], 5);
//   EXPECT_EQ(test[2], 9);
//   EXPECT_EQ(test[3], 2);
// }