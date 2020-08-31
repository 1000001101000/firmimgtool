# firmimgtool

Fork of the original tool used to build/extract firmware images for the first generation of Buffalo NAS devices (PowerPC and MIPS based Linkstation/Terastation/Kurobox).

So far I have:
- Created a simple Makefile that supports cross compile.
- Changed some data types to allow it to work on 64-bit platforms
- made other minor changes to address compiler warnings. 
- set up workflow to generate binaries for relavant platforms.


The original file can be found here:
https://web.archive.org/web/20170912222312/http://www.geocities.jp/trstat/firmhack.html

Looks like a slightly newer version also here:
https://web.archive.org/web/20150922131032/http://downloads.buffalo.nas-central.org/ALL_LS_KB_PPC/DevelopmentTools/Flash/Utilities/firmimgtool.c
