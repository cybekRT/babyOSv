CLUSTER_EMPTY	equ 0x000
CLUSTER_LAST	equ 0xFF8

ENTRY_REMOVED	equ 0xE5
ENTRY_LAST	equ 0x00

struc FAT12_BPB
	._unused1		resb 3
	.oem			resb 8
	.bytesPerSector		resw 1
	.sectorsPerCluster	resb 1
	.reservedSectors	resw 1
	.fatsCount		resb 1
	.rootEntriesCount	resw 1
	.totalSectors		resw 1
	.mediaType		resb 1
	.sectorsPerFat		resw 1
	.sectorsPerTrack	resw 1
	.headsCount		resw 1
	.hiddenSectors		resd 1
	.totalSectorsHigh	resd 1
	; EBR
	.driveNumber		resb 1
	._unused2		resb 1
	.signature		resb 1
	.volumeId		resb 4
	.label			resb 11
	.systemId		resb 8
	.bootCode		resb 448
	.bootSignature		resb 2
endstruc

;struc FAT12_Handle
;	.driveNumber	resb 1
;	.bpbPtr		resb 2
;	.fatPtr		resb 2
;	.bufferPtr	resb 2
;endstruc

struc FAT12_Directory
	.firstCluster	resb 2
	.currentCluster	resb 2
	.currentOffset	resb 2
	.align		resb 10
	.buffer		resb 512
endstruc

FAT12_ATTR_ReadOnly		equ 0b1
FAT12_ATTR_Hidden		equ 0b10
FAT12_ATTR_System		equ 0b100
FAT12_ATTR_VolumeLabel		equ 0b1000
FAT12_ATTR_Directory		equ 0b10000
FAT12_ATTR_Archive		equ 0b100000
FAT12_ATTR_Device		equ 0b1000000
FAT12_ATTR_Reserved		equ 0b10000000
