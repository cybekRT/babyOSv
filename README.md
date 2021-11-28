# babyOSv

32-bit operating system for x86 (386/486) written in C.

Features currently "implemented":
- Protected mode
- Physical and logical memory allocator
- Kernel-space threads with preemptive scheduler
- Block devices: FDD HDD/ATA
- Partition types: MBR
- Filesystems: FAT12/FAT16/FAT32 virtual(BlkFS)
- Keyboard support
- Fancy terminal with printf-like syntax
- Skeleton of VFS
- Demo shell to test VFS
- 320x200 8bpp graphics mode (13h)

TODO:
- Improve VFS to be super flexible
- Executing programs in kernel-space
- GUI
- ...
- User-space

# Screenshots
Demo shell with uptime and list of threads
![](Screenshots/2021-11-28_another_random_screenshot.png)

13h mode splash read from floppy
![](Screenshots/2021-11-28_splash_from_floppy.png)