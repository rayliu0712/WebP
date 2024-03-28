cd $(dirname "$0")
gcc -o WebP ../main.c -Os -static -lwebp -lpng -lz -lm