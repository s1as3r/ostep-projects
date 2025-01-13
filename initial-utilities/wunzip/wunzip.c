#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const char USAGE[100] = "wunzip: file1 [file2 ...]\n";

int wunzip(int file_count, char *filepaths[]) {
  int file_idx = 0;
  FILE *file = fopen(filepaths[0], "r");
  if (file == NULL) {
    return errno;
  }

  int b1, b2, b3, b4;
  char ch;
  while (true) {
    b1 = fgetc(file);

    if (b1 == EOF) {
      if (file_idx == file_count - 1) {
        fclose(file);
        break;
      }
      file_idx++;
      fclose(file);
      file = fopen(filepaths[file_idx], "r");
      if (file == NULL) {
        return errno;
      }
      continue;
    }

    b2 = fgetc(file);
    b3 = fgetc(file);
    b4 = fgetc(file);
    ch = fgetc(file);

    int count = (b4 << 24) + (b3 << 16) + (b2 << 8) + b1;
    for (int i = 0; i < count; i++) {
      putchar(ch);
    }
  }

  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("%s", USAGE);
    return EXIT_FAILURE;
  }

  return wunzip(argc - 1, &argv[1]);
}
