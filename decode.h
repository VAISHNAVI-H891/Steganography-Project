#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>  //for FILE *
#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * decoding secret file to stego Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)

// Structure to hold decoding related imformation
typedef struct _DecodeInfo
{
   /* Stego Image Info */     
    char *stego_image_fname;   
    FILE *fptr_stego_image;

    /* magic string */
    char *magic_data;

    /* Secret File Info */
    char *secret_fname;     
    FILE *fptr_secret;
    int size_secret_file;
    char *extn_secret_file;
    int extn_size;
   
} DecodeInfo;


/* --- function prototype for Decoding --- */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo);

/* Get File pointers for i/p stego and o/p decoded files */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode a byte from LSB of image data array */
Status decode_byte_from_lsb(char *data, char *image_buffer);

/* Decode integer size values from LSB of image bytes */
Status decode_size_from_lsb(unsigned char *buffer, int *size);

/* Decode actual data bits from image into a buffer */
Status decode_data_from_image(char *buffer, int size, FILE *fptr_stego_image);

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(int file_extn, DecodeInfo *decInfo);

/* Decode file extn size */
Status decode_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

#endif
