#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const char USAGE[100] = "wzip: file1 [file2 ...]\n";

void print_bytes(int num) {
  printf("%c", (num >> 0) & 0xFF);
  printf("%c", (num >> 8) & 0xFF);
  printf("%c", (num >> 16) & 0xFF);
  printf("%c", (num >> 24) & 0xFF);
}

int wzip(int file_count, char *filepaths[]) {
  int file_idx = 0;
  FILE *file = fopen(filepaths[0], "r");
  if (file == NULL) {
    return errno;
  }

  int n = 1;
  char prev = fgetc(file);
  if (prev == EOF && file_idx == file_count - 1) {
    return EXIT_SUCCESS;
  }

  char curr;
  while (true) {
    curr = fgetc(file);

    if (curr == EOF) {
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

    if (curr == prev) {
      n++;
    } else {
      print_bytes(n);
      printf("%c", prev);
      prev = curr;
      n = 1;
    }
  }

  if (n != 0) {
    print_bytes(n);
    printf("%c", prev);
  }

  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("%s", USAGE);
    return EXIT_FAILURE;
  }

  return wzip(argc - 1, &argv[1]);
}
