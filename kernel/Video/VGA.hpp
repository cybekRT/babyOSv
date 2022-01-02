#include"HAL.hpp"

#define RGB(r, g, b) (( r & 0b11100000 ) | (( g & 0b11000000 ) >> 3) | (( b & 0b11100000 ) >> 5))

namespace VGA
{
	u8 Read_3C0(u8 index);
	void Write_3C0(u8 index, u8 value);

	u8 Read_3C2();
	void Write_3C2(u8 value);

	u8 Read_3C4(u8 index);
	void Write_3C4(u8 index, u8 value);

	void Read_3C8(u8 index, u8* r, u8* g, u8* b);
	void Write_3C8(u8 index, u8 r, u8 g, u8 b);

	u8 Read_3CE(u8 index);
	void Write_3CE(u8 index, u8 value);

	u8 Read_3D4(u8 index);
	void Write_3D4(u8 index, u8 value);

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
		 * This field is used to specify the number of character clocks per scan line.  This field, along with the dot rate selected, controls the horizontal refresh rate of the VGA by specifying the amount of time one scan line takes.  This field is not programmed with the actual number of character clocks, however.  Due to timing factors of the VGA hardware (which, for compatibility purposes has been emulated by VGA compatible  chipsets), the actual horizontal total is 5 character clocks more than the value stored in this field, thus one needs to subtract 5 from the actual horizontal total value desired before programming it into this register.
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
		 * This field is used to control the point that the sequencer stops outputting pixel values from display memory, and sequences the pixel value specified by the Overscan Palette Index field for the remainder of the scan line.  The overscan begins the character clock after the the value programmed into this field.  This register should be programmed with the number of character clocks in the active display - 1.  Note that the active display may be affected by the Display Enable Skew field. 
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
		 * This field is used to specify the character clock at which the horizontal blanking period begins.  During the horizontal blanking period, the VGA hardware forces the DAC into a blanking state, where all of the intensities output are at minimum value, no matter what color information the attribute controller is sending to the DAC.  This field works in conjunction with the End Horizontal Blanking field to specify the horizontal blanking period.  Note that the horizontal blanking can be programmed to appear anywhere within the scan line, as well as being programmed to a value greater than the Horizontal Total field preventing the horizontal blanking from occurring at all.
		 */
		u8 startHorizontalBlanking;

		CRTC_StartHorizontalBlanking(u8 startHorizontalBlanking) : 
			startHorizontalBlanking(startHorizontalBlanking)
		{

		}

		void Read_Conv(u8 regValue) override
		{
			startHorizontalBlanking = regValue;
		}

