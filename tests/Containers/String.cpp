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

/****************************** Assignments ******************************/

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

/****************************** Add operators ******************************/

TEST(String, AddChar)
{
	String str = String("abc") + 'd' + 'e' + 'f';

	EXPECT_EQ(str.Length(), 6);
	EXPECT_EQ(str, "abcdef");
}

TEST(String, AddCharArray)
{
	String str = String("abc") + "def";

	EXPECT_EQ(str.Length(), 6);
	EXPECT_EQ(str, "abcdef");
}

TEST(String, AddString)
{
	String str = String("abc") + String("def");

	EXPECT_EQ(str.Length(), 6);
	EXPECT_EQ(str, "abcdef");
}

TEST(String, AddAssignChar)
{
	String str("abc");
	str += 'd';
	str += 'e';
	str += 'f';

	EXPECT_EQ(str.Length(), 6);
	EXPECT_EQ(str, "abcdef");
}

TEST(String, AddAssignCharArray)
{
	String str("abc");
	str += "def";

	EXPECT_EQ(str.Length(), 6);
	EXPECT_EQ(str, "abcdef");
}

TEST(String, AddAssignString)
{
	String str("abc");
	str += String("def");

	EXPECT_EQ(str.Length(), 6);
	EXPECT_EQ(str, "abcdef");
}

/****************************** Comparisons ******************************/

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

/****************************** Array operator ******************************/

TEST(String, ArrayOperator)
{
	String str(testStr);
	
	EXPECT_EQ(str[0], testStr[0]);
	EXPECT_EQ(str[1], testStr[1]);
	EXPECT_EQ(str[str.Length()-1], testStr[str.Length()-1]);

	str[0] = 'c';
	EXPECT_EQ(str[0], 'c');
	if(testStr[0] != 'c')
		EXPECT_NE(str[0], testStr[0]);

	str[0] = 'x';
	EXPECT_EQ(str[0], 'x');

	str[str.Length()-1] = 'x';
	EXPECT_EQ(str[str.Length()-1], 'x');
}

TEST(String, ArrayOperatorConst)
{
	const String str(testStr);
	
	EXPECT_EQ(str[0], testStr[0]);
	EXPECT_EQ(str[1], testStr[1]);
	EXPECT_EQ(str[str.Length()-1], testStr[str.Length()-1]);

	EXPECT_EQ(str[0], testStr[0]);
	EXPECT_EQ(str[1], testStr[1]);
	EXPECT_EQ(str[str.Length()-1], testStr[str.Length()-1]);
}

/****************************** Insertion ******************************/

TEST(String, AddAt)
{
	String str;

	str.AddAt(0, 'Y');
	str.AddAt(1, 'o');
	str.AddAt(2, 'L');
	str.AddAt(3, 'o');

	EXPECT_EQ(str.Length(), 4);
	EXPECT_EQ(str, "YoLo");

	str.AddAt(0, '#');
	EXPECT_EQ(str.Length(), 5);
	EXPECT_EQ(str, "#YoLo");

	str.AddAt(3, 'X');
	str.AddAt(4, 'D');

	EXPECT_EQ(str.Length(), 7);
	EXPECT_EQ(str, "#YoXDLo");
}

TEST(String, RemoveAt)
{
	String str("1234567890");

	str.RemoveAt(0);
	EXPECT_EQ(str.Length(), 9);
	EXPECT_EQ(str, "234567890");

	str.RemoveAt(3);
	EXPECT_EQ(str.Length(), 8);
	EXPECT_EQ(str, "23467890");

	str.RemoveAt(7);
	EXPECT_EQ(str.Length(), 7);
	EXPECT_EQ(str, "2346789");

	// str.RemoveAt(1337);
	// EXPECT_EQ(str.Length(), 7);
	// EXPECT_EQ(str, "2346789");
}

TEST(String, Clear)
{
	String str(testStr);
	auto len = strlen(testStr);

	str.Clear();
	EXPECT_EQ(str.Length(), 0);
	EXPECT_EQ(strlen(testStr), len);
}

TEST(String, Length)
{
	String str;

	EXPECT_EQ(str.Length(), 0);

	str = testStr;
	EXPECT_EQ(str.Length(), strlen(testStr));

	str.Clear();
	EXPECT_EQ(str.Length(), 0);

	char tmp[123457];
	for(unsigned a = 1; a <= 123456; a++)
	{
		str += 'x';
		tmp[a - 1] = 'x';
		ASSERT_EQ(str.Length(), a);
	}

	tmp[123456] = 0;
	EXPECT_EQ(str, tmp);
	EXPECT_EQ(strcmp(str.Data(), tmp), 0);
}
