#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>
#include "common.h"

/* Function Definitions */

/*
 * Detect image type by reading magic bytes.
 * Sets is_bmp and header_size in encInfo.
 * BMP magic: "BM" (bytes 0-1)
 */
Status detect_image_type(EncodeInfo *encInfo)
{
    char magic[2];
    fseek(encInfo->fptr_src_image, 0, SEEK_SET);
    fread(magic, 1, 2, encInfo->fptr_src_image);
    fseek(encInfo->fptr_src_image, 0, SEEK_SET);

    if (magic[0] == 'B' && magic[1] == 'M')
    {
        encInfo->is_bmp = 1;
        encInfo->header_size = 54;
        printf("Image type: BMP\n");
    }
    else
    {
        encInfo->is_bmp = 0;
        encInfo->header_size = 0;
        printf("Image type: Non-BMP (treating as raw bytes)\n");
    }
    return e_success;
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file (skip in message mode)
    if (encInfo->message_mode == 0)
    {
        encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
        // Do Error handling
        if (encInfo->fptr_secret == NULL)
        {
        	perror("fopen");
        	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        	return e_failure;
        }
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Accept any file as source image (no extension check)
    encInfo->src_image_fname = argv[2];

    if (strcmp(argv[3],"-m")==0)
    {
        encInfo->message_mode = 1;
        encInfo->secret_message = argv[4];
        if (argv[5] != NULL)
        {
            encInfo->stego_image_fname = argv[5];
        }
        else
        {
            encInfo->stego_image_fname = "encode.bmp";
        }
    }
    else if (strcmp(argv[3],"-m")!=0)
    {
        encInfo->message_mode = 0;
        encInfo->secret_fname = argv[3];
        if (argv[4] != NULL)
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            encInfo->stego_image_fname = "encode.bmp";
        }
    }
    else
    {
        return e_failure;
    }
    return e_success;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    uint size = ftell(fptr);
    fseek(fptr,0,SEEK_SET);
    return size;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // Detect image type
    detect_image_type(encInfo);

    // Get image capacity
    if (encInfo->is_bmp)
    {
        encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    }
    else
    {
        // For non-BMP, use total file size as capacity
        encInfo->image_capacity = get_file_size(encInfo->fptr_src_image);
    }

    if (encInfo->message_mode == 1)
    {
        encInfo->size_secret_file = strlen(encInfo->secret_message);
    }
    else
    {
        encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    }

    // Header overhead: magic(2) + extn_size(4) + extn(n) + file_size(4) = 10 + extn bytes
    // In bits: (10 + extn_len) * 8
    // We always have capacity if we can append, so just return success
    printf("Image capacity: %u bytes\n", encInfo->image_capacity);
    printf("Secret data size: %ld bytes\n", encInfo->size_secret_file);

    return e_success;
}

Status copy_image_header(EncodeInfo *encInfo)
{
    char *str = (char *)malloc(encInfo->header_size);
    if (str == NULL)
    {
        return e_failure;
    }
    fseek(encInfo->fptr_src_image, 0, SEEK_SET);
    fread(str, encInfo->header_size, 1, encInfo->fptr_src_image);
    fwrite(str, encInfo->header_size, 1, encInfo->fptr_stego_image);
    free(str);
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0;i<8;i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE ) | ((data >> i) & 1);
    }
    return e_success;
}

