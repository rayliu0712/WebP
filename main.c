#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webp/decode.h>
#include <png.h>

void pause() {
    printf("\nPress Enter to exit . . . ");
    char c;
    scanf("%c", &c);
}

void enter_exit(int status) {
    pause();
    exit(status);
}

void dump_version() {
    puts("[ Version ] v1.6");
    puts("[ Date    ] 2024/04/04 Thursday Sunny");
    puts("[ Artist  ] Ray Liu");
    puts("[ Source  ] https://github.com/rayliu0712/WebP");
    enter_exit(0);
}

void terminate(const char *format, const void *filename) {
    printf("[ ERROR ] ");
    #ifdef _WIN32
        printf(format, (wchar_t *) filename);
    #elif defined(__linux__)
        printf(format, (char *) filename);
    #endif
    puts("");
    enter_exit(1);
}

void run(FILE *webpfile, FILE *pngfile) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png)))
        terminate("The function \"png_create_write_struct()\" or \"png_create_info_struct()\" in libpng went wrong", NULL);

    fseek(webpfile, 0, SEEK_END);
    size_t webpsize = ftell(webpfile);
    uint8_t *webpdata = (uint8_t *) malloc(webpsize);

    fseek(webpfile, 0, SEEK_SET);
    fread(webpdata, 1, webpsize, webpfile);
    fclose(webpfile);

    int height, width;
    uint8_t *rgba = WebPDecodeRGBA(webpdata, webpsize, &width, &height);
    png_init_io(png, pngfile);
    png_set_IHDR(
        png, info, width, height, 
        8,
        PNG_COLOR_TYPE_RGB_ALPHA,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);
    
    // Allocate memory for one row (4 bytes per pixel - RGBA)
    png_bytep row = (png_bytep) malloc(4 * width * sizeof(png_byte));
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            int i = width * y + x;

            row[x * 4    ] = rgba[i * 4    ];  // Red
            row[x * 4 + 1] = rgba[i * 4 + 1];  // Green
            row[x * 4 + 2] = rgba[i * 4 + 2];  // Blue
            row[x * 4 + 3] = rgba[i * 4 + 3];  // Alpha
        }
        png_write_row(png, row);
    }
    png_write_end(png, NULL);
    fclose(pngfile);

    png_destroy_write_struct(&png, (png_infopp) NULL);
    png_free_data(png, info, PNG_FREE_ALL, -1);
    WebPFree(rgba);
    free(webpdata);
    free(row);
}


#ifdef _WIN32
#include <locale.h>
int wmain(int argc, wchar_t *argv[]) {
    if (argc == 1)
        dump_version();
    else {
        setlocale(LC_ALL, "");

        size_t len = wcslen(argv[1]);
        wchar_t *webppath = argv[1];
        wchar_t pngpath[len];

        if (webppath[len - 1] != 'p' ||
            webppath[len - 2] != 'b' ||
            webppath[len - 3] != 'e' ||
            webppath[len - 4] != 'w' ||
            webppath[len - 5] != '.')
            terminate("Input file \"%ls\" is not WebP", (void *) webppath);

        wcsncpy(pngpath, webppath, len - 1);
        pngpath[len - 1] = '\0';
        pngpath[len - 2] = 'g';
        pngpath[len - 3] = 'n';
        pngpath[len - 4] = 'p';

        if (_wfopen(pngpath, L"rb"))
            terminate("Output file \"%ls\" already exists", (void *) pngpath);

        run(_wfopen(webppath, L"rb"), _wfopen(pngpath , L"wb"));
    }

    return 0;
}

#elif defined(__linux__)
int main(int argc, char *argv[]) {
    if (argc == 1)
        dump_version();
    else {
        size_t len = strlen(argv[1]);
        char *webppath = argv[1];
        char pngpath[len];

        if (webppath[len - 1] != 'p' ||
            webppath[len - 2] != 'b' ||
            webppath[len - 3] != 'e' ||
            webppath[len - 4] != 'w' ||
            webppath[len - 5] != '.')
            terminate("Input file \"%s\" is not WebP", (void *) webppath);

        strncpy(pngpath, webppath, len - 1);
        pngpath[len - 1] = '\0';
        pngpath[len - 2] = 'g';
        pngpath[len - 3] = 'n';
        pngpath[len - 4] = 'p';

        if (fopen(pngpath, "rb"))
            terminate("Output file \"%s\" already exists", (void *) pngpath);

        run(fopen(webppath, "rb"), fopen(pngpath , "wb"));
    }

    return 0;
}

#endif