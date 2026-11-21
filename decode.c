#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "decode.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2], ".bmp"))                         // Check whether input file is BMP
    {
        decInfo->stego_image_fname = argv[2];           // Store BMP filename
    }
    else
    {
        return e_failure;
    }

    if(argv[3] != NULL)                                 // Check if output filename is given
    {
        char *token = strtok(argv[3], ".");             // Remove extension from filename

        if(token != NULL)
        {
            strcpy(decInfo->secret_fname, token);       // Store output filename
        }
    }
    else
    {
        strcpy(decInfo->secret_fname, "output");        // Use default filename
    }

    return e_success;
}

Status open_file(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb"); // Open BMP in read mode

    if(decInfo->fptr_stego_image == NULL)              // Check file opening
    {
        perror("");
        return e_failure;
    }

    return e_success;
}

Status skip_bmp_header(FILE *fptr_stego_image)
{
    if(fseek(fptr_stego_image, 54, SEEK_SET) != 0)     // Skip 54-byte BMP header
    {
        return e_failure;
    }

    return e_success;
}

Status decode_int_from_image(FILE *fptr_stego_image, int *data)
{
    char image_buffer[32];

    if(fread(image_buffer, 32, 1, fptr_stego_image) != 1) // Read 32 image bytes
    {
        return e_failure;
    }

    *data = 0;                                           // Initialize decoded integer

    for(int i = 0; i < 32; i++)
    {
        int bit = image_buffer[i] & 1;                  // Extract LSB from each byte

        *data = *data | (bit << i);                     // Reconstruct integer bit by bit
    }

    return e_success;
}

Status decode_byte_from_lsb(char *image_buffer, char *data)
{
    *data = 0;                                          // Initialize decoded character

    for(int i = 0; i < 8; i++)
    {
        int bit = image_buffer[i] & 1;                  // Extract LSB from image byte

        *data = *data | (bit << i);                     // Reconstruct character bit by bit
    }

    return e_success;
}

Status decode_string_from_image(char *data, int size,
                                FILE *fptr_stego_image)
{
    char image_buffer[8];

    for(int i = 0; i < size; i++)
    {
        if(fread(image_buffer, 8, 1, fptr_stego_image) != 1) // Read 8 bytes for one character
        {
            return e_failure;
        }

        if(decode_byte_from_lsb(image_buffer, &data[i]) == e_failure) // Decode one character
        {
            return e_failure;
        }
    }

    data[size] = '\0';                                  // Add null character to string

    return e_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    int magic_size;

    if(decode_int_from_image(decInfo->fptr_stego_image, &magic_size) == e_failure) // Decode magic size
    {
        return e_failure;
    }

    if(magic_size <= 0 || magic_size >= sizeof(decInfo->magic)) // Validate magic size
    {
        return e_failure;
    }

    if(decode_string_from_image(decInfo->magic, magic_size,
                                decInfo->fptr_stego_image) == e_failure) // Decode magic string
    {
        return e_failure;
    }

    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    if(decode_int_from_image(decInfo->fptr_stego_image,
                             &decInfo->extn_size) == e_failure) // Decode extension size
    {
        return e_failure;
    }

    if(decInfo->extn_size <= 0 || decInfo->extn_size >= MAX_FILE_SUFFIX) // Validate extension size
    {
        return e_failure;
    }

    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    if(decode_string_from_image(decInfo->extn_secret_file,
                                decInfo->extn_size,
                                decInfo->fptr_stego_image) == e_failure) // Decode extension
    {
        return e_failure;
    }

    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    int size;

    if(decode_int_from_image(decInfo->fptr_stego_image, &size) == e_failure) // Decode file size
    {
        return e_failure;
    }

    decInfo->size_secret_file = size;                   // Store secret file size

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char data;
    char image_buffer[8];

    for(long i = 0; i < decInfo->size_secret_file; i++) // Decode each byte of secret file
    {
        if(fread(image_buffer, 8, 1, decInfo->fptr_stego_image) != 1) // Read 8 image bytes
        {
            return e_failure;
        }

        if(decode_byte_from_lsb(image_buffer, &data) == e_failure) // Decode one byte
        {
            return e_failure;
        }

        if(fwrite(&data, 1, 1, decInfo->fptr_secret) != 1) // Write decoded byte to output file
        {
            return e_failure;
        }
    }

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    char user_magic[20];

    if(open_file(decInfo) == e_failure)                  // Open stego image
    {
        printf("ERROR: Unable to open stego image\n");
        return e_failure;
    }

    printf("Stego image opened successfully\n");

    if(skip_bmp_header(decInfo->fptr_stego_image) == e_failure) // Skip BMP header
    {
        printf("ERROR: Unable to skip BMP header\n");
        return e_failure;
    }

    if(decode_magic_string(decInfo) == e_failure)       // Decode magic string
    {
        printf("ERROR: Unable to decode magic string\n");
        return e_failure;
    }

    printf("Enter the magic string: ");
    scanf(" %19[^\n]", user_magic);                     // Read magic string from user

    if(strcmp(user_magic, decInfo->magic) != 0)         // Compare magic strings
    {
        printf("ERROR: Magic string mismatch\n");
        return e_failure;
    }

    printf("Magic string matched successfully\n");

    if(decode_secret_file_extn_size(decInfo) == e_failure) // Decode extension size
    {
        printf("ERROR: Unable to decode extension size\n");
        return e_failure;
    }

    if(decode_secret_file_extn(decInfo) == e_failure)   // Decode file extension
    {
        printf("ERROR: Unable to decode extension\n");
        return e_failure;
    }

    printf("Secret file extension: %s\n",
           decInfo->extn_secret_file);

    strcat(decInfo->secret_fname,
           decInfo->extn_secret_file);                  // Add extension to output filename

    decInfo->fptr_secret =
        fopen(decInfo->secret_fname, "wb");             // Create output file in binary mode

    if(decInfo->fptr_secret == NULL)                    // Check output file opening
    {
        perror("");
        return e_failure;
    }

    if(decode_secret_file_size(decInfo) == e_failure)   // Decode secret file size
    {
        printf("ERROR: Unable to decode secret file size\n");
        return e_failure;
    }

    printf("Secret file size: %ld bytes\n",
           decInfo->size_secret_file);

    if(decode_secret_file_data(decInfo) == e_failure)   // Decode secret file data
    {
        printf("ERROR: Unable to decode secret file data\n");
        return e_failure;
    }

    printf("Secret file decoded successfully\n");

    fclose(decInfo->fptr_stego_image);                  // Close BMP file
    fclose(decInfo->fptr_secret);                       // Close output file

    return e_success;
}