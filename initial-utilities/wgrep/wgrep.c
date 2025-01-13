#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char USAGE[100] = "wgrep: searchterm [file ...]\n";
const char ERRSTR[100] = "wgrep: cannot open file\n";

int wgrep(FILE *file, const char *search) {
  char *line = NULL;
  size_t read = 0;
  int retval;
  while ((retval = getline(&line, &read, file)) != 0 && retval != -1) {
    if (strstr(line, search) != NULL) {
      printf("%s", line);
    }
    free(line);
    read = 0;
    line = NULL;
  }

  if (retval == -1) {
    return errno;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf(USAGE);
    return EXIT_FAILURE;
  }

  if (argc == 2) {
    if (wgrep(stdin, argv[1]) != EXIT_SUCCESS) {
      printf(ERRSTR);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }

  for (int idx = 2; idx < argc; idx++) {
    FILE *file = fopen(argv[idx], "r");
    if (file == NULL) {
      printf(ERRSTR);
      return EXIT_FAILURE;
    }

    if (wgrep(file, argv[1]) != EXIT_SUCCESS) {
      fclose(file);
      return EXIT_FAILURE;
    }
    fclose(file);
  }

  return EXIT_SUCCESS;
}
