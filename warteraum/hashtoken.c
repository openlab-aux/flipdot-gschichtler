#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "auth.h"

int main(int argc, char **argv) {
  if(argc != 2) {
    fputs("Usage: ", stderr);
    fputs(argv[0], stderr);
    fputs(" TOKEN_STRING", stderr);
    return 1;
  }

  const char *t = argv[1];
  size_t t_len = strlen(t);

  uint8_t output[SCRYPT_OUTPUT_LEN];

  int res = HASH_TOKEN(t, t_len, output);

  if(res == 0) {
    putchar('{');
    for(size_t i = 0; i < SCRYPT_OUTPUT_LEN; i++) {
      printf(" 0x%x", output[i]);

      if(SCRYPT_OUTPUT_LEN != i + 1) {
        fputs(",", stdout);
      }
    }

    puts(" }");

    return 0;
  } else {
    return 1;
  }
}
