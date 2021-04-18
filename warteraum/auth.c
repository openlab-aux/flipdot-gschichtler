#include "tokens.h"
#include "auth.h"

#define HASH_TOKEN(token, size, output)                         \
  scrypt_kdf((const uint8_t *) token, size, salt, sizeof(salt), \
             SCRYPT_N, SCRYPT_r, SCRYPT_p, output, SCRYPT_OUTPUT_LEN)

bool auth_verify(struct http_string_s token) {
  uint8_t hashed[SCRYPT_OUTPUT_LEN];

  int hash_result = HASH_TOKEN(token.buf, token.len, hashed);

  if(hash_result != 0) {
    return false;
  }

  bool token_matches = false;
  size_t token_count = sizeof(tokens) / (sizeof(uint8_t) * SCRYPT_OUTPUT_LEN);

  for(size_t i = 0; i < token_count && !token_matches; i++) {
    token_matches = true;
    for(size_t j = 0; j < SCRYPT_OUTPUT_LEN && token_matches; j++) {
      token_matches = tokens[i][j] == hashed[j];
    }
  }

  return token_matches;
}
