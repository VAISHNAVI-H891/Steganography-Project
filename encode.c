#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* --- Description for check_operation_type Function --->
    * Input : argc(argument count), argv(argument values)
    * Output : OperationType (e_encode / e_decode / e_unsupported)
    * Description: 
    * Checks command-line arguments to decide whether user wants to perform encoding or decoding. 
    * Returns e_encode if "-e/-E", e_decode if "-d/-D", otherwise e_unsupported.
*/

/* Check operation type */
OperationType check_operation_type(int argc,char *argv[])
{
    if(argc < 2)
        return e_unsupported;

    //convert second char of argv[1] to lowercase
    char op = tolower(argv[1][1]);

    if(op == 'e')                       // If argument is -e/-E
        return e_encode;                // return encode operation
    if(op == 'd')                       // if argument is -d/-D
        return e_decode;                // return decode operation
    else
        return e_unsupported;
}


/* --- Description for read_and_validate_encode_args Function --->
 * Input: argc, argv, encInfo (EncodeInfo structure)
 * Output: Status (e_success / e_failure)
 * Description: 
 * Validates arguments for encoding mode.          
 * Extracts source image file, secret file, and output file.            
 * Ensures image ends with .bmp and secret file exists.
 */

/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(int argc, char *argv[], EncodeInfo *encInfo)
{
    // check for correct number of arguments
    if(argc < 4 || argc > 5)  
    {
        return e_failure;
    }

    //validate source image (must end with .bmp)
    if(strstr(argv[2],".bmp") == NULL)
    {
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];    

    //validate secret file (must contain a dot, like .txt)
    if(strstr(argv[3],".") == NULL)
    {
        return e_failure;
    }
    encInfo->secret_fname = argv[3];  

    //validate output stego image (if given by CLA) else use default
    if(argc == 5)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        printf("INFO : Output File not mentioned. Creating stego.bmp as default\n");
        encInfo->stego_image_fname = "stego.bmp";   
    }
    return e_success;
}


/* --- Description for open_files Function --->
 * Input: encInfo (structure containing file names)
 * Output: Status (e_success/e_failure)
 * Description: Opens source image, secret file and stego image file.
 * Returns e_failure if any file cannot be opened.
 */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}


/* --- Description for check_capacity Function --->
 * Input: encInfo (source and secret files)
 * Output: Status (e_success/e_failure)
 * Description: Ensures that source image has enough space to hide
 * secret file data along with magic string and metadata.
 */
Status check_capacity(EncodeInfo *encInfo)
{
    uint image_data_bytes = get_image_size_for_bmp(encInfo->fptr_src_image);
    uint secret_file_size = get_file_size(encInfo->fptr_secret);

    // each secret byte requires 8 image bytes (1 LSB per image byte)
    uint image_capacity_for_secret_bytes = image_data_bytes / 8;

    // required bytes: magic string + 32 bits for size + extension + 32 bits + secret
    uint required_bytes = strlen(MAGIC_STRING) + 32/8 + strlen(encInfo->extn_secret_file) + 32/8 + secret_file_size;

    if(image_capacity_for_secret_bytes >= required_bytes)
        return e_success;
    else
        return e_failure;
}


/* --- Description for get_image_size_for_bmp Function --->
 * Input: fptr_image
 * Output: total byte capacity (width * height * 3)
 * Description: Reads width and height from BMP header and returns total image data size.
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);
    return width * height * 3;
}


/* --- Description for get_file_size Function --->
 * Input: fptr
 * Output: file size in bytes
 * Description: Seeks to end of file to get size, then rewinds pointer to start.
 */
uint get_file_size(FILE *fptr)
{
    uint size;
    fseek(fptr, 0, SEEK_END);
    size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    return size;
}


/* --- Description for copy_bmp_header Function --->
 * Input: fptr_src_image, fptr_dest_image
 * Output: Status
 * Description: Copies 54-byte BMP header from source to destination.
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    unsigned char buffer[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(buffer, 1, 54, fptr_src_image);
    fwrite(buffer, 1, 54, fptr_dest_image);
    return e_success;
}


/* --- Description for encode_byte_to_lsb Function --->
 * Input: data (1 byte), image_buffer (8 bytes)
 * Output: Status
 * Description: Encodes one byte into the LSB of 8 bytes of image data.
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & (~1)) | ((data >> i) & 1);
    }
    return e_success;
}


/* --- Description for encode_size_to_lsb Function --->
 * Input: size (int), image_buffer (32 bytes)
 * Output: Status
 * Description: Encodes a 32-bit integer into the LSBs of 32 bytes of image data.
 */
Status encode_size_to_lsb(int size, char *image_buffer)
{
    for(int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & (~1)) | ((size >> i) & 1);
    }
    return e_success;
}


