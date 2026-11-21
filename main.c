/*      NAME        : SWETHA M
        DATE        : 23.09.2026
        DESCRIPTION : Developed an image steganography system using C to securely hide secre text files 
                      inside BMP images using the Least Significant Bit (LSB) technique. 
                      Implemented encoding of magic string, file extension, file size, and secret file 
                      data using file handling, bitwise operations, pointers, and dynamic memory allocation.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "encode.h"
#include "decode.h"
#include "types.h"


OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else if(strcmp(argv[1], "-h") == 0)
    {
        return e_help;
    }
    else
    {
        return e_unsupported;
    }
}


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Invalid arguments\n");
        return 0;
    }

    OperationType type = check_operation_type(argv);


    /* ================= ENCODING ================= */

    if(type == e_encode)
    {
        if(argc < 4)
        {
            printf("Invalid arguments\n");
            printf("./a.out -e beautiful.bmp secret.txt stego.bmp\n");
            return e_failure;
        }

        EncodeInfo encInfo;

        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            if(do_encoding(&encInfo) == e_success)
            {
                printf("Encoding completed successfully\n");
            }
            else
            {
                printf("Encoding failed\n");
            }
        }
        else
        {
            printf("Invalid encoding arguments\n");
            return e_failure;
        }
    }


    /* ================= DECODING ================= */

    else if(type == e_decode)
    {
        if(argc < 3)
        {
            printf("Invalid arguments\n");
            printf("./a.out -d stego.bmp output\n");
            return e_failure;
        }

        DecodeInfo decInfo;

        if(read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if(do_decoding(&decInfo) == e_success)
            {
                printf("Decoding completed successfully\n");
            }
            else
            {
                printf("Decoding failed\n");
            }
        }
        else
        {
            printf("Invalid decoding arguments\n");
            return e_failure;
        }
    }


    /* ================= HELP ================= */

    else if(type == e_help)
    {
        printf("Usage:\n");
        printf("Encoding : ./a.out -e <source.bmp> <secret_file> <stego.bmp>\n");
        printf("Decoding : ./a.out -d <stego.bmp> <output_file>\n");
    }

    /* ================= UNSUPPORTED ================= */

    else
    {
        printf("Invalid Input!!\n");
        printf("./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("./a.out -d stego.bmp output\n");
        return e_failure;
    }

    return e_success;
}