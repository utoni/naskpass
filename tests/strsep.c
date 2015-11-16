#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc, char **argv)
{
  int i = 0;
  char *tok, *p_tok;
  static char **arr;

  int arrsiz = 1000;
  static char *delim;
  static char *str;

  if (argc == 1) {
    printf("automatic test ..\n");
    delim = strdup(" ");
    str = strdup("this is a simple string, which should be extracted to 12 strings");
  } else if (argc != 4) {
    fprintf(stderr, "usage: %s [ARR_SIZ] [DELIM] [STRING]\n", argv[0]);
    exit(1);
  } else {
    arrsiz = atoi(argv[1]);
    delim = strdup(argv[2]);
    str = strdup(argv[3]);
  }

  arr = calloc(arrsiz, sizeof(char *));
  p_tok = str;
  while ( (tok = strsep(&p_tok, delim)) != NULL ) {
    arr[i] = tok;
    i++;
  }

  i = 0;
  while ( arr[i] != NULL ) {
    printf("ARRAY[%d]: %s\n", i, arr[i]);
    i++;
  }

  if (argc == 1) {
    if (i == 12) {
      return 0;
    } else return -1;
  }
  return 0;
}
