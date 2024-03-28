cd /D "%~dp0"
windres icon.rc -O coff -o icon.res
gcc -o WebP ..\main.c icon.res -municode -Os -static -lwebp -lpng -lz