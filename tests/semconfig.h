#ifndef CONFIG_H
#define CONFIG_H 1

#define LOG(text) printf("%s\n", text);
#define CMD(cmd) LOG(cmd); cmd;
#define TESTSEM "/testsem"
#define CNTSEM "/testcnt"

#endif
