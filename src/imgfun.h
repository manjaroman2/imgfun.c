#include <stdio.h>
#include <errno.h>
#include <spng.h>

#define IMGFUN_INTERPOLATE_MAX 10

const char *imgfun_color_type_str(enum spng_color_type color_type);
int imgfun_open_pngs(const char *files[], size_t n);