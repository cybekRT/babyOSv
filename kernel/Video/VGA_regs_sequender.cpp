#include"VGA_regs_sequencer.hpp"
#include"VGA_regs.hpp"

void VGA::Registers::Sequencer::Write()
{
	VGA::Registers::Seq_ResetRegister seq0 = {
			.syncReset = syncReset,
			.asyncReset = asyncReset
	};

	Seq_ClockingModeRegister seq1 = {
		.screenDisabled = screenDisabled,
		.shift4Enabled = shift4Enabled,
		.dotClockRate = dotClockRate,
		.shiftLoadRate = shiftLoadRate,
		.dot8Mode = dot8Mode
	};

	Seq_MapMaskRegister seq2 = {
		.memoryPlaneWriteEnabled = { memoryPlaneWriteEnabled[0], memoryPlaneWriteEnabled[1], memoryPlaneWriteEnabled[2], memoryPlaneWriteEnabled[3] }
	};

	Seq_CharacterMapSelectRegister seq3 = {
		.characterSetASelect = characterSetASelect,
		.characterSetBSelect = characterSetBSelect
	};

	Seq_SequencerMemoryModeRegister seq4 = {
		.chain4Enabled = chain4Enabled,
		.oddEvenHostMemoryWriteAddressingDisabled = oddEvenHostMemoryWriteAddressingDisabled,
		.extendedMemory = extendedMemory
	};

	seq0.Write();
	seq1.Write();
	seq2.Write();
	seq3.Write();
	seq4.Write();
}