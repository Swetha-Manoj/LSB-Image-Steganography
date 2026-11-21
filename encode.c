#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Source image validation
    if(strstr(argv[2], ".bmp"))                         // Check source image is BMP
    {
        encInfo->src_image_fname = argv[2];             // Store source image filename
    }
    else
    {
        return e_failure;
    }

    // Secret file validation
    if(strstr(argv[3], ".txt"))                         // Check secret file is TXT
    {
        encInfo->secret_fname = argv[3];                // Store secret filename
    }
    else
    {
        return e_failure;
    }

    // Store secret file extension
    char *dot = strrchr(encInfo->secret_fname, '.');   // Find last '.' in filename

    if(dot != NULL)
    {
        strcpy(encInfo->extn_secret_file, dot);          // Store file extension
    }
    else
    {
        return e_failure;
    }

    // Destination image validation
    if(argv[4] == NULL)                                 // Check destination filename
    {
        encInfo->stego_image_fname = "stego.bmp";       // Use default output filename
    }
    else
    {
        if(strstr(argv[4], ".bmp"))                     // Check destination is BMP
        {
            encInfo->stego_image_fname = argv[4];       // Store destination filename
        }
        else
        {
            return e_failure;
        }
    }

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb"); // Open source image

    if(encInfo->fptr_src_image == NULL)                // Check source file
    {
        perror("fopen");
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");      // Open secret file

    if(encInfo->fptr_secret == NULL)                   // Check secret file
    {
        perror("fopen");
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb"); // Create stego image

    if(encInfo->fptr_stego_image == NULL)              // Check destination file
    {
        perror("fopen");
        return e_failure;
    }

    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char header[54];

    rewind(fptr_src_image);                             // Move source pointer to beginning
    rewind(fptr_stego_image);                           // Move destination pointer to beginning

    if(fread(header, 54, 1, fptr_src_image) != 1)     // Read 54-byte BMP header
    {
        return e_failure;
    }

    if(fwrite(header, 54, 1, fptr_stego_image) != 1)   // Write BMP header
    {
        return e_failure;
    }

    return e_success;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    fseek(fptr_image, 18, SEEK_SET);                    // Move to BMP width field

    fread(&width, sizeof(int), 1, fptr_image);         // Read image width
    printf("width = %u\n", width);

    fread(&height, sizeof(int), 1, fptr_image);        // Read image height
    printf("height = %u\n", height);

    rewind(fptr_image);                                 // Return pointer to beginning

    return width * height * 3;                          // Calculate RGB image capacity
}

Status check_capacity(EncodeInfo *encInfo)
{
    int size_info;

    fseek(encInfo->fptr_secret, 0, SEEK_END);           // Move to end of secret file

    encInfo->size_secret_file = ftell(encInfo->fptr_secret); // Get secret file size

    rewind(encInfo->fptr_secret);                       // Return to beginning

    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image); // Get image capacity

    printf("Enter the magic string: ");

    scanf(" *%*[^\n]", encInfo->magic);                // Read magic string

    size_info = sizeof(int) + strlen(encInfo->magic) +
                sizeof(int) + strlen(encInfo->extn_secret_file) +
                sizeof(int) + encInfo->size_secret_file; // Calculate required space

    if((size_info * 8) <= encInfo->image_capacity)     // Check image has enough capacity
    {
        return e_success;
    }

    return e_failure;
}

void encode_int_image(char *arr, int data)
{
    for(int i = 0; i < 32; i++)
    {
        arr[i] = arr[i] & (~1);                         // Clear LSB of image byte

        int get_bit = (data & (1 << i)) >> i;           // Extract one bit from integer

        arr[i] = arr[i] | get_bit;                      // Store bit in image LSB
    }
}

Status encode_size_of_MS(EncodeInfo *encInfo)
{
    int size = strlen(encInfo->magic);                  // Get magic string size
    char arr[32];

    if(fread(arr, 32, 1, encInfo->fptr_src_image) != 1) // Read 32 image bytes
    {
        return e_failure;
    }

    encode_int_image(arr, size);                        // Encode size into image LSBs

    if(fwrite(arr, 32, 1, encInfo->fptr_stego_image) != 1) // Write modified bytes
    {
        return e_failure;
    }

    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        image_buffer[i] = image_buffer[i] & (~1);       // Clear LSB

        int get_bit = (data & (1 << i)) >> i;           // Extract one bit from data

        image_buffer[i] = image_buffer[i] | get_bit;    // Store bit in image LSB
    }

    return e_success;
}

