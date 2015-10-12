rem
rem  *** QEMU-tef_em1d emulator execution ***
rem  Usage: qemu-net.bat <rom.bin> <sd.img>
rem
rem  -dipsw dbgsw=on  : Start T-Monitor only
rem  -dipsw dbgsw=off : Start T-Kernel
rem
rem  -nographic       : No LCD window
rem
rem -tp xmin=944,xmax=64,ymin=912,ymax=80,xchg=on : Touch Panel
rem
C:\qemu\bin\qemu-tef_em1d.exe ^
-cpu arm1176jzf-s ^
-kernel %1 ^
-sd %2 ^
-serial tcp:127.0.0.1:10000,server ^
-rtc base=localtime ^
-dipsw dbgsw=on ^
-nographic ^
-net nic,macaddr=52:54:00:12:34:56 ^
-net tap,ifname=MY-TAP
