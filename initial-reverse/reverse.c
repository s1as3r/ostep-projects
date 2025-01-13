#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SVEC_GROWTH_FACTOR 2

// clang-format off
const char ERR_INPUT_OUTPUT_SAME[100] = "reverse: input and output file must differ\n";
const char ERR_CANNOT_OPEN_FILE[100] = "reverse: cannot open file '%s'\n";
const char USAGE[100] = "usage: reverse <input> <output>\n";
// clang-format on

typedef struct StrVector {
  char **data;
  size_t capacity;
  size_t len;
} StrVector;

StrVector svec_new(int cap) {
  char **data = malloc(cap * sizeof(char *));
  if (data == NULL) {
    fprintf(stderr, "malloc failed");
    exit(EXIT_FAILURE);
  }

  return (StrVector){.data = data, .capacity = cap, .len = 0};
}

void svec_push(StrVector *svec, char *str) {
  if (svec->len == svec->capacity) {
    size_t new_cap = SVEC_GROWTH_FACTOR * svec->capacity;
    svec->data = realloc((void *)svec->data, sizeof(char *) * new_cap);
    svec->capacity = new_cap;

    if (svec->data == NULL) {
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }
  }

  svec->data[svec->len] = str;
  svec->len++;
}

char *svec_pop(StrVector *svec) {
  if (svec->len == 0) {
    return NULL;
  }
  svec->len--;
  return svec->data[svec->len];
}

void svec_free(StrVector *svec) {
  char *s;
  while ((s = svec_pop(svec)) != NULL) {
    free(s);
  }
  free(svec->data);
}

int reverse(FILE *from, FILE *to) {
  StrVector svec = svec_new(10);
  char *line = NULL;
  size_t n = 0;
  while (true) {
    int ret = getline(&line, &n, from);
    if (ret == -1 || ret == 0) {
      free(line);
      break;
    }
    svec_push(&svec, line);
    line = NULL;
    n = 0;
  }

  while ((line = svec_pop(&svec)) != NULL) {
    fprintf(to, "%s", line);
    free(line);
  }
  svec_free(&svec);

  return EXIT_SUCCESS;
}

bool are_files_same(char *f1, char *f2) {
  FILE *file1 = fopen(f1, "r");
  FILE *file2 = fopen(f2, "r");

  if (file1 == NULL || file2 == NULL) {
    return false;
  }

  struct stat s1;
  struct stat s2;
  fstat(fileno(file1), &s1);
  fstat(fileno(file2), &s2);

  fclose(file1);
  fclose(file2);
  return s1.st_ino == s2.st_ino;
}

int from_file_to_stdout(char *filepath) {
  FILE *file = fopen(filepath, "r");
  if (file == NULL) {
    fprintf(stderr, ERR_CANNOT_OPEN_FILE, filepath);
    return EXIT_FAILURE;
  }

  int retval = reverse(file, stdout);
  fclose(file);
  return retval;
}

int from_file_to_file(char *input_filepath, char *output_filepath) {
  if (are_files_same(input_filepath, output_filepath)) {
    fprintf(stderr, ERR_INPUT_OUTPUT_SAME);
    return EXIT_FAILURE;
  }

  FILE *from = fopen(input_filepath, "r");
  if (from == NULL) {
    fprintf(stderr, ERR_CANNOT_OPEN_FILE, input_filepath);
    return EXIT_FAILURE;
  }

  FILE *to = fopen(output_filepath, "w");
  if (to == NULL) {
    fprintf(stderr, ERR_CANNOT_OPEN_FILE, output_filepath);
    return EXIT_FAILURE;
  }

  int retval = reverse(from, to);
  fclose(from);
  fclose(to);
  return retval;
}

int main(int argc, char *argv[]) {
  switch (argc) {
  case 1:
    return reverse(stdin, stdout);
  case 2:
    return from_file_to_stdout(argv[1]);
  case 3:
    return from_file_to_file(argv[1], argv[2]);
  default:
    fprintf(stderr, "%s", USAGE);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
