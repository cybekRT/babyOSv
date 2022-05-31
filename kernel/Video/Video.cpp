#include"Video.hpp"
#include"FS/VFS.hpp"

namespace Video
{
	Driver* currentDriver = nullptr;
	Bitmap* screen = nullptr;

	bool Init(Driver* drv)
	{
		currentDriver = drv;

		return true;
	}

	/*
	 * Driver wrappers
	 */

	Array<Mode> GetAvailableModes()
	{
		return currentDriver->GetAvailableModes();
	}

	Mode GetCurrentMode()
	{
		return currentDriver->GetCurrentMode();
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

	/*
	 * Helpers
	 */

	void ClampRect(Rect& r, Rect bounds)
	{
		if(r.x < 0)
		{
			r.w += r.x;
			r.x = 0;
		}

		if(r.y < 0)
		{
			r.h += r.y;
			r.y = 0;
		}

		if(r.x + r.w > bounds.w)
			r.w = bounds.w - r.x;

		if(r.y + r.h > bounds.h)
			r.h = bounds.h - r.y;
	}

	/*
	 * Driver-independent logic
	 */

	Bitmap* GetScreen()
	{
		return screen;
	}

	void ClearScreen()
	{
		for(unsigned a = 0; a < screen->width * screen->height; a++)
		{
			screen->pixels[a] = Color(0, 0, 0);
		}
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

	struct BitmapFileHeader
	{
		char header[2];
		u32 totalSize;
		u16 _unused1;
		u16 _unused2;
		u32 dataOffset;
	} __attribute__((packed));

	struct BitmapCoreHeader
	{
		// 0E 	14 	4 	The size of this header (12 bytes)
		u32 size;
		// 12 	18 	2 	The bitmap width in pixels (unsigned 16-bit)
		u16 imageWidth;
		// 14 	20 	2 	The bitmap height in pixels (unsigned 16-bit)
		u16 imageHeight;
		// 16 	22 	2 	The number of color planes, must be 1
		u16 colorPlanes;
		// 18 	24 	2 	The number of bits per pixel
		u16 bitsPerPixel;
	} __attribute__((packed));

	struct BitmapInfoHeader
	{
		// Offset (hex) 	Offset (dec) 	Size (bytes) 	Windows BITMAPINFOHEADER[2]
		// 0E 	14 	4 	the size of this header, in bytes (40)
		u32		size;
		// 12 	18 	4 	the bitmap width in pixels (signed integer)
		s32		imageWidth;
		// 16 	22 	4 	the bitmap height in pixels (signed integer)
		s32		imageHeight;
		// 1A 	26 	2 	the number of color planes (must be 1)
		u16		colorPlanes;
		// 1C 	28 	2 	the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32.
		u16		bitsPerPixel;
		// 1E 	30 	4 	the compression method being used. See the next table for a list of possible values
		u32		compressionMethod;
		// 22 	34 	4 	the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps.
		u32		imageSize;
		// 26 	38 	4 	the horizontal resolution of the image. (pixel per metre, signed integer)
		s32		dpiHorizontal;
		// 2A 	42 	4 	the vertical resolution of the image. (pixel per metre, signed integer)
		s32		dpiVertical;
		// 2E 	46 	4 	the number of colors in the color palette, or 0 to default to 2n
		u32		colorsInPalette;
		// 32 	50 	4 	the number of important colors used, or 0 when every color is important; generally ignored
		u32		importantColors;
	} __attribute__((packed));

	bool LoadBitmap(String path, Bitmap** bmp)
	{
		u32 read;
		FS::File* f = nullptr;
		VFS::FileOpen(path.Data(), &f);
		if(!f)
		{
			Print("No file: %s\n", path.Data());
			return false;
		}

		BitmapFileHeader bmpHdr;
		VFS::FileRead(f, (u8*)&bmpHdr, sizeof(bmpHdr), &read);
		if(read != sizeof(bmpHdr))
		{
			Print("Invalid size: %d/%d\n", read, sizeof(bmpHdr));
			return false;
		}

		BitmapInfoHeader coreHdr;
		VFS::FileRead(f, (u8*)&coreHdr, sizeof(coreHdr), &read);
		if(read != sizeof(coreHdr))
		{
			Print("Invalid size: %d/%d\n", read, sizeof(coreHdr));
			return false;
		}

		Print("Header: %c%c\n", bmpHdr.header[0], bmpHdr.header[1]);
		Print("Total size: %d\n", bmpHdr.totalSize);
		Print("Offset: %d\n", bmpHdr.dataOffset);

		Print("Core header size: %d\n", coreHdr.size);
		Print("Image size: %dx%d\n", coreHdr.imageWidth, coreHdr.imageHeight);
		Print("Color planes: %d\n", coreHdr.colorPlanes);
		Print("Bits per pixel: %d\n", coreHdr.bitsPerPixel);

		if(coreHdr.bitsPerPixel != 24)
		{
			Print("Invalid bits per pixel: %d\n", coreHdr.bitsPerPixel);
			return false;
		}

		CreateBitmap(coreHdr.imageWidth, coreHdr.imageHeight, bmp);

		VFS::FileSetPointer(f, bmpHdr.dataOffset);
		for(unsigned a = 0; a < coreHdr.imageWidth * coreHdr.imageHeight; a++)
		{
			u8 col[3];
			static int cnt = 0;
			VFS::FileRead(f, (u8*)col, 3, &read);
			u32 offset;
			VFS::FileGetPointer(f, &offset);
			// Print("Offset: %d (%d) (#%d)\n", offset, cnt++, read);
			if(read != 3)
			{
				Print("Unepected end of file");
				return false;
			}

			u32 x = a % coreHdr.imageWidth;
			u32 y = a / coreHdr.imageWidth;
			u32 nativeY = coreHdr.imageHeight - 1 - y;
			(*bmp)->pixels[nativeY * coreHdr.imageWidth + x] = Color(col[0], col[1], col[2]);
		}

		VFS::FileClose(&f);
		return true;
	}

	void FreeBitmap(Bitmap* bmp)
	{
		if(!bmp)
			return;

		delete[] bmp;
	}

	void SetBlending(Bitmap* bmp, BlendingMethod method)
	{
		bmp->blending.method = method;
	}

	void SetBlendingStaticAlpha(Bitmap* bmp, u8 alpha)
	{
		bmp->blending.staticValue = alpha;
	}

	void PutPixel(Bitmap* bmp, Point p, Color c)
	{
		bmp->pixels[p.y * bmp->width + p.x] = c;
	}

	void DrawLine(Bitmap* bmp, Point p1, Point p2, Color c)
	{
		ASSERT(false, "Not implemented yet");
	}

	void DrawRect(Bitmap* bmp, Rect r, Color c)
	{
		Mode m = currentDriver->GetCurrentMode();

		if(r.x < -r.w || r.y < -r.h || r.x >= (int)m.width || r.y >= (int)m.height)
		{
			Print("Oopsie... %dx%d, %dx%d (%d %d %d %d)\n", r.x, r.y, r.w, r.h,
				r.x < -r.w, r.y < -r.h, r.x >= (int)m.width, r.y >= (int)m.height);
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

	void DrawBitmapNoBlending(Rect srcRect, Bitmap* src, Rect dstRect, Bitmap* dst, int sx, int sy, int dx, int dy)
	{
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

	void DrawBitmap(Rect srcRect, Bitmap* src, Rect dstRect, Bitmap* dst)
	{
		// TODO: clamp
		// ClampRect(srcRect)

		int sx = srcRect.x;
		int sy = srcRect.y;
		int dx = dstRect.x;
		int dy = dstRect.y;

		if(src->blending.method == BlendingMethod::None)
			DrawBitmapNoBlending(srcRect, src, dstRect, dst, sx, sy, dx, dy);
		else
		{
			bool printed = false;
			for(unsigned y = 0; y < srcRect.h; y++)
			{
				for(unsigned x = 0; x < srcRect.w; x++)
				{
					auto srcPix = src->pixels[sy * src->width + sx];
					auto dstPix = dst->pixels[dy * dst->width + dx];
					u8 srcAlpha = (src->blending.method == BlendingMethod::Static) ? src->blending.staticValue : srcPix.a;
					u8 dstAlpha = 255 - srcAlpha;

					if(srcAlpha == 0)
						continue;
					else if(srcAlpha == 255)
						dst->pixels[dy * dst->width + dx] = srcPix;
					else
					{
						u32 r, g, b;
						r = (srcAlpha * (u32)srcPix.r + dstAlpha * (u32)dstPix.r) / 255;
						g = (srcAlpha * (u32)srcPix.g + dstAlpha * (u32)dstPix.g) / 255;
						b = (srcAlpha * (u32)srcPix.b + dstAlpha * (u32)dstPix.b) / 255;

						if(!printed)
						{
						Print("Src: %d,%d,%d,%d, Dst: %d,%d,%d,%d, Final: %d,%d,%d\n",
							srcPix.r, srcPix.g, srcPix.b, srcAlpha,
							dstPix.r, dstPix.g, dstPix.b, dstAlpha,
							r, g, b);
						Print("  %d + %d = %d\n",
							srcAlpha * (u32)srcPix.g,
							dstAlpha * (u32)dstPix.g,
							srcAlpha * (u32)srcPix.g + dstAlpha * (u32)dstPix.g);
							printed = true;
						}

						dst->pixels[dy * dst->width + dx] = Color(r, g, b);
					}

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
}