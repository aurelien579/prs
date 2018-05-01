#include "log.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

static FILE *log_file = NULL;

static void _log(const char *level, const char *id, const char *msg, va_list args)
{
    FILE *out = log_file;
    if (!out) out = stdout;

    char date_buf[32];
    time_t rawtime;
    struct tm *timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    strftime(date_buf, sizeof(date_buf), "%H:%M:%S", timeinfo);
    
    fprintf(out, "[%s] %s [%s] ", level, date_buf, id);
    vfprintf(out, msg, args);
    fprintf(out, "\n");
}

int log_init(const char *filename)
{
    log_file = fopen(filename, "a");
    if (log_file == NULL) return -1;
    
    return 0;
}

void log_close()
{
    if (log_file) {
        fclose(log_file);
    }
}

void log_error(const char *id, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    _log("ERROR", id, msg, args);

    va_end(args);
}

void log_errno(const char *id, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    char real_msg[512];
    snprintf(real_msg, 512, "%s: %s", strerror(errno), msg);

    _log("ERROR", id, real_msg, args);

    va_end(args);
}

void log_debug(const char *id, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    _log("DEBUG", id, msg, args);

    va_end(args);
}