Status encode_data_to_image(char *data, int size, EncodeInfo *encInfo)
{
    for(int i=0;i<size;i++)
    {
        int read = fread(encInfo->image_data,8,1,encInfo->fptr_src_image);
        if (read != 1)
        {
            // Pad with zeros if source file is exhausted
            memset(encInfo->image_data, 0, 8);
        }
        encode_byte_to_lsb(data[i], encInfo->image_data);
        fwrite(encInfo->image_data, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_size_to_lsb(int size, EncodeInfo *encInfo)
{
    char str[32];
    int read = fread(str, 32, 1, encInfo->fptr_src_image);
    if (read != 1)
    {
        // Pad with zeros if source file is exhausted
        memset(str, 0, 32);
    }
    for(int i=0;i<32;i++)
    {
        str[i] = (str[i] & 0xFE) | ((size >> i ) & 1);
    }
    fwrite(str,32,1,encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    encode_size_to_lsb(size, encInfo);
    return e_success;
}

Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(magic_string, strlen(magic_string), encInfo);
    return e_success;
}

Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image(file_extn, strlen(file_extn),encInfo);
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    encode_size_to_lsb(file_size, encInfo);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char *str = (char *)malloc(encInfo->size_secret_file);
    if (str == NULL)
    {
        return e_failure;
    }
    fseek(encInfo->fptr_secret,0,SEEK_SET);
    fread(str,encInfo->size_secret_file,1,encInfo->fptr_secret);
    Status ret = encode_data_to_image(str, encInfo->size_secret_file,encInfo);
    free(str);
    return ret;
}

Status copy_remaining_img_data(EncodeInfo *encInfo)
{
    int length = encInfo->header_size + encInfo->image_capacity - ftell(encInfo->fptr_stego_image);
    if (length <= 0)
    {
        return e_success;
    }
    char *str = (char *)malloc(length);
    if (str == NULL)
    {
        return e_failure;
    }
    fread(str,length,1,encInfo->fptr_src_image);
    fwrite(str,length,1,encInfo->fptr_stego_image);
    free(str);
    return e_success;
}

/*
 * Encode data that overflows beyond the image's LSB capacity.
 * This data is appended after the image content.
 * For BMP, viewers ignore extra data so the image looks normal.
 */
Status encode_overflow_data(char *data, int size, EncodeInfo *encInfo)
{
    fwrite(data, size, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo)==e_success)
    {
        printf("Open file success\n");
        {
            if(check_capacity(encInfo)==e_success)
            {
                printf("Check capacity is success\n");
                if(copy_image_header(encInfo)==e_success)
                {
                    printf("Image header copied successfully\n");
                    if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                    {
                        printf("Encoded Magic string successfully\n");
                        if (encInfo->message_mode == 1)
                        {
                            strcpy(encInfo->extn_secret_file, ".txt");
                        }
                        else
                        {
                            strcpy(encInfo->extn_secret_file ,strstr(encInfo->secret_fname,"."));
                        }
                        if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo)==e_success)
                        {
                            printf("Encoded secret file extn size successfully\n");
                            if(encode_secret_file_extn(encInfo->extn_secret_file ,encInfo)==e_success)
                            {
                                printf("Encoded secret file extn successfully\n");
                                if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_success)
                                {
                                    printf("Encoded secret file size successfully\n");

                                    // Calculate how much data fits in LSB
                                    // Overhead: magic(2*8) + extn_size(32) + extn(n*8) + file_size(32) = 64 + n*8 bits = 8 + n bytes
                                    uint overhead = 2 + 4 + strlen(encInfo->extn_secret_file) + 4; // in bytes
                                    uint lsb_capacity = encInfo->image_capacity - encInfo->header_size;
                                    uint lsb_data_capacity = (lsb_capacity > overhead) ? (lsb_capacity - overhead) : 0;

                                    int secret_size = encInfo->size_secret_file;
                                    int overflow_size = (secret_size > lsb_data_capacity) ? (secret_size - lsb_data_capacity) : 0;
                                    int lsb_encode_size = (secret_size > lsb_data_capacity) ? lsb_data_capacity : secret_size;

                                    if (overflow_size > 0)
                                    {
                                        printf("Info: Secret data overflows image LSB capacity by %d bytes\n", overflow_size);
                                        printf("Info: Overflow data will be appended after image content\n");
                                    }

                                    // Encode data that fits in LSB
                                    Status data_status = e_success;
                                    if (lsb_encode_size > 0)
                                    {
                                        if (encInfo->message_mode == 1)
                                        {
                                            data_status = encode_data_to_image(encInfo->secret_message, lsb_encode_size, encInfo);
                                        }
                                        else
                                        {
                                            // Read and encode partial data from file
                                            char *str = (char *)malloc(lsb_encode_size);
                                            if (str == NULL)
                                            {
                                                return e_failure;
                                            }
                                            fseek(encInfo->fptr_secret, 0, SEEK_SET);
                                            fread(str, lsb_encode_size, 1, encInfo->fptr_secret);
                                            data_status = encode_data_to_image(str, lsb_encode_size, encInfo);
                                            free(str);
                                        }
                                    }

                                    if(data_status == e_success)
                                    {
                                        printf("Encoded secret data successfully\n");
                                        if(copy_remaining_img_data(encInfo)==e_success)
                                        {
                                            printf("Copied remaining data successfully\n");

                                            // Append overflow data after image content
                                            if (overflow_size > 0)
                                            {
                                                if (encInfo->message_mode == 1)
                                                {
                                                    encode_overflow_data(encInfo->secret_message + lsb_encode_size, overflow_size, encInfo);
                                                }
                                                else
                                                {
                                                    char *overflow = (char *)malloc(overflow_size);
                                                    if (overflow == NULL)
                                                    {
                                                        return e_failure;
                                                    }
                                                    fseek(encInfo->fptr_secret, lsb_encode_size, SEEK_SET);
                                                    fread(overflow, overflow_size, 1, encInfo->fptr_secret);
                                                    encode_overflow_data(overflow, overflow_size, encInfo);
                                                    free(overflow);
                                                }
                                                printf("Appended overflow data successfully\n");
                                            }
                                        }
                                        else
                                        {
                                            printf("Failed to copy remaining data\n");
                                            return e_failure;
                                        }
                                    }
                                    else
                                    {
                                        printf("Failed to encode secret data\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failed to encode secret file size\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed to encode secret file extn\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("Failed to encode secret file extn size\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to copy magic string\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed to copy image header\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to check capacity\n");
                return e_failure;
            }
        }
    }
    else
    {
        printf("Failed to open\n");
        return e_failure;
    }
    return e_success;
}
