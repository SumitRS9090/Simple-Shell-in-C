#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int myshell_cd(char **args);
int myshell_help(char **args);
int myshell_quit(char **args);

char *cmd_list[] = {
  "cd",
  "help",
  "quit"
};

int (*cmd_funcs[]) (char **) = {
  &myshell_cd,
  &myshell_help,
  &myshell_quit
};

int num_cmds() {
  return sizeof(cmd_list) / sizeof(char *);
}

int myshell_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "myshell: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("myshell");
    }
  }
  return 1;
}

int myshell_help(char **args) {
  int i;
  printf("Custom Shell\n");
  printf("Type program names and arguments, and press enter.\n");
  printf("These commands are built-in:\n");

  for (i = 0; i < num_cmds(); i++) {
    printf("  %s\n", cmd_list[i]);
  }

  printf("Use man for more info on external commands.\n");
  return 1;
}

int myshell_quit(char **args) {
  return 0;
}

int myshell_run(char **args) {
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("myshell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("myshell");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int myshell_exec(char **args) {
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < num_cmds(); i++) {
    if (strcmp(args[0], cmd_list[i]) == 0) {
      return (*cmd_funcs[i])(args);
    }
  }

  return myshell_run(args);
}

char *myshell_getline(void) {
#define MYSH_BUFF_SIZE 1024
  int bufsize = MYSH_BUFF_SIZE;
  int pos = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[pos] = '\0';
      return buffer;
    } else {
      buffer[pos] = c;
    }
    pos++;

    if (pos >= bufsize) {
      bufsize += MYSH_BUFF_SIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define MYSH_TOK_BUFF 64
#define MYSH_TOK_DELIM " \t\r\n\a"

char **myshell_tokenize(char *line) {
  int bufsize = MYSH_TOK_BUFF, pos = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **backup;

  if (!tokens) {
    fprintf(stderr, "myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, MYSH_TOK_DELIM);
  while (token != NULL) {
    tokens[pos] = token;
    pos++;

    if (pos >= bufsize) {
      bufsize += MYSH_TOK_BUFF;
      backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        free(backup);
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, MYSH_TOK_DELIM);
  }
  tokens[pos] = NULL;
  return tokens;
}

void myshell_loop(void) {
  char *line;
  char **args;
  int running;

  do {
    printf("$ ");
    line = myshell_getline();
    args = myshell_tokenize(line);
    running = myshell_exec(args);

    free(line);
    free(args);
  } while (running);
}

int main(int argc, char **argv) {
  myshell_loop();
  return EXIT_SUCCESS;
}
