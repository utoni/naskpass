#ifndef LOG_H
#define LOG_H 1

int log_init(char* file);

void log_free(void);

int logs(char* format, ...);

#endif
