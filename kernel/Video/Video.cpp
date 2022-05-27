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
		for(unsigned y = 0; y < screen->height; y++)
		{
			for(unsigned x = 0; x < screen->width; x++)
			{
				screen->pixels[y * screen->width + x] = Color(0, 0, 0);
			}
		}
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
	 * Helpers
	 */

	void ClampRect(Rect& r, Rect bounds)
	{
		Print("Clamping (%dx%d, %dx%d) ->", r.x, r.y, r.w, r.h);

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

		if(r.x + r.w > bounds.w)
			r.w = bounds.w - r.x;

		if(r.y + r.h > bounds.h)
			r.h = bounds.h - r.y;

		Print("(%dx%d, %dx%d)\n", r.x, r.y, r.w, r.h);
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
		currentDriver->UpdateBuffer(screen, Rect(0, 0, screen->width, screen->height));
	}

	void UpdateScreen(Rect r)
	{
		ClampRect(r, Rect(0, 0, screen->width, screen->height));
		currentDriver->UpdateBuffer(screen, r);
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

		if(r.x < -r.w || r.y < -r.h || r.x >= m.width || r.y >= m.height)
		{
			Print("Oopsie... %dx%d, %dx%d (%d %d %d %d)\n", r.x, r.y, r.w, r.h,
				r.x < -r.w, r.y < -r.h, r.x >= m.width, r.y >= m.height);
			return;
		}

		ClampRect(r, Video::Rect(0, 0, m.width, m.height));

		for(unsigned y = r.y; y < r.y + r.h; y++)
		{
			for(unsigned x = r.x; x < r.x + r.w; x++)
			{
				bmp->pixels[y * bmp->width + x] = c;
			}
		}
	}

	void DrawBitmap(Rect srcRect, Bitmap* src, Rect dstRect, Bitmap* dst)
	{
		// TODO: clamp
		// ClampRect(srcRect)

		int sx = srcRect.x;
		int sy = srcRect.y;
		int dx = dstRect.x;
		int dy = dstRect.y;

		for(unsigned y = 0; y < srcRect.h; y++)
		{
			for(unsigned x = 0; x < srcRect.w; x++)
			{
				dst->pixels[dy * dst->width + dx] = src->pixels[sy * src->width + sx];

				sx++;
				dx++;
			}

			sx = srcRect.x;
			dx = dstRect.x;
			sy++;
			dy++;
		}
	}
}