#pragma once

#include"VGA.hpp"

namespace VGA
{
	namespace Registers
	{
		/*
		* CRTC
		*/

		typedef u8 (*RegReadFunc)(u8 index);
		typedef void (*RegWriteFunc)(u8 index, u8 value);

		template<RegReadFunc _Read, RegWriteFunc _Write, u8 _regIndex, u8 _regMask>
		struct VGARegister
		{
			VGARegister()
			{

			}

			void Read()
			{
				u8 reg = _Read(_regIndex);
				Read_Conv(reg);
			}

			void Write()
			{
				u8 regValue = _Read(_regIndex);
				regValue &= ~_regMask;
				u8 newBits = Write_Conv();
				regValue |= (newBits & _regMask);
				_Write(_regIndex, regValue);
			}

			virtual void Read_Conv(u8 regValue) = 0;
			virtual u8 Write_Conv() = 0;
		};

		struct CRTC_HorizontalTotal : public VGARegister<Read_3D4, Write_3D4, 0x00, 0xff>
		{
			using VGARegister::VGARegister;

			/**
			 * This field is used to specify the number of character clocks per scan line.
			 * This field, along with the dot rate selected, controls the horizontal refresh
			 * rate of the VGA by specifying the amount of time one scan line takes.
			 * This field is not programmed with the actual number of character clocks, however.
			 * Due to timing factors of the VGA hardware (which, for compatibility purposes has been emulated by VGA compatible  chipsets),
			 * the actual horizontal total is 5 character clocks more than the value stored in this field,
			 * thus one needs to subtract 5 from the actual horizontal total value desired before programming it into this register.
			 */
			u8 horizontalTotal;

			CRTC_HorizontalTotal(u8 horizontalTotal) :
				horizontalTotal(horizontalTotal)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				horizontalTotal = regValue;
			}

			u8 Write_Conv() override
			{
				return horizontalTotal;
			}
		};

		struct CRTC_EndHorizontalDisplay : public VGARegister<Read_3D4, Write_3D4, 0x01, 0xff>
		{
			using VGARegister::VGARegister;

			/**
			 * This field is used to control the point that the sequencer stops outputting pixel values from display memory,
			 * and sequences the pixel value specified by the Overscan Palette Index field for the remainder of the scan line.
			 * The overscan begins the character clock after the the value programmed into this field.
			 * This register should be programmed with the number of character clocks in the active display - 1.
			 * Note that the active display may be affected by the Display Enable Skew field.
			 */
			u8 endHorizontalDisplay;

			CRTC_EndHorizontalDisplay(u8 endHorizontalDisplay) :
				endHorizontalDisplay(endHorizontalDisplay)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				endHorizontalDisplay = regValue;
			}

