#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"

int main(int argc, char *argv[])
{
    
    int ret = check_operation_type(argv);

    if(ret == e_encode)
    { 
        EncodeInfo E1;
        printf("Info: Encoding is selected\n");
        {
            if(read_and_validate_encode_args(argv, &E1)==e_success)
            {
                printf("Info: Read and validate args is success\n");
                if(do_encoding(&E1)==e_success)
                {
                    printf("Encoding is success\n");
                }
                else
                {
                    printf("Failed to encode\n");
                }
            }
            else
            {
                printf("Info : Read and validate args failed\n");
                return 5;
            }
        }
    }
    else if(ret == e_decode)
    { 
        DecodeInfo D1;   
        printf("Info: Decoding is selected\n");
        if(read_and_validate_decode_args(argv, &D1)==e_success)
            {
                printf("Info: Read and validate args is success\n");
                if(do_decoding(&D1)==e_success)
                {
                    printf("Decoding is success\n");
                }
                else
                {
                    printf("Failed to decode\n");
                }
            }
        else
        {
            printf("Info : Read and validate args failed\n");
            return 5;
        }
    }
    else
    {
        printf("Error : Unsupported type\n");
        printf("Usage : For Encoding: ./a.out -e beautiful.bmp secret.txt [stegno.bmp]\n");
        printf("      : For Decoding: ./a.out -d stegno.bmp [decode.txt]\n");
    }
    return 0;
}

OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-e")==0)
        return e_encode;
    else if(strcmp(argv[1],"-d")==0)
        return e_decode;
    else   
        return e_unsupported;
}