# LSB Image Steganography in C

This project implements image steganography using the Least Significant Bit (LSB) technique in C. It allows hiding secret data inside a BMP image and retrieving the hidden data from the encoded image.

The project includes separate modules for encoding and decoding, making the implementation easier to organize and maintain.

## Features

- Encode secret data into a BMP image
- Decode hidden data from a stego image
- Uses the Least Significant Bit (LSB) technique
- Supports BMP image files
- Modular implementation using separate source and header files
- Handles encoding and decoding operations

## Technologies Used

- C programming
- File handling
- Bitwise operations
- Pointers
- Structures
- Modular programming

## Project Structure

```text
LSB Image Steganography/
├── main.c
├── encode.c
├── encode.h
├── decode.c
├── decode.h
├── types.h
├── beautiful.bmp
└── .gitignore
