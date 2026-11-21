#ifndef DECODE_H
#define DECODE_H

#include "types.h"

#define MAX_FILE_SUFFIX 5

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Secret File Info */
    char secret_fname[100];
    FILE *fptr_secret;

    char extn_secret_file[MAX_FILE_SUFFIX];
    int extn_size;
    long size_secret_file;

    /* Magic String */
    char magic[20];

} DecodeInfo;


/* Check operation type */
OperationType check_operation_type(char *argv[]);

/* Read and validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Open stego image and output file */
Status open_file(DecodeInfo *decInfo);

/* Skip BMP header */
Status skip_bmp_header(FILE *fptr_stego_image);

/* Decode integer from image */
Status decode_int_from_image(FILE *fptr_stego_image, int *data);

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode string from image */
Status decode_string_from_image(char *data, int size, FILE *fptr_stego_image);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode one byte from image */
Status decode_byte_from_lsb(char *image_buffer, char *data);

#endif