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
	} __attribute__((packed));

	struct Point
	{
		s32 x, y;

		Point() : x(0), y(0) {}
		Point(s32 x, s32 y) : x(x), y(y) {}
	};

	struct Rect
	{
		s32 x, y;
		s32 w, h;

		Rect() : x(0), y(0), w(0), h(0) {}
		Rect(s32 x, s32 y, s32 w, s32 h) : x(x), y(y), w(w), h(h) {}
	};

	struct Bitmap
	{
		u32 width;
		u32 height;
		Color pixels[];
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

	bool Init();

	// Direct API wrapper
	bool SetDriver(Driver* drv);
	void GetAvailableModes(Container::LinkedList<Mode>& modes);
	Mode GetMode();
	bool SetMode(Mode mode);
	void Clear();
	Color GetPixel(u32 x, u32 y);
	void SetPixel(u32 x, u32 y, Color c);

	// High-level API
	Bitmap* GetScreen();
	void UpdateScreen();
	void UpdateScreen(Rect r);

	void CreateBitmap(u32 w, u32 h, Bitmap** bmp);
	void LoadBitmap(u8* path, Bitmap** bmp);
	void FreeBitmap(Bitmap* bmp);

	void PutPixel(Bitmap* bmp, Point p, Color c);
	void DrawLine(Bitmap* bmp, Point p1, Point p2, Color c);
	void DrawRect(Bitmap* bmp, Rect r, Color c);
}