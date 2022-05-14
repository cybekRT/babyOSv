#include"Video.hpp"
#include"FS/VFS.hpp"

namespace Video
{
	Driver* currentDriver = nullptr;
	Bitmap* screen = nullptr;

	/*
	 * Driver wrappers
	 */

	bool SetDriver(Driver* drv)
	{
		currentDriver = drv;

		return true;
	}

	void GetAvailableModes(List<Mode>& modes)
	{
		currentDriver->GetAvailableModes(modes);
	}

	Mode GetMode()
	{
		return currentDriver->GetMode();
	}

	bool SetMode(Mode mode)
	{
		auto res = currentDriver->SetMode(mode);

		if(res)
		{
			FreeBitmap(screen);
			CreateBitmap(mode.width, mode.height, &screen);
		}

		return res;
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

	bool Init()
	{
		return true;
	}

	Bitmap* GetScreen()
	{
		return screen;
	}

	void UpdateScreen()
	{
		for(unsigned y = 0; y < screen->height; y++)
		{
			for(unsigned x = 0; x < screen->width; x++)
			{
				currentDriver->SetPixel(x, y, screen->pixels[y * screen->width + x]);
			}
		}
	}

	void UpdateScreen(Rect r)
	{
		UpdateScreen(); // TODO
	}

	void CreateBitmap(u32 w, u32 h, Bitmap** bmp)
	{
		(*bmp) = (Bitmap*)new u8[sizeof(Bitmap) + w * h * sizeof(Color)];
		(*bmp)->width = w;
		(*bmp)->height = h;

		for(unsigned a = 0; a < w * h; a++)
			(*bmp)->pixels[a] = Color(255, 255, 255, 255);
	}

	void LoadBitmap(u8* path, Bitmap** bmp)
	{

	}

	void FreeBitmap(Bitmap* bmp)
	{
		if(!bmp)
			return;

		delete[] bmp;
	}

	void PutPixel(Bitmap* bmp, Point p, Color c)
	{
		bmp->pixels[p.y * bmp->width + p.x] = c;
	}

	void DrawLine(Bitmap* bmp, Point p1, Point p2, Color c)
	{

	}

	void DrawRect(Bitmap* bmp, Rect r, Color c)
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
		for(unsigned y = r.y; y < r.y + r.h; y++)
		{
			for(unsigned x = r.x; x < r.x + r.w; x++)
			{
				bmp->pixels[y * bmp->width + x] = c;
			}
		}
	}
}