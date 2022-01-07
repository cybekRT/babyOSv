#include"VGA.hpp"
#include"HAL.hpp"
#include"VGA_regs.hpp"

VGA::Registers::MiscOutput misc
{
	.vSyncPolarity = 0,
	.hSyncPolarity = 1,
	.oddEvenPageSelect = 1,
	.clockSelect = VGA::Registers::MiscOutput::Clock_25MHz,
	.ramEnabled = 1,
	.inputOutputAddressSelect = 1,
};

VGA::Registers::CRTController crtc = {
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

VGA::Registers::GraphicsController gc
{
	.setResetValue =
	{
		0, 0, 0, 0
	},
	.setResetEnable =
	{
		0, 0, 0, 0
	},
	.memoryPlaneWriteEnable =
	{
		0, 0, 0, 0
	},
	.logicalOperation = VGA::Registers::GC_DataRotateRegister::LO_Normal,
	.rotateCount = 0,
	.mapSelect = 0,
	.readMode = VGA::Registers::GC_GraphicsModeRegister::RM_0,
	.writeMode = VGA::Registers::GC_GraphicsModeRegister::WM_0,
	.shiftColor256 = 1,
	.shiftInterleaved = 0,
	.hostOddEven = 0,
	.alphanumericModeDisabled = 1,
	.chainOE = 0,
	.memoryMapSelect = 0b01,
	.colorDontCare =
	{
		1, 1, 1, 1
	},
	.bitMask = 0xff,
};

VGA::Registers::Sequencer seq = {
	.syncReset = true,
	.asyncReset = true,
	.screenDisabled = true,
	.shift4Enabled = false,
	.dotClockRate = false,
	.shiftLoadRate = false,
	.dot8Mode = false,
	.memoryPlaneWriteEnabled = { 1, 1, 1, 1 },
	.characterSetASelect = 0,
	.characterSetBSelect = 0,
	.chain4Enabled = true,
	.oddEvenHostMemoryWriteAddressingDisabled = true,
	.extendedMemory = true
};

VGA::Registers::Attributes attr = {
	.paletteIndex = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	.paletteBits54Select = false,
	.color8BitEnabled = true,
	.pixelPanningMode = false,
	.blinkEnabled = false,
	.lineGraphicsEnabled = false,
	.monochromeEmulation = false,
	.attributeControllerGraphicsEnabled = true,
	.overscanPaletteIndex = 0,
	.planeEnabled = { true, true, true, true },
	.pixelShiftCount = 0,
	.colorSelect76 = false,
	.colorSelect54 = false
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

void SetMode13H()
{
	// Disable screen
	HAL::In8(0x3DA);
	HAL::Out8(0x3C0, 0x00);

	misc.Write();
	seq.Write();

	// Unblock CRTC registers
	VGA::Registers::CRTC_EndHorizontalBlanking accessRetraceRegs;
	accessRetraceRegs.Read();
	accessRetraceRegs.enableVerticalRetraceAccess = true;
	accessRetraceRegs.Write();

	VGA::Registers::CRTC_VerticalRetraceEndRegister accessTimingRegs;
	accessTimingRegs.Read();
	accessTimingRegs.protectEnabled = false;
	accessTimingRegs.Write();

	crtc.Write();
	gc.Write();
	attr.Write();

	/* lock 16-color palette and unblank display */
	HAL::In8(0x3DA);
	HAL::Out8(0x3C0, 0x20);
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
		HAL::Out8(0x3C2, value);
	}

	u8 Read_3C4(u8 index)
	{
		HAL::Out8(0x3C4, index);
		return HAL::In8(0x3C5);
	}

	void Write_3C4(u8 index, u8 value)
	{
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
		HAL::Out8(0x3CE, index);
		HAL::Out8(0x3CF, value);
	}

	u8 Read_3D4(u8 index)
	{
		HAL::Out8(0x3D4, index);
		return HAL::In8(0x3D5);
	}

	void Write_3D4(u8 index, u8 value)
	{
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

		SetMode13H();

		// Set RGB palette
		for(unsigned a = 0; a < 256; a++)
		{
			// RGB323
			u8 r = ((a >> 5) & 0b111) << 3;
			u8 g = ((a >> 3) & 0b011) << 4;
			u8 b = ((a >> 0) & 0b111) << 3;

			Write_3C8(a, r, g, b);
		}

		return true;
	}

	void SetCursor(bool enabled)
	{
		Registers::CRTC_CursorStartRegister cur;
		cur.Read();
		cur.cursorDisabled = !enabled;
		cur.Write();
	}
}
