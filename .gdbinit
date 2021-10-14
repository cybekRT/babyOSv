target remote :1234
set pagination off
set disassembly-flavor intel
set disassemble-next-line on
display/i $pc
# monitor system_reset
