#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "bmp.h"

struct __attribute__ ((packed)) bmp_header {
    char fileIdentifier[2]; // must be BM
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t imageDataOffset;
};

struct __attribute__ ((packed)) bmp_info {
    uint32_t headerSize;
    int32_t imageWidth;
    int32_t imageHeight;
    uint16_t colorPlanes;
    uint16_t bitsPerPixel;
    uint32_t compressionMethod;
    uint32_t imageSize;
    int32_t horizontalRes;
    int32_t verticalRes;
    uint32_t colorsInPalette;
    uint32_t numImportantColors;
};

struct bmp_image {
    struct bmp_header header;
    struct bmp_info info;
    uint8_t *data; //todo MIGRATE ALL int AND longs TO uint8_t
};

bmp_image_t *bmp_from_path(const char *path) {
    FILE *fp = fopen(path, "rb");

    if (fp == NULL) {
        printf("Could not open file %s: ", path);
        printf("%s\n", strerror(errno));
        return NULL;
    }

    bmp_image_t *image = malloc(sizeof(bmp_image_t));

    fread(&image->header, sizeof(struct bmp_header), 1, fp);
    fread(&image->info, sizeof(struct bmp_info), 1, fp);

    const size_t header_size = sizeof(struct bmp_header) + sizeof(struct bmp_info);

//    if (header_size != image->header.imageDataOffset) {
//        printf("Image contains extra data!");
//        goto _ABORT;
//    }

    if (image->info.bitsPerPixel != 8) {
        printf("Image does not contains 8 bits per pixel.");
        goto _ABORT;
    }

    if (image->info.compressionMethod != 0) {
        printf("Image is compressed");
        goto _ABORT;
    }

//    if (image->header.imageDataOffset - header_size != 0) {
//        printf("Image contains extra data between header and bitmap!");
//        goto _ABORT;
//    }

    image->data = malloc(image->info.imageSize);
    fread(image->data, image->info.imageSize, 1, fp);

    fclose(fp);

    return image;

    _ABORT:

    free(image);
    fclose(fp);

    return NULL;
}


int bmp_save(const bmp_image_t *image, const char *path) {
    FILE *fp = fopen(path, "wb+");

    if (fp == NULL) {
        printf("Could not open file %s: ", path);
        printf("%s\n", strerror(errno));
        return 0;
    }

    fwrite(&image->header, sizeof(struct bmp_header), 1, fp);
    fwrite(&image->info, sizeof(struct bmp_info), 1, fp);

    fwrite(image->data, image->info.imageSize, 1, fp);

    fclose(fp);

    return 1;
}

void bmp_free(bmp_image_t *image) {
    free(image->data);
    free(image);
}

uint8_t *bmp_get_data_buffer(bmp_image_t *image) {
    return image->data;
}

uint32_t bmp_get_image_size(bmp_image_t *image) {
    return image->info.imageSize;
}

int bmp_check_size(bmp_image_t *image, long size) {
    return image->info.imageSize > size * 8;
}