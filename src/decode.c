#include <stdio.h>
#include "decode.h"
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "common.h"

/*
 * Detect image type by reading magic bytes.
 * BMP magic: "BM" (bytes 0-1)
 */
Status detect_image_type_decode(DecodeInfo *decInfo)
{
    char magic[2];
    fseek(decInfo->fptr_stego_image, 0, SEEK_SET);
    fread(magic, 1, 2, decInfo->fptr_stego_image);
    fseek(decInfo->fptr_stego_image, 0, SEEK_SET);

    if (magic[0] == 'B' && magic[1] == 'M')
    {
        decInfo->is_bmp = 1;
        decInfo->header_size = 54;
        printf("Image type: BMP\n");
    }
    else
    {
        decInfo->is_bmp = 0;
        decInfo->header_size = 0;
        printf("Image type: Non-BMP (treating as raw bytes)\n");
    }
    return e_success;
}

Status open_files_decode(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if(decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
    	return e_failure;
    }

    if (decInfo->terminal_mode == 0)
    {
        decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");
        if(decInfo->fptr_secret == NULL)
        {
            perror("fopen");
        	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);
        	return e_failure;
        }
    }
    return e_success;
}

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Accept any file as stego image
    decInfo->stego_image_fname = argv[2];

    if(argv[3]!=NULL)
    {
        decInfo->secret_fname = argv[3];
        decInfo->terminal_mode = 0;
    }
    else
    {
        decInfo->terminal_mode = 1;
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
        if (fread(decInfo->image_data, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }
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
    // Calculate LSB capacity and overhead
    uint lsb_capacity = decInfo->file_size - decInfo->header_size;
    uint overhead = 2 + 4 + strlen(decInfo->extn_secret_file) + 4; // magic + extn_size + extn + file_size
    uint lsb_data_capacity = (lsb_capacity > overhead) ? (lsb_capacity - overhead) : 0;

    int secret_size = decInfo->size_secret_file;
    int lsb_decode_size = (secret_size > lsb_data_capacity) ? lsb_data_capacity : secret_size;
    int overflow_size = (secret_size > lsb_data_capacity) ? (secret_size - lsb_data_capacity) : 0;

    char *secret_data = (char *)malloc(secret_size);
    if (secret_data == NULL)
    {
        fprintf(stderr, "ERROR: Unable to allocate memory for secret data\n");
        return e_failure;
    }

    // Decode data from LSB
    if (lsb_decode_size > 0)
    {
        Status ret = decode_data_from_image(secret_data, lsb_decode_size, decInfo);
        if (ret != e_success)
        {
            free(secret_data);
            return e_failure;
        }
    }

    // Read overflow data appended after the image content
    if (overflow_size > 0)
    {
        printf("Info: Reading %d bytes of overflow data from end of file\n", overflow_size);
        fread(secret_data + lsb_decode_size, overflow_size, 1, decInfo->fptr_stego_image);
    }

    if (decInfo->terminal_mode == 1)
    {
        printf("Message: ");
        fwrite(secret_data, secret_size, 1, stdout);
        printf("\n");
    }
    else
    {
        fwrite(secret_data, secret_size, 1, decInfo->fptr_secret);
        fclose(decInfo->fptr_secret);
    }
    free(secret_data);

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if(open_files_decode(decInfo)==e_success)
    {
        printf("Open file success\n");

        // Detect image type
        detect_image_type_decode(decInfo);

        // Get total file size
        fseek(decInfo->fptr_stego_image, 0, SEEK_END);
        decInfo->file_size = ftell(decInfo->fptr_stego_image);
        fseek(decInfo->fptr_stego_image, 0, SEEK_SET);

        // Skip header
        fseek(decInfo->fptr_stego_image, decInfo->header_size, SEEK_SET);

        if(decode_magic_string(decInfo)==e_success)
        {
            printf("Decoded magic string successfully\n");
            if(decode_secret_file_extn_size(decInfo)==e_success)
            {
                printf("Decoded secret file extn size successfully\n");
                if(decode_secret_file_extn(decInfo)==e_success)
                {
                    printf("Decoded file extn successfully\n");
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
