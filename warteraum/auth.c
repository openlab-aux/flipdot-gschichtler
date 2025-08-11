#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "auth.h"
#include "http_string.h"

// buffer containing the currently used salt
static struct http_string_s salt;
// buffer containing _all_ hashed tokens. This works because
// they have a fixed length in their hashed form, so we can
// easily loop through them.
static struct http_string_s tokens;

bool auth_init(void) {
  http_string_clear(&salt);
  http_string_clear(&tokens);

  char *salt_file = getenv("WARTERAUM_SALT_FILE");
  char *tokens_file = getenv("WARTERAUM_TOKENS_FILE");

  if(salt_file != NULL && tokens_file != NULL) {
    errno = 0;

    salt = http_string_fread(salt_file);
    if(errno != 0) {
      perror("Error: Failed reading WARTERAUM_SALT_FILE");
      return false;
    }

    tokens = http_string_fread(tokens_file);
    if(errno != 0) {
      perror("Error: Failed reading WARTERAUM_TOKENS_FILE");
      return false;
    }

    return true;
  } else {
    fputs("Warning: Missing necessary file(s) to setup auth\n", stderr);
    return false;
  }
}

void auth_cleanup(void) {
  http_string_free(&salt);
  http_string_free(&tokens);
}

bool auth_verify(struct http_string_s token) {
  if(HTTP_STRING_EMPTY(salt) || HTTP_STRING_EMPTY(tokens)) {
    return false;
  }

  uint8_t hashed[SCRYPT_OUTPUT_LEN];

  int hash_result = HASH_TOKEN(salt, token, hashed);

  if(hash_result != 0) {
    return false;
  }

  bool token_matches = false;
  int token_count = tokens.len / SCRYPT_OUTPUT_LEN;

  if(token_count * SCRYPT_OUTPUT_LEN != tokens.len)
    fputs("Warning: length of tokens file is not a whole multiple of SCRYPT_OUTPUT_LEN\n", stderr);

  for(int i = 0; i < token_count && !token_matches; i++) {
    token_matches = true;
    const char *expected = tokens.buf + (i * SCRYPT_OUTPUT_LEN);

    // hopefully constant time equality
    for(int j = 0; j < SCRYPT_OUTPUT_LEN; j++) {
      token_matches &= hashed[j] == (uint8_t) expected[j];
    }
  }

  return token_matches;
}
