#include"VGA.hpp"
#include"HAL.hpp"

#define	VGA_AC_INDEX		0x3C0
#define	VGA_AC_WRITE		0x3C0
#define	VGA_AC_READ		0x3C1
#define	VGA_MISC_WRITE		0x3C2
#define VGA_SEQ_INDEX		0x3C4
#define VGA_SEQ_DATA		0x3C5
#define	VGA_DAC_READ_INDEX	0x3C7
#define	VGA_DAC_WRITE_INDEX	0x3C8
#define	VGA_DAC_DATA		0x3C9
#define	VGA_MISC_READ		0x3CC
#define VGA_GC_INDEX 		0x3CE
#define VGA_GC_DATA 		0x3CF
/*			COLOR emulation		MONO emulation */
#define VGA_CRTC_INDEX		0x3D4		/* 0x3B4 */
#define VGA_CRTC_DATA		0x3D5		/* 0x3B5 */
#define	VGA_INSTAT_READ		0x3DA

#define	VGA_NUM_SEQ_REGS	5
#define	VGA_NUM_CRTC_REGS	25
#define	VGA_NUM_GC_REGS		9
#define	VGA_NUM_AC_REGS		21
#define	VGA_NUM_REGS		(1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
				VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

VGA::CRTC_HorizontalTotal crtc0(0x5F);
VGA::CRTC_EndHorizontalDisplay crtc1(0x4F);
VGA::CRTC_StartHorizontalBlanking crtc2(0x50);
VGA::CRTC_EndHorizontalBlanking crtc3(true, 0, 2);
VGA::CRTC_StartHorizontalRetrace crtc4(0x54);
VGA::CRTC_EndHorizontalRetrace crtc5(32, 0, 0);
VGA::CRTC_VerticalTotal crtc6(0xBF);
VGA::CRTC_OverflowRegister crtc7(256, 256, 256, 256, 256);
VGA::CRTC_PresetRowScanRegister crtc8(0, 0);
VGA::CRTC_MaximumScanLineRegister crtc9(false, 512, 0, 1);
VGA::CRTC_CursorStartRegister crtc10(false, 0);
VGA::CRTC_CursorEndRegister crtc11(0, 0);
VGA::CRTC_StartAddressHighRegister crtc12(0x00);
VGA::CRTC_StartAddressLowRegister crtc13(0x00);
VGA::CRTC_CursorLocationHighRegister crtc14(0x00);
VGA::CRTC_CursorLocationLowRegister crtc15(0x00);
VGA::CRTC_VerticalRetraceStartRegister crtc16(0x9C);
VGA::CRTC_VerticalRetraceEndRegister crtc17(0, 0, 14);
VGA::CRTC_VerticalDisplayEndRegister crtc18(0x8F);
VGA::CRTC_OffsetRegister crtc19(0x28);
VGA::CRTC_UnderlineLocationRegister crtc20(true, false, 0);
VGA::CRTC_VerticalBlankingStartRegister crtc21(0x96);
VGA::CRTC_VerticalBlankingEndRegister crtc22(0xB9);
VGA::CRTC_ModeControlRegister crtc23(true, false, true, false, false, true, true);
VGA::CRTC_LineCompareRegister crtc24(0xFF);

unsigned char g_40x25_text[] =
{
/* MISC */
	0x67,
/* SEQ */
	0x03, 0x08, 0x03, 0x00, 0x02,
/* CRTC */
	0x2D, 0x27, 0x28, 0x90, 0x2B, 0xA0, 0xBF, 0x1F,
	0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0xA0,
	0x9C, 0x8E, 0x8F, 0x14, 0x1F, 0x96, 0xB9, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00,
};

unsigned char g_80x25_text[] =
{
/* MISC */
	0x67,
/* SEQ */
	0x03, 0x00, 0x03, 0x00, 0x02,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
	0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
	0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00
};

unsigned char g_320x200x256[] =
{
/* MISC */
	0x63,
/* SEQ */
	0x03, 0x01, 0x0F, 0x00, 0x0E,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
	0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x9C, 0x0E, 0x8F, 0x28,	0x40, 0x96, 0xB9, 0xA3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x41, 0x00, 0x0F, 0x00,	0x00
};

// MISC
VGA::MiscOutput mo
{
	.vSyncPolarity = 0,
	.hSyncPolarity = 1,
	.oddEvenPageSelect = 1,
	.clockSelect = VGA::MiscOutput::Clock_25MHz,
	.ramEnabled = 1,
	.inputOutputAddressSelect = 1,
};

