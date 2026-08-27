#include "stegano_wrapper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int stegano_encode_file(const char *src_image, const char *secret_file, const char *output_image)
{
    EncodeInfo encInfo;
    memset(&encInfo, 0, sizeof(EncodeInfo));

    encInfo.src_image_fname = (char *)src_image;
    encInfo.secret_fname = (char *)secret_file;
    encInfo.stego_image_fname = (char *)output_image;
    encInfo.message_mode = 0;

    return (do_encoding(&encInfo) == e_success) ? 0 : 1;
}

int stegano_encode_message(const char *src_image, const char *message, const char *output_image)
{
    EncodeInfo encInfo;
    memset(&encInfo, 0, sizeof(EncodeInfo));

    encInfo.src_image_fname = (char *)src_image;
    encInfo.secret_message = (char *)message;
    encInfo.stego_image_fname = (char *)output_image;
    encInfo.message_mode = 1;

    return (do_encoding(&encInfo) == e_success) ? 0 : 1;
}

int stegano_decode_to_file(const char *stego_image, const char *output_file)
{
    DecodeInfo decInfo;
    memset(&decInfo, 0, sizeof(DecodeInfo));

    decInfo.stego_image_fname = (char *)stego_image;
    decInfo.secret_fname = (char *)output_file;
    decInfo.terminal_mode = 0;

    return (do_decoding(&decInfo) == e_success) ? 0 : 1;
}

char *stegano_decode_to_string(const char *stego_image)
{
    DecodeInfo decInfo;
    memset(&decInfo, 0, sizeof(DecodeInfo));

    decInfo.stego_image_fname = (char *)stego_image;
    decInfo.terminal_mode = 1;

    /* Redirect stdout to capture output */
    /* Instead, we'll modify decode to return data directly */
    /* For now, use the file-based approach with a temp file */

    char temp_file[] = "/tmp/stegano_decode_XXXXXX";
    int fd = mkstemp(temp_file);
    if (fd < 0) return NULL;
    close(fd);

    decInfo.secret_fname = temp_file;
    decInfo.terminal_mode = 0;

    if (do_decoding(&decInfo) != e_success)
    {
        unlink(temp_file);
        return NULL;
    }

    /* Read the temp file */
    FILE *f = fopen(temp_file, "r");
    if (f == NULL)
    {
        unlink(temp_file);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *result = (char *)malloc(size + 1);
    if (result == NULL)
    {
        fclose(f);
        unlink(temp_file);
        return NULL;
    }

    fread(result, size, 1, f);
    result[size] = '\0';

    fclose(f);
    unlink(temp_file);

    return result;
}
