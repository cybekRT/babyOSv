#pragma once

#include"Containers/Array.hpp"
#include"Containers/String.hpp"

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

	enum class BlendingMethod
	{
		None,
		Static,
		Alpha
	};

	struct BlendingMode
	{
		BlendingMethod method;
		u8 staticValue;
	};

	struct Bitmap
	{
		u32 width;
		u32 height;
		BlendingMode blending;
		Color pixels[];
	};

	struct Driver
	{
		Array<Mode> (*GetAvailableModes)();
		Mode (*GetCurrentMode)();
		bool (*SetMode)(Mode mode);

		void (*UpdateBuffer)(const Bitmap*, const Rect& rect);
	};

	bool Init(Driver* drv);

	// Direct API wrapper
	Array<Mode> GetAvailableModes();
	Mode GetCurrentMode();
	bool SetMode(Mode mode);

	// High-level API
	Bitmap* GetScreen();
	void ClearScreen();
	void UpdateScreen();
	void UpdateScreen(Rect r);

	void CreateBitmap(u32 w, u32 h, Bitmap** bmp);
	bool LoadBitmap(String path, Bitmap** bmp);
	void FreeBitmap(Bitmap* bmp);

	void SetBlending(Bitmap* bmp, BlendingMethod method);
	void SetBlendingStaticAlpha(Bitmap* bmp, u8 alpha);

	void PutPixel(Bitmap* bmp, Point p, Color c);
	void DrawLine(Bitmap* bmp, Point p1, Point p2, Color c);
	void DrawRect(Bitmap* bmp, Rect r, Color c);
	void DrawBitmap(Rect srcRect, Bitmap* src, Rect dstRect, Bitmap* dst);
}