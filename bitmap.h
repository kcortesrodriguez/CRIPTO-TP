#ifndef CRIPTO_TP_BITMAP_H
#define CRIPTO_TP_BITMAP_H

/****************************************************************
 * bitmap.h - Header library for loading bitmaps and converting *
 * them to ascii. Also, will do other stuff with bitmaps in the *
 * future.                                                      *
 ****************************************************************
 * Created by Philip '5n4k3' Simonson            (05-03-2018)   *
 ****************************************************************
 */

#ifndef PRS_BITMAP_H
#define PRS_BITMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

enum {
    BITMAP_NO_ERROR,
    BITMAP_CREATE_ERROR,
    BITMAP_OPEN_ERROR,
    BITMAP_WRITE_ERROR,
    BITMAP_UNKNOWN_ERROR
};

#pragma pack(push, 1)
typedef struct BITMAP_FILE_HEADER {
    unsigned short header;
    unsigned int size;
    unsigned short res1;
    unsigned short res2;
    unsigned int offset;
} BITMAP_HEADER;

typedef struct BITMAP_INFO_HEADER {
    unsigned int size;
    unsigned int width;
    unsigned int height;
    unsigned short col_planes;
    unsigned short bpp;
    unsigned int compression;
    unsigned int image_size;
    unsigned int h_res;
    unsigned int v_res;
    unsigned int num_cols;
    unsigned int num_imp;
} BITMAP_INFO;

typedef struct BITMAP {
    BITMAP_HEADER file;
    BITMAP_INFO info;
    char RGBQUAD[1024];
} BITMAP;
#pragma pack(pop)

typedef struct BITMAP_FILE {
    BITMAP header;
    unsigned char error;
    char fname[MAX_PATH];
    unsigned char *data;
} BITMAP_FILE;

/* destroy_BMP:  frees bitmap resources */
static void destroy_BMP(BITMAP_FILE *bmp) {
    if (bmp) {
        if (bmp->data)
            free(bmp->data);
        free(bmp);
    }
}

/* check_BMP:  checks for error; reports if one found */
static int check_BMP(BITMAP_FILE *bmp) {
    if (bmp) {
        switch (bmp->error) {
            case BITMAP_NO_ERROR:
#ifdef DEBUG
                printf("Error: None\n");
#endif
                break;
            case BITMAP_CREATE_ERROR:
                printf("Error: Cannot create %s\n",
                       bmp->fname);
                destroy_BMP(bmp);
                break;
            case BITMAP_OPEN_ERROR:
                printf("Error: Cannot open file %s\n",
                       bmp->fname);
                destroy_BMP(bmp);
                break;
            case BITMAP_WRITE_ERROR:
                printf("Error: Cannot write file %s\n",
                       bmp->fname);
                destroy_BMP(bmp);
                break;
            case BITMAP_UNKNOWN_ERROR:
                printf("Error: Unknown error\n");
                destroy_BMP(bmp);
                break;
            default:
                printf("Not a valid error code.\n");
                break;
        }
        return bmp->error;
    }
    printf("Error: BITMAP_FILE variable not set\n");
    return 6;
}

/* load_BMP:  loads an image file into a bitmap data structure */
static BITMAP_FILE *load_BMP(const char *filename) {
    BITMAP_FILE *bmp;
    unsigned int image_size;
    unsigned int file_size;
    FILE *fp;

    bmp = (BITMAP_FILE *) malloc(sizeof(BITMAP_FILE));
    if (!bmp) {
        return NULL;
    }

    /* set bitmap name */
    memset(bmp->fname, 0, sizeof bmp->fname);
    strcpy(bmp->fname, filename);
    if ((fp = fopen(filename, "rb")) == NULL) {
        bmp->error = BITMAP_OPEN_ERROR;
        return bmp;
    }

    /* BMP HEADER */
    fread(&bmp->header.file.header, 2, 1, fp);
    fread(&bmp->header.file.size, 4, 1, fp);
    fread(&bmp->header.file.res1, 2, 1, fp);
    fread(&bmp->header.file.res2, 2, 1, fp);
    fread(&bmp->header.file.offset, 4, 1, fp);

    /* BMP HEADER INFO */
    fread(&bmp->header.info.size, 4, 1, fp);
    fread(&bmp->header.info.width, 4, 1, fp);
    fread(&bmp->header.info.height, 4, 1, fp);
    fread(&bmp->header.info.col_planes, 2, 1, fp);
    fread(&bmp->header.info.bpp, 2, 1, fp);
    fread(&bmp->header.info.compression, 4, 1, fp);
    fread(&bmp->header.info.image_size, 4, 1, fp);
    fread(&bmp->header.info.h_res, 4, 1, fp);
    fread(&bmp->header.info.v_res, 4, 1, fp);
    fread(&bmp->header.info.num_cols, 4, 1, fp);
    fread(&bmp->header.info.num_imp, 4, 1, fp);

    image_size = bmp->header.info.image_size;
    file_size = image_size + bmp->header.file.offset;

    if (bmp->header.file.header == 0x4D42 &&
        bmp->header.file.size == file_size) {
        fseek(fp, bmp->header.file.offset, SEEK_SET);
        bmp->data = malloc(sizeof(unsigned char) * image_size);
        if (bmp->data) {
            fread(bmp->data, 1, image_size, fp);
            if (VERBOSE) printf("\nValid image loaded.\n");
            fclose(fp);
            bmp->error = BITMAP_NO_ERROR;
        } else {
            printf("Cannot alloc for image data.\n");
            fclose(fp);
            bmp->error = BITMAP_OPEN_ERROR;
        }
    } else {
        printf("File Size: %u\nSize compared to: "
               "%u\nImage is invalid.\n",
               bmp->header.file.size,
               file_size);
        fclose(fp);
        bmp->error = BITMAP_OPEN_ERROR;
    }
    return bmp;
}

