#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DB_FILE ".kv_db.txt"

/// a simple linked-list based kv store
typedef struct kvs {
  int key;
  char *val;
  struct kvs *next;
} KVStore;

KVStore *g_kvs_root = NULL;

void trim_ending_newline(char *str) {
  size_t end = strlen(str) - 1;
  while (str[end] == '\n') {
    str[end] = '\0';
    end--;
  }
}

int init_db() {
  FILE *file = fopen(DB_FILE, "r");
  if (file == NULL) {
    if (errno == 2) {
      // file no found in our case is fine since the close_db function will
      // create the file for writing
      return EXIT_SUCCESS;
    }
    perror("error opening db file");
    return EXIT_FAILURE;
  }

  g_kvs_root = malloc(sizeof(KVStore));
  KVStore *kvs = g_kvs_root;
  KVStore *last = NULL;

  char *val;
  size_t val_len;
  int key;
  int count = 0;

  char *line = NULL;
  char *line_start;
  size_t read = 0;
  int retval;
  while ((retval = getline(&line, &read, file)) != 0 && retval != -1) {
    line_start = line;
    // dont need to check for err here because only the kv prog will modify the file
    // and we know that the key is valid
    key = atoi(strsep(&line, ","));
    val = strsep(&line, ",");
    trim_ending_newline(val);

    kvs->key = key;
    val_len = strlen(val) + 1;
    kvs->val = malloc(val_len * sizeof(char));
    strncpy(kvs->val, val, val_len);

    kvs->next = malloc(sizeof(KVStore));
    last = kvs;

    kvs = kvs->next;
    count++;

    free(line_start);
    line = NULL;
    read = 0;
  }
  free(line);
  fclose(file);

  if (count == 0) {
    free(g_kvs_root);
    g_kvs_root = NULL;
    return EXIT_SUCCESS;
  }
  free(last->next);
  last->next = NULL;

  return EXIT_SUCCESS;
}

int close_db() {
  FILE *file = fopen(DB_FILE, "w");
  if (file == NULL) {
    perror("error opening file for writing");
    return EXIT_FAILURE;
  }

  KVStore *kvs = g_kvs_root;
  KVStore *tmp;
  while (kvs != NULL) {
    fprintf(file, "%d,%s\n", kvs->key, kvs->val);
    free(kvs->val);
    tmp = kvs->next;
    free(kvs);
    kvs = tmp;
  }
  fclose(file);
  return EXIT_SUCCESS;
}

int clear() {
  KVStore *kvs = g_kvs_root;
  KVStore *tmp;
  while (kvs != NULL) {
    free(kvs->val);
    tmp = kvs->next;
    free(kvs);
    kvs = tmp;
  }
  g_kvs_root = NULL;
  return EXIT_SUCCESS;
}

int all() {
  KVStore *kvs = g_kvs_root;
  while (kvs != NULL) {
    fprintf(stdout, "%d,%s\n", kvs->key, kvs->val);
    kvs = kvs->next;
  }
  return EXIT_SUCCESS;
}

int get(int key) {
  KVStore *kvs = g_kvs_root;
  while (kvs != NULL) {
    if (kvs->key == key) {
      fprintf(stdout, "%d,%s\n", kvs->key, kvs->val);
      return EXIT_SUCCESS;
    }
    kvs = kvs->next;
  }
  fprintf(stdout, "%d not found\n", key);
  return EXIT_SUCCESS;
}

int delete(int key) {
  if (g_kvs_root == NULL) {
    fprintf(stdout, "%d not found\n", key);
    return EXIT_FAILURE;
  }

  KVStore *prev = g_kvs_root;
  KVStore *kvs = g_kvs_root->next;
  if (prev->key == key) {
    free(prev->val);
    free(prev);
    g_kvs_root = kvs;
    return EXIT_SUCCESS;
  }

  while (kvs != NULL) {
    if (kvs->key == key) {
      free(kvs->val);
      prev->next = kvs->next;
      free(kvs);
      return EXIT_SUCCESS;
    }
    prev = kvs;
    kvs = kvs->next;
  }

  fprintf(stdout, "%d not found\n", key);
  return EXIT_FAILURE;
}

int put(int key, char *val) {
  size_t val_len = strlen(val) + 1;
  if (g_kvs_root == NULL) {
    g_kvs_root = malloc(sizeof(KVStore));
    g_kvs_root->key = key;
    g_kvs_root->val = malloc(val_len * sizeof(char));
    strncpy(g_kvs_root->val, val, val_len);
    g_kvs_root->next = NULL;
    return EXIT_SUCCESS;
  }

  KVStore *kvs = g_kvs_root;
  while (true) {
    if (kvs->key == key) {
      free(kvs->val);
      kvs->val = malloc(val_len * sizeof(char));
      strncpy(kvs->val, val, val_len);
      return EXIT_SUCCESS;
    }

    if (kvs->next == NULL) {
      kvs->next = malloc(sizeof(KVStore));
      kvs->next->key = key;
      kvs->next->val = malloc(val_len * sizeof(char));
      strncpy(kvs->next->val, val, val_len);
      kvs->next->next = NULL;
      return EXIT_SUCCESS;
    }
    kvs = kvs->next;
  }
}

bool is_valid_command(char *cmd) {
  return strlen(cmd) == 1 && strstr("acgdp", cmd) != NULL;
}

int run(char *cmd) {
  int count = 0;
  char *tok;

  char *separated_cmd[3];
  while ((tok = strsep(&cmd, ",")) != NULL) {
    if (count == 3) {
      fprintf(stdout, "bad command, too many subtokens\n");
      return EXIT_FAILURE;
    }

    separated_cmd[count] = tok;
    count++;
  }

  int key;
  if (count > 1) {
    char *endptr = NULL;
    key = strtol(separated_cmd[1], &endptr, 10);
    if (*endptr != '\0') {
      fprintf(stdout, "bad command, invalid key\n");
      return EXIT_FAILURE;
    }
  }

  if (!is_valid_command(separated_cmd[0])) {
    fprintf(stdout, "bad command\n");
    return EXIT_FAILURE;
  }

  char cmd_char = separated_cmd[0][0];
  switch (count) {
  case 0:
    return EXIT_SUCCESS;
  case 1:
    if (cmd_char == 'c') {
      return clear();
    } else if (cmd_char == 'a') {
      return all();
    }
    break;
  case 2:
    if (cmd_char == 'g') {
      return get(key);
    } else if (cmd_char == 'd') {
      return delete (key);
    }
    break;
  case 3:
    if (cmd_char == 'p') {
      return put(key, separated_cmd[2]);
    }
    break;
  }
  fprintf(stdout, "bad command\n");
  return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
  if (init_db() != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  for (int i = 1; i < argc; i++) {
    if (run(argv[i]) != EXIT_SUCCESS) {
      close_db();
      return EXIT_FAILURE;
    }
  }
  return close_db();
}
