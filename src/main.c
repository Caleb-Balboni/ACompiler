#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "tokenizer/tokenizer.h"
#include "tokenizer/tokens.h"
#include "utils/arraylist.h"
#include "utils/hashtable.h"
#include "parser/parser.h"
#include "assembler/assembler.h"

unsigned long getFileCharCount(FILE* file) {
  char c;
  unsigned long count = 0;
  while ((c = fgetc(file)) != EOF) {
    count += 1;
  }
  fseek(file, 0, SEEK_SET);
  return count;
}

typedef enum {
  MODE_EXECUTABLE,
  MODE_ASM_ONLY,
} compile_mode_t;

typedef struct {
  compile_mode_t mode;
  const char* input;
  const char* output;
} cli_args_t;

static void usage(const char* prog) {
  fprintf(stderr,
    "usage: %s [-S] [-o <output>] <file.av>\n"
    "  default: assemble and link to an executable (a.out)\n"
    "  -S:      stop after emitting assembly (.s)\n"
    "  -o:      override output path\n",
    prog);
}

static int parse_cli_args(int argc, char* argv[], cli_args_t* out) {
  out->mode = MODE_EXECUTABLE;
  out->input = NULL;
  out->output = NULL;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-S") == 0) {
      out->mode = MODE_ASM_ONLY;
    } else if (strcmp(argv[i], "-o") == 0) {
      if (i + 1 >= argc) return -1;
      out->output = argv[++i];
    } else if (argv[i][0] == '-') {
      return -1;
    } else {
      if (out->input != NULL) return -1;
      out->input = argv[i];
    }
  }
  if (out->input == NULL) return -1;
  return 0;
}

static char* swap_extension(const char* input, const char* new_ext) {
  size_t len = strlen(input);
  const char* dot = strrchr(input, '.');
  size_t base = dot ? (size_t)(dot - input) : len;
  size_t new_len = base + strlen(new_ext) + 1;
  char* out = malloc(new_len);
  memcpy(out, input, base);
  strcpy(out + base, new_ext);
  return out;
}

static int run_assembler_and_linker(const char* asm_path, const char* exe_path) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return -1;
  }
  if (pid == 0) {
    execlp("gcc", "gcc", asm_path, "-o", exe_path, (char*)NULL);
    perror("execlp gcc");
    _exit(127);
  }
  int status;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    return -1;
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    fprintf(stderr, "gcc failed to assemble/link %s\n", asm_path);
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  cli_args_t args;
  if (parse_cli_args(argc, argv, &args) != 0) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  FILE* source_file = fopen(args.input, "r");
  if (source_file == NULL) {
    fprintf(stderr, "could not open input file: %s\n", args.input);
    return EXIT_FAILURE;
  }

  char* asm_path_owned = NULL;
  const char* asm_path = NULL;
  const char* exe_path = NULL;

  if (args.mode == MODE_ASM_ONLY) {
    if (args.output) {
      asm_path = args.output;
    } else {
      asm_path_owned = swap_extension(args.input, ".s");
      asm_path = asm_path_owned;
    }
  } else {
    asm_path_owned = swap_extension(args.input, ".s");
    asm_path = asm_path_owned;
    exe_path = args.output ? args.output : "a.out";
  }

  unsigned int char_count = getFileCharCount(source_file);
  printf("welcome to ACompiler\n");
  ArrayList* array = tokenize(source_file, char_count);
  for (int i = 0; i < array->length; i++) {
    Token* temp = (Token*)get_list(array, i);
    printf("[%d] TOKEN: type=%d, lexeme='%s'\n", i, temp->type, temp->lexeme);
  }
  Node* head = parse_program(array);
  print_ast(head);
  asm_ctx* ctx = asm_init(asm_path);
  gen_program(ctx, head);
  asm_free(ctx);
  printf("wrote assembly to %s\n", asm_path);
  free_node(head);
  destroy_list(array);
  fclose(source_file);

  int rc = EXIT_SUCCESS;
  if (args.mode == MODE_EXECUTABLE) {
    if (run_assembler_and_linker(asm_path, exe_path) != 0) {
      rc = EXIT_FAILURE;
    } else {
      printf("wrote executable to %s\n", exe_path);
    }
  }

  free(asm_path_owned);
  return rc;
}
