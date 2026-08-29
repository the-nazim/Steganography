#ifndef STEGANO_WRAPPER_H
#define STEGANO_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "encode.h"
#include "decode.h"

/* Encode a file into an image */
int stegano_encode_file(const char *src_image, const char *secret_file, const char *output_image);

/* Encode a message into an image */
int stegano_encode_message(const char *src_image, const char *message, const char *output_image);

/* Decode to file */
int stegano_decode_to_file(const char *stego_image, const char *output_file);

/* Decode to terminal (returns decoded string, caller must free) */
char *stegano_decode_to_string(const char *stego_image);

/* Structure to hold decoded file info */
typedef struct {
    char extension[16];   /* e.g. ".pdf", ".mp3", ".jpg" */
    long file_size;       /* size of hidden file in bytes */
    int has_hidden_data;  /* 1 if magic string found, 0 otherwise */
} SteganoFileInfo;

/* Get info about hidden data in a stego image (without extracting) */
int stegano_get_info(const char *stego_image, SteganoFileInfo *info);

#ifdef __cplusplus
}
#endif

#endif
