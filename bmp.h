#ifndef STEGOBMP_BMP_H
#define STEGOBMP_BMP_H

#include <stdint.h>

typedef struct bmp_image bmp_image_t;

bmp_image_t *bmp_from_path(const char *path);

int bmp_save(const bmp_image_t *image, const char *path);

void bmp_free(bmp_image_t *image);

uint8_t *bmp_get_data_buffer(bmp_image_t *image);

uint32_t bmp_get_image_size(bmp_image_t *image);

int bmp_check_size(bmp_image_t *image, long size);

#endif //STEGOBMP_BMP_H