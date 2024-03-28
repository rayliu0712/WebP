#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <webp/decode.h>
#include <png.h>

void dumpVersion() {
    puts("v1.5  2024/03/28  ---Ray Liu");
    puts("https://github.com/rayliu0712/FuckWebP");
}

void terminate(const char *format, const wchar_t *wfilename, const char *cfilename) {
    printf("[ ERROR ] ");

    if (wfilename)
        printf(format, wfilename);
    else
        printf(format, cfilename);

    printf("\n\n");

    system("pause");
    exit(1);
}

void run(const wchar_t *wfilename, const char *cfilename) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png)))
        terminate("The function \"png_create_write_struct()\" or \"png_create_info_struct()\" in libpng went wrong", NULL, NULL);

    size_t len;
    FILE* webpFile, *pngFile;
    
    #ifdef _WIN32
        len = wcslen(wfilename);
        const wchar_t *webpPath = wfilename;
        wchar_t pngPath[len];

        if (webpPath[len - 1] != 'p' ||
            webpPath[len - 2] != 'b' ||
            webpPath[len - 3] != 'e' ||
            webpPath[len - 4] != 'w' ||
            webpPath[len - 5] != '.')
            terminate("Input file \"%ls\" is not WebP", webpPath, NULL);

        wcsncpy(pngPath, webpPath, len - 1);
        pngPath[len - 1] = '\0';
        pngPath[len - 2] = 'g';
        pngPath[len - 3] = 'n';
        pngPath[len - 4] = 'p';

        if (_wfopen(pngPath, L"rb"))
            terminate("Output file \"%ls\" already exists", pngPath, NULL);

        webpFile = _wfopen(webpPath, L"rb");
        pngFile  = _wfopen(pngPath , L"wb");

    #elif defined(__linux__)
        len = strlen(cfilename);
        const char *webpPath = cfilename;
        char pngPath[len];

        if (webpPath[len - 1] != 'p' ||
            webpPath[len - 2] != 'b' ||
            webpPath[len - 3] != 'e' ||
            webpPath[len - 4] != 'w' ||
            webpPath[len - 5] != '.')
            terminate("Input file \"%s\" is not WebP", NULL, webpPath);

        strncpy(pngPath, webpPath, len - 1);
        pngPath[len - 1] = '\0';
        pngPath[len - 2] = 'g';
        pngPath[len - 3] = 'n';
        pngPath[len - 4] = 'p';

        if (fopen(pngPath, "rb"))
            terminate("Output file \"%s\" already exists", NULL, pngPath);

        webpFile = fopen(webpPath, "rb");
        pngFile  = fopen(pngPath , "wb");

    #endif

    fseek(webpFile, 0, SEEK_END);
    size_t webpSize = ftell(webpFile);
    uint8_t webpData[webpSize];
    
    fseek(webpFile, 0, SEEK_SET);
    fread(webpData, 1, webpSize, webpFile);
    fclose(webpFile);

    int height, width;
    uint8_t *rgba = WebPDecodeRGBA(webpData, webpSize, &height, &width);
    png_init_io(png, pngFile);
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
    fclose(pngFile);

    png_destroy_write_struct(&png, (png_infopp) NULL);
    png_free_data(png, info, PNG_FREE_ALL, -1);
    WebPFree(rgba);
    free(row);
}


#ifdef _WIN32
#include <locale.h>
int wmain(int argc, wchar_t *argv[]) {
    if (argc == 1)
        terminate("No input file", NULL, NULL);

    if (argv[1] == L"--version" || argv[1] == L"--help")
        dumpVersion();
    else {
        setlocale(LC_ALL, "");
        run(argv[1], NULL);
    }

    return 0;
}

#elif defined(__linux__)
int main(int argc, char *argv[]) {
    if (argc == 2 && (argv[1] == "--version" || argv[1] == "--help"))
        dumpVersion();
    else
        run(NULL, argv[1]);

    return 0;
}

#endif