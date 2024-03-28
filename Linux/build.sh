cd $(dirname "$0")
gcc -o FuckWebP ../main.c -Os -static -lwebp -lpng -lz -lm