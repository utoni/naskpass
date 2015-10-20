#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc, char **argv)
{
  int i = 0;
  char *tok, *p_tok;
  char **arr;

  if (argc != 4) {
    fprintf(stderr, "usage: %s [ARR_SIZ] [DELIM] [STRING]\n", argv[0]);
    exit(1);
  }

  arr = calloc(atoi(argv[1]), sizeof(char *));
  p_tok = argv[3];
  while ( (tok = strsep(&p_tok, argv[2])) != NULL ) {
    arr[i] = tok;
    i++;
  }

  i = 0;
  while ( arr[i] != NULL ) {
    printf("ARRAY[%d]: %s\n", i, arr[i]);
    i++;
  }
  return 0;
}
