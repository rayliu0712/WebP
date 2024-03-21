#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <locale.h>
#include <webp/decode.h>
#include <png.h>

void terminate(char *e, ...) {
    va_list args;
    va_start(args, e);
    printf("[ ERROR ] ");
    vprintf(e, args);
    printf("\n\n");
    va_end(args);

    system("pause");
    exit(1);
}

int wmain(int argc, wchar_t *argv[]) {
    setlocale(LC_ALL, "");

    if (argc == 1)
        terminate("Error : No Input File");

    if (argv[1] == L"--help" || argv[1] == L"--version" || argv[1] == L"-v") {
        puts("v1.0  2024/03/21  ---Ray Liu");
        puts("https://github.com/rayliu0712/FuckWebP");
        return 0;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png)))
        terminate("The function \"png_create_write_struct()\" or \"png_create_info_struct()\" in libpng went wrong");

/*=========================================================================================================================*/

    size_t len = wcslen(argv[1]);
    wchar_t *webpPath = argv[1];
    wchar_t pngPath[len];

    if (webpPath[len - 1] != 'p' ||
        webpPath[len - 2] != 'b' ||
        webpPath[len - 3] != 'e' ||
        webpPath[len - 4] != 'w' ||
        webpPath[len - 5] != '.')
        terminate("Input File \"%ls\" is not WebP", webpPath);

    wcsncpy(pngPath, webpPath, len - 1);
    pngPath[len - 1] = '\0';
    pngPath[len - 2] = 'g';
    pngPath[len - 3] = 'n';
    pngPath[len - 4] = 'p';

    if (_wfopen(pngPath, L"rb"))
        terminate("Output File \"%ls\" already exists", pngPath);

/*=========================================================================================================================*/

    FILE *webpFile = _wfopen(webpPath, L"rb");
    FILE *pngFile  = _wfopen(pngPath , L"wb");

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
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
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

/*=========================================================================================================================*/

    png_destroy_write_struct(&png, (png_infopp) NULL);
    png_free_data(png, info, PNG_FREE_ALL, -1);
    WebPFree(rgba);
    free(row);

    return 0;
}