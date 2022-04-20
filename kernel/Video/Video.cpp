#include"Video.hpp"

namespace Video
{
	Driver* currentDriver = nullptr;

	/*
	 * Driver wrappers
	 */

	bool SetDriver(Driver* drv)
	{
		currentDriver = drv;

		return true;
	}

	void GetAvailableModes(Container::LinkedList<Mode>& modes)
	{
		currentDriver->GetAvailableModes(modes);
	}

	Mode GetMode()
	{
		return currentDriver->GetMode();
	}

	bool SetMode(Mode mode)
	{
		return currentDriver->SetMode(mode);
	}

	void Clear()
	{
		currentDriver->Clear();
	}

	Color GetPixel(u32 x, u32 y)
	{
		return currentDriver->GetPixel(x, y);
	}

	void SetPixel(u32 x, u32 y, Color c)
	{
		currentDriver->SetPixel(x, y, c);
	}

	/*
	 * Driver-independent logic
	 */

	void DrawRect(Rect r, Color c)
	{
		Mode m = currentDriver->GetMode();

		Print("Mode size: %dx%d\n", m.width, m.height);

		if(r.x < -r.w || r.y < -r.h || r.x >= m.width || r.y >= m.height)
		{
			Print("Oopsie... %dx%d, %dx%d (%d %d %d %d)\n", r.x, r.y, r.w, r.h,
				r.x < -r.w, r.y < -r.h, r.x >= m.width, r.y >= m.height);
			return;
		}

		if(r.x < 0)
		{
			r.w += r.x;
			r.x = 0;
		}

		if(r.y < 0)
		{
			r.h += r.h;
			r.y = 0;
		}

		if(r.x + r.w > m.width)
			r.w = m.width - r.x;

		if(r.y + r.h > m.height)
			r.h = m.height - r.y;

		Print("Drawing rect: %dx%d - %dx%d\n", r.x, r.y, r.w, r.h);

		auto f = currentDriver->SetPixel;

		for(unsigned y = r.y; y < r.y + r.h; y++)
		{
			for(unsigned x = r.x; x < r.x + r.w; x++)
			{
				f(x, y, c);
			}
		}
	}
}