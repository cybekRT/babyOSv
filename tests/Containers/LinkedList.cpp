#include<gtest/gtest.h>
#include"Container/LinkedList.hpp"

Mutex::Mutex() {}
Mutex::~Mutex() {}
void Mutex::Lock() {}
void Mutex::Unlock() {}

#define ContainerType LinkedList
#include"IContainer.hpp"
