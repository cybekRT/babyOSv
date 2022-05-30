#include"Video.hpp"
#include"Video_VGA.hpp"
#include"VGA.hpp"

using namespace Video;

namespace VGA
{
	static Mode modeText = {
		.type = Mode::Type::Text,
		.width = 80,
		.height = 25,
		.bpp = 0,
	};

	static Mode modeGfx = {
		.type = Mode::Type::Graphical,
		.width = 320,
		.height = 200,
		.bpp = 8,
	};

	static u8* buffer = (u8*)0x800a0000;

	static Mode currentMode = modeText;

	static Array<Mode> GetAvailableModes()
	{
		Array<Mode> modes;
		modes.PushBack(modeText);
		modes.PushBack(modeGfx);
		return modes;
	}

	static Mode GetCurrentMode()
	{
		return currentMode;
	}

	static bool SetMode(Mode mode)
	{
		currentMode = mode;

		if(mode.type == Mode::Type::Text)
		{
			return false;
		}
		else
		{
			return VGA::Init();
		}
	}

	// static void Clear()
	// {
	// 	for(unsigned a = 0; a < 320*200; a++)
	// 	{
	// 		buffer[a] = RGB2VGA(0, 0, 0);
	// 	}
	// }

	// static Color GetPixel(u32 x, u32 y)
	// {
	// 	unsigned a = y * 320 + x;
	// 	u8 cols[] = VGA2RGB(buffer[a]);

	// 	return Color(cols[0], cols[1], cols[2]);
	// }

	// static void SetPixel(u32 x, u32 y, Color c)
	// {
	// 	unsigned a = y * 320 + x;

	// 	if(c.a == 255)
	// 	{
	// 		buffer[a] = RGB2VGA(c.r, c.g, c.b);
	// 	}
	// 	else
	// 	{
	// 		Color oc = VGA2RGB(buffer[a]);
	// 		oc.a = 255 - c.a;

	// 		u32 r = (c.r * c.a + oc.r * oc.a) / 255;
	// 		u32 g = (c.g * c.a + oc.g * oc.a) / 255;
	// 		u32 b = (c.b * c.a + oc.b * oc.a) / 255;

	// 		buffer[a] = RGB2VGA(r, g, b);
	// 	}
	// }

	static void UpdateBuffer(const Bitmap* bmp, const Rect& rect)
	{
		ASSERT(bmp->width == currentMode.width, "Invalid buffer width");
		ASSERT(bmp->height == currentMode.height, "Invalid buffer height");

		ASSERT(rect.x + rect.w <= currentMode.width, "Invalid rect width");
		ASSERT(rect.y + rect.h <= currentMode.height, "Invalid rect height");

		for(unsigned y = rect.y; y < rect.y + rect.h; y++)
		{
			for(unsigned x = rect.x; x < rect.x + rect.w; x++)
			{
				// SetPixel(x, y, bmp->pixels[y * bmp->width + x]);
				auto c = bmp->pixels[y * bmp->width + x];
				buffer[y * currentMode.width + x] = RGB2VGA(c.r, c.g, c.b);
			}
		}
	}
}

namespace Video
{
	Driver vgaDriver = {
		.GetAvailableModes = VGA::GetAvailableModes,
		.GetCurrentMode = VGA::GetCurrentMode,
		.SetMode = VGA::SetMode,
		.UpdateBuffer = VGA::UpdateBuffer,
	};
}