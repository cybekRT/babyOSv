%define OS_NAME 'babyOSv 0.1'

BPB_LOC equ 0x500
ROOT_LOC equ BPB_LOC + 0x200 ;0x700
; (0x7c00 + 0x200)
FAT_LOC equ ROOT_LOC + 0x200 ;0x900
FAT2_LOC equ FAT_LOC + 0x200 ;0x900 + 0x200
;FAT_LOC equ (ROOT_LOC + 0x200)

BOOT2_ADDR equ FAT2_LOC + 0x200 ;0xC00 + 0x200
KERNEL_ADDR equ 0x3000

; info line test.c:56
; info symbol 0x400225
; list *0xAABBCCDD
