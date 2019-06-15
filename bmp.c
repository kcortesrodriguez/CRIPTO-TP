#include "bmp.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR -1
#define OK 1
BMPImage * read_bmp(FILE * fd) {

    BMPImage * image = new_bmp_image();

    size_t read = 0;

    if(image == NULL) {
        return NULL;
    }
    image->fd = fd;

    read = fread(&image->header, sizeof(image->header), 1, image->fd); //Leer header

    if(read != 1) {
        fclose(fd);
        return NULL;
    }

    if(!bmp_valid_header(&image->header)) {
        return NULL;
    }

    if(fseek(image->fd, image->header.offset, 0) != 0) { //Ir al pixel array
        return NULL;
    }
    image->data = malloc(image->header.image_size_bytes);

    read = fread(image->data, sizeof(uint8_t), image->header.image_size_bytes, image->fd); //Leer los pixeles

    if(read != image->header.image_size_bytes) {
        return NULL;
    }

    size_t extra_header_size = image->header.offset - sizeof(image->header);

    if(fseek(image->fd, sizeof(image->header), 0) != 0) {
        return NULL;
    }
    image->extra_header = malloc(extra_header_size);

    read = fread(image->extra_header, sizeof(uint8_t), extra_header_size, image->fd);

    if(read != extra_header_size) {
        return NULL;
    }

    return image;

}

BMPImage * new_bmp_image() {
    BMPImage * image = (BMPImage *) malloc(sizeof(BMPImage));
    return image;
}

bool bmp_valid_header(BMPHeader * header) {
    if(header->type != BMP_TYPE) {
        return false;
    }
    return true;

}

void free_bmp(BMPImage * image) {
    free(image);
}

/* Returns the size of the extra header */
int get_extra_header_size(BMPImage * image) {
    return image->header.offset - sizeof(image->header);
}



/* Copies an image */

BMPImage * copy_bmp(BMPImage * src) {
    BMPImage * new_image = new_bmp_image();
    memcpy(&new_image->header, &src->header, sizeof(src->header));
    new_image->data = malloc(src->header.image_size_bytes);
    memcpy(new_image->data, src->data, src->header.image_size_bytes);

    int extra_header_size = get_extra_header_size(src);
    new_image->extra_header = malloc(extra_header_size);
    memcpy(new_image->extra_header, src->extra_header, extra_header_size);
    new_image->fd = src->fd;
    return new_image;
}

/* Writes a BMP image into fd */

int write_bmp(BMPImage * image, FILE * fd) {
    int written = fwrite(&image->header, sizeof(image->header),1, fd);
    if(written != 1) {
        return ERROR;
    }
    int extra_header_size = get_extra_header_size(image);
    written = fwrite(image->extra_header, sizeof(*image->extra_header),extra_header_size, fd);
    if(written != extra_header_size) {
        return ERROR;
    }
    written = fwrite(image->data, sizeof(*image->data), image->header.image_size_bytes, fd);
    if(written != image->header.image_size_bytes) {
        return ERROR;
    }
    return OK;
}


BMPImage *
create_bmp(const uint8_t * values, uint32_t h, uint32_t w, uint8_t * extraHeader)
{
    BMPImage * image = new_bmp_image();

    image->header.type =  0x4d42;
    image->header.size = h*w + 1078;
    image->header.reserved1 = 0;
    image->header.reserved2 = 0;
    image->header.offset = 54;
    image->header.dib_header_size = 40;
    image->header.width_px = w;
    image->header.height_px = h;
    image->header.num_planes = 1;
    image->header.bits_per_pixel = 8;
    image->header.compression = 0;
    image->header.image_size_bytes = w*h;
    image->header.x_resolution_ppm = 0;
    image->header.y_resolution_ppm = 0;
    image->header.num_colors = 256;
    image->header.important_colors = 0;

    image->extra_header = calloc(1078, 1);
    image->data = malloc(w*h);
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            image->data[i*w + j] = (uint8_t) values[i*w + j];
        }
    }

    return image;

}