VGA::GC_00 gc00
{
	.setResetValue =
	{
		0, 0, 0, 0
	}
};

VGA::GC_01 gc01
{
	.setResetEnable =
	{
		0, 0, 0, 0
	}
};

VGA::GC_02 gc02
{
	.memoryPlaneWriteEnable =
	{
		0, 0, 0, 0
	}
};

VGA::GC_03 gc03
{
	.logicalOperation = VGA::GC_03::LO_Normal,
	.rotateCount = 0
};

VGA::GC_04 gc04
{
	.mapSelect = 0
};

VGA::GC_05 gc05
{
	.readMode = VGA::GC_05::RM_0,
	.writeMode = VGA::GC_05::WM_0,
	.shiftColor256 = 1,
	.shiftInterleaved = 0,
	.hostOddEven = 0,
};

VGA::GC_06 gc06
{
	.alphanumericModeDisabled = 1,
	.chainOE = 0,
	.memoryMapSelect = 0b01,
};

VGA::GC_07 gc07
{
	.colorDontCare =
	{
		1, 1, 1, 1
	},
};

//0xFF
VGA::GC_08 gc08
{
	.bitMask = 0xff,
};

void write_regs(unsigned char *regs)
{
	unsigned i;

/* write MISCELLANEOUS reg */
	//HAL::Out8(VGA_MISC_WRITE, *regs);
	//VGA::Write_3C2(*regs);

	mo.Write();

	regs++;
/* write SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		//HAL::Out8(VGA_SEQ_INDEX, i);
		//HAL::Out8(VGA_SEQ_DATA, *regs);
		VGA::Write_3C4(i, *regs);
		regs++;
	}
/* unlock CRTC registers */
	HAL::Out8(VGA_CRTC_INDEX, 0x03);
	HAL::Out8(VGA_CRTC_DATA, HAL::In8(VGA_CRTC_DATA) | 0x80);
	HAL::Out8(VGA_CRTC_INDEX, 0x11);
	HAL::Out8(VGA_CRTC_DATA, HAL::In8(VGA_CRTC_DATA) & ~0x80);
/* make sure they remain unlocked */
	regs[0x03] |= 0x80;
	regs[0x11] &= ~0x80;
/* write CRTC regs */
	crtc0.Write();
	crtc1.Write();
	crtc2.Write();
	crtc3.Write();
	crtc4.Write();
	crtc5.Write();
	crtc6.Write();
	crtc7.Write();
	crtc8.Write();
	crtc9.Write();
	crtc10.Write();
	crtc11.Write();
	crtc12.Write();
	crtc13.Write();
	crtc14.Write();
	crtc15.Write();
	crtc16.Write();
	crtc17.Write();
	crtc18.Write();
	crtc19.Write();
	crtc20.Write();
	crtc21.Write();
	crtc22.Write();
	crtc23.Write();
	crtc24.Write();
	Print("\n----------\n");
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		//HAL::Out8(VGA_CRTC_INDEX, i);
		//HAL::Out8(VGA_CRTC_DATA, *regs);
		// VGA::Write_3D4(i, *regs);
		Print("3D4: [%d] <- %x\t", (int)i, (int)*regs);
		if(i%3 == 2)
			Print("\n");
		regs++;
	}

/* write GRAPHICS CONTROLLER regs */
		gc00.Write();
		gc01.Write();
		gc02.Write();
		gc03.Write();
		gc04.Write();
		gc05.Write();
		gc06.Write();
		gc07.Write();
		gc08.Write();
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		//VGA::Write_3CE(i, *regs);
		//HAL::Out8(VGA_GC_INDEX, i);
		//HAL::Out8(VGA_GC_DATA, *regs);
		regs++;
	}
/* write ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		//r(void)HAL::In8(VGA_INSTAT_READ);
		//HAL::Out8(VGA_AC_INDEX, i);
		//HAL::Out8(VGA_AC_WRITE, *regs);

		VGA::Write_3C0(i, *regs);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)HAL::In8(VGA_INSTAT_READ);
	HAL::Out8(VGA_AC_INDEX, 0x20);
}

static void set_plane(unsigned p)
{
	unsigned char pmask;

	p &= 3;
	pmask = 1 << p;
/* set read plane */
	HAL::Out8(VGA_GC_INDEX, 4);
	HAL::Out8(VGA_GC_DATA, p);
/* set write plane */
	HAL::Out8(VGA_SEQ_INDEX, 2);
	HAL::Out8(VGA_SEQ_DATA, pmask);
}

