#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "common.h"


/*Name: Vaishnavi R Hujaratti[25017E_269]
Date:06/10/2025
Description: 
* This project implements LSB (Least Significant Bit) Image Steganography in C language.
* It allows you to hide any secret file (e.g., .txt, .pdf, .c, .exe) inside a BMP image and later extract it back without loss.
* Steganography ensures that the hidden data is invisible to the human eye, since only the least significant bits of image pixels are modified.


/*------------------------------------------------------------------------------------------------------------*/
/* --- Description for main Function --->
---------------------------------------------------------------------------------------------------------------
 * Input  : argc, argv (Command line arguments)
 * Output : int (e_success / e_failure)
 * Description:
 *      1. Determines operation type (encode/decode) based on arguments.
 *      2. Performs encoding if '-e' or '-E' is specified.
 *      3. Performs decoding if '-d' or '-D' is specified.
 *      4. Prints error messages and usage instructions for invalid arguments.
 */
int main(int argc,char *argv[])
{
    EncodeInfo encInfo;   // Structure to hold encoding info
    DecodeInfo decInfo;   // Structure to hold decoding info

    // Function call to check operation type (-e/-d)
    OperationType res = check_operation_type(argc,argv);

    switch(res)
    {
        case e_encode :
        {
            // Read and validate encoding arguments
            if(read_and_validate_encode_args(argc, argv, &encInfo) == e_success)
            {
                // Perform encoding
                if(do_encoding(&encInfo) == e_success)
                {
                    printf("INFO : ## Encoding Done Successfully ##\n");
                }   
                else
                {
                    printf("INFO : ## Encoding Failed ##\n");
                    return e_failure;
                }
            }
            else
            {
                // Invalid arguments for encoding
                printf("INFO : ## Invalid Arguments for Encoding ##\n");
                printf("Usage : <./a.out> -e/-E <.bmp_file> <.txt_file> [output file]\n");
                return e_failure;
            }
        }
        break;

        case e_decode :
        {
            // Read and validate decoding arguments
            if (read_and_validate_decode_args(argc, argv, &decInfo) == e_success)
            {
                // Open stego image for decoding
                if (open_decode_files(&decInfo) == e_success)
                {
                    // Perform decoding
                    if (do_decoding(&decInfo) == e_success)
                    {
                        printf("INFO : ## Decoding Done Successfully ##\n");
                    }
                    else
                    {
                        printf("INFO : ## Decoding Failed ##\n");
                    }
                }
                else
                {
                    printf("INFO : ## Failed to Open Files for Decoding ##\n");
                }
            }
            else
            {
                // Invalid arguments for decoding
                printf("INFO : ## Invalid Arguments for Decoding ##\n");
                printf("Usage : <./a.out> -d/-D <.bmp_file> [output file]\n");
            }
        }
        break;

        default :
        {
            // Invalid operation type
            printf("INFO : ## Invalid Arguments ##\n");
            printf("For Encoding --> Usage : <./a.out> -e/-E <.bmp_file> <.txt_file> [output file]\n");
            printf("For Decoding --> Usage : <./a.out> -d/-D <.bmp_file> [output file]\n");
            return e_failure;
        }

    }
}
