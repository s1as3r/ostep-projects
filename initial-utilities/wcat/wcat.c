#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 1000

const char USAGE[100] = "Usage: ./wcat FILE [FILE]...\n";

int wcat_file(const char *filepath) {
  FILE *file = fopen(filepath, "r");
  if (file == NULL) {
    // perror("error opening file");
    return errno;
  }

  char buf[BUFSIZE];
  while (fgets(buf, BUFSIZE, file) != NULL) {
    printf("%s", buf);
  }

  // check if there was an error when reading file
  if (!feof(file)) {
    // perror("error reading file");
    return errno;
  }

  fclose(file);
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  for (int idx = 1; idx < argc; idx++) {
    if (wcat_file(argv[idx]) != EXIT_SUCCESS) {
      printf("wcat: cannot open file\n");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