namespace VGA
{
	u8 Read_3C0(u8 index)
	{
		HAL::In8(0x3DA);
		HAL::Out8(0x3C0, index);
		return HAL::In8(0x3C1);
	}

	void Write_3C0(u8 index, u8 value)
	{
		// return;

		HAL::In8(0x3DA);
		HAL::Out8(0x3C0, index);
		HAL::Out8(0x3C0, value);
	}

	u8 Read_3C2()
	{
		return HAL::In8(0x3CC);
	}

	void Write_3C2(u8 value)
	{
		// return;

		HAL::Out8(0x3C2, value);
	}

	u8 Read_3C4(u8 index)
	{
		HAL::Out8(0x3C4, index);
		return HAL::In8(0x3C5);
	}

	void Write_3C4(u8 index, u8 value)
	{
		// return;

		HAL::Out8(0x3C4, index);
		return HAL::Out8(0x3C5, value);
	}

	// DAC
	void Read_3C8(u8 index, u8* r, u8* g, u8* b)
	{
		HAL::Out8(0x3C8, index);
		*r = HAL::In8(0x3C9);
		*g = HAL::In8(0x3C9);
		*b = HAL::In8(0x3C9);
	}

	void Write_3C8(u8 index, u8 r, u8 g, u8 b)
	{
		// return;

		HAL::Out8(0x3C8, index);
		HAL::Out8(0x3C9, r);
		HAL::Out8(0x3C9, g);
		HAL::Out8(0x3C9, b);
	}

	u8 Read_3CE(u8 index)
	{
		HAL::Out8(0x3CE, index);
		return HAL::In8(0x3CF);
	}

	void Write_3CE(u8 index, u8 value)
	{
		// Print("Reg: %d, Val: %x\n", index, value);
		// return;

		HAL::Out8(0x3CE, index);
		HAL::Out8(0x3CF, value);
	}

	u8 Read_3D4(u8 index)
	{
		// Enable register
		// u8 v = Read_3C2();
		// v |= 1;
		// Write_3C2(v);

		HAL::Out8(0x3D4, index);
		return HAL::In8(0x3D5);
	}

	void Write_3D4(u8 index, u8 value)
	{
		// Enable register
		// u8 v = Read_3C2();
		// v |= 1;
		// Write_3C2(v);
		
		// static int yolo = 0;
		// Print("3D4: [%d] <- %x\t", (int)index, (int)value);
		// if((++yolo) % 3 == 2)
		// 	Print("\n");

		// return;

		HAL::Out8(0x3D4, index);
		HAL::Out8(0x3D5, value);
	}

	/*
	 * Horizontal timing
	 */

	union EndHorizontalBlankingRegister
	{
		u8 value;
		struct
		{
			u8 blankingEnd0_4 : 5;
			u8 displaySkew : 2;
			u8 _unused1 : 1;
		};
	};

	union EndHorizontalRetraceRegister
	{
		u8 value;
		struct
		{
			u8 retraceEnd : 5;
			u8 _unused2 : 2;
			u8 blankingEnd5 : 1;
		};
	};

	void HorizontalTiming::Read()
	{
		EndHorizontalBlankingRegister ehbr;
		EndHorizontalRetraceRegister ehrr;

		total = Read_3D4(0x00);
		displayEnd = Read_3D4(0x01);
		blankingStart = Read_3D4(0x02);
		ehbr.value = Read_3D4(0x03);
		retraceStart = Read_3D4(0x04);
		ehrr.value = Read_3D4(0x05);

		displaySkew = ehbr.displaySkew;
		blankingEnd = (ehbr.blankingEnd0_4 | (ehrr.blankingEnd5 << 5));
		retraceEnd = ehrr.retraceEnd;
	}

	void HorizontalTiming::Write()
	{
		EndHorizontalBlankingRegister ehbr;
		ehbr.displaySkew = displaySkew;
		ehbr.blankingEnd0_4 = (blankingEnd & 0b11111);

		EndHorizontalRetraceRegister ehrr;
		ehrr.blankingEnd5 = (blankingEnd >> 5);
		ehrr.retraceEnd = retraceEnd;

		Write_3D4(0x00, total);
		Write_3D4(0x01, displayEnd);
		Write_3D4(0x02, blankingStart);
		Write_3D4(0x03, ehbr.value);
		Write_3D4(0x04, retraceStart);
		Write_3D4(0x05, ehrr.value);
	}

	/*
	 * Vertical timing
	 */