		u8 Write_Conv() override
		{
			return startHorizontalBlanking;
		}
	};

	struct CRTC_EndHorizontalBlanking : public VGARegister<Read_3D4, Write_3D4, 0x03, 0xff>
	{
		using VGARegister::VGARegister;
		
		/**
		 * This field was used in the IBM EGA to provide access to the light pen input values as the light pen registers were mapped over CRTC indexes 10h-11h.  The VGA lacks capability for light pen input, thus this field is normally forced to 1 (although always writing it as 1 might be a good idea for compatibility) , which in the EGA would enable access to the vertical retrace fields instead of the light pen fields. 
		 */
		bool enableVerticalRetraceAccess;

		/**
		 * This field affects the timings of the display enable circuitry in the VGA. The value of this field is the number of character clocks that the display enable "signal" is delayed.  In all the VGA/SVGA chipsets I've tested, including a PS/2 VGA this field is always programmed to 0.  Programming it to non-zero values results in the overscan being displayed over the number of characters programmed into this field at the beginning of the scan line, as well as the end of the active display being shifted the number of characters programmed into this field.  The characters that extend past the normal end of the active display can be garbled in certain circumstances that is dependent on the particular VGA implementation.  According to documentation from IBM, "This skew control is needed to provide sufficient time for the CRT controller to read a character and attribute code from the video buffer, to gain access to the character generator, and go through the Horizontal PEL Panning register in the attribute controller. Each access requires the 'display enable' signal to be skewed one character clock so that the video output is synchronized with the horizontal and vertical retrace signals." as well as "Note: Character skew is not adjustable on the Type 2 video and the bits are ignored; however, programs should set these bits for the appropriate skew to maintain compatibility."  This may be required for some early IBM VGA implementations or may be simply an unused "feature" carried over along with its register description from the IBM EGA implementations that require the use of this field. 
		 */
		u8 displayEnableSkew;

		/**
		 * This contains bits 4-0 of the End Horizontal Blanking field which specifies the end of the horizontal blanking period.  Bit 5 is located After the period has begun as specified by the Start Horizontal Blanking field, the 6-bit value of this field is compared against the lower 6 bits of the character clock.  When a match occurs, the horizontal blanking signal is disabled.  This provides from 1 to 64 character clocks although some implementations may match in the character clock specified by the Start Horizontal Blanking field, in which case the range is 0 to 63.  Note that if blanking extends past the end of the scan line, it will end on the first match of this field on the next scan line.
		 */
		u8 endHorizontalBlanking;

		CRTC_EndHorizontalBlanking(bool enableVerticalRetraceAccess, 
								   u8 displayEnableSkew, 
								   u8 endHorizontalBlanking) : 
			enableVerticalRetraceAccess(enableVerticalRetraceAccess),
			displayEnableSkew(displayEnableSkew),
			endHorizontalBlanking(endHorizontalBlanking)
		{

		}

		void Read_Conv(u8 regValue) override
		{
			enableVerticalRetraceAccess = !!(regValue & (1 << 7));
			displayEnableSkew = (regValue & 0b1100000) >> 5;
			endHorizontalBlanking = (regValue & 0b11111) >> 0;
		}

		u8 Write_Conv() override
		{
			u8 reg = 0;
			reg |= ((int)enableVerticalRetraceAccess) << 7;
			reg |= (displayEnableSkew << 5) & 0b1100000;
			reg |= (endHorizontalBlanking << 0) & 0b11111;
			return reg;
		}
	};

	struct CRTC_StartHorizontalRetrace : public VGARegister<Read_3D4, Write_3D4, 0x04, 0xff>
	{
		using VGARegister::VGARegister;
		
		/**
		 * This field specifies the character clock at which the VGA begins sending the horizontal synchronization pulse to the display which signals the monitor to retrace back to the left side of the screen.  The end of this pulse is controlled by the End Horizontal Retrace field.  This pulse may appear anywhere in the scan line, as well as set to a position beyond the Horizontal Total field which effectively disables the horizontal synchronization pulse.
		 */
		u8 startHorizontalRetrace;

		CRTC_StartHorizontalRetrace(u8 startHorizontalRetrace) : 
			startHorizontalRetrace(startHorizontalRetrace)
		{

		}

		void Read_Conv(u8 regValue) override
		{
			startHorizontalRetrace = regValue;
		}

		u8 Write_Conv() override
		{
			return startHorizontalRetrace;
		}
	};

	struct CRTC_EndHorizontalRetrace : public VGARegister<Read_3D4, Write_3D4, 0x05, 0xff>
	{
		using VGARegister::VGARegister;
		
		/**
		 * This contains bit 5 of the End Horizontal Blanking field.  See the End Horizontal Blanking Register for details. 
		 */
		u8 endHorizontalBlanking_5;

		/**
		 * This field delays the start of the horizontal retrace period by the number of character clocks equal to the value of this field.  From observation, this field is programmed to 0, with the exception of the 40 column text modes where this field is set to 1.  The VGA hardware simply acts as if this value is added to the Start Horizontal Retrace field.  According to IBM documentation, "For certain modes, the 'horizontal retrace' signal takes up the entire blanking interval. Some internal timings are generated by the falling edge of the 'horizontal retrace' signal. To ensure that the signals are latched properly, the 'retrace' signal is started before the end of the 'display enable' signal and then skewed several character clock times to provide the proper screen centering."  This does not appear to be the case, leading me to believe this is yet another holdout from the IBM EGA implementations that do require the use of this field. 
		 */
		u8 horizontalRetraceSkew;

		/**
		 * This field specifies the end of the horizontal retrace period, which begins at the character clock specified in the Start Horizontal Retrace field.  The horizontal retrace signal is enabled until the lower 5 bits of the character counter match the 5 bits of this field.  This provides for a horizontal retrace period from 1 to 32 character clocks.  Note that some implementations may match immediately instead of 32 clocks away, making the effective range 0 to 31 character clocks.
		 */
		u8 endHorizontalRetrace;

		CRTC_EndHorizontalRetrace(u8 endHorizontalBlanking_5, u8 horizontalRetraceSkew, u8 endHorizontalRetrace) : 
			endHorizontalBlanking_5(endHorizontalBlanking_5),
			horizontalRetraceSkew(horizontalRetraceSkew),
			endHorizontalRetrace(endHorizontalRetrace)
		{

		}

		void Read_Conv(u8 regValue) override
		{
			endHorizontalBlanking_5 = (!!(regValue & (1 << 7))) << 5;
			horizontalRetraceSkew = (regValue & 0b1100000) >> 5;
			endHorizontalRetrace = (regValue & 0b11111) >> 0;
		}

		u8 Write_Conv() override
		{
			u8 reg = 0;
			reg |= (!!endHorizontalBlanking_5) << 7;
			reg |= (horizontalRetraceSkew << 5) & 0b1100000;
			reg |= (endHorizontalRetrace << 0) & 0b11111;
			return reg;
		}
	};

	struct CRTC_VerticalTotal : public VGARegister<Read_3D4, Write_3D4, 0x06, 0xff>
	{
		using VGARegister::VGARegister;
		
		/**
		 * This contains the lower 8 bits of the Vertical Total field.  Bits 9-8 of this field are located in the Overflow Register. This field determines the number of scanlines in the active display and thus the length of each vertical retrace.  This field contains the value of the scanline counter at the beginning of the last scanline in the vertical period.
		 */
		u16 verticalTotal;

		CRTC_VerticalTotal(u16 verticalTotal) : 
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
		u16 startVerticalBlanking_8;

		CRTC_OverflowRegister(u16 verticalRetraceStart_8_9, 
							  u16 verticalDisplayEnd_8_9, 
							  u16 verticalTotal_8_9, 
							  u16 lineCompare_8, 
							  u16 startVerticalBlanking_8) : 
			verticalRetraceStart_8_9(verticalRetraceStart_8_9),
			verticalDisplayEnd_8_9(verticalDisplayEnd_8_9),
			verticalTotal_8_9(verticalTotal_8_9),
			lineCompare_8(lineCompare_8),
			startVerticalBlanking_8(startVerticalBlanking_8)
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
			startVerticalBlanking_8 = (!!(regValue & (1 << 3))) << 8;
		}

		u8 Write_Conv() override
		{
			u8 reg = 0;
			reg |= (!!(verticalRetraceStart_8_9 & (1 << 9)))	<< 7;
			reg |= (!!(verticalDisplayEnd_8_9 & (1 << 9)))		<< 6;
			reg |= (!!(verticalTotal_8_9 & (1 << 9)))			<< 5;
			reg |= (!!(lineCompare_8 & (1 << 8)))				<< 4;
			reg |= (!!(startVerticalBlanking_8 & (1 << 8)))		<< 3;
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
		 * The value of this field is added to the Start Address Register when calculating the display memory address for the upper left hand pixel or character of the screen. This allows for a maximum shift of 15, 31, or 35 pixels without having to reprogram the Start Address Register. 
		 */
		u8 bytePanning;

		/**
		 * This field is used when using text mode or any mode with a non-zero Maximum Scan Line field to provide for more precise vertical scrolling than the Start Address Register provides. The value of this field specifies how many scan lines to scroll the display upwards. Valid values range from 0 to the value of the Maximum Scan Line field. Invalid values may cause undesired effects and seem to be dependent upon the particular VGA implementation.
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
		 *  "When this bit is set to 1, 200-scan-line video data is converted to 400-scan-line output. To do this, the clock in the row scan counter is divided by 2, which allows the 200-line modes to be displayed as 400 lines on the display (this is called double scanning; each line is displayed twice). When this bit is set to 0, the clock to the row scan counter is equal to the horizontal scan rate." 
		 */
		bool scanDoubling;

		/**
		 * Specifies bit 9 of the Line Compare field. See the Line Compare Register for details. 
		 */
		u16 lineCompare_9;

		/**
		 * Specifies bit 9 of the Start Vertical Blanking field.  See the Start Vertical Blanking Register for details. 
		 */
		u16 startVerticalBlanking_9;

		/**
		 * In text modes, this field is programmed with the character height - 1 (scan line numbers are zero based.) In graphics modes, a non-zero value in this field will cause each scan line to be repeated by the value of this field + 1.
		 */
		u8 maximumScanLine;

		CRTC_MaximumScanLineRegister(bool scanDoubling,
									 u16 lineCompare_9,
									 u16 startVerticalBlanking_9,
									 u8 maximumScanLine) : 
			scanDoubling(scanDoubling),
			lineCompare_9(lineCompare_9),
			startVerticalBlanking_9(startVerticalBlanking_9),
			maximumScanLine(maximumScanLine)
		{

		}

		void Read_Conv(u8 regValue) override
		{
			scanDoubling = (!!(regValue & (1 << 7)));
			lineCompare_9 = (!!(regValue & (1 << 6))) << 9;
			startVerticalBlanking_9 = (!!(regValue & (1 << 5))) << 9;
			maximumScanLine = (regValue & 0b11111);
		}

		u8 Write_Conv() override
		{
			u8 reg = 0;
			reg |= ((int)scanDoubling)						<< 7;
			reg |= (!!(lineCompare_9 & (1 << 9)))			<< 6;
			reg |= (!!(startVerticalBlanking_9 & (1 << 9)))	<< 5;
			reg |= (maximumScanLine & 0b11111)				<< 0;
			return reg;
		}
	};

	struct CRTC_CursorStartRegister : public VGARegister<Read_3D4, Write_3D4, 0x0A, 0xFF>
	{
		using VGARegister::VGARegister;
		
		/**
		 * This field controls whether or not the text-mode cursor is displayed. Values are:
		 * 0 -- Cursor Enabled
		 * 1 -- Cursor Disabled
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
		 * This field was necessary in the EGA to synchronize the cursor with internal timing. In the VGA it basically is added to the cursor location. In some cases when this value is non-zero and the cursor is near the left or right edge of the screen, the cursor will not appear at all, or a second cursor above and to the left of the actual one may appear. This behavior may not be the same on all VGA compatible adapter cards.
		 */
		u8 cursorSkew;

		/**
		 * This field controls the appearance of the text-mode cursor by specifying the scan line location within a character cell at which the cursor should end, with the top-most scan line in a character cell being 0 and the bottom being with the value of the Maximum Scan Line field. If this field is less than the Cursor Scan Line Start field, the cursor is not drawn. Some graphics adapters, such as the IBM EGA display a split-block cursor instead.
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
		 * This contains the bits 7-0 of the Start Address field. The upper 8 bits are specified by the Start Address High Register. The Start Address field specifies the display memory address of the upper left pixel or character of the screen. Because the standard VGA has a maximum of 256K of memory, and memory is accessed 32 bits at a time, this 16-bit field is sufficient to allow the screen to start at any memory address. Normally this field is programmed to 0h, except when using virtual resolutions, paging, and/or split-screen operation. Note that the VGA display will wrap around in display memory if the starting address is too high. (This may or may not be desirable, depending on your intentions.)
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
		 * This field specifies bits 7-0 of the Cursor Location field. When the VGA hardware is displaying text mode and the text-mode cursor is enabled, the hardware compares the address of the character currently being displayed with sum of value of this field and the sum of the Cursor Skew field. If the values equal then the scan lines in that character specified by the Cursor Scan Line Start field and the Cursor Scan Line End field are replaced with the foreground color.
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
		 * This field specifies bits 7-0 of the Vertical Retrace Start field.  Bits 9-8 are located in the Overflow Register.  This field controls the start of the vertical retrace pulse which signals the display to move up to the beginning of the active display.  This field contains the value of the vertical scanline counter at the beginning of the first scanline where the vertical retrace signal is asserted.
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
		 * This field is used to protect the video timing registers from being changed by programs written for earlier graphics chipsets that attempt to program these registers with values unsuitable for VGA timings.  When this field is set to 1, the CRTC register indexes 00h-07h ignore write access, with the exception of bit 4 of the Overflow Register, which holds bit 8 of the Line Compare field. 
		 */
		bool protectEnabled;

		/**
		 * Nearly all video chipsets include a few registers that control memory, bus, or other timings not directly related to the output of the video card.  Most VGA/SVGA implementations ignore the value of this field; however, in the least, IBM VGA adapters do utilize it and thus for compatibility with these chipsets this field should be programmed.  This register is used in the IBM VGA hardware to control the number of DRAM refresh cycles per scan line.  The three refresh cycles per scanline is appropriate for the IBM VGA horizontal frequency of approximately 31.5 kHz.  For horizontal frequencies greater than this, this setting will work as the DRAM will be refreshed more often.  However, refreshing not often enough for the DRAM can cause memory loss.  Thus at some point slower than 31.5 kHz the five refresh cycle setting should be used.  At which particular point this should occur, would require better knowledge of the IBM VGA's schematics than I have available.  According to IBM documentation, "Selecting five refresh cycles allows use of the VGA chip with 15.75 kHz displays." which isn't really enough to go by unless the mode you are defining has a 15.75 kHz horizontal frequency. 
		 */
		bool memoryRefreshBandwidth; // TODO: enum

		/**
		 * This field determines the end of the vertical retrace pulse, and thus its length.  This field contains the lower four bits of the vertical scanline counter at the beginning of the scanline immediately after the last scanline where the vertical retrace signal is asserted.
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
		 * This contains the bits 7-0 of the Vertical Display End field.  Bits 9-8 are located in the Overflow Register.  The field contains the value of the vertical scanline counter at the beggining of the scanline immediately after the last scanline of active display.
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
		 * This field specifies the address difference between consecutive scan lines or two lines of characters. Beginning with the second scan line, the starting scan line is increased by twice the value of this register multiplied by the current memory address size (byte = 1, word = 2, double-word = 4) each line. For text modes the following equation is used:
         * 		Offset = Width / ( MemoryAddressSize * 2 )
		 * and in graphics mode, the following equation is used:
         * 		Offset = Width / ( PixelsPerAddress * MemoryAddressSize * 2 )
		 * where Width is the width in pixels of the screen. This register can be modified to provide for a virtual resolution, in which case Width is the width is the width in pixels of the virtual screen. PixelsPerAddress is the number of pixels stored in one display memory address, and MemoryAddressSize is the current memory addressing size.
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
		 *  "When this bit is set to 1, memory addresses are doubleword addresses. See the description of the word/byte mode bit (bit 6) in the CRT Mode Control Register" 
		 */
		bool doubleWordAddressing;

		/**
		 *  "When this bit is set to 1, the memory-address counter is clocked with the character clock divided by 4, which is used when doubleword addresses are used." 
		 */
		bool divideMemoryAddressClockBy4;

		/**
		 *  "These bits specify the horizontal scan line of a character row on which an underline occurs. The value programmed is the scan line desired minus 1."
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
		 * This contains bits 7-0 of the Start Vertical Blanking field.  Bit 8 of this field is located in the Overflow Register, and bit 9 is located in the Maximum Scan Line Register.  This field determines when the vertical blanking period begins, and contains the value of the vertical scanline counter at the beginning of the first vertical scanline of blanking.
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
		 * This field determines when the vertical blanking period ends, and contains the value of the vertical scanline counter at the beginning of the vertical scanline immediately after the last scanline of blanking.
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
		 *  "When set to 0, this bit disables the horizontal and vertical retrace signals and forces them to an inactive level. When set to 1, this bit enables the horizontal and vertical retrace signals. This bit does not reset any other registers or signal outputs." 
		 */
		bool syncEnabled;

		/**
		 * "When this bit is set to 0, the word mode is selected. The word mode shifts the memory-address counter bits to the left by one bit; the most-significant bit of the counter appears on the least-significant bit of the memory address outputs.  The doubleword bit in the Underline Location register (0x14) also controls the addressing. When the doubleword bit is 0, the word/byte bit selects the mode. When the doubleword bit is set to 1, the addressing is shifted by two bits. When set to 1, bit 6 selects the byte address mode." 
		 */
		bool wordByteMode;

		/**
		 * "This bit selects the memory-address bit, bit MA 13 or MA 15, that appears on the output pin MA 0, in the word address mode. If the VGA is not in the word address mode, bit 0 from the address counter appears on the output pin, MA 0. When set to 1, this bit selects MA 15. In odd/even mode, this bit should be set to 1 because 256KB of video memory is installed on the system board. (Bit MA 13 is selected in applications where only 64KB is present. This function maintains compatibility with the IBM Color/Graphics Monitor Adapter.)" 
		 */
		bool addressWrapSelect;

		/**
		 * "When this bit is set to 0, the address counter uses the character clock. When this bit is set to 1, the address counter uses the character clock input divided by 2. This bit is used to create either a byte or word refresh address for the display buffer." 
		 */
		bool divideMemoryAddressClockBy2;

		/**
		 * "This bit selects the clock that controls the vertical timing counter. The clocking is either the horizontal retrace clock or horizontal retrace clock divided by 2. When this bit is set to 1. the horizontal retrace clock is divided by 2. Dividing the clock effectively doubles the vertical resolution of the CRT controller. The vertical counter has a maximum resolution of 1024 scan lines because the vertical total value is 10-bits wide. If the vertical counter is clocked with the horizontal retrace divided by 2, the vertical resolution is doubled to 2048 scan lines." 
		 */
		bool divideScanLineClockBy2;

		/**
		 *  "This bit selects the source of bit 14 of the output multiplexer. When this bit is set to 0, bit 1 of the row scan counter is the source. When this bit is set to 1, the bit 14 of the address counter is the source." 
		 */
		bool mapDisplayAddress14;

		/**
		 *  "This bit selects the source of bit 13 of the output multiplexer. When this bit is set to 0, bit 0 of the row scan counter is the source, and when this bit is set to 1, bit 13 of the address counter is the source. The CRT controller used on the IBM Color/Graphics Adapter was capable of using 128 horizontal scan-line addresses. For the VGA to obtain 640-by-200 graphics resolution, the CRT controller is  programmed for 100 horizontal scan lines with two scan-line addresses per character row. Row scan  address bit 0 becomes the most-significant address bit to the display buffer. Successive scan lines of  the display image are displaced in 8KB of memory. This bit allows compatibility with the graphics modes of earlier adapters."
		 */
		bool mapDisplayAddress13;

		CRTC_ModeControlRegister(bool syncEnabled, 
								 bool wordByteMode, 
								 bool addressWrapSelect,
								 bool divideMemoryAddressClockBy2, 
								 bool divideScanLineClockBy2,
								 bool mapDisplayAddress14, 
								 bool mapDisplayAddress13) : 
			syncEnabled(syncEnabled),
			wordByteMode(wordByteMode),
			addressWrapSelect(addressWrapSelect),
			divideMemoryAddressClockBy2(divideMemoryAddressClockBy2),
			divideScanLineClockBy2(divideScanLineClockBy2),
			mapDisplayAddress14(mapDisplayAddress14),
			mapDisplayAddress13(mapDisplayAddress13)
		{

		}

		void Read_Conv(u8 regValue) override
		{
			syncEnabled					= !!(regValue & 0b10000000);
			wordByteMode				= !!(regValue & 0b01000000);
			addressWrapSelect			= !!(regValue & 0b00100000);
			divideMemoryAddressClockBy2	= !!(regValue & 0b00001000);
			divideScanLineClockBy2		= !!(regValue & 0b00000100);
			mapDisplayAddress14			= !!(regValue & 0b00000010);
			mapDisplayAddress13			= !!(regValue & 0b00000001);
		}

		u8 Write_Conv() override
		{
			u8 reg = 0;
			reg |= ((int)syncEnabled & 0b1)					<< 7;
			reg |= ((int)wordByteMode & 0b1)				<< 6;
			reg |= ((int)addressWrapSelect & 0b1)			<< 5;
			reg |= ((int)divideMemoryAddressClockBy2 & 0b1)	<< 3;
			reg |= ((int)divideScanLineClockBy2 & 0b1)		<< 2;
			reg |= ((int)mapDisplayAddress14 & 0b1)			<< 1;
			reg |= ((int)mapDisplayAddress13 & 0b1)			<< 0;
			return reg;
		}
	};

	struct CRTC_LineCompareRegister : public VGARegister<Read_3D4, Write_3D4, 0x18, 0xFF>
	{
		using VGARegister::VGARegister;
		
		/**
		 * This field specifies bits 7-0 of the Line Compare field. Bit 9 of this field is located in the Maximum Scan Line Register, and bit 8 of this field is located in the Overflow Register. The Line Compare field specifies the scan line at which a horizontal division can occur, providing for split-screen operation. If no horizontal division is required, this field should be set to 3FFh. When the scan line counter reaches the value in the Line Compare field, the current scan line address is reset to 0 and the Preset Row Scan is presumed to be 0. If the Pixel Panning Mode field is set to 1 then the Pixel Shift Count and Byte Panning fields are reset to 0 for the remainder of the display cycle.
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

	struct HorizontalTiming
	{
		u8 total;
		u8 displayEnd;
		u8 blankingStart;
		u8 blankingEnd;
		u8 displaySkew;
		u8 retraceStart;
		u8 retraceEnd;

		void Read();
		void Write();
	};

	struct VerticalTiming
	{
		u16 total;
		u16 displayEnd;
		u16 blankingStart;
		u16 blankingEnd;
		u16 retraceStart;
		u8 retraceEnd;

		void Read();
		void Write();
	};

	/**
	 * Misc
	 */

	struct MiscOutput
	{
		/**
		 * Determines the polarity of the vertical sync pulse and can be used (with HSP) to control the vertical size
		 * of the display by utilizing the autosynchronization feature of VGA displays.
		 * = 0 selects a positive vertical retrace sync pulse.
		 */
		bool vSyncPolarity;

		/**
		 * Determines the polarity of the horizontal sync pulse.
  		 * = 0 selects a positive horizontal retrace sync pulse.
		 */
		bool hSyncPolarity;

		/**
		 * Selects the upper/lower 64K page of memory when the system is in an eve/odd mode (modes 0,1,2,3,7).
  		 * = 0 selects the low page
  		 * = 1 selects the high page
		 */
		bool oddEvenPageSelect;

		/**
		 * This field controls the selection of the dot clocks used in driving the display timing.
		 * The standard hardware has 2 clocks available to it, nominally 25 Mhz and 28 Mhz.
		 * It is possible that there may be other "external" clocks that can be selected by programming this register with the undefined values.
		 * The possible valuse of this register are:
		 *
    	 * 00 -- select 25 Mhz clock (used for 320/640 pixel wide modes)
    	 * 01 -- select 28 Mhz clock (used for 360/720 pixel wide modes)
    	 * 10 -- undefined (possible external clock)
    	 * 11 -- undefined (possible external clock)
		*/
		enum ClockSelect
		{
			Clock_25MHz	= 0b00,
			Clock_28MHz	= 0b01,
			Undefined1	= 0b10,
			Undefined2	= 0b11,
		} clockSelect;

		/**
		 * Controls system access to the display buffer.
		 * = 0 disables address decode for the display buffer from the system
		 * = 1 enables address decode for the display buffer from the system
		 */
		bool ramEnabled;

		/**
		 * This bit selects the CRT controller addresses.
		 * When set to 0, this bit sets the CRT controller addresses to 0x03Bx and the address
		 * for the Input Status Register 1 to 0x03BA for compatibility withthe monochrome adapter.
		 * When set to 1, this bit sets CRT controller addresses to 0x03Dx and
		 * the Input Status Register 1 address to 0x03DA for compatibility with the color/graphics adapter.
		 * The Write addresses to the Feature Control register are affected in the same manner.
		 */
		bool inputOutputAddressSelect;

		void Read()
		{
			u8 reg = Read_3C2();
			vSyncPolarity = !!(reg & (1 << 7));
			hSyncPolarity = !!(reg & (1 << 6));
			oddEvenPageSelect = !!(reg & (1 << 5));

			clockSelect = (ClockSelect)((reg & 0b1100) >> 2);

			ramEnabled = !!(reg & (1 << 1));
			inputOutputAddressSelect = !!(reg & (1 << 0));
		}

		void Write()
		{
			u8 reg = Read_3C2();
			reg &= ~(0b11101111);
			reg |= ((int)vSyncPolarity) << 7;
			reg |= ((int)hSyncPolarity) << 6;
			reg |= ((int)oddEvenPageSelect) << 5;
			reg |= ((int)clockSelect) << 2;
			reg |= ((int)ramEnabled) << 1;
			reg |= ((int)inputOutputAddressSelect) << 0;
			Write_3C2(reg);
		}
	};

	struct InputStatus
	{
		/**
		 * When set to 1, this bit indicates that the display is in a vertical retrace interval.
		 * This bit can be programmed, through the Vertical Retrace End register,
		 * to generate an interrupt at the start of the vertical retrace.
		 */
		bool vRetrace;

		/**
		 * When set to 1, this bit indicates a horizontal or vertical retrace interval.
		 * This bit is the real-time status of the inverted 'display enable' signal.
		 * Programs have used this status bit to restrict screen updates to the inactive display
		 * intervals in order to reduce screen flicker.
		 * The video subsystem is designed to eliminate this software requirement;
		 * screen updates may be made at any time without screen degradation.
		 */
		bool displayDisabled;

		void Read()
		{
			u8 reg = HAL::In8(0x3DA);// Read_3DA();
			vRetrace = !!(reg & (1 << 3));
			displayDisabled = !!(reg & (1 << 0));
		}
	};

	/**
	 * Sequencer
	 */

	/**
	 * The Sequencer Registers are accessed via a pair of registers, the Sequencer Address Register and the Sequencer Data Register. See the Accessing the VGA Registers section for more detals. The Address Register is located at port 3C4h and the Data Register is located at port 3C5h.
	 *
     * Index 00h -- Reset Register
     * Index 01h -- Clocking Mode Register
     * Index 02h -- Map Mask Register
     * Index 03h -- Character Map Select Register
     * Index 04h -- Sequencer Memory Mode Register
	 *
	 */

	struct Seq_00 // Reset Register
	{
		/**
		 * When set to 0, this bit commands the sequencer to synchronously clear and halt.
		 * Bits 1 and 0 must be 1 to allow the sequencer to operate.
		 * To prevent the loss of data, bit 1 must be set to 0 during the active display interval
		 * before changing the clock selection.
		 * The clock is changed through the Clocking Mode register or the Miscellaneous Output register.
		 */
		bool syncReset;

		/**
		 * When set to 0, this bit commands the sequencer to asynchronously clear and halt.
		 * Resetting the sequencer with this bit can cause loss of video data
		 */
		bool asyncReset;

		void Read()
		{
			u8 reg = Read_3C4(0x00);
			syncReset = !!(reg & (1 << 1));
			asyncReset = !!(reg & (1 << 0));
		}

		void Write()
		{
			u8 reg = Read_3C4(0x00);
			reg &= ~(0b11);
			reg |= ((int)syncReset) << 1;
			reg |= ((int)asyncReset) << 0;
			Write_3C4(0x00, reg);
		}
	};

	struct Seq_01 // Clocking Mode Register
	{
		/**
		 * When set to 1, this bit turns off the display and assigns maximum memory bandwidth to the system.
		 * Although the display is blanked, the synchronization pulses are maintained.
		 * This bit can be used for rapid full-screen updates.
		 */
		bool screenDisabled;

		/**
		 * When the Shift 4 field and the Shift Load Field are set to 0, the video serializers are loaded
		 * every character clock.
		 * When the Shift 4 field is set to 1, the video serializers are loaded every forth character clock,
		 * which is useful when 32 bits are fetched per cycle and chained together in the shift registers.
		 */
		bool shift4Enabled;

		/**
		 * When set to 0, this bit selects the normal dot clocks derived from the sequencer master clock input.
		 * When this bit is set to 1, the master clock will be divided by 2 to generate the dot clock.
		 * All other timings are affected because they are derived from the dot clock.
		 * The dot clock divided by 2 is used for 320 and 360 horizontal PEL modes.
		 */
		bool dotClockRate;

		/**
		 * When this bit and bit 4 are set to 0, the video serializers are loaded every character clock.
		 * When this bit is set to 1, the video serializers are loaded every other character clock,
		 * which is useful when 16 bits are fetched per cycle and chained together in the shift registers.
		 * The Type 2 video behaves as if this bit is set to 0; therefore, programs should set it to 0.
		 */
		bool shiftLoadRate;

		/**
		 * This field is used to select whether a character is 8 or 9 dots wide.
		 * This can be used to select between 720 and 640 pixel modes (or 360 and 320) and also is used to
		 * provide 9 bit wide character fonts in text mode.
		 * The possible values for this field are:
		 *
    	 * 0 - Selects 9 dots per character.
    	 * 1 - Selects 8 dots per character.
		 *
		 */
		bool dot8Mode;

		void Read()
		{
			u8 reg = Read_3C4(0x01);
			screenDisabled = !!(reg & (1 << 5));
			shift4Enabled = !!(reg & (1 << 4));
			dotClockRate = !!(reg & (1 << 3));
			shiftLoadRate = !!(reg & (1 << 2));
			dot8Mode = !!(reg & (1 << 0));
		}

		void Write()
		{
			u8 reg = Read_3C4(0x01);
			reg &= ~(0b00111101);
			reg |= ((int)screenDisabled) << 5;
			reg |= ((int)shift4Enabled) << 4;
			reg |= ((int)dotClockRate) << 3;
			reg |= ((int)shiftLoadRate) << 2;
			reg |= ((int)dot8Mode) << 0;
			Write_3C4(0x01, reg);
		}
	};

	struct Seq_02 // MapMaskRegister
	{
		/**
		 * Bits 3-0 of this field correspond to planes 3-0 of the VGA display memory.
		 * If a bit is set, then write operations will modify the respective plane of display memory.
		 * If a bit is not set then write operations will not affect the respective plane of display memory.
		 */
		bool memoryPlaneWriteEnabled[4];

		void Read()
		{
			u8 reg = Read_3C4(0x02);
			memoryPlaneWriteEnabled[0] = !!(reg & (1 << 0));
			memoryPlaneWriteEnabled[1] = !!(reg & (1 << 1));
			memoryPlaneWriteEnabled[2] = !!(reg & (1 << 2));
			memoryPlaneWriteEnabled[3] = !!(reg & (1 << 3));
		}

		void Write()
		{
			u8 reg = Read_3C4(0x02);
			reg &= ~(0b00001111);
			reg |= ((int)memoryPlaneWriteEnabled[0]) << 0;
			reg |= ((int)memoryPlaneWriteEnabled[1]) << 1;
			reg |= ((int)memoryPlaneWriteEnabled[2]) << 2;
			reg |= ((int)memoryPlaneWriteEnabled[3]) << 3;
			Write_3C4(0x02, reg);
		}
	};

	struct Seq_03 // CharacterMapSelect
	{
		/**
		 * This field is used to select the font that is used in text mode when bit 3 of the attribute byte for a character is set to 1. Note that this field is not contiguous in order to provide EGA compatibility. The font selected resides in plane 2 of display memory at the address specified by this field, as follows:
		 *
    	 * 000b -- Select font residing at 0000h - 1FFFh
    	 * 001b -- Select font residing at 4000h - 5FFFh
    	 * 010b -- Select font residing at 8000h - 9FFFh
    	 * 011b -- Select font residing at C000h - DFFFh
    	 * 100b -- Select font residing at 2000h - 3FFFh
    	 * 101b -- Select font residing at 6000h - 7FFFh
    	 * 110b -- Select font residing at A000h - BFFFh
    	 * 111b -- Select font residing at E000h - FFFFh
		 *
		 */
		u8 characterSetASelect;
		/**
		 * This field is used to select the font that is used in text mode when bit 3 of the attribute byte for a character is set to 0. Note that this field is not contiguous in order to provide EGA compatibility. The font selected resides in plane 2 of display memory at the address specified by this field, identical to the mapping used by Character Set A Select above.
		 */
		u8 characterSetBSelect;

		void Read()
		{
			u8 reg = Read_3C4(0x03);
			characterSetASelect = ((reg & 0b00100000) >> 3) | ((reg & 0b1100) >> 2);
			characterSetBSelect = ((reg & 0b00010000) >> 2) | ((reg & 0b0011) >> 0);
		}

		void Write()
		{
			u8 reg = Read_3C4(0x03);
			reg &= ~(0b00111111);
			reg |= (characterSetASelect & 0b100) << 3;
			reg |= (characterSetASelect & 0b011) << 2;
			reg |= (characterSetBSelect & 0b100) << 2;
			reg |= (characterSetBSelect & 0b011) << 0;
			Write_3C4(0x03, reg);
		}
	};

	struct Seq_04 // SequencerMemoryMode
	{
		/**
		 * This bit controls the map selected during system read operations.
		 * When set to 0, this bit enables system addresses to sequentially access data within a bit
		 * map by using the Map Mask register.
		 * When set to 1, this bit causes the two low-order bits to select the map accessed as shown below.
		 * Address Bits
		 * A0 A1            Map Selected
		 * 0   0              0
		 * 0   1              1
		 * 1   0              2
		 * 1   1              3
		 */
		bool chain4Enabled;

		/**
		 * When this bit is set to 0, even system addresses access maps 0 and 2, while odd system addresses access
		 * maps 1 and 3.
		 * When this bit is set to 1, system addresses sequentially access data within a bit map,
		 * and the maps are accessed according to the value in the Map Mask register (index 0x02).
		 */
		bool oddEvenHostMemoryWriteAddressingDisabled;

		/**
		 * When set to 1, this bit enables the video memory from 64KB to 256KB.
		 * This bit must be set to 1 to enable the character map selection described for the previous register.
		 */
		bool extendedMemory;

		void Read()
		{
			u8 reg = Read_3C4(0x04);
			chain4Enabled = !!(reg & 0b1000);
			oddEvenHostMemoryWriteAddressingDisabled = !!(reg & 0b0100);
			extendedMemory = !!(reg & 0b0010);
		}

		void Write()
		{
			u8 reg = Read_3C4(0x04);
			reg &= ~(0b1110);
			reg |= ((int)chain4Enabled) << 3;
			reg |= ((int)oddEvenHostMemoryWriteAddressingDisabled) << 2;
			reg |= ((int)extendedMemory) << 1;
			Write_3C4(0x04, reg);
		}
	};



	/**
	 * Graphics Controller
	 */

	/**
	 * The Graphics Registers are accessed via a pair of registers, the Graphics Address Register and the Graphics Data Register.
	 * See the Accessing the VGA Registers section for more details.
	 * The Address Register is located at port 3CEh and the Data Register is located at port 3CFh.
	 *
	 * Index 00h -- Set/Reset Register
     * Index 01h -- Enable Set/Reset Register
     * Index 02h -- Color Compare Register
     * Index 03h -- Data Rotate Register
     * Index 04h -- Read Map Select Register
     * Index 05h -- Graphics Mode Register
     * Index 06h -- Miscellaneous Graphics Register
     * Index 07h -- Color Don't Care Register
     * Index 08h -- Bit Mask Register
	 *
	 */

	struct GC_00 // SetResetRegister
	{
		/**
		 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
		 * This field is used by Write Mode 0 and Write Mode 3 (See the Write Mode field.)
		 * In Write Mode 0, if the corresponding bit in the Enable Set/Reset field is set,
		 * and in Write Mode 3 regardless of the Enable Set/Reset field, the value of the
		 * bit in this field is expanded to 8 bits and substituted for the data of the
		 * respective plane and passed to the next stage in the graphics pipeline,
		 * which for Write Mode 0 is the Logical Operation unit and for Write Mode 3 is the Bit Mask unit.
		 */
		bool setResetValue[4];

		void Read()
		{
			u8 setResetReg = Read_3CE(0x00);
			setResetValue[0] = !!(setResetReg & 0b0001);
			setResetValue[1] = !!(setResetReg & 0b0010);
			setResetValue[2] = !!(setResetReg & 0b0100);
			setResetValue[3] = !!(setResetReg & 0b1000);
		}

		void Write()
		{
			u8 setResetReg = Read_3CE(0x00);
			setResetReg &= ~(0b1111);
			setResetReg |= ((int)setResetValue[0]) << 0;
			setResetReg |= ((int)setResetValue[1]) << 1;
			setResetReg |= ((int)setResetValue[2]) << 2;
			setResetReg |= ((int)setResetValue[3]) << 3;
			Write_3CE(0x00, setResetReg);
		}
	};

	struct GC_01 // EnableSetResetRegister
	{
		/**
		 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
		 * This field is used in Write Mode 0 (See the Write Mode field) to select
		 * whether data for each plane is derived from host data or from expansion of the respective bit in the Set/Reset field.
		 */
		bool setResetEnable[4];

		void Read()
		{
			u8 enableSetResetReg = Read_3CE(0x01);
			setResetEnable[0] = !!(enableSetResetReg & 0b0001);
			setResetEnable[1] = !!(enableSetResetReg & 0b0010);
			setResetEnable[2] = !!(enableSetResetReg & 0b0100);
			setResetEnable[3] = !!(enableSetResetReg & 0b1000);
		}

		void Write()
		{
			u8 enSetResetReg = Read_3CE(0x01);
			enSetResetReg &= ~(0b1111);
			enSetResetReg |= ((int)setResetEnable[0]) << 0;
			enSetResetReg |= ((int)setResetEnable[1]) << 1;
			enSetResetReg |= ((int)setResetEnable[2]) << 2;
			enSetResetReg |= ((int)setResetEnable[3]) << 3;
			Write_3CE(0x01, enSetResetReg);
		}
	};

	struct GC_02 // ColorCompareRegister
	{
		/**
		 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
		 * This field holds a reference color that is used by Read Mode 1 (See the Read Mode field.)
		 * Read Mode 1 returns the result of the comparison between this value and a location of display memory, modified by the Color Don't Care field.
		 */
		bool memoryPlaneWriteEnable[4];

		void Read()
		{
			u8 mapMaskRegister = Read_3CE(0x02);
			memoryPlaneWriteEnable[0] = !!(mapMaskRegister & 0b0001);
			memoryPlaneWriteEnable[1] = !!(mapMaskRegister & 0b0010);
			memoryPlaneWriteEnable[2] = !!(mapMaskRegister & 0b0100);
			memoryPlaneWriteEnable[3] = !!(mapMaskRegister & 0b1000);
		}

		void Write()
		{
			u8 mapMaskReg = Read_3CE(0x02);
			mapMaskReg &= ~(0b1111);
			mapMaskReg |= ((int)memoryPlaneWriteEnable[0]) << 0;
			mapMaskReg |= ((int)memoryPlaneWriteEnable[1]) << 1;
			mapMaskReg |= ((int)memoryPlaneWriteEnable[2]) << 2;
			mapMaskReg |= ((int)memoryPlaneWriteEnable[3]) << 3;
			Write_3CE(0x02, mapMaskReg);
		}
	};

	struct GC_03 // DataRotateRegister
	{
		/**
		 * This field is used in Write Mode 0 and Write Mode 2 (See the Write Mode field.) The logical operation stage of the graphics pipeline is 32 bits wide (1 byte * 4 planes) and performs the operations on its inputs from the previous stage in the graphics pipeline and the latch register. The latch register remains unchanged and the result is passed on to the next stage in the pipeline. The results based on the value of this field are:
		 *
    	 * 00b - Result is input from previous stage unmodified.
    	 * 01b - Result is input from previous stage logical ANDed with latch register.
    	 * 10b - Result is input from previous stage logical ORed with latch register.
    	 * 11b - Result is input from previous stage logical XORed with latch register.
		 *
		 */
		enum LogicalOperation
		{
			LO_Normal	= 0,
			LO_AND		= 1,
			LO_OR		= 2,
			LO_XOR		= 3,
		} logicalOperation;

		/**
		 * This field is used in Write Mode 0 and Write Mode 3 (See the Write Mode field.)
		 * In these modes, the host data is rotated to the right by the value specified by the value of this field.
		 * A rotation operation consists of moving bits 7-1 right one position to bits 6-0, simultaneously wrapping bit 0 around to bit 7,
		 * and is repeated the number of times specified by this field.
		 */
		u8 rotateCount;

		void Read()
		{
			u8 dataRotateReg = Read_3CE(0x03);
			logicalOperation = (LogicalOperation)((dataRotateReg >> 3) & 0b11);
			rotateCount = (dataRotateReg >> 0) & 0b111;
		}

		void Write()
		{
			u8 dataRotateReg = Read_3CE(0x03);
			dataRotateReg &= ~(0b11111);
			dataRotateReg |= (logicalOperation << 3) & 0b11000;
			dataRotateReg |= (rotateCount << 0) & 0b111;
			Write_3CE(0x03, dataRotateReg);
		}
	};

	struct GC_04 // ReadMapSelectRegister
	{
		/**
		 * This value of this field is used in Read Mode 0 (see the Read Mode field) to specify the display memory plane to transfer data from.
		 * Due to the arrangement of video memory, this field must be modified four times to read one or more pixels values in the planar video modes.
		 */
		u8 mapSelect;

		void Read()
		{
			u8 v = Read_3CE(0x04);
			mapSelect = v & 0b11;
		}

		void Write()
		{
			u8 v = Read_3CE(0x04);
			v &= ~(0b11);
			v |= (mapSelect << 0) & 0b11;
			Write_3CE(0x04, v);
		}
	};

	struct GC_05 // GraphicsModeRegister
	{
		/**
		 * This field selects between two read modes, simply known as Read Mode 0, and Read Mode 1, based upon the value of this field:
		 *
    	 * 0b - Read Mode 0: In this mode, a byte from one of the four planes is returned on read operations.
		 * 		The plane from which the data is returned is determined by the value of the Read Map Select field.
		 * 1b - Read Mode 1: In this mode, a comparison is made between display memory and a reference color defined by the Color Compare field.
		 * 		Bit planes not set in the Color Don't Care field then the corresponding color plane is not considered in the comparison.
		 * 		Each bit in the returned result represents one comparison between the reference color, with the bit being set if the comparison is true.
		 *
		 */
		enum ReadMode
		{
			RM_0 = 0,
		} readMode;

		/**
		 * This field selects between four write modes, simply known as Write Modes 0-3, based upon the value of this field:
		 *
    	 * 00b - Write Mode 0: In this mode, the host data is first rotated as per the Rotate Count field,
		 * 		then the Enable Set/Reset mechanism selects data from this or the Set/Reset field.
		 * 		Then the selected Logical Operation is performed on the resulting data and the data in the latch register.
		 * 		Then the Bit Mask field is used to select which bits come from the resulting data and which come from the latch register.
		 * 		Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory.
    	 * 01b - Write Mode 1: In this mode, data is transferred directly from the 32 bit latch register to display memory,
		 * 		affected only by the Memory Plane Write Enable field. The host data is not used in this mode.
    	 * 10b - Write Mode 2: In this mode, the bits 3-0 of the host data are replicated across all 8 bits of their respective planes.
		 * 		Then the selected Logical Operation is performed on the resulting data and the data in the latch register.
		 * 		Then the Bit Mask field is used to select which bits come from the resulting data and which come from the latch register.
		 * 		Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory.
    	 * 11b - Write Mode 3: In this mode, the data in the Set/Reset field is used as if the Enable Set/Reset field were set to 1111b.
		 * 		Then the host data is first rotated as per the Rotate Count field, then logical ANDed with the value of the Bit Mask field.
		 * 		The resulting value is used on the data obtained from the Set/Reset field in the same way that the Bit Mask field would ordinarily be used.
		 * 		to select which bits come from the expansion of the Set/Reset field and which come from the latch register.
		 * 		Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory.
		 *
		 */
		enum WriteMode
		{
			WM_0 = 0,
			WM_3 = 3,
		} writeMode;

		/**
		 * When set to 0, this bit allows bit 5 to control the loading of the shift registers.
		 * When set to 1, this bit causes the shift registers to be loaded in a manner that supports the 256-color mode.
		 */
		bool shiftColor256;

		/**
		 * When set to 1, this bit directs the shift registers in the graphics controller to format the serial data stream
		 * with even-numbered bits from both maps on even-numbered maps, and odd-numbered bits from both maps on the odd-numbered maps.
		 * This bit is used for modes 4 and 5.
		 */
		bool shiftInterleaved;

		/**
		 * When set to 1, this bit selects the odd/even addressing mode used by the IBM Color/Graphics Monitor Adapter.
		 * Normally, the value here follows the value of Memory Mode register bit 2 in the sequencer.
		 */
		bool hostOddEven;

		void Read()
		{
			u8 reg = Read_3CE(0x05);
			readMode = (ReadMode)((reg >> 3) & 0b1);
			writeMode = (WriteMode)((reg >> 0) & 0b11);
			shiftColor256 = !!(reg & (1 << 6));
			shiftInterleaved = !!(reg & (1 << 5));
			hostOddEven = !!(reg & (1 << 4));
		}

		void Write()
		{
			u8 reg = Read_3CE(0x05);
			reg &= ~(0b01111011);
			reg |= ((int)writeMode) << 0;
			reg |= ((int)readMode) << 3;
			reg |= ((int)shiftColor256) << 6;
			reg |= ((int)shiftInterleaved) << 5;
			reg |= ((int)hostOddEven) << 4;
			Write_3CE(0x05, reg);
		}
	};

	struct GC_06 // Miscellaneous Graphics Register
	{
		/**
		 * This bit controls alphanumeric mode addressing.
		 * When set to 1, this bit selects graphics modes, which also disables the character generator latches.
		 */
		bool alphanumericModeDisabled;

		/**
		 * Chain O/E -- Chain Odd/Even Enable
		 * "When set to 1, this bit directs the system address bit, A0, to be replaced by a higher-order bit.
		 * The odd map is then selected when A0 is 1, and the even map when A0 is 0."
		 */
		bool chainOE;

		/**
		 * This field specifies the range of host memory addresses that is decoded by the VGA hardware and mapped into display memory accesses.
		 * The values of this field and their corresponding host memory ranges are:
		 *
		 * 00b -- A0000h-BFFFFh (128K region)
		 * 01b -- A0000h-AFFFFh (64K region)
		 * 10b -- B0000h-B7FFFh (32K region)
		 * 11b -- B8000h-BFFFFh (32K region)
		 *
		 */
		u8 memoryMapSelect;

		void Read()
		{
			u8 v = Read_3CE(0x06);
			alphanumericModeDisabled = !!(v & 0b1);
			chainOE = !!(v & 0b10);
			memoryMapSelect = (v >> 2) & 0b11;
		}

		void Write()
		{
			u8 v = Read_3CE(0x06);
			v &= ~(0b1111);
			v |= (memoryMapSelect << 2) & 0b1100;
			v |= (chainOE << 1) & 0b10;
			v |= (alphanumericModeDisabled << 0) & 0b1;
			Write_3CE(0x06, v);
		}
	};

	struct GC_07 // ColorDontCareRegister
	{
		/**
		 * Bits 3-0 of this field represent planes 3-0 of the VGA display memory.
		 * This field selects the planes that are used in the comparisons made by Read Mode 1 (See the Read Mode field.)
		 * Read Mode 1 returns the result of the comparison between the value of the Color Compare field and a location of display memory.
		 * If a bit in this field is set, then the corresponding display plane is considered in the comparison.
		 * If it is not set, then that plane is ignored for the results of the comparison.
		 */
		bool colorDontCare[4];

		void Read()
		{
			u8 v = Read_3CE(0x07);
			colorDontCare[0] = !!(v & 0b0001);
			colorDontCare[1] = !!(v & 0b0010);
			colorDontCare[2] = !!(v & 0b0100);
			colorDontCare[3] = !!(v & 0b1000);
		}

		void Write()
		{
			u8 v = Read_3CE(0x07);
			v &= ~(0b1111);
			v |= ((int)colorDontCare[0]) << 0;
			v |= ((int)colorDontCare[1]) << 1;
			v |= ((int)colorDontCare[2]) << 2;
			v |= ((int)colorDontCare[3]) << 3;
			Write_3CE(0x07, v);
		}
	};

	struct GC_08 // BitMaskRegister
	{
		/**
		 * This field is used in Write Modes 0, 2, and 3 (See the Write Mode field.)
		 * It it is applied to one byte of data in all four display planes.
		 * If a bit is set, then the value of corresponding bit from the previous stage in the graphics pipeline is selected; otherwise the value of the corresponding bit in the latch register is used instead.
		 * In Write Mode 3, the incoming data byte, after being rotated is logical ANDed with this byte and the resulting value is used in the same way this field would normally be used by itself.
		 */
		u8 bitMask;

		void Read()
		{
			bitMask = Read_3CE(0x08);
		}

		void Write()
		{
			Write_3CE(0x08, bitMask);
		}
	};

	bool Init();

	void SetCursor(bool enabled);
}