/* --- Description for encode_data_to_image Function --->
 * Input: data (char array), size (int), fptr_src_image, fptr_stego_image, encInfo
 * Output: Status
 * Description: Encodes multiple bytes into the image by repeatedly calling encode_byte_to_lsb.
 */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo)
{
    unsigned char arr[8];
    for(int i = 0; i < size; i++)
    {
        fread(arr, 8, 1, fptr_src_image);
        encode_byte_to_lsb(data[i], arr);
        fwrite(arr, 8, 1, fptr_stego_image);
    }
    return e_success;
}


/* --- Description for encode_magic_string Function --->
 * Input: magic_string, encInfo
 * Output: Status
 * Description: Stores predefined MAGIC_STRING in the image to verify decoding later.
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(MAGIC_STRING, strlen(MAGIC_STRING), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}


/* --- Description for encode_secret_file_extn Function --->
 * Input: file_extn, encInfo
 * Output: Status
 * Description: Encodes secret file extension (like ".txt") into image.
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}


/* --- Description for encode_secret_file_extn_size Function --->
 * Input: size, fptr_src_image, fptr_stego_image
 * Output: Status
 * Description: Encodes size of secret file extension using 32 LSBs.
 */
Status encode_secret_file_extn_size(int size, FILE* fptr_src_image, FILE* fptr_stego_image)
{
    char arr[32];
    fread(arr, 32, 1, fptr_src_image);
    encode_size_to_lsb(size, arr);
    fwrite(arr, 32, 1, fptr_stego_image);
    return e_success;
}


/* --- Description for encode_secret_file_size Function --->
 * Input: file_size, encInfo
 * Output: Status
 * Description: Encodes size of secret file using 32 LSBs.
 */
Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
    char arr[32];
    fread(arr, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, arr);
    fwrite(arr, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}


/* --- Description for encode_secret_file_data Function --->
 * Input: encInfo
 * Output: Status
 * Description: Reads secret file into buffer and encodes byte by byte into image.
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    char *buffer = malloc(encInfo->size_secret_file);
    if(buffer == NULL)
    {
        perror("malloc failed\n");
        return e_failure;
    }
    fread(buffer, 1, encInfo->size_secret_file, encInfo->fptr_secret);
    encode_data_to_image(buffer, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    free(buffer);
    return e_success;
}


/* --- Description for copy_remaining_img_data Function --->
 * Input: fptr_src, fptr_dest
 * Output: Status
 * Description: Copies remaining bytes from source image to stego image after encoding.
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch, 1, 1, fptr_src) == 1)
        fwrite(&ch, 1, 1, fptr_dest);
    return e_success;
}


/* --- Description for do_encoding Function --->
 * Input: encInfo
 * Output: Status
 * Description: Master function to drive encoding process:
 * Steps:
 * 1. Open files.
 * 2. Check capacity.
 * 3. Copy BMP header.
 * 4. Encode magic string.
 * 5. Encode secret file extension size and extension.
 * 6. Encode secret file size and data.
 * 7. Copy remaining image bytes to stego.
 */
Status do_encoding(EncodeInfo *encInfo)
{
    printf(": Opening required files\n");
    if (open_files(encInfo) == e_success)
    {
        // get the actual size of secret.txt
        encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
        printf("INFO : Opened SkeletonCode/beautiful.bmp\n");
        printf("INFO : Opened secret\n");
        printf("INFO : Opened stego.bmp\n");
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR: Failed to Open files \n");
        return e_failure;
    }

    printf("INFO : ## Encoding Procedure Started ##\n");
    printf("INFO : Checking for SkeletonCode/beautiful.bmp capacity to handle secret\n");
    if (check_capacity(encInfo) == e_success)
    {
        printf("INFO : Done. Found OK\n");
    }
    else
    {
        printf("ERROR : Image cannot hold secret data\n");
        return e_failure;
    }

    printf("INFO : Copying Image Header\n"); 
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to copy bmp header\n");
        return e_failure;
    }


    printf("INFO : Encoding Magic String Signature\n"); 
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode magic string\n");
        return e_failure;
    }

    printf("INFO : Encoding secret.txt File Extenstion Size\n"); 
    // Extract extension
    strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname, "."));
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode secret file extn size\n");
        return e_failure;
    }

    printf("INFO : Encoding secret.txt File Extenstion\n"); 
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode secret file extn\n");
        return e_failure;
    }

    printf("INFO : Encoding secret.txt File Size\n");
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode secret file size\n");
        return e_failure;
    }

    printf("INFO : Encoding secret.txt File Data\n"); 
    if (encode_secret_file_data(encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode secret file data\n");
        return e_failure;
    }
    
    printf("INFO : Copying Left Over Data\n"); 
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to copy remaining data successfully\n");
        return e_failure;
    }

    // close all the opened files
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}
