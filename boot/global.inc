%define OS_NAME 'babyOSv   v0.1'

;BPB_LOC equ 0x500
;ROOT_LOC equ 0x700
; (0x7c00 + 0x200)
;FAT_LOC equ 0x900
;FAT_LOC equ (ROOT_LOC + 0x200)

BOOT2_ADDR	equ 0xC00
KERNEL_ADDR equ 0x3000

; info line test.c:56
; info symbol 0x400225
; list *0xAABBCCDD
