#include"VGA_regs_attributes.hpp"
#include"VGA_regs.hpp"

void VGA::Registers::Attributes::Write()
{
	ATTR_PaletteRegister at0 = {
		.paletteIndex = { paletteIndex[0], paletteIndex[1], paletteIndex[2], paletteIndex[3],
			paletteIndex[4], paletteIndex[5], paletteIndex[6], paletteIndex[7],
			paletteIndex[8], paletteIndex[9], paletteIndex[10], paletteIndex[11],
			paletteIndex[12], paletteIndex[13], paletteIndex[14], paletteIndex[15] }
	};

	ATTR_ModeControlRegister at1 = {
		.paletteBits54Select = paletteBits54Select,
		.color8BitEnabled = color8BitEnabled,
		.pixelPanningMode = pixelPanningMode,
		.blinkEnabled = blinkEnabled,
		.lineGraphicsEnabled = lineGraphicsEnabled,
		.monochromeEmulation = monochromeEmulation,
		.attributeControllerGraphicsEnabled = attributeControllerGraphicsEnabled
	};

	ATTR_OverscanColorRegister at2 = {
		.overscanPaletteIndex = overscanPaletteIndex
	};

	ATTR_ColorPlaneEnableRegister at3 = {
		.planeEnabled = { planeEnabled[0], planeEnabled[1], planeEnabled[2], planeEnabled[3] }
	};

	ATTR_HorizontalPixelPanningRegister at4 = {
		.pixelShiftCount = pixelShiftCount
	};

	ATTR_ColorSelectRegister at5 = {
		.colorSelect76 = colorSelect76,
		.colorSelect54 = colorSelect54
	};

	at0.Write();
	at1.Write();
	at2.Write();
	at3.Write();
	at4.Write();
	at5.Write();
}