#include<Container/LinkedList.h>

class Path
{
protected:
	LinkedList<char*> paths;
public:
	static const u32 MaxLength = 255;

	Path();
	~Path();

	void Add(char* dir);
	void GoUp();
}