			u8 Write_Conv() override
			{
				return endHorizontalDisplay;
			}
		};

		struct CRTC_StartHorizontalBlanking : public VGARegister<Read_3D4, Write_3D4, 0x02, 0xff>
		{
			using VGARegister::VGARegister;

			/**
			 * This field is used to specify the character clock at which the horizontal blanking period begins.
			 * During the horizontal blanking period, the VGA hardware forces the DAC into a blanking state,
			 * where all of the intensities output are at minimum value, no matter what color information the attribute controller is sending to the DAC.
			 * This field works in conjunction with the End Horizontal Blanking field to specify the horizontal blanking period.
			 * Note that the horizontal blanking can be programmed to appear anywhere within the scan line, as well as being programmed to a
			 * value greater than the Horizontal Total field preventing the horizontal blanking from occurring at all.
			 */
			u8 horizontalBlankingStart;

			CRTC_StartHorizontalBlanking(u8 horizontalBlankingStart) :
				horizontalBlankingStart(horizontalBlankingStart)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				horizontalBlankingStart = regValue;
			}

			u8 Write_Conv() override
			{
				return horizontalBlankingStart;
			}
		};

		struct CRTC_EndHorizontalBlanking : public VGARegister<Read_3D4, Write_3D4, 0x03, 0xff>
		{
			using VGARegister::VGARegister;

			/**
			 * This field was used in the IBM EGA to provide access to the light pen input values as the light pen registers
			 * were mapped over CRTC indexes 10h-11h.
			 * The VGA lacks capability for light pen input, thus this field is normally forced to 1
			 * (although always writing it as 1 might be a good idea for compatibility) , which in the EGA would enable access to the
			 * vertical retrace fields instead of the light pen fields.
			 */
			bool enableVerticalRetraceAccess;

			/**
			 * This field affects the timings of the display enable circuitry in the VGA. The value of this field is the number of
			 * character clocks that the display enable "signal" is delayed.
			 * In all the VGA/SVGA chipsets I've tested, including a PS/2 VGA this field is always programmed to 0.
			 * Programming it to non-zero values results in the overscan being displayed over the number of characters programmed
			 * into this field at the beginning of the scan line, as well as the end of the active display being shifted the number of
			 * characters programmed into this field.
			 * The characters that extend past the normal end of the active display can be garbled in certain circumstances that is
			 * dependent on the particular VGA implementation.
			 * According to documentation from IBM, "This skew control is needed to provide sufficient time for the CRT controller to read a
			 * character and attribute code from the video buffer, to gain access to the character generator, and go through the Horizontal
			 * PEL Panning register in the attribute controller. Each access requires the 'display enable' signal to be skewed one character
			 * clock so that the video output is synchronized with the horizontal and vertical retrace signals."
			 * as well as
			 * "Note: Character skew is not adjustable on the Type 2 video and the bits are ignored; however, programs should set these bits
			 * for the appropriate skew to maintain compatibility."
			 * This may be required for some early IBM VGA implementations or may be simply an unused "feature" carried over along with its
			 * register description from the IBM EGA implementations that require the use of this field.
			 */
			u8 displayEnableSkew;

			/**
			 * This contains bits 4-0 of the End Horizontal Blanking field which specifies the end of the horizontal blanking period.
			 * Bit 5 is located After the period has begun as specified by the Start Horizontal Blanking field, the 6-bit value of this
			 * field is compared against the lower 6 bits of the character clock.
			 * When a match occurs, the horizontal blanking signal is disabled.
			 * This provides from 1 to 64 character clocks although some implementations may match in the character clock specified by
			 * the Start Horizontal Blanking field, in which case the range is 0 to 63.
			 * Note that if blanking extends past the end of the scan line, it will end on the first match of this field on the next scan line.
			 */
			u8 horizontalBlankingEnd;

			CRTC_EndHorizontalBlanking(bool enableVerticalRetraceAccess,
									u8 displayEnableSkew,
									u8 horizontalBlankingEnd) :
				enableVerticalRetraceAccess(enableVerticalRetraceAccess),
				displayEnableSkew(displayEnableSkew),
				horizontalBlankingEnd(horizontalBlankingEnd)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				enableVerticalRetraceAccess = !!(regValue & (1 << 7));
				displayEnableSkew = (regValue & 0b1100000) >> 5;
				horizontalBlankingEnd = (regValue & 0b11111) >> 0;
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= ((int)enableVerticalRetraceAccess) << 7;
				reg |= (displayEnableSkew << 5) & 0b1100000;
				reg |= (horizontalBlankingEnd << 0) & 0b11111;
				return reg;
			}
		};

		struct CRTC_StartHorizontalRetrace : public VGARegister<Read_3D4, Write_3D4, 0x04, 0xff>
		{
			using VGARegister::VGARegister;

			/**
			 * This field specifies the character clock at which the VGA begins sending the horizontal synchronization
			 * pulse to the display which signals the monitor to retrace back to the left side of the screen.
			 * The end of this pulse is controlled by the End Horizontal Retrace field.
			 * This pulse may appear anywhere in the scan line, as well as set to a position beyond the Horizontal Total field
			 * which effectively disables the horizontal synchronization pulse.
			 */
			u8 horizontalRetraceStart;

			CRTC_StartHorizontalRetrace(u8 horizontalRetraceStart) :
				horizontalRetraceStart(horizontalRetraceStart)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				horizontalRetraceStart = regValue;
			}

			u8 Write_Conv() override
			{
				return horizontalRetraceStart;
			}
		};

		struct CRTC_EndHorizontalRetrace : public VGARegister<Read_3D4, Write_3D4, 0x05, 0xff>
		{
			using VGARegister::VGARegister;

			/**
			 * This contains bit 5 of the End Horizontal Blanking field.
			 * See the End Horizontal Blanking Register for details.
			 */
			u8 horizontalBlankingEnd_5;

			/**
			 * This field delays the start of the horizontal retrace period by the number of character clocks equal to the value of this field.
			 * From observation, this field is programmed to 0, with the exception of the 40 column text modes where this field is set to 1.
			 * The VGA hardware simply acts as if this value is added to the Start Horizontal Retrace field.
			 * According to IBM documentation,
			 * "For certain modes, the 'horizontal retrace' signal takes up the entire blanking interval. Some internal timings are generated
			 * by the falling edge of the 'horizontal retrace' signal. To ensure that the signals are latched properly, the 'retrace' signal
			 * is started before the end of the 'display enable' signal and then skewed several character clock times to provide the proper screen centering."
			 * This does not appear to be the case, leading me to believe this is yet another holdout from the IBM EGA implementations that do require
			 * the use of this field.
			 */
			u8 horizontalRetraceSkew;

			/**
			 * This field specifies the end of the horizontal retrace period, which begins at the character clock specified
			 * in the Start Horizontal Retrace field.  The horizontal retrace signal is enabled until the lower 5 bits of the character counter
			 * match the 5 bits of this field.  This provides for a horizontal retrace period from 1 to 32 character clocks.
			 * Note that some implementations may match immediately instead of 32 clocks away, making the effective range 0 to 31 character clocks.
			 */
			u8 horizontalRetraceEnd;

			CRTC_EndHorizontalRetrace(u8 horizontalBlankingEnd_5, u8 horizontalRetraceSkew, u8 horizontalRetraceEnd) :
				horizontalBlankingEnd_5(horizontalBlankingEnd_5),
				horizontalRetraceSkew(horizontalRetraceSkew),
				horizontalRetraceEnd(horizontalRetraceEnd)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				horizontalBlankingEnd_5 = (!!(regValue & (1 << 7))) << 5;
				horizontalRetraceSkew = (regValue & 0b1100000) >> 5;
				horizontalRetraceEnd = (regValue & 0b11111) >> 0;
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= (!!horizontalBlankingEnd_5) << 7;
				reg |= (horizontalRetraceSkew << 5) & 0b1100000;
				reg |= (horizontalRetraceEnd << 0) & 0b11111;
				return reg;
			}
		};

		struct CRTC_VerticalTotal : public VGARegister<Read_3D4, Write_3D4, 0x06, 0xff>
		{
			using VGARegister::VGARegister;

			/**
			 * This contains the lower 8 bits of the Vertical Total field.
			 * Bits 9-8 of this field are located in the Overflow Register.
			 * This field determines the number of scanlines in the active display and thus the length of each vertical retrace.
			 * This field contains the value of the scanline counter at the beginning of the last scanline in the vertical period.
			 */
			u8 verticalTotal;

			CRTC_VerticalTotal(u8 verticalTotal) :
				verticalTotal(verticalTotal)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				verticalTotal = regValue;
			}

			u8 Write_Conv() override
			{
				return verticalTotal;
			}
		};

		struct CRTC_OverflowRegister : public VGARegister<Read_3D4, Write_3D4, 0x07, 0xff>
		{
			using VGARegister::VGARegister;

			/**
			 * Specifies bit 8 of the Vertical Retrace Start field.  See the Vertical Retrace Start Register for details.
			 * Specifies bit 9 of the Vertical Retrace Start field.  See the Vertical Retrace Start Register for details.
			 */
			u16 verticalRetraceStart_8_9;

			/**
			 * Specifies bit 8 of the Vertical Display End field.  See the Vertical Display End Register for details.
			 * Specifies bit 9 of the Vertical Display End field.  See the Vertical Display End Register for details.
			 */
			u16 verticalDisplayEnd_8_9;

			/**
			 * Specifies bit 8 of the Vertical Total field.  See the Vertical Total Register for details.
			 * Specifies bit 9 of the Vertical Total field.  See the Vertical Total Register for details.
			 */
			u16 verticalTotal_8_9;

			/**
			 * Specifies bit 8 of the Line Compare field. See the Line Compare Register for details.
			 */
			u16 lineCompare_8;

			/**
			 * Specifies bit 8 of the Start Vertical Blanking field.  See the Start Vertical Blanking Register for details.
			 */
			u16 verticalBlankingStart_8;

			CRTC_OverflowRegister(u16 verticalRetraceStart_8_9,
								u16 verticalDisplayEnd_8_9,
								u16 verticalTotal_8_9,
								u16 lineCompare_8,
								u16 verticalBlankingStart_8) :
				verticalRetraceStart_8_9(verticalRetraceStart_8_9),
				verticalDisplayEnd_8_9(verticalDisplayEnd_8_9),
				verticalTotal_8_9(verticalTotal_8_9),
				lineCompare_8(lineCompare_8),
				verticalBlankingStart_8(verticalBlankingStart_8)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				verticalRetraceStart_8_9 = (!!(regValue & (1 << 2))) << 8
										| (!!(regValue & (1 << 7))) << 9;
				verticalDisplayEnd_8_9 = (!!(regValue & (1 << 1))) << 8
									| (!!(regValue & (1 << 6))) << 9;
				verticalTotal_8_9 = (!!(regValue & (1 << 0))) << 8
								| (!!(regValue & (1 << 5))) << 9;
				lineCompare_8 = (!!(regValue & (1 << 4))) << 8;
				verticalBlankingStart_8 = (!!(regValue & (1 << 3))) << 8;
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= (!!(verticalRetraceStart_8_9 & (1 << 9)))	<< 7;
				reg |= (!!(verticalDisplayEnd_8_9 & (1 << 9)))		<< 6;
				reg |= (!!(verticalTotal_8_9 & (1 << 9)))			<< 5;
				reg |= (!!(lineCompare_8 & (1 << 8)))				<< 4;
				reg |= (!!(verticalBlankingStart_8 & (1 << 8)))		<< 3;
				reg |= (!!(verticalRetraceStart_8_9 & (1 << 8)))	<< 2;
				reg |= (!!(verticalDisplayEnd_8_9 & (1 << 8)))		<< 1;
				reg |= (!!(verticalTotal_8_9 & (1 << 8)))			<< 0;
				return reg;
			}
		};

		struct CRTC_PresetRowScanRegister : public VGARegister<Read_3D4, Write_3D4, 0x08, 0x7f>
		{
			using VGARegister::VGARegister;

			/**
			 * The value of this field is added to the Start Address Register when calculating the display memory address for the
			 * upper left hand pixel or character of the screen.
			 * This allows for a maximum shift of 15, 31, or 35 pixels without having to reprogram the Start Address Register.
			 */
			u8 bytePanning;

			/**
			 * This field is used when using text mode or any mode with a non-zero Maximum Scan Line field to provide for more precise
			 * vertical scrolling than the Start Address Register provides.
			 * The value of this field specifies how many scan lines to scroll the display upwards.
			 * Valid values range from 0 to the value of the Maximum Scan Line field.
			 * Invalid values may cause undesired effects and seem to be dependent upon the particular VGA implementation.
			 */
			u8 presetRowScan;

			CRTC_PresetRowScanRegister(u8 bytePanning, u8 presetRowScan) :
				bytePanning(bytePanning),
				presetRowScan(presetRowScan)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				bytePanning = (regValue & 0b01100000) << 5;
				presetRowScan = (regValue & 0b11111) >> 0;
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= (bytePanning << 5) & 0b01100000;
				reg |= (presetRowScan << 0) & 0b11111;
				return reg;
			}
		};

		struct CRTC_MaximumScanLineRegister : public VGARegister<Read_3D4, Write_3D4, 0x09, 0xff>
		{
			using VGARegister::VGARegister;

			/**
			 *  "When this bit is set to 1, 200-scan-line video data is converted to 400-scan-line output.
			 * To do this, the clock in the row scan counter is divided by 2, which allows the 200-line modes to be displayed
			 * as 400 lines on the display (this is called double scanning; each line is displayed twice).
			 * When this bit is set to 0, the clock to the row scan counter is equal to the horizontal scan rate."
			 */
			bool scanDoubling;

			/**
			 * Specifies bit 9 of the Line Compare field. See the Line Compare Register for details.
			 */
			u16 lineCompare_9;

			/**
			 * Specifies bit 9 of the Start Vertical Blanking field.  See the Start Vertical Blanking Register for details.
			 */
			u16 verticalBlankingStart_9;

			/**
			 * In text modes, this field is programmed with the character height - 1 (scan line numbers are zero based.)
			 * In graphics modes, a non-zero value in this field will cause each scan line to be repeated by the value of this field + 1.
			 */
			u8 maximumScanLine;

			CRTC_MaximumScanLineRegister(bool scanDoubling,
										u16 lineCompare_9,
										u16 verticalBlankingStart_9,
										u8 maximumScanLine) :
				scanDoubling(scanDoubling),
				lineCompare_9(lineCompare_9),
				verticalBlankingStart_9(verticalBlankingStart_9),
				maximumScanLine(maximumScanLine)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				scanDoubling = (!!(regValue & (1 << 7)));
				lineCompare_9 = (!!(regValue & (1 << 6))) << 9;
				verticalBlankingStart_9 = (!!(regValue & (1 << 5))) << 9;
				maximumScanLine = (regValue & 0b11111);
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= ((int)scanDoubling)						<< 7;
				reg |= (!!(lineCompare_9 & (1 << 9)))			<< 6;
				reg |= (!!(verticalBlankingStart_9 & (1 << 9)))	<< 5;
				reg |= (maximumScanLine & 0b11111)				<< 0;
				return reg;
			}
		};

		struct CRTC_CursorStartRegister : public VGARegister<Read_3D4, Write_3D4, 0x0A, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This field controls whether or not the text-mode cursor is displayed. Values are:
			 * 0 - Cursor Enabled
			 * 1 - Cursor Disabled
			 */
			bool cursorDisabled;

			/**
			 * This field controls the appearance of the text-mode cursor by specifying the scan line
			 * location within a character cell at which the cursor should begin, with the top-most
			 * scan line in a character cell being 0 and the bottom being with the value of the Maximum Scan Line field.
			 */
			u8 cursorScanLineStart;

			CRTC_CursorStartRegister(bool cursorDisabled, u8 cursorScanLineStart) :
				cursorDisabled(cursorDisabled),
				cursorScanLineStart(cursorScanLineStart)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				cursorDisabled = (!!(regValue & (1 << 5)));
				cursorScanLineStart = (regValue & 0b11111) >> 0;
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= ((int)cursorDisabled) << 5;
				reg |= (cursorScanLineStart & 0b11111) << 0;
				return reg;
			}
		};

		struct CRTC_CursorEndRegister : public VGARegister<Read_3D4, Write_3D4, 0x0B, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This field was necessary in the EGA to synchronize the cursor with internal timing.
			 * In the VGA it basically is added to the cursor location.
			 * In some cases when this value is non-zero and the cursor is near the left or right edge of the screen,
			 * the cursor will not appear at all, or a second cursor above and to the left of the actual one may appear.
			 * This behavior may not be the same on all VGA compatible adapter cards.
			 */
			u8 cursorSkew;

			/**
			 * This field controls the appearance of the text-mode cursor by specifying the scan line location within a
			 * character cell at which the cursor should end, with the top-most scan line in a character cell being 0 and
			 * the bottom being with the value of the Maximum Scan Line field.
			 * If this field is less than the Cursor Scan Line Start field, the cursor is not drawn.
			 * Some graphics adapters, such as the IBM EGA display a split-block cursor instead.
			 */
			u8 cursorScanLineEnd;

			CRTC_CursorEndRegister(u8 cursorSkew, u8 cursorScanLineEnd) :
				cursorSkew(cursorSkew), cursorScanLineEnd(cursorScanLineEnd)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				cursorSkew = (regValue & 0b1100000) >> 5;
				cursorScanLineEnd = (regValue & 0b11111) >> 0;
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= (cursorSkew & 0b11) << 5;
				reg |= (cursorScanLineEnd & 0b11111) << 0;
				return reg;
			}
		};

		struct CRTC_StartAddressHighRegister : public VGARegister<Read_3D4, Write_3D4, 0x0C, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This contains specifies bits 15-8 of the Start Address field. See the Start Address Low Register for details.
			 */
			u8 startAddressHigh;

			CRTC_StartAddressHighRegister(u8 startAddressHigh) :
				startAddressHigh(startAddressHigh)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				startAddressHigh = regValue;
			}

			u8 Write_Conv() override
			{
				return startAddressHigh;
			}
		};

		struct CRTC_StartAddressLowRegister : public VGARegister<Read_3D4, Write_3D4, 0x0D, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This contains the bits 7-0 of the Start Address field.
			 * The upper 8 bits are specified by the Start Address High Register.
			 * The Start Address field specifies the display memory address of the upper left pixel or character of the screen.
			 * Because the standard VGA has a maximum of 256K of memory, and memory is accessed 32 bits at a time,
			 * this 16-bit field is sufficient to allow the screen to start at any memory address.
			 * Normally this field is programmed to 0h, except when using virtual resolutions, paging, and/or split-screen operation.
			 * Note that the VGA display will wrap around in display memory if the starting address is too high.
			 * (This may or may not be desirable, depending on your intentions.)
			 */
			u8 startAddressLow;

			CRTC_StartAddressLowRegister(u8 startAddressLow) :
				startAddressLow(startAddressLow)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				startAddressLow = regValue;
			}

			u8 Write_Conv() override
			{
				return startAddressLow;
			}
		};

		struct CRTC_CursorLocationHighRegister : public VGARegister<Read_3D4, Write_3D4, 0x0E, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This field specifies bits 15-8 of the Cursor Location field. See the Cursor Location Low Register for details.
			 */
			u8 cursorLocationHigh;

			CRTC_CursorLocationHighRegister(u8 cursorLocationHigh) :
				cursorLocationHigh(cursorLocationHigh)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				cursorLocationHigh = regValue;
			}

			u8 Write_Conv() override
			{
				return cursorLocationHigh;
			}
		};

		struct CRTC_CursorLocationLowRegister : public VGARegister<Read_3D4, Write_3D4, 0x0F, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This field specifies bits 7-0 of the Cursor Location field.
			 * When the VGA hardware is displaying text mode and the text-mode cursor is enabled,
			 * the hardware compares the address of the character currently being displayed with sum of value of this
			 * field and the sum of the Cursor Skew field.
			 * If the values equal then the scan lines in that character specified by the Cursor Scan Line Start field and the
			 * Cursor Scan Line End field are replaced with the foreground color.
			 */
			u8 cursorLocationLow;

			CRTC_CursorLocationLowRegister(u8 cursorLocationLow) :
				cursorLocationLow(cursorLocationLow)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				cursorLocationLow = regValue;
			}

			u8 Write_Conv() override
			{
				return cursorLocationLow;
			}
		};

		struct CRTC_VerticalRetraceStartRegister : public VGARegister<Read_3D4, Write_3D4, 0x10, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This field specifies bits 7-0 of the Vertical Retrace Start field.
			 * Bits 9-8 are located in the Overflow Register.
			 * This field controls the start of the vertical retrace pulse which signals the display to move up to the
			 * beginning of the active display.
			 * This field contains the value of the vertical scanline counter at the beginning of the first scanline
			 * where the vertical retrace signal is asserted.
			 */
			u8 verticalRetraceStart;

			CRTC_VerticalRetraceStartRegister(u8 verticalRetraceStart) :
				verticalRetraceStart(verticalRetraceStart)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				verticalRetraceStart = regValue;
			}

			u8 Write_Conv() override
			{
				return verticalRetraceStart;
			}
		};

		struct CRTC_VerticalRetraceEndRegister : public VGARegister<Read_3D4, Write_3D4, 0x11, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This field is used to protect the video timing registers from being changed by programs written for
			 * earlier graphics chipsets that attempt to program these registers with values unsuitable for VGA timings.
			 * When this field is set to 1, the CRTC register indexes 00h-07h ignore write access,
			 * with the exception of bit 4 of the Overflow Register, which holds bit 8 of the Line Compare field.
			 */
			bool protectEnabled;

			/**
			 * Nearly all video chipsets include a few registers that control memory, bus, or other timings not directly related to the
			 * output of the video card.  Most VGA/SVGA implementations ignore the value of this field;
			 * however, in the least, IBM VGA adapters do utilize it and thus for compatibility with these chipsets this field should be programmed.
			 * This register is used in the IBM VGA hardware to control the number of DRAM refresh cycles per scan line.
			 * The three refresh cycles per scanline is appropriate for the IBM VGA horizontal frequency of approximately 31.5 kHz.
			 * For horizontal frequencies greater than this, this setting will work as the DRAM will be refreshed more often.
			 * However, refreshing not often enough for the DRAM can cause memory loss.
			 * Thus at some point slower than 31.5 kHz the five refresh cycle setting should be used.
			 * At which particular point this should occur, would require better knowledge of the IBM VGA's schematics than I have available.
			 * According to IBM documentation,
			 * "Selecting five refresh cycles allows use of the VGA chip with 15.75 kHz displays."
			 * which isn't really enough to go by unless the mode you are defining has a 15.75 kHz horizontal frequency.
			 */
			bool memoryRefreshBandwidth; // TODO: enum

			/**
			 * This field determines the end of the vertical retrace pulse, and thus its length.
			 * This field contains the lower four bits of the vertical scanline counter at the beginning of the scanline immediately
			 * after the last scanline where the vertical retrace signal is asserted.
			 */
			u8 verticalRetraceEnd;

			CRTC_VerticalRetraceEndRegister(bool protectEnabled,
											bool memoryRefreshBandwidth,
											u8 verticalRetraceEnd) :
				protectEnabled(protectEnabled),
				memoryRefreshBandwidth(memoryRefreshBandwidth),
				verticalRetraceEnd(verticalRetraceEnd)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				protectEnabled = !!(regValue & 0b10000000);
				memoryRefreshBandwidth = !!(regValue & 0b1000000);
				verticalRetraceEnd = (regValue & 0b1111) >> 0;
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= ((int)protectEnabled & 0b1) << 7;
				reg |= ((int)memoryRefreshBandwidth & 0b1) << 6;
				reg |= (verticalRetraceEnd & 0b11111) << 0;
				return reg;
			}
		};

		struct CRTC_VerticalDisplayEndRegister : public VGARegister<Read_3D4, Write_3D4, 0x12, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This contains the bits 7-0 of the Vertical Display End field.
			 * Bits 9-8 are located in the Overflow Register.
			 * The field contains the value of the vertical scanline counter at the beggining of the scanline immediately
			 * after the last scanline of active display.
			 */
			u8 verticalDisplayEnd;

			CRTC_VerticalDisplayEndRegister(u8 verticalDisplayEnd) :
				verticalDisplayEnd(verticalDisplayEnd)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				verticalDisplayEnd = regValue;
			}

			u8 Write_Conv() override
			{
				return verticalDisplayEnd;
			}
		};

		struct CRTC_OffsetRegister : public VGARegister<Read_3D4, Write_3D4, 0x13, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This field specifies the address difference between consecutive scan lines or two lines of characters.
			 * Beginning with the second scan line, the starting scan line is increased by twice the value of this register
			 * multiplied by the current memory address size (byte = 1, word = 2, double-word = 4) each line.
			 * For text modes the following equation is used:
			 * 		Offset = Width / ( MemoryAddressSize * 2 )
			 * and in graphics mode, the following equation is used:
			 * 		Offset = Width / ( PixelsPerAddress * MemoryAddressSize * 2 )
			 * where Width is the width in pixels of the screen.
			 * This register can be modified to provide for a virtual resolution, in which case Width is the width is the
			 * width in pixels of the virtual screen.
			 * PixelsPerAddress is the number of pixels stored in one display memory address, and MemoryAddressSize is the current memory addressing size.
			 */
			u8 offset;

			CRTC_OffsetRegister(u8 offset) :
				offset(offset)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				offset = regValue;
			}

			u8 Write_Conv() override
			{
				return offset;
			}
		};

		struct CRTC_UnderlineLocationRegister : public VGARegister<Read_3D4, Write_3D4, 0x14, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 *  "When this bit is set to 1, memory addresses are doubleword addresses.
			 * See the description of the word/byte mode bit (bit 6) in the CRT Mode Control Register"
			 */
			bool doubleWordAddressing;

			/**
			 *  "When this bit is set to 1, the memory-address counter is clocked with the character clock
			 * divided by 4, which is used when doubleword addresses are used."
			 */
			bool divideMemoryAddressClockBy4;

			/**
			 *  "These bits specify the horizontal scan line of a character row on which an underline occurs.
			 * The value programmed is the scan line desired minus 1."
			 */
			u8 underlineLocation;

			CRTC_UnderlineLocationRegister(bool doubleWordAddressing,
										bool divideMemoryAddressClockBy4,
										u8 underlineLocation) :
				doubleWordAddressing(doubleWordAddressing),
				divideMemoryAddressClockBy4(divideMemoryAddressClockBy4),
				underlineLocation(underlineLocation)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				doubleWordAddressing = !!(regValue & 0b01000000);
				divideMemoryAddressClockBy4 = !!(regValue & 0b00100000);
				underlineLocation = (regValue & 0b11111) >> 0;
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= ((int)doubleWordAddressing & 0b1) << 6;
				reg |= ((int)divideMemoryAddressClockBy4 & 0b1) << 5;
				reg |= (underlineLocation & 0b11111) << 0;
				return reg;
			}
		};

		struct CRTC_VerticalBlankingStartRegister : public VGARegister<Read_3D4, Write_3D4, 0x15, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This contains bits 7-0 of the Start Vertical Blanking field.
			 * Bit 8 of this field is located in the Overflow Register, and bit 9 is located in the Maximum Scan Line Register.
			 * This field determines when the vertical blanking period begins, and contains the value of the vertical scanline
			 * counter at the beginning of the first vertical scanline of blanking.
			 */
			u8 verticalBlankingStart;

			CRTC_VerticalBlankingStartRegister(u8 verticalBlankingStart) :
				verticalBlankingStart(verticalBlankingStart)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				verticalBlankingStart = regValue;
			}

			u8 Write_Conv() override
			{
				return verticalBlankingStart;
			}
		};

		struct CRTC_VerticalBlankingEndRegister : public VGARegister<Read_3D4, Write_3D4, 0x16, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This field determines when the vertical blanking period ends, and contains the value of the
			 * vertical scanline counter at the beginning of the vertical scanline immediately after the last scanline of blanking.
			 */
			u8 verticalBlankingEnd;

			CRTC_VerticalBlankingEndRegister(u8 verticalBlankingEnd) :
				verticalBlankingEnd(verticalBlankingEnd)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				verticalBlankingEnd = regValue;
			}

			u8 Write_Conv() override
			{
				return verticalBlankingEnd;
			}
		};

		struct CRTC_ModeControlRegister : public VGARegister<Read_3D4, Write_3D4, 0x17, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * "When set to 0, this bit disables the horizontal and vertical retrace signals and forces them to an inactive level.
			 * When set to 1, this bit enables the horizontal and vertical retrace signals. This bit does not reset any other registers or signal outputs."
			 */
			bool syncEnabled;

			/**
			 * "When this bit is set to 0, the word mode is selected.
			 * The word mode shifts the memory-address counter bits to the left by one bit;
			 * the most-significant bit of the counter appears on the least-significant bit of the memory address outputs.
			 * The doubleword bit in the Underline Location register (0x14) also controls the addressing.
			 * When the doubleword bit is 0, the word/byte bit selects the mode. When the doubleword bit is set to 1,
			 * the addressing is shifted by two bits. When set to 1, bit 6 selects the byte address mode."
			 */
			bool wordByteMode;

			/**
			 * "This bit selects the memory-address bit, bit MA 13 or MA 15, that appears on the output pin MA 0, in the word address mode.
			 * If the VGA is not in the word address mode, bit 0 from the address counter appears on the output pin, MA 0.
			 * When set to 1, this bit selects MA 15.
			 * In odd/even mode, this bit should be set to 1 because 256KB of video memory is installed on the system board.
			 * (Bit MA 13 is selected in applications where only 64KB is present.
			 * This function maintains compatibility with the IBM Color/Graphics Monitor Adapter.)"
			 */
			bool addressWrapSelect;

			/**
			 * "When this bit is set to 0, the address counter uses the character clock.
			 * When this bit is set to 1, the address counter uses the character clock input divided by 2.
			 * This bit is used to create either a byte or word refresh address for the display buffer."
			 */
			bool divideMemoryAddressClockBy2;

			/**
			 * "This bit selects the clock that controls the vertical timing counter.
			 * The clocking is either the horizontal retrace clock or horizontal retrace clock divided by 2.
			 * When this bit is set to 1. the horizontal retrace clock is divided by 2.
			 * Dividing the clock effectively doubles the vertical resolution of the CRT controller.
			 * The vertical counter has a maximum resolution of 1024 scan lines because the vertical total value is 10-bits wide.
			 * If the vertical counter is clocked with the horizontal retrace divided by 2, the vertical resolution is doubled to 2048 scan lines."
			 */
			bool divideScanLineClockBy2;

			/**
			 *  "This bit selects the source of bit 13 of the output multiplexer.
			 * When this bit is set to 0, bit 0 of the row scan counter is the source, and
			 * when this bit is set to 1, bit 13 of the address counter is the source.
			 * The CRT controller used on the IBM Color/Graphics Adapter was capable of using 128 horizontal scan-line addresses.
			 * For the VGA to obtain 640-by-200 graphics resolution, the CRT controller is  programmed for 100 horizontal scan lines
			 * with two scan-line addresses per character row.
			 * Row scan  address bit 0 becomes the most-significant address bit to the display buffer.
			 * Successive scan lines of the display image are displaced in 8KB of memory.
			 * This bit allows compatibility with the graphics modes of earlier adapters."
			 */
			bool mapDisplayAddress13;

			/**
			 *  "This bit selects the source of bit 14 of the output multiplexer.
			 * When this bit is set to 0, bit 1 of the row scan counter is the source.
			 * When this bit is set to 1, the bit 14 of the address counter is the source."
			 */
			bool mapDisplayAddress14;

			CRTC_ModeControlRegister(bool syncEnabled,
									bool wordByteMode,
									bool addressWrapSelect,
									bool divideMemoryAddressClockBy2,
									bool divideScanLineClockBy2,
									bool mapDisplayAddress13,
									bool mapDisplayAddress14) :
				syncEnabled(syncEnabled),
				wordByteMode(wordByteMode),
				addressWrapSelect(addressWrapSelect),
				divideMemoryAddressClockBy2(divideMemoryAddressClockBy2),
				divideScanLineClockBy2(divideScanLineClockBy2),
				mapDisplayAddress13(mapDisplayAddress13),
				mapDisplayAddress14(mapDisplayAddress14)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				syncEnabled					= !!(regValue & 0b10000000);
				wordByteMode				= !!(regValue & 0b01000000);
				addressWrapSelect			= !!(regValue & 0b00100000);
				divideMemoryAddressClockBy2	= !!(regValue & 0b00001000);
				divideScanLineClockBy2		= !!(regValue & 0b00000100);
				mapDisplayAddress13			= !!(regValue & 0b00000001);
				mapDisplayAddress14			= !!(regValue & 0b00000010);
			}

			u8 Write_Conv() override
			{
				u8 reg = 0;
				reg |= ((int)syncEnabled & 0b1)					<< 7;
				reg |= ((int)wordByteMode & 0b1)				<< 6;
				reg |= ((int)addressWrapSelect & 0b1)			<< 5;
				reg |= ((int)divideMemoryAddressClockBy2 & 0b1)	<< 3;
				reg |= ((int)divideScanLineClockBy2 & 0b1)		<< 2;
				reg |= ((int)mapDisplayAddress13 & 0b1)			<< 0;
				reg |= ((int)mapDisplayAddress14 & 0b1)			<< 1;
				return reg;
			}
		};

		struct CRTC_LineCompareRegister : public VGARegister<Read_3D4, Write_3D4, 0x18, 0xFF>
		{
			using VGARegister::VGARegister;

			/**
			 * This field specifies bits 7-0 of the Line Compare field.
			 * Bit 9 of this field is located in the Maximum Scan Line Register, and bit 8 of this field is located in the Overflow Register.
			 * The Line Compare field specifies the scan line at which a horizontal division can occur, providing for split-screen operation.
			 * If no horizontal division is required, this field should be set to 3FFh.
			 * When the scan line counter reaches the value in the Line Compare field, the current scan line address is reset to 0 and the
			 * Preset Row Scan is presumed to be 0.
			 * If the Pixel Panning Mode field is set to 1 then the Pixel Shift Count and Byte Panning fields are reset to 0 for the remainder
			 * of the display cycle.
			 */
			u8 lineCompare;

			CRTC_LineCompareRegister(u8 lineCompare) :
				lineCompare(lineCompare)
			{

			}

			void Read_Conv(u8 regValue) override
			{
				lineCompare = regValue;
			}

			u8 Write_Conv() override
			{
				return lineCompare;
			}
		};

		// http://www.osdever.net/FreeVGA/vga/crtcreg.htm
	}
}