	union OverflowRegister
	{
		u8 value;
		struct
		{
			u8 total8 : 1;
			u8 displayEnd8 : 1;
			u8 retraceStart8 : 1;
			u8 blankingStart8 : 1;
			u8 _unused1 : 1;
			u8 total9 : 1;
			u8 displayEnd9 : 1;
			u8 retraceStart9 : 1;
		};
	};

	union MaximumScanLineRegister
	{
		u8 value;
		struct
		{
			u8 _unused1 : 5;
			u8 blankingStart9 : 1;
			u8 _unused2 : 2;
		};
	};

	union VerticalRetraceEndRegister
	{
		u8 value;
		struct
		{
			u8 retraceEnd : 4;
			u8 _unused1 : 4;
		};
	};

	void VerticalTiming::Read()
	{
		OverflowRegister oreg;
		MaximumScanLineRegister mslr;
		VerticalRetraceEndRegister vrer;
		u16 total0_7;
		u16 retraceStart0_7;
		u16 displayEnd0_7;
		u16 blankingStart0_7;

		total0_7 = Read_3D4(0x06);
		oreg.value = Read_3D4(0x07);
		mslr.value = Read_3D4(0x09);
		retraceStart0_7 = Read_3D4(0x06);
		vrer.value = Read_3D4(0x11);
		displayEnd0_7 = Read_3D4(0x12);
		blankingStart0_7 = Read_3D4(0x15);
		blankingEnd = Read_3D4(0x16);

		total = (total0_7) | (oreg.total8 << 8) | (oreg.total9 << 9);
		displayEnd = (displayEnd0_7) | (oreg.displayEnd8 << 8) | (oreg.displayEnd9 << 9);
		blankingStart = (blankingStart0_7) | (oreg.blankingStart8 << 8);
		blankingEnd;
		retraceStart = (retraceStart0_7) | (oreg.retraceStart8 << 8) | (oreg.retraceStart9 << 9);
		retraceEnd;
	}

	void VerticalTiming::Write()
	{
		OverflowRegister oreg;
		MaximumScanLineRegister mslr;
		VerticalRetraceEndRegister vrer;

		oreg.value = Read_3D4(0x07);
		mslr.value = Read_3D4(0x09);
		vrer.value = Read_3D4(0x16);

		oreg.retraceStart8 = !!(retraceStart & 0x100);
		oreg.retraceStart9 = !!(retraceStart & 0x200);
		oreg.displayEnd8 = !!(displayEnd & 0x100);
		oreg.displayEnd9 = !!(displayEnd & 0x200);
		oreg.total8 = !!(total & 0x100);
		oreg.total9 = !!(total & 0x200);
		oreg.blankingStart8 = !!(blankingStart & 0x100);

		mslr.blankingStart9 = !!(blankingStart & 0x200);

		vrer.retraceEnd = retraceEnd;

		Write_3D4(0x06, total & 0xff);
		Write_3D4(0x07, oreg.value);
		Write_3D4(0x09, mslr.value);
		Write_3D4(0x10, retraceStart & 0xff);
		Write_3D4(0x11, vrer.value);
		Write_3D4(0x12, displayEnd & 0xff);
		Write_3D4(0x15, blankingStart);
		Write_3D4(0x16, blankingEnd & 0x7f);
	}

	/*
	 * Init
	 */

	bool Init()
	{
		// http://www.osdever.net/FreeVGA/vga/vga.htm#register
		// https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
		// http://xkr47.outerspace.dyndns.org/progs/mode%2013h%20without%20using%20bios.htm
		// https://01.org/sites/default/files/documentation/ilk_ihd_os_vol3_part1r2_0.pdf
		// https://www.amazon.com/dp/0201624907

		// Enable reg 3D4
		u8 v = Read_3C2();
		v |= 1;
		Write_3C2(v);

		// Disable 3D4 write protect
		v = Read_3D4(0x11);
		v &= ~0x80;
		Write_3D4(0x11, v);

		write_regs(g_320x200x256);
		

		for(unsigned a = 0; a < 256; a++)
		{
			// RGB323
			u8 r = ((a >> 5) & 0b111) << 3;
			u8 g = ((a >> 3) & 0b011) << 4;
			u8 b = ((a >> 0) & 0b111) << 3;

			Write_3C8(a, r, g, b);

			// u32 v = a << 2;
			// Write_3C8(a, v, v, v);
		}



		return true;
	}

	void SetCursor(bool enabled)
	{
		// TODO: lol
		Write_3D4(0xA, 0b00100000);
	}
}
