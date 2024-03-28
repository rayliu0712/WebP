cd /D "%~dp0"
windres icon.rc -O coff -o icon.res
gcc -o FuckWebP ..\main.c icon.res -Os -static -municode -lwebp -lpng -lz