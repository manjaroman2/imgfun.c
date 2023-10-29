#include <stdio.h>
#include <errno.h>
#include <spng.h>
#include <imgfun.h>

#define IMGFUN_INTERPOLATE_MAX 10

const char *color_type_str(enum spng_color_type color_type)
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

int open_pngs(const char *files[], size_t n)
{
    if (n > IMGFUN_INTERPOLATE_MAX)
    {
        printf("more than %d images are not supported at the moment (got %ld)", IMGFUN_INTERPOLATE_MAX, n);
        return 1;
    }
    FILE *pngs[n];
    spng_ctx *ctxs[n];
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
            goto closeFiles;
        }
        spng_ctx *ctx = spng_ctx_new(0);
        ctxs[i] = ctx;
        if (ctx == NULL)
        {
            printf("ERROR: spng_ctx_new() failed.\n");
            ret = 1;
            goto error;
        }

        if (spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE) > 0)
        {
            printf("ERROR: spng_set_crc_action() failed.\n");
            ret = 1;
            goto error;
        }
        size_t lim = 1024 * 1024 * 64;

        if (spng_set_chunk_limits(ctx, lim, lim) > 0)
        {
            printf("ERROR: spng_set_chunk_limits() failed.\n");
            ret = 1;
            goto error;
        }

        if (spng_set_png_file(ctx, png) > 0)
        {
            printf("ERROR: spng_set_png_file() failed.\n");
            ret = 1;
            goto error;
        }

        struct spng_ihdr ihdr;
        ret = spng_get_ihdr(ctx, &ihdr);

        if (ret)
        {
            printf("spng_get_ihdr() failed: error %s\n", spng_strerror(ret));
            ret = 1;
            goto error;
        }

        const char *color_name = color_type_str(ihdr.color_type);

        printf("width: %u\n"
               "height: %u\n"
               "bit depth: %u\n"
               "color type: %u - %s\n",
               ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type, color_name);

        printf("compression method: %u\n"
               "filter method: %u\n"
               "interlace method: %u\n",
               ihdr.compression_method, ihdr.filter_method, ihdr.interlace_method);

        struct spng_plte plte = {0};

        ret = spng_get_plte(ctx, &plte); 

        if (ret || ret != SPNG_ECHUNKAVAIL) 
        {
            printf("spng_get_plte() error: %s\n", spng_strerror(ret));
            goto error;
        }
        
    }

error:
    for (size_t j; j < i; j++)
    {
        spng_ctx_free(ctxs[j]);
    }
closeFiles:
    for (size_t j; j < i; j++)
    {
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
    return open_pngs(argv + 1, argc - 1);
}