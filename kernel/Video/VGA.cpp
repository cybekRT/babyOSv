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
#include"VGA_regs.hpp"
VGA::Registers::CRTC_HorizontalTotal crtc0(0x5F);
VGA::Registers::CRTC_EndHorizontalDisplay crtc1(0x4F);
VGA::Registers::CRTC_StartHorizontalBlanking crtc2(0x50);
VGA::Registers::CRTC_EndHorizontalBlanking crtc3(true, 0, 2);
VGA::Registers::CRTC_StartHorizontalRetrace crtc4(0x54);
VGA::Registers::CRTC_EndHorizontalRetrace crtc5(32, 0, 0);
VGA::Registers::CRTC_VerticalTotal crtc6(0xBF);
VGA::Registers::CRTC_OverflowRegister crtc7(256, 256, 256, 256, 256);
VGA::Registers::CRTC_PresetRowScanRegister crtc8(0, 0);
VGA::Registers::CRTC_MaximumScanLineRegister crtc9(false, 512, 0, 1);
VGA::Registers::CRTC_CursorStartRegister crtc10(false, 0);
VGA::Registers::CRTC_CursorEndRegister crtc11(0, 0);
VGA::Registers::CRTC_StartAddressHighRegister crtc12(0x00);
VGA::Registers::CRTC_StartAddressLowRegister crtc13(0x00);
VGA::Registers::CRTC_CursorLocationHighRegister crtc14(0x00);
VGA::Registers::CRTC_CursorLocationLowRegister crtc15(0x00);
VGA::Registers::CRTC_VerticalRetraceStartRegister crtc16(0x9C);
VGA::Registers::CRTC_VerticalRetraceEndRegister crtc17(0, 0, 14);
VGA::Registers::CRTC_VerticalDisplayEndRegister crtc18(0x8F);
VGA::Registers::CRTC_OffsetRegister crtc19(0x28);
VGA::Registers::CRTC_UnderlineLocationRegister crtc20(true, false, 0);
VGA::Registers::CRTC_VerticalBlankingStartRegister crtc21(0x96);
VGA::Registers::CRTC_VerticalBlankingEndRegister crtc22(0xB9);
VGA::Registers::CRTC_ModeControlRegister crtc23(true, false, true, false, false, true, true);
VGA::Registers::CRTC_LineCompareRegister crtc24(0xFF);

#include"VGA_regs_crtc.hpp"
VGA::Registers::CRTC crtc = {
	.horizontalTotal = 0x5F,
	.horizontalDisplayEnd = 0x4F,
	.horizontalBlankingStart = 0x50,
	.enableVerticalRetraceAccess = true,
	.displayEnableSkew = false,
	.horizontalBlankingEnd =  34,
	.horizontalRetraceStart = 0x54,
	.horizontalRetraceSkew = false,
	.horizontalRetraceEnd = 0,
	.verticalTotal = 256 + 0xBF,
	.bytePanning = 0,
	.presetRowScan = 0,
	.scanDoubling = false,
	.maximumScanLine = 1,
	.cursorDisabled = false,
	.cursorScanLineStart = 0,
	.cursorSkew = false,
	.cursorScanLineEnd = 0,
	.startAddress = 0,
	.cursorLocation = 0,
	.verticalRetraceStart = 256 + 0x9C,
	.protectEnabled = false,
	.memoryRefreshBandwidth = 0,
	.verticalRetraceEnd = 14,
	.verticalDisplayEnd = 256 + 0x8F,
	.offset = 0x28,
	.doubleWordAddressing = true,
	.divideMemoryAddressClockBy4 = false,
	.underlineLocation = 0,
	.verticalBlankingStart = 256 + 0x96,
	.verticalBlankingEnd = 0xB9,
	.syncEnabled = true,
	.wordByteMode = false,
	.addressWrapSelect = true,
	.divideMemoryAddressClockBy2 = false,
	.divideScanLineClockBy2 = false,
	.mapDisplayAddress13 = true,
	.mapDisplayAddress14 = true,
	.lineCompare = 512 + 256 + 255,
};

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

VGA::Seq_00 seq0 = {
	.syncReset = true,
	.asyncReset = true
};

VGA::Seq_01 seq1 = {
	.screenDisabled = true,
	.shift4Enabled = false,
	.dotClockRate = false,
	.shiftLoadRate = false,
	.dot8Mode = false,
};

VGA::Seq_02 seq2 = {
	.memoryPlaneWriteEnabled = { 1, 1, 1, 1 }
};

VGA::Seq_03 seq3 = {
	.characterSetASelect = 0,
	.characterSetBSelect = 0,
};

