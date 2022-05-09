#include<gtest/gtest.h>
// #include"FS/Path.hpp"
// #include"FS/Path.cpp"

// using Container::String;
// using FS::Path;

// TEST(Path, Root)
// {
// 	Path p;
// 	EXPECT_EQ(p.ToString(), "/");
// }

// TEST(Path, DirectoriesCount)
// {
// 	auto path = "/var/log/dmesg.txt";

// 	Path p(path);
// 	EXPECT_EQ(p.ToString(), path);
// 	EXPECT_EQ(p.paths.Size(), 3);
// 	EXPECT_EQ(p.paths[0], "var");
// 	EXPECT_EQ(p.paths[1], "log");
// 	EXPECT_EQ(p.paths[2], "dmesg.txt");
// }