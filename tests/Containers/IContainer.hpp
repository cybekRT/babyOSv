#define TestSuiteName2(x, y) x ## y
#define TestSuiteName(x) TestSuiteName2(x, Test)
#define TestSuite TestSuiteName(ContainerType)
#define TestedContainer Container::ContainerType

TEST(TestSuite, PushBack) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0u);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3u);
}

TEST(TestSuite, PopBack) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0u);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3u);

  EXPECT_EQ(test.PopBack(), 6);
  EXPECT_EQ(test.Size(), 2u);

  EXPECT_EQ(test.PopBack(), 3);
  EXPECT_EQ(test.Size(), 1u);

  EXPECT_EQ(test.PopBack(), 1);
  EXPECT_EQ(test.Size(), 0u);
}

TEST(TestSuite, PushFront) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0u);

  test.PushFront(1);
  test.PushFront(3);
  test.PushFront(6);

  EXPECT_EQ(test.Size(), 3u);
}

TEST(TestSuite, PopFront) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0u);

  test.PushFront(1);
  test.PushFront(3);
  test.PushFront(6);

  EXPECT_EQ(test.Size(), 3u);

  EXPECT_EQ(test.PopFront(), 6);
  EXPECT_EQ(test.Size(), 2u);

  EXPECT_EQ(test.PopFront(), 3);
  EXPECT_EQ(test.Size(), 1u);

  EXPECT_EQ(test.PopFront(), 1);
  EXPECT_EQ(test.Size(), 0u);
}

TEST(TestSuite, Clear) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0u);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3u);

  test.Clear();

  EXPECT_EQ(test.Size(), 0u);
}

TEST(TestSuite, RemoveAt) {
  TestedContainer<int> test;

  EXPECT_EQ(test.Size(), 0u);

  test.PushBack(1);
  test.PushBack(3);
  test.PushBack(6);

  EXPECT_EQ(test.Size(), 3u);

  test.Clear();

  EXPECT_EQ(test.Size(), 0u);
}

/*****************************/
static unsigned constructorCalled = 0;
static unsigned destructorCalled = 0;
static unsigned copyOperatorCalled = 0;
class MemTest
{
	public:
		MemTest()
		{
			constructorCalled++;
		}

		MemTest(const MemTest& v)
		{
			constructorCalled++;
		}

		~MemTest()
		{
			destructorCalled++;
		}

		MemTest& operator=(const MemTest& v)
		{
			copyOperatorCalled++;
			return *this;
		}
};

TEST(TestSuite, MemoryLeak) {
	TestedContainer<MemTest> test;

	const unsigned maxItems = 5;

	for(unsigned a = 0; a < maxItems; a++)
		test.PushBack(MemTest());

	EXPECT_EQ(constructorCalled, maxItems);
	EXPECT_EQ(destructorCalled, maxItems);
	EXPECT_EQ(copyOperatorCalled, maxItems);

	test.Clear();
	EXPECT_EQ(constructorCalled + copyOperatorCalled, destructorCalled);
}
