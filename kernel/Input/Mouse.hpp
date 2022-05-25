#include"PS2.hpp"

namespace Mouse
{
	bool Init();

	void FIFOAddCmd(u8 v);
	void FIFOAddData(u8 v);
}