@echo off

gcc test.c piece_table.c -o test.exe

.\test.exe

echo:
echo Press enter to exit
set /p input=
exit