Status encode_data_to_image(char *data, int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image)
{
    char image[8];

    for(int i = 0; i < size; i++)
    {
        if(fread(image, 8, 1, fptr_src_image) != 1)     // Read 8 image bytes
        {
            return e_failure;
        }

        if(encode_byte_to_lsb(data[i], image) == e_failure) // Encode one character
        {
            return e_failure;
        }

        if(fwrite(image, 8, 1, fptr_stego_image) != 1)  // Write modified image bytes
        {
            return e_failure;
        }
    }

    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    if(encode_data_to_image((char*)magic_string,
                            strlen(magic_string),
                            encInfo->fptr_src_image,
                            encInfo->fptr_stego_image) == e_failure) // Encode magic string
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    int size = strlen(encInfo->extn_secret_file);       // Get extension size
    char arr[32];

    if(fread(arr, 32, 1, encInfo->fptr_src_image) != 1) // Read 32 image bytes
    {
        return e_failure;
    }

    encode_int_image(arr, size);                        // Encode extension size

    if(fwrite(arr, 32, 1, encInfo->fptr_stego_image) != 1) // Write modified bytes
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    if(encode_data_to_image((char*)file_extn,
                            strlen(file_extn),
                            encInfo->fptr_src_image,
                            encInfo->fptr_stego_image) == e_failure) // Encode extension
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char arr[32];

    if(fread(arr, 32, 1, encInfo->fptr_src_image) != 1) // Read 32 image bytes
    {
        return e_failure;
    }

    encode_int_image(arr, (int)file_size);               // Encode secret file size

    if(fwrite(arr, 32, 1, encInfo->fptr_stego_image) != 1) // Write encoded size
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char *data;

    data = malloc(encInfo->size_secret_file);            // Allocate memory for secret data

    if(data == NULL)                                     // Check memory allocation
    {
        return e_failure;
    }

    rewind(encInfo->fptr_secret);                        // Move secret file to beginning

    if(fread(data, encInfo->size_secret_file, 1,
             encInfo->fptr_secret) != 1)                 // Read secret file data
    {
        free(data);
        return e_failure;
    }

    if(encode_data_to_image(data,
                            encInfo->size_secret_file,
                            encInfo->fptr_src_image,
                            encInfo->fptr_stego_image) == e_failure) // Encode secret data
    {
        free(data);
        return e_failure;
    }

    free(data);                                          // Release allocated memory

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while(fread(&ch, 1, 1, fptr_src) == 1)              // Read remaining image data
    {
        if(fwrite(&ch, 1, 1, fptr_dest) != 1)           // Copy data to stego image
        {
            return e_failure;
        }
    }

    return e_success;
}

Status close_files(FILE *fptr_src_image,
                   FILE *fptr_secret,
                   FILE *fptr_stego_image)
{
    if(fclose(fptr_src_image) != 0)                     // Close source image
    {
        return e_failure;
    }

    if(fclose(fptr_secret) != 0)                        // Close secret file
    {
        return e_failure;
    }

    if(fclose(fptr_stego_image) != 0)                   // Close stego image
    {
        return e_failure;
    }

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_failure)                // Open source, secret and destination files
    {
        printf("Not able open files\n");
        return e_failure;
    }

    printf("Opened All files Successfully\n");

    if(check_capacity(encInfo) == e_failure)            // Check image capacity
    {
        printf("ERROR: Image does not have enough space\n");
        return e_failure;
    }

    if(copy_bmp_header(encInfo->fptr_src_image,
                       encInfo->fptr_stego_image) == e_failure) // Copy BMP header
    {
        printf("ERROR: Unable to copy header\n");
        return e_failure;
    }

    printf("Header copied Successfully\n");

    if(encode_size_of_MS(encInfo) == e_failure)         // Encode magic string size
    {
        printf("ERROR: Unable to encode magic string size\n");
        return e_failure;
    }

    if(encode_magic_string(encInfo->magic, encInfo) == e_failure) // Encode magic string
    {
        printf("ERROR: Unable to encode magic string\n");
        return e_failure;
    }

    if(encode_secret_file_extn_size(encInfo) == e_failure) // Encode extension size
    {
        printf("ERROR: Unable to encode extension size\n");
        return e_failure;
    }

    if(encode_secret_file_extn(encInfo->extn_secret_file,
                               encInfo) == e_failure)   // Encode secret file extension
    {
        printf("ERROR: Unable to encode secret file extension\n");
        return e_failure;
    }

    if(encode_secret_file_size(encInfo->size_secret_file,
                               encInfo) == e_failure)   // Encode secret file size
    {
        printf("ERROR: Unable to encode secret file size\n");
        return e_failure;
    }

    if(encode_secret_file_data(encInfo) == e_failure)   // Encode secret file data
    {
        printf("ERROR: Unable to encode secret file data\n");
        return e_failure;
    }

    if(copy_remaining_img_data(encInfo->fptr_src_image,
                               encInfo->fptr_stego_image) == e_failure) // Copy remaining image data
    {
        printf("ERROR: Unable to copy remaining image data\n");
        return e_failure;
    }

    if(close_files(encInfo->fptr_src_image,
                   encInfo->fptr_secret,
                   encInfo->fptr_stego_image) == e_failure) // Close all files
    {
        printf("ERROR: Unable to close files\n");
        return e_failure;
    }

    return e_success;
}