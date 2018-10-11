#ifndef LOG_H
#define LOG_H 1

#ifdef DEBUG
#define logs_dbg(fmt, ...) logs(fmt, __VA_ARGS__)
#else
#define logs_dbg(fmt, ...)
#endif

int log_init(char* file);

void log_free(void);

int logs(const char* format, ...);

#endif
