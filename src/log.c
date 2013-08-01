#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "debug_frmwrk.h"

const int MAX_LOG_LINE_LENGTH = 120;

void debug_no_newline(const char* format, ...) {
#ifdef __DEBUG__
    va_list args;
    va_start(args, format);
    char buffer[MAX_LOG_LINE_LENGTH];
    vsnprintf(buffer, MAX_LOG_LINE_LENGTH, format, args);

    _printf(buffer);
    va_end(args);
#endif // __DEBUG__
}

void initialize_logging() {
    debug_frmwrk_init();
}
