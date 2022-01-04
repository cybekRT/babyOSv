namespace GFX
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

	bool Init();

	void Clear();
	Color GetPixel(u32 x, u32 y);
	void SetPixel(u32 x, u32 y, Color c);
}