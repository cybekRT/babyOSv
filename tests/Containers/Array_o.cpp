// #include<gtest/gtest.h>
// #include"Container/Array.hpp"

// #define ContainerType Array
// #include"IContainer.hpp"

// TEST(TestSuite, OperatorArray)
// {
// 	TestedContainer<int> test;

// 	test.PushBack(1);
// 	test.PushBack(5);
// 	test.PushBack(9);
// 	test.PushBack(2);

// 	EXPECT_EQ(test.Size(), 4u);
// 	EXPECT_EQ(test[0], 1);
// 	EXPECT_EQ(test[1], 5);
// 	EXPECT_EQ(test[2], 9);
// 	EXPECT_EQ(test[3], 2);
// }

// #include<vector>

// TEST(Array, Clear)
// {
// 	static int constructors = 0;
// 	static int destructors = 0;
// 	static int opEq = 0;

// 	class Agent
// 	{
// 		public:
// 			Agent() { constructors++; }
// 			Agent(const Agent&) { constructors++; }
// 			~Agent() { destructors++; }
// 			Agent& operator=(const Agent& arg) { opEq++; }
// 	};

// 	Container::Array<Agent> agents(8);

// 	agents.PushBack(Agent());
// 	agents.PushBack(Agent());
// 	agents.PushBack(Agent());
// 	EXPECT_EQ(agents.Size(), 3);
// 	agents.Clear();

// 	EXPECT_EQ(agents.Size(), 0);
// 	EXPECT_EQ(constructors, 3);
// 	EXPECT_EQ(opEq, 3);
// 	EXPECT_EQ(destructors, 6);

// 	// std::vector<Agent> agents;

// 	// agents.push_back(Agent());
// 	// agents.push_back(Agent());
// 	// agents.push_back(Agent());
// 	// EXPECT_EQ(agents.size(), 3);
// 	// agents.clear();

// 	// EXPECT_EQ(agents.size(), 0);
// 	// EXPECT_EQ(constructors, 3);
// 	// EXPECT_EQ(opEq, 3);
// 	// EXPECT_EQ(destructors, 3);
// }
