#ifndef OPT_H
#define OPT_H 1

#define OPT(opt_index) config_opts[opt_index]
#define SETOPT_str(opt_index, value) { OPT(opt_index).found = 1; OPT(opt_index).opt.str = value; }
#define GETOPT(opt_index) (OPT(opt_index).found != 0 ? OPT(opt_index).opt : OPT(opt_index).def)


union opt_entry {
  char *str;
  int dec;
};

struct opt {
  union opt_entry opt;
  unsigned char found;
  const union opt_entry def;
};

enum opt_index {
  FIFO_PATH = 0,
  CRYPT_CMD,
  LOG_FILE
};


extern struct opt config_opts[];

void
usage(char *arg0);

int
parse_cmd(int argc, char **argv);

#endif
