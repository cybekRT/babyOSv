#include<gtest/gtest.h>
#include"Container/String.hpp"
#include"Container/String.cpp"

using Container::String;

const char* testStr = "TestString";
const char* otherTestStr = "AnotherTestingString";
const char* superLongStr = "TestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTestTest";

/****************************** Constructors ******************************/

TEST(String, ConstructEmpty)
{
	String str;

	EXPECT_NE(str.Data(), nullptr);
	EXPECT_EQ(str.Length(), 0);
}

TEST(String, ConstructFromCharArray)
{
	String str(testStr);

	EXPECT_NE(str.Data(), testStr);
	EXPECT_EQ(strcmp(str.Data(), testStr), 0);
	EXPECT_EQ(str.Length(), strlen(testStr));

	str.Clear();
	EXPECT_NE(strcmp(str.Data(), testStr), 0);
	EXPECT_EQ(str.Length(), 0);
}

TEST(String, ConstructFromOtherString)
{
	String str1(testStr);
	String str2(str1);

	EXPECT_NE(str1.Data(), str2.Data());
	EXPECT_EQ(strcmp(str1.Data(), testStr), 0);
	EXPECT_EQ(strcmp(str2.Data(), testStr), 0);
	EXPECT_EQ(str1.Length(), str2.Length());

	str1.Clear();
	EXPECT_EQ(str1.Length(), 0);
	EXPECT_EQ(str2.Length(), strlen(testStr));
}

/****************************** Operators = ******************************/

TEST(String, AssignCharArray)
{
	String str;
	str = testStr;

	EXPECT_NE(str.Data(), testStr);
	EXPECT_EQ(strcmp(str.Data(), testStr), 0);
	EXPECT_EQ(str.Length(), strlen(testStr));
}

TEST(String, AssignOtherString)
{
	String str1(testStr);
	String str2;
	str2 = str1;

	EXPECT_NE(str2.Data(), testStr);
	EXPECT_NE(str1.Data(), str2.Data());
	EXPECT_EQ(strcmp(str1.Data(), str2.Data()), 0);
	EXPECT_EQ(str1.Length(), str2.Length());
}

/****************************** Compare ******************************/

TEST(String, CompareEqualityWithCharArray)
{
	String str(testStr);

	EXPECT_TRUE(str == testStr);
	EXPECT_FALSE(str == otherTestStr);
	EXPECT_FALSE(str == superLongStr);
}

TEST(String, CompareEqualityWithString)
{
	String str1(testStr);
	String str2(testStr);
	String strOther1(otherTestStr);
	String strOther2(superLongStr);

	EXPECT_TRUE(str1 == str2);
	EXPECT_FALSE(str1 == strOther1);
	EXPECT_FALSE(str1 == strOther2);
}

TEST(String, CompareNonEqualityWithCharArray)
{
	String str(testStr);

	EXPECT_FALSE(str != testStr);
	EXPECT_TRUE(str != otherTestStr);
	EXPECT_TRUE(str != superLongStr);
}

TEST(String, CompareNonEqualityWithString)
{
	String str1(testStr);
	String str2(testStr);
	String strOther1(otherTestStr);
	String strOther2(superLongStr);

	EXPECT_FALSE(str1 != str2);
	EXPECT_TRUE(str1 != strOther1);
	EXPECT_TRUE(str1 != strOther2);
}
