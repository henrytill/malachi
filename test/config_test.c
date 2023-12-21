#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

#define TEST(e)                                                   \
  if (!(e)) {                                                     \
    (void)fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); \
    exit(EXIT_FAILURE);                                           \
  }

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
  printf("%s:%s: Starting...\n", __FILE__, __func__);
  TEST(strcmp(platform_to_string(PLATFORM_UNKNOWN), "Unknown") == 0);
  TEST(strcmp(platform_to_string(PLATFORM_WINDOWS), "Windows") == 0);
  TEST(strcmp(platform_to_string(PLATFORM_MACOS), "MacOS") == 0);
  TEST(strcmp(platform_to_string(PLATFORM_LINUX), "Linux") == 0);
  printf("%s:%s: Passed!\n", __FILE__, __func__);
  return EXIT_SUCCESS;
}