/* write_BMP:  write BMP out to file */
static int write_BMP(BITMAP_FILE *bmp) {
    FILE *fp;

    if (bmp) {
        if ((fp = fopen(bmp->fname, "wb")) == NULL) {
            fprintf(stderr, "Cannot open file for writing.\n");
            bmp->error = BITMAP_OPEN_ERROR;
            return 1;
        }

        if (fwrite(&bmp->header, 1, sizeof(BITMAP), fp) !=
            sizeof(BITMAP)) {
            bmp->error = BITMAP_WRITE_ERROR;
            return 1;
        }

        fseek(fp, bmp->header.file.offset, SEEK_SET);
        if (fwrite(bmp->data, 1, bmp->header.info.image_size, fp) !=
            bmp->header.info.image_size) {
            bmp->error = BITMAP_WRITE_ERROR;
            return 1;
        }
        fclose(fp);
    }
    return 0;
}

/* create_BMP:  make a blank BMP file; from given width/height/bitsperpixel */
static BITMAP_FILE *create_BMP(const char *filename, unsigned int w,
                               unsigned int h, unsigned short bpp) {
    BITMAP_FILE *bmp;
    const int pixel_byte_size = h * w * bpp / 8;
    const int file_size = sizeof(BITMAP) + pixel_byte_size;

    bmp = (BITMAP_FILE *) calloc(1, sizeof(BITMAP_FILE));
    if (!bmp) {
        return NULL;
    }
    bmp->data = (unsigned char *) malloc(pixel_byte_size);
    if (!bmp->data) {
        bmp->error = BITMAP_CREATE_ERROR;
        return bmp;
    }

    /* setup filename for bmp */
    memset(bmp->fname, 0, sizeof bmp->fname);
    strcpy(bmp->fname, filename);

    /* setup bitmap file header */
    bmp->header.file.header = 0x4D42;
    bmp->header.file.size = file_size;
    bmp->header.file.res1 = 0;
    bmp->header.file.res2 = 0;
    bmp->header.file.offset = sizeof(BITMAP);

    /* setup bitmap info header */
    bmp->header.info.size = sizeof(BITMAP_INFO);
    bmp->header.info.width = w;
    bmp->header.info.height = h;
    bmp->header.info.col_planes = 1;
    bmp->header.info.bpp = bpp;
    bmp->header.info.compression = 0;
    bmp->header.info.image_size = pixel_byte_size;
    bmp->header.info.num_cols = 256;
    bmp->header.info.h_res = 0;
    bmp->header.info.v_res = 0;
    bmp->header.info.num_imp = 0;


    // RGBQUAD aColors[256]
    int RGBQUAD_index = 0;
    for (int i = 0; i < 256; i++) {
        // 3 times i
        bmp->header.RGBQUAD[RGBQUAD_index] = (uint8_t) i;
        bmp->header.RGBQUAD[RGBQUAD_index + 1] = (uint8_t) i;
        bmp->header.RGBQUAD[RGBQUAD_index + 2] = (uint8_t) i;

        // 00
        bmp->header.RGBQUAD[RGBQUAD_index + 3] = 0x00;

        RGBQUAD_index = RGBQUAD_index + 4;
    }

    return bmp;
}

/* display_info_BMP:  display info about the bitmap file */
static void display_info_BMP(BITMAP_FILE *bmp) {
    if (bmp) {
        printf("***INFO BELOW ***\n"
               "***********************************\n"
               "BITMAP HEADER            : %u\n"
               "BITMAP SIZE              : %u\n"
               "BITMAP OFFSET            : %u\n"
               "BITMAP WIDTH             : %u\n"
               "BITMAP HEIGHT            : %u\n"
               "BITMAP BITS PER PIXEL    : %u\n"
               "***********************************\n",
               bmp->header.file.header, bmp->header.file.size,
               bmp->header.file.offset, bmp->header.info.width,
               bmp->header.info.height, bmp->header.info.bpp);
/*		printf("*** Data Below ***\n"
			"***********************************\n"
			"%s\n"
			"***********************************\n",
			bmp->data); */
    }
}

#endif

#endif //CRIPTO_TP_BITMAP_H
