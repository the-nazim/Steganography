#include <stdio.h>
#include "decode.h"
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "common.h"

Status open_files_decode(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    if(decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
    	return e_failure;
    }

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");
    if(decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);
    	return e_failure;
    }
    return e_success;
}

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strcmp(strstr(argv[2],"."),".bmp")==0)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    if(argv[3]!=NULL)
    {
        decInfo->secret_fname = argv[3];
    }
    else
    {
        decInfo->secret_fname = "decode.txt";
    }
    return e_success;
}

Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    *data = 0;
    for (int i = 0; i < 8; i++)
    {
        *data |= ((image_buffer[i] & 1) << i);
    }
    return e_success;
}

Status decode_data_from_image(char *data, int size, DecodeInfo *decInfo)
{
    for (int i = 0; i < size; i++)
    {
        fread(decInfo->image_data, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&data[i], decInfo->image_data);
    }
    return e_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic_string[3];
    decode_data_from_image(magic_string, 2, decInfo);
    magic_string[2] = '\0';

    if (strcmp(magic_string, MAGIC_STRING) == 0)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    int size = 0;
    char str[32];
    fread(str, 32, 1, decInfo->fptr_stego_image);
    
    for (int i = 0; i < 32; i++)
    {
        size |= ((str[i] & 1) << i);
    }
    
    decInfo->size_secret_file = size;
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    decode_data_from_image(decInfo->extn_secret_file, decInfo->size_secret_file, decInfo);
    decInfo->extn_secret_file[decInfo->size_secret_file] = '\0';
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    int size = 0;
    char str[32];
    fread(str, 32, 1, decInfo->fptr_stego_image);

    for (int i = 0; i < 32; i++)
    {
        size |= ((str[i] & 1) << i);
    }

    decInfo->size_secret_file = size;
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char *secret_data = (char *)malloc(decInfo->size_secret_file);
    if (secret_data == NULL)
    {
        fprintf(stderr, "ERROR: Unable to allocate memory for secret data\n");
        return e_failure;
    }

    decode_data_from_image(secret_data, decInfo->size_secret_file, decInfo);

    fwrite(secret_data, decInfo->size_secret_file, 1, decInfo->fptr_secret);
    fclose(decInfo->fptr_secret);
    free(secret_data);

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if(open_files_decode(decInfo)==e_success)
    {
        printf("Open file success\n");
        fseek(decInfo->fptr_stego_image, 54, SEEK_SET);
        if(decode_magic_string(decInfo)==e_success)
        {
            printf("Decoded magic string successfully\n");
            if(decode_secret_file_extn_size(decInfo)==e_success)
            {
                printf("Decoded secret file extn size successfully\n");
                if(decode_secret_file_extn(decInfo)==e_success)
                {
                    printf("Decoed file extn successfully\n");
                    if(decode_secret_file_size(decInfo)==e_success)
                    {
                        printf("Decoded secret file size successfully\n");
                        if(decode_secret_file_data(decInfo)==e_success)
                        {
                            printf("Decoded secret file data successfully\n");
                        }
                        else
                        {
                            printf("Failed to decode secret file data\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to decode secret file size\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed to decode file extn\n");
                    return e_failure;
                }
            }
            else
            {
               printf("Failed to decode file extn size\n");
               return e_failure;
            }
        }
        else
        {
            printf("Failed to decode magic string\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open\n");
        return e_failure;
    }
    return e_success;
}