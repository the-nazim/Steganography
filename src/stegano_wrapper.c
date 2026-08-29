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

    /* Suppress stdout during encoding (FTXUI controls terminal) */
    FILE *old_stdout = stdout;
    stdout = fopen("/dev/null", "w");
    Status ret = do_encoding(&encInfo);
    fclose(stdout);
    stdout = old_stdout;

    return (ret == e_success) ? 0 : 1;
}

int stegano_encode_message(const char *src_image, const char *message, const char *output_image)
{
    EncodeInfo encInfo;
    memset(&encInfo, 0, sizeof(EncodeInfo));

    encInfo.src_image_fname = (char *)src_image;
    encInfo.secret_message = (char *)message;
    encInfo.stego_image_fname = (char *)output_image;
    encInfo.message_mode = 1;

    /* Suppress stdout during encoding (FTXUI controls terminal) */
    FILE *old_stdout = stdout;
    stdout = fopen("/dev/null", "w");
    Status ret = do_encoding(&encInfo);
    fclose(stdout);
    stdout = old_stdout;

    return (ret == e_success) ? 0 : 1;
}

int stegano_decode_to_file(const char *stego_image, const char *output_file)
{
    DecodeInfo decInfo;
    memset(&decInfo, 0, sizeof(DecodeInfo));

    decInfo.stego_image_fname = (char *)stego_image;
    decInfo.secret_fname = (char *)output_file;
    decInfo.terminal_mode = 0;

    /* Suppress stdout during decoding (FTXUI controls terminal) */
    FILE *old_stdout = stdout;
    stdout = fopen("/dev/null", "w");
    Status ret = do_decoding(&decInfo);
    fclose(stdout);
    stdout = old_stdout;

    return (ret == e_success) ? 0 : 1;
}

int stegano_get_info(const char *stego_image, SteganoFileInfo *info)
{
    DecodeInfo decInfo;
    memset(&decInfo, 0, sizeof(DecodeInfo));
    memset(info, 0, sizeof(SteganoFileInfo));

    decInfo.stego_image_fname = (char *)stego_image;
    decInfo.terminal_mode = 0;

    decInfo.fptr_stego_image = fopen(stego_image, "rb");
    if (decInfo.fptr_stego_image == NULL)
    {
        info->has_hidden_data = 0;
        return 1;
    }

    /* Detect image type */
    char magic[2];
    fseek(decInfo.fptr_stego_image, 0, SEEK_SET);
    fread(magic, 1, 2, decInfo.fptr_stego_image);
    if (magic[0] == 'B' && magic[1] == 'M')
    {
        decInfo.is_bmp = 1;
        decInfo.header_size = 54;
    }
    else
    {
        decInfo.is_bmp = 0;
        decInfo.header_size = 0;
    }

    /* Get total file size */
    fseek(decInfo.fptr_stego_image, 0, SEEK_END);
    decInfo.file_size = ftell(decInfo.fptr_stego_image);
    fseek(decInfo.fptr_stego_image, 0, SEEK_SET);

    /* Skip header */
    fseek(decInfo.fptr_stego_image, decInfo.header_size, SEEK_SET);

    /* Check for magic string */
    if (decode_magic_string(&decInfo) != e_success)
    {
        info->has_hidden_data = 0;
        fclose(decInfo.fptr_stego_image);
        return 1;
    }
    info->has_hidden_data = 1;

    /* Decode extension size */
    if (decode_secret_file_extn_size(&decInfo) != e_success)
    {
        fclose(decInfo.fptr_stego_image);
        return 1;
    }

    /* Decode extension */
    if (decode_secret_file_extn(&decInfo) != e_success)
    {
        fclose(decInfo.fptr_stego_image);
        return 1;
    }
    strncpy(info->extension, decInfo.extn_secret_file, sizeof(info->extension) - 1);
    info->extension[sizeof(info->extension) - 1] = '\0';

    /* Decode file size */
    if (decode_secret_file_size(&decInfo) != e_success)
    {
        fclose(decInfo.fptr_stego_image);
        return 1;
    }
    info->file_size = decInfo.size_secret_file;

    fclose(decInfo.fptr_stego_image);
    return 0;
}

char *stegano_decode_to_string(const char *stego_image)
{
    DecodeInfo decInfo;
    memset(&decInfo, 0, sizeof(DecodeInfo));

    decInfo.stego_image_fname = (char *)stego_image;
    decInfo.terminal_mode = 0;

    /* For decode-to-string, we decode to a temp file, then read it back */
    char temp_file[] = "/tmp/stegano_decode_XXXXXX";
    int fd = mkstemp(temp_file);
    if (fd < 0) return NULL;
    close(fd);

    decInfo.secret_fname = temp_file;

    /* Suppress stdout during decoding (FTXUI controls terminal) */
    FILE *old_stdout = stdout;
    stdout = fopen("/dev/null", "w");
    Status ret = do_decoding(&decInfo);
    fclose(stdout);
    stdout = old_stdout;

    if (ret != e_success)
    {
        unlink(temp_file);
        return NULL;
    }

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
