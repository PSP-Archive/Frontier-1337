@ECHO OFF

REM # Convert as68k to c file
as68k --output-c fe2.s
REM as68k --output-i386 fe2.s

REM # fe2 c file part1 (Os for best results)
make fe2.c.part1

REM # fe2 c file part2 (O2 for best results)
make fe2.c.part2

make

REM # Clean
rem MOVE EBOOT.PBP I:\PSP\GAME\FRONTIER_1337_v0.3.0
rem MOVE EBOOT.PBP ./FRONTIER_1337
rem MOVE fe2.s.bin ./FRONTIER_1337
DEL *.elf *.SFO *.prx
CD src
DEL *.o

pause