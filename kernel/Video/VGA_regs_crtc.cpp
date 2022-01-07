#include"VGA_regs.hpp"
#include"VGA_regs_crtc.hpp"

void VGA::Registers::CRTController::Read()
{
	/*
	u8 horizontalTotal;
	u8 endHorizontalDisplay;
	u8 startHorizontalBlanking;
	bool enableVerticalRetraceAccess;
	u8 displayEnableSkew;
	u8 endHorizontalBlanking; // 0-4+5
	u8 startHorizontalRetrace;
	u8 horizontalRetraceSkew;
	u8 endHorizontalRetrace;
	u16 verticalTotal; // 0-7+8+9
	u8 bytePanning;
	u8 presetRowScan;
	bool scanDoubling;
	u8 maximumScanLine;
	bool cursorDisabled;
	u8 cursorScanLineStart;
	u8 cursorSkew;
	u8 cursorScanLineEnd;
	u16 startAddress; // low + high
	u16 cursorLocationLow; // low + high
	u16 verticalRetraceStart; // 0-7 + 8 + 9
	bool protectEnabled;
	bool memoryRefreshBandwidth; // TODO: enum
	u8 verticalRetraceEnd;
	u16 verticalDisplayEnd; // 0-7 + 8 + 9
	u8 offset;
	bool doubleWordAddressing;
	bool divideMemoryAddressClockBy4;
	u8 underlineLocation;
	u16 verticalBlankingStart; // 0-7+8+9
	u8 verticalBlankingEnd;
	bool syncEnabled;
	bool wordByteMode;
	bool addressWrapSelect;
	bool divideMemoryAddressClockBy2;
	bool divideScanLineClockBy2;
	bool mapDisplayAddress14;
	bool mapDisplayAddress13;
	u16 lineCompare; // 0-7+8+9

	CRTC_HorizontalTotal
	CRTC_EndHorizontalDisplay
	CRTC_StartHorizontalBlanking
	CRTC_EndHorizontalBlanking
	CRTC_StartHorizontalRetrace
	CRTC_EndHorizontalRetrace
	CRTC_VerticalTotal
	CRTC_OverflowRegister
	CRTC_PresetRowScanRegister
	CRTC_MaximumScanLineRegister
	CRTC_CursorStartRegister
	CRTC_CursorEndRegister
	CRTC_StartAddressHighRegister
	CRTC_StartAddressLowRegister
	CRTC_CursorLocationHighRegister
	CRTC_CursorLocationLowRegister
	CRTC_VerticalRetraceStartRegister
	CRTC_VerticalRetraceEndRegister
	CRTC_VerticalDisplayEndRegister
	CRTC_OffsetRegister
	CRTC_UnderlineLocationRegister
	CRTC_VerticalBlankingStartRegister
	CRTC_VerticalBlankingEndRegister
	CRTC_ModeControlRegister
	CRTC_LineCompareRegister
	*/
}

void VGA::Registers::CRTController::Write()
{
	CRTC_HorizontalTotal(horizontalTotal).Write();
	CRTC_EndHorizontalDisplay(horizontalDisplayEnd).Write();
	CRTC_StartHorizontalBlanking(horizontalBlankingStart).Write();
	CRTC_EndHorizontalBlanking(enableVerticalRetraceAccess, displayEnableSkew, horizontalBlankingEnd).Write();
	CRTC_StartHorizontalRetrace(horizontalRetraceStart).Write();
	CRTC_EndHorizontalRetrace(horizontalBlankingEnd, horizontalRetraceSkew, horizontalRetraceEnd).Write();
	CRTC_VerticalTotal(verticalTotal).Write();
	CRTC_OverflowRegister(verticalRetraceStart, verticalDisplayEnd, verticalTotal, lineCompare, verticalBlankingStart).Write();
	CRTC_PresetRowScanRegister(bytePanning, presetRowScan).Write();
	CRTC_MaximumScanLineRegister(scanDoubling, lineCompare, verticalBlankingStart, maximumScanLine).Write();
	CRTC_CursorStartRegister(cursorDisabled, cursorScanLineStart).Write();
	CRTC_CursorEndRegister(cursorSkew, cursorScanLineEnd).Write();
	CRTC_StartAddressHighRegister(startAddress >> 8).Write();
	CRTC_StartAddressLowRegister(startAddress & 0xFF).Write();
	CRTC_CursorLocationHighRegister(cursorLocation >> 8).Write();
	CRTC_CursorLocationLowRegister(cursorLocation & 0xFF).Write();
	CRTC_VerticalRetraceStartRegister(verticalRetraceStart).Write();
	CRTC_VerticalRetraceEndRegister(protectEnabled, memoryRefreshBandwidth, verticalRetraceEnd).Write();
	CRTC_VerticalDisplayEndRegister(verticalDisplayEnd).Write();
	CRTC_OffsetRegister(offset).Write();
	CRTC_UnderlineLocationRegister(doubleWordAddressing, divideMemoryAddressClockBy4, underlineLocation).Write();
	CRTC_VerticalBlankingStartRegister(verticalBlankingStart).Write();
	CRTC_VerticalBlankingEndRegister(verticalBlankingEnd).Write();
	CRTC_ModeControlRegister(syncEnabled, wordByteMode, addressWrapSelect, divideMemoryAddressClockBy2, divideScanLineClockBy2, mapDisplayAddress14, mapDisplayAddress13).Write();
	CRTC_LineCompareRegister(lineCompare).Write();
}
