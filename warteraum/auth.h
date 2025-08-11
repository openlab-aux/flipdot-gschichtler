#ifndef WARTERAUM_AUTH_H
#define WARTERAUM_AUTH_H

#include <stdbool.h>
#include <stdint.h>
#include <scrypt-kdf.h>

#include "../third_party/httpserver.h/httpserver.h"

#define SCRYPT_OUTPUT_LEN 32

#define SCRYPT_N 16384
#define SCRYPT_r 8
#define SCRYPT_p 1

#define HASH_TOKEN(salt, tok, output)                                   \
  scrypt_kdf((const uint8_t *) token.buf, token.len,                    \
             (const uint8_t *) salt.buf, salt.len,                      \
             SCRYPT_N, SCRYPT_r, SCRYPT_p, output, SCRYPT_OUTPUT_LEN)

bool auth_init(void);
void auth_cleanup(void);

bool auth_verify(struct http_string_s token);

#endif
