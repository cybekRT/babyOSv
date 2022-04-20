#pragma once

#include"Container/LinkedList.hpp"

namespace Video
{
	struct Mode
	{
		enum class Type
		{
			Text,
			Graphical
		};

		Type type;
		u32 width;
		u32 height;
		u8 bpp;
	};

	struct Color
	{
		u8 r, g, b;
		u8 a;

		Color(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a)
		{

		}
	};

	struct Rect
	{
		s32 x, y;
		s32 w, h;

		Rect() : x(0), y(0), w(0), h(0) {}
		Rect(s32 x, s32 y, s32 w, s32 h) : x(x), y(y), w(w), h(h) {}
	};

	struct Driver
	{
		void (*GetAvailableModes)(Container::LinkedList<Mode>& modes);
		Mode (*GetMode)();
		bool (*SetMode)(Mode mode);

		void (*Clear)();
		Color (*GetPixel)(u32 x, u32 y);
		void (*SetPixel)(u32 x, u32 y, Color c);
	};

	bool SetDriver(Driver* drv);
	void GetAvailableModes(Container::LinkedList<Mode>& modes);
	Mode GetMode();
	bool SetMode(Mode mode);
	void Clear();
	Color GetPixel(u32 x, u32 y);
	void SetPixel(u32 x, u32 y, Color c);

	void DrawRect(Rect r, Color c);
}