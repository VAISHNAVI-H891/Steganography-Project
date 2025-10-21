#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/*-----------------------------------------------------------------------------------------------*/
/* --- Description for read_and_validate_decode_args Function --->
---------------------------------------------------------------------------------------------------

 * Input : argc, argv, decInfo
 * Output: Status (e_success / e_failure)
 * Description: Validates command line arguments for decoding and
 * stores stego image file name and optional output file name.
 */
Status read_and_validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo)
{
     if(argc < 3 || argc > 4)  // Check argument count
     {
        return e_failure;
     }
     if(strstr(argv[2],".bmp") == NULL) // Validate stego BMP image
     {
        return e_failure;
     }
     
     decInfo->stego_image_fname = argv[2]; // Store stego image file name

     if(argc == 4)   // If user provided output secret file name
     {
            decInfo->secret_fname = argv[3];
     }
     else
     {
        decInfo->secret_fname = NULL; // Otherwise NULL
     }

     return e_success;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* --- Description for open_decode_files Function --->
-----------------------------------------------------------------------------------------------------------------------

 * Input : decInfo
 * Output: Status
 * Description: Opens stego image file for reading.
 */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"rb"); // Open stego image
    
    if(decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr,"ERROR : Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    
    return e_success;
}

/*----------------------------------------------------------------------------------------------------------------------------------*/
/* --- Description for decode_byte_from_lsb Function --->
------------------------------------------------------------------------------------------------------------------------------------

 * Input : image_buffer, data pointer
 * Output: Status
 * Description: Decodes 1 byte from 8 LSBs of image buffer bytes.
 */
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    unsigned char ch = 0; // Variable to hold decoded byte

    for(int i = 0; i < 8; i++)
    {
        ch = ch | (image_buffer[i] & 1) << i; // Extract LSB and place at correct bit
    }

    *data = ch; // Store decoded byte
    return e_success;
}

/*-----------------------------------------------------------------------------------------------------------------------------------*/
/* --- Description for decode_size_from_lsb Function --->
--------------------------------------------------------------------------------------------------------------------------------------

 * Input : buffer (32 bytes), size pointer
 * Output: Status
 * Description: Decodes 32-bit integer from LSBs of buffer.
 */
Status decode_size_from_lsb(unsigned char *buffer, int *size)
{
    int num = 0; // Variable to store decoded number
    for (int i = 0; i < 32; i++)
    {
        num = num | (buffer[i] & 1) << i; // Extract each LSB and reconstruct integer
    }
    *size = num; // Store in output pointer
    return e_success;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
/* --- Description for decode_data_from_image Function --->
-------------------------------------------------------------------------------------------------------------------------------------------

 * Input : buffer, size, fptr_stego_image
 * Output: Status
 * Description: Decodes multiple bytes from stego image into buffer.
 */
Status decode_data_from_image(char *buffer, int size, FILE *fptr_stego_image)
{
    unsigned char arr[8];

    for(int i = 0 ; i < size ; i++)
    {
        if(fread(arr,8,1,fptr_stego_image) != 1) // Read 8 bytes
            return e_failure;
        decode_byte_from_lsb(&buffer[i],arr); // Decode single byte
    }

    return e_success;
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/* --- Description for decode_magic_string Function --->
------------------------------------------------------------------------------------------------------------------------------------------

 * Input : decInfo
 * Output: Status
 * Description: Decodes and verifies magic string from stego image.
 */
Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET); // Skip BMP header

    decInfo->magic_data = malloc(strlen(MAGIC_STRING) + 1); // Allocate memory for magic string

    decode_data_from_image(decInfo->magic_data, strlen(MAGIC_STRING), decInfo->fptr_stego_image); // Decode magic string

    decInfo->magic_data[strlen(MAGIC_STRING)] = '\0'; // Null terminate

    if (strcmp(decInfo->magic_data, MAGIC_STRING) == 0) // Compare with original
        return e_success;
    else
        return e_failure;
}

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/* --- Description for decode_secret_file_extn Function --->
------------------------------------------------------------------------------------------------------------------------------------------

 * Input : file_extn size, decInfo
 * Output: Status
 * Description: Decodes secret file extension from stego image.
 */
Status decode_secret_file_extn(int file_extn, DecodeInfo *decInfo)
{
   decInfo->extn_secret_file = malloc(file_extn + 1); // Allocate memory

   decode_data_from_image(decInfo->extn_secret_file, file_extn, decInfo->fptr_stego_image); // Decode extension

   decInfo->extn_secret_file[file_extn] = '\0'; // Null terminate
   return e_success;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------*/
/* --- Description for decode_file_extn_size Function --->
-------------------------------------------------------------------------------------------------------------------------------------------

 * Input : decInfo
 * Output: Status
 * Description: Decodes the length of secret file extension from 32 LSBs.
 */
Status decode_file_extn_size(DecodeInfo *decInfo)
{
   unsigned char buffer[32];

   if(fread(buffer,1,32,decInfo->fptr_stego_image) != 32) // Read 32 bytes
      return e_failure;

   if (decode_size_from_lsb(buffer, &decInfo->extn_size) == e_failure) // Decode size
      return e_failure;

   return e_success;
}

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/* --- Description for decode_secret_file_size Function --->
---------------------------------------------------------------------------------------------------------------------------------------------

 * Input : decInfo
 * Output: Status
 * Description: Decodes size of secret file from 32 LSBs.
 */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
   char arr[32];
   int size = 0;

   fread(arr,32,1,decInfo->fptr_stego_image); // Read 32 bytes
   decode_size_from_lsb(arr, &size);           // Decode integer
   decInfo->size_secret_file = size;           // Store in structure

   return e_success;
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/
/* --- Description for decode_secret_file_data Function --->
---------------------------------------------------------------------------------------------------------------------------------------

 * Input : decInfo
 * Output: Status
 * Description: Decodes secret file data byte-by-byte and writes to output file.
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    unsigned char arr[8];
    unsigned char ch;

    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(arr, 8, 1, decInfo->fptr_stego_image); // Read 8 bytes
        decode_byte_from_lsb((char *)&ch, arr);       // Decode byte
        fputc(ch, decInfo->fptr_secret);             // Write to secret file
    }
    return e_success;
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/
/* --- Description for do_decoding Function --->
----------------------------------------------------------------------------------------------------------------------------------------

 * Input : decInfo
 * Output: Status
 * Description: Master function to perform entire decoding procedure:
 * open files, decode magic string, file extension size, extension,
 * secret size, secret data, and finalize secret file with correct extension.
 */
Status do_decoding(DecodeInfo *decInfo)
{
    if( open_decode_files(decInfo) != e_success) // Open stego image
    {
        printf("ERROR : Failed to open files.\n");
        return e_failure;
    }

    char base_name[50];

    if(decInfo->secret_fname == NULL)  // Default output name if not provided
        strcpy(base_name, "decoded"); 
    else
        strcpy(base_name, decInfo->secret_fname); // Use user-provided name

    strtok(base_name, ".");              // Remove extension if any

    decInfo->secret_fname = malloc(strlen(base_name) + 1); // Allocate memory for clean filename
    strcpy(decInfo->secret_fname, base_name);              // Copy base name

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb"); // Create output file
    if(decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", decInfo->secret_fname);
        return e_failure;
    }
    printf("INFO : Decoding file created as %s\n",decInfo->secret_fname);

    printf("INFO : Decoding Magic String Signature\n");
    if(decode_magic_string(decInfo) == e_success)
        printf("INFO : Done. Magic string Matched\n");
    else
    {
        printf("ERROR : Magic String not matched\n");
        return e_failure;
    }

    printf("INFO : Decoding secret.txt File Extension Size\n");
    if (decode_file_extn_size(decInfo) == e_success)
        printf("INFO : Done\n");
    else
    {
        printf("ERROR : Failed Decoding of Secret.txt File Extension Size\n");
        return e_failure;
    }

    printf("INFO : Decoding Secret File Extension\n");
    if (decode_secret_file_extn(decInfo->extn_size, decInfo) == e_success)
        printf("INFO : Done\n");
    else
    {
        printf("ERROR : Failed Decoding of Secret.txt File Extension\n");
        return e_failure;
    }

    char newname[64]; 
    sprintf(newname, "%s%s",decInfo->secret_fname, decInfo->extn_secret_file); // Append extension

    fclose(decInfo->fptr_secret);            // Close temporary file
    rename(decInfo->secret_fname, newname);  // Rename to final name
    decInfo->fptr_secret = fopen(newname,"a"); // Reopen final file in append mode
    if(decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", decInfo->secret_fname);
        return e_failure;
    }
    printf("INFO : The final Decoded file with Extension : %s\n",newname);

    printf("INFO : Decoding secret.txt File Size\n");
    if (decode_secret_file_size(decInfo) == e_success)
        printf("INFO : Done\n");
    else
    {
        printf("ERROR : Failed Decoding of secret.txt file size\n");
        return e_failure;
    }

    printf("INFO : Decoding secret.txt File Data\n");
    if (decode_secret_file_data(decInfo) == e_success)
        printf("INFO : Done\n");
    else
    {
        printf("ERROR : Failed Decoding of secret.txt file data\n");
        return e_failure;
    }

    fclose(decInfo->fptr_stego_image);   // Close stego image
    fclose(decInfo->fptr_secret);        // Close secret file

    free(decInfo->extn_secret_file);     // Free allocated memory
    free(decInfo->secret_fname);

    return e_success;
}
