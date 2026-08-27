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

#ifdef __cplusplus
}
#endif

#endif
