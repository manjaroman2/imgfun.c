#include <imgfun.h> 

const char *imgfun_color_type_str(enum spng_color_type color_type)
{
    switch (color_type)
    {
    case SPNG_COLOR_TYPE_GRAYSCALE:
        return "grayscale";
    case SPNG_COLOR_TYPE_TRUECOLOR:
        return "truecolor";
    case SPNG_COLOR_TYPE_INDEXED:
        return "indexed color";
    case SPNG_COLOR_TYPE_GRAYSCALE_ALPHA:
        return "grayscale with alpha";
    case SPNG_COLOR_TYPE_TRUECOLOR_ALPHA:
        return "truecolor with alpha";
    default:
        return "(invalid)";
    }
}

int imgfun_open_pngs(const char *files[], size_t n)
{
    if (n > IMGFUN_INTERPOLATE_MAX)
    {
        printf("more than %d images are not supported at the moment (got %ld)", IMGFUN_INTERPOLATE_MAX, n);
        return 1;
    }
    FILE *pngs[n];
    spng_ctx *ctxs[n];
    unsigned char *images[n];
    size_t i;
    int ret = 0;
    for (i = 0; i < n; i++)
    {
        printf("%s\n", files[i]);
        FILE *png = fopen(files[i], "rb");
        pngs[i] = png;
        if (png == NULL)
        {
            printf("error code %d opening file %s\n", errno, files[i]);
            ret = 1;
            goto deallocAllPrevious;
        }
        spng_ctx *ctx = spng_ctx_new(0);
        ctxs[i] = ctx;
        if (ctx == NULL)
        {
            printf("ERROR: spng_ctx_new() failed.\n");
            ret = 1;
            goto deallocFile;
        }

        if (spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE) > 0)
        {
            printf("ERROR: spng_set_crc_action() failed.\n");
            ret = 1;
            goto deallocCtx;
        }

        size_t lim = 1024 * 1024 * 64;

        if (spng_set_chunk_limits(ctx, lim, lim) > 0)
        {
            printf("ERROR: spng_set_chunk_limits() failed.\n");
            ret = 1;
            goto deallocCtx;
        }

        if (spng_set_png_file(ctx, png) > 0)
        {
            printf("ERROR: spng_set_png_file() failed.\n");
            ret = 1;
            goto deallocCtx;
        }

        struct spng_ihdr ihdr;
        ret = spng_get_ihdr(ctx, &ihdr);

        if (ret > 0)
        {
            printf("spng_get_ihdr() failed: error %s\n", spng_strerror(ret));
            ret = 1;
            goto deallocCtx;
        }

        const char *color_name = imgfun_color_type_str(ihdr.color_type);

        printf("width: %u\n"
               "height: %u\n"
               "bit depth: %u\n"
               "color type: %u - %s\n",
               ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type, color_name);

        printf("compression method: %u\n"
               "filter method: %u\n"
               "interlace method: %u\n",
               ihdr.compression_method, ihdr.filter_method, ihdr.interlace_method);

        int fmt = SPNG_FMT_PNG;
        if (ihdr.color_type == SPNG_COLOR_TYPE_INDEXED)
            fmt = SPNG_FMT_RGB8;

        size_t image_size;
        ret = spng_decoded_image_size(ctx, fmt, &image_size);

        if (ret > 0)
        {
            printf("spng_decode_image_size() error: %s\n", spng_strerror(ret));
            ret = 1;
            goto deallocCtx;
        }

        unsigned char *image;
        image = malloc(image_size);

        if (image == NULL)
        {
            printf("malloc error: %d. buy more RAM lol!\n", errno);
            ret = 1;
            goto deallocCtx;
        }

        images[i] = image;

        ret = spng_decode_image(ctx, image, image_size, SPNG_FMT_RGBA8, 0);
        if (ret > 0)
        {
            printf("spng_decode_image() error: %s\n", spng_strerror(ret));
            ret = 1;
            goto deallocImage;
        }

        continue;
    deallocImage:
        free(image);
    deallocCtx:
        spng_ctx_free(ctx);
    deallocFile:
        fclose(png);
        goto deallocAllPrevious;
    }

deallocAllPrevious:
    for (size_t j = 0; j < i; j++)
    {
        free(images[j]);
        spng_ctx_free(ctxs[j]);
        if (fclose(pngs[j]) < 0)
        {
            printf("error code %d closing file %s\n", errno, files[j]);
        }
    }
    return ret;
}

int main(int argc, const char *argv[])
{
    if (argc < 3)
    {
        printf("ERROR: Needs 2 input files.\n");
        exit(1);
    }
    return imgfun_open_pngs(argv + 1, argc - 1);
}