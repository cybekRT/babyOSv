#define TestSuiteName2(x, y) x ## y
#define TestSuiteName(x) TestSuiteName2(x, Test)
#define TestSuite TestSuiteName(ContainerType)
#define TestedContainer Container::ContainerType

TEST(TestSuite, PushBack) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3);
//   EXPECT_EQ(test[0], 1);
//   EXPECT_EQ(test[1], 3);
//   EXPECT_EQ(test[2], 6);
}

TEST(TestSuite, PopBack) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3);
//   EXPECT_EQ(test[0], 1);
//   EXPECT_EQ(test[1], 3);
//   EXPECT_EQ(test[2], 6);

  EXPECT_EQ(test.PopBack(), 6);
  EXPECT_EQ(test.Size(), 2);
//   EXPECT_EQ(test[0], 1);
//   EXPECT_EQ(test[1], 3);

  EXPECT_EQ(test.PopBack(), 3);
  EXPECT_EQ(test.Size(), 1);
//   EXPECT_EQ(test[0], 1);

  EXPECT_EQ(test.PopBack(), 1);
  EXPECT_EQ(test.Size(), 0);
}

TEST(TestSuite, PushFront) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0);

  test.PushFront(1);
  test.PushFront(3);
  test.PushFront(6);

//   EXPECT_EQ(test[0], 6);
//   EXPECT_EQ(test[1], 3);
//   EXPECT_EQ(test[2], 1);

  EXPECT_EQ(test.Size(), 3);
}

TEST(TestSuite, PopFront) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0);

  test.PushFront(1);
  test.PushFront(3);
  test.PushFront(6);

  EXPECT_EQ(test.Size(), 3);
//   EXPECT_EQ(test[0], 6);
//   EXPECT_EQ(test[1], 3);
//   EXPECT_EQ(test[2], 1);

  EXPECT_EQ(test.PopFront(), 6);
  EXPECT_EQ(test.Size(), 2);
//   EXPECT_EQ(test[0], 3);
//   EXPECT_EQ(test[1], 1);

  EXPECT_EQ(test.PopFront(), 3);
  EXPECT_EQ(test.Size(), 1);
//   EXPECT_EQ(test[0], 1);

  EXPECT_EQ(test.PopFront(), 1);
  EXPECT_EQ(test.Size(), 0);
}

TEST(TestSuite, Clear) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3);

  test.Clear();

  EXPECT_EQ(test.Size(), 0);
}

TEST(TestSuite, RemoveAt) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3);

  test.Clear();

  EXPECT_EQ(test.Size(), 0);
}