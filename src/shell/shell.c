#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>


static const size_t maxTries = 5;

int main(int argc, char **argv) {
  struct termios old, new;

  int passfifo_fd = open("/lib/cryptsetup/passfifo", O_WRONLY);
  if (passfifo_fd < 0) {
    perror("open");
    return -1;
  }

  if ( tcgetattr(0, &old) != 0 )
    return -1;
  new = old;
  new.c_lflag &= ~ECHO;

  if ( tcsetattr(0, TCSANOW, &new) != 0 )
    return -1;

  size_t tries = 0;
  while (tries++ < maxTries) {
    printf("[%u/%u] Enter a passphrase: ", (unsigned int)tries, (unsigned int)maxTries);
    char *line = calloc(128, sizeof(char));
    size_t len = 128;
    ssize_t read = getline(&line, &len, stdin);
    if (write(passfifo_fd, line, len) == -1)
      perror("write");
    printf("\n");
    sleep(3);
  }

  if ( tcsetattr(0, TCSANOW, &old) != 0 )
    return -1;

  return 0;
}
