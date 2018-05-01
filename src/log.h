#ifndef LOG_H
#define LOG_H

int log_init(const char *filename);
void log_close();

void log_error(const char *id, const char *msg, ...);
void log_errno(const char *id, const char *msg, ...);
void log_debug(const char *id, const char *msg, ...);

#endif
