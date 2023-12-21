#include <stdio.h>
#include <stdlib.h>

#include "platform.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const enum platform p = platform_get();

  printf("Platform: %s\n", platform_to_string(p));

  return EXIT_SUCCESS;
}
