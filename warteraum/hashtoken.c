#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "http_string.h"
#include "auth.h"

int main(int argc, char **argv) {
  if(argc != 3) {
    fputs("Usage: ", stderr);
    fputs(argv[0], stderr);
    fputs(" SALT_FILE TOKEN_STRING", stderr);
    return 1;
  }

  errno = 0;

  struct http_string_s salt = http_string_fread(argv[1]);

  if(errno != 0) {
    perror("Couldn't read the salt file");
    return 1;
  }

  struct http_string_s token;

  token.buf = argv[2];
  token.len = strlen(token.buf);

  uint8_t output[SCRYPT_OUTPUT_LEN];

  int res = HASH_TOKEN(salt, token, output);

  if(res == 0) {
    size_t written = fwrite(output, sizeof(uint8_t), SCRYPT_OUTPUT_LEN, stdout);

    return !(written == SCRYPT_OUTPUT_LEN);
  } else {
    return 1;
  }
}
