#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "opt.h"

#define CONFIG_OPT(default_val) { {0},0,{default_val} }

struct opt config_opts[] =  { CONFIG_OPT(DEFAULT_FIFO), CONFIG_OPT(NULL), CONFIG_OPT(NULL) };
const int opt_siz = ( sizeof(config_opts)/sizeof(config_opts[0]) );


void
usage(char *arg0)
{
  fprintf(stderr, "\n%s (%s)\n  %s\n", PKGNAME, VERSION, PKGDESC);
  fprintf(stderr, "  Written by %s (%s).\n", AUTHOR, AUTHOR_EMAIL);
  fprintf(stderr, "  License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\n");
  fprintf(stderr, "  Command:\n\t%s [args]\n", arg0);
  fprintf(stderr, "  Arguments:\n"
                  "\t-h this\n"
                  "\t-l [logfile]\n"
                  "\t-f [passfifo] default: %s\n"
                  "\t-c [cryptcreate]\n", GETOPT(FIFO_PATH).str);
}

int
parse_cmd(int argc, char **argv)
{
  int opt;

  while ((opt = getopt(argc, argv, "hf:c:l::")) != -1) {
    switch (opt) {
      case 'h':
        usage(argv[0]);
        return 1;
      case 'f':
        if (optarg)
          SETOPT_str(FIFO_PATH, strdup(optarg));
        break;
      case 'c':
        if (optarg)
          SETOPT_str(CRYPT_CMD, strdup(optarg));
        break;
      case 'l':
        if (optarg) {
          SETOPT_str(LOG_FILE, strdup(optarg));
        } else SETOPT_str(LOG_FILE, NULL);
        break;
      default:
        usage(argv[0]);
        return 1;
    }
  }
  return 0;
}