VGA::Seq_04 seq4 = {
	.chain4Enabled = true,
	.oddEvenHostMemoryWriteAddressingDisabled = true,
	.extendedMemory = true
};

/*
	0x00,	0x00
	*/

VGA::ATTR_PaletteRegister attrPalette = {
	.paletteIndex = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
};

VGA::ATTR_ModeControlRegister attrModeControl = {
	.paletteBits54Select = false,
	.color8BitEnabled = true,
	.pixelPanningMode = false,
	.blinkEnabled = false,
	.lineGraphicsEnabled = false,
	.monochromeEmulation = false,
	.attributeControllerGraphicsEnabled = true
};

VGA::ATTR_OverscanColorRegister attrOverscanColor = {
	.overscanPaletteIndex = 0
};

VGA::ATTR_ColorPlaneEnableRegister attrColorPlaneEnable = {
	.planeEnabled = { true, true, true, true }
};

VGA::ATTR_HorizontalPixelPanningRegister attrPixelPanning = {
	.pixelShiftCount = 0
};

VGA::ATTR_ColorSelectRegister attrColorSelect = {
	.colorSelect76 = false,
	.colorSelect54 = false
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

VGA::GC_SetResetRegister gc00
{
	.setResetValue =
	{
		0, 0, 0, 0
	}
};

VGA::GC_EnableSetResetRegister gc01
{
	.setResetEnable =
	{
		0, 0, 0, 0
	}
};

VGA::GC_ColorCompareRegister gc02
{
	.memoryPlaneWriteEnable =
	{
		0, 0, 0, 0
	}
};

VGA::GC_DataRotateRegister gc03
{
	.logicalOperation = VGA::GC_DataRotateRegister::LO_Normal,
	.rotateCount = 0
};

VGA::GC_ReadMapSelectRegister gc04
{
	.mapSelect = 0
};

VGA::GC_GraphicsModeRegister gc05
{
	.readMode = VGA::GC_GraphicsModeRegister::RM_0,
	.writeMode = VGA::GC_GraphicsModeRegister::WM_0,
	.shiftColor256 = 1,
	.shiftInterleaved = 0,
	.hostOddEven = 0,
};

VGA::GC_MiscellaneousGraphicsRegister gc06
{
	.alphanumericModeDisabled = 1,
	.chainOE = 0,
	.memoryMapSelect = 0b01,
};

VGA::GC_ColorDontCareRegister gc07
{
	.colorDontCare =
	{
		1, 1, 1, 1
	},
};

VGA::GC_BitMaskRegister gc08
{
	.bitMask = 0xff,
};

void write_regs(unsigned char *regs)
{
	mo.Write();

	seq0.Write();
	seq1.Write();
	seq2.Write();
	seq3.Write();
	seq4.Write();

	// Unblock CRTC registers
	VGA::Registers::CRTC_EndHorizontalBlanking accessRetraceRegs;
	accessRetraceRegs.Read();
	accessRetraceRegs.enableVerticalRetraceAccess = true;
	accessRetraceRegs.Write();

	VGA::Registers::CRTC_VerticalRetraceEndRegister accessTimingRegs;
	accessTimingRegs.Read();
	accessTimingRegs.protectEnabled = false;
	accessTimingRegs.Write();

	// crtc0.Write();
	// crtc1.Write();
	// crtc2.Write();
	// crtc3.Write();
	// crtc4.Write();
	// crtc5.Write();
	// crtc6.Write();
	// crtc7.Write();
	// crtc8.Write();
	// crtc9.Write();
	// crtc10.Write();
	// crtc11.Write();
	// crtc12.Write();
	// crtc13.Write();
	// crtc14.Write();
	// crtc15.Write();
	// crtc16.Write();
	// crtc17.Write();
	// crtc18.Write();
	// crtc19.Write();
	// crtc20.Write();
	// crtc21.Write();
	// crtc22.Write();
	// crtc23.Write();
	// crtc24.Write();
	// Print("\n=====\n");
	crtc.Write();

	// __asm("cli");
	// for(;;)
	// 	HALT;

	gc00.Write();
	gc01.Write();
	gc02.Write();
	gc03.Write();
	gc04.Write();
	gc05.Write();
	gc06.Write();
	gc07.Write();
	gc08.Write();

	attrPalette.Write();
	attrModeControl.Write();
	attrOverscanColor.Write();
	attrColorPlaneEnable.Write();
	attrPixelPanning.Write();
	attrColorSelect.Write();

	/* lock 16-color palette and unblank display */
	(void)HAL::In8(VGA_INSTAT_READ);
	HAL::Out8(VGA_AC_INDEX, 0x20);
}

static void set_plane(unsigned p)
{
	return;
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
