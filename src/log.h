#ifndef _LOG_H_
#define _LOG_H_

/* Public: Construct a string for the given format and args and output it on
 *      whatever debug interface the current platform is using. This function
 *      could be implemented in multiple ways - UART, regular printf, etc. The
 *      actual effect depends on the platform.
 *
 *      This has to be a macro because we can't unpack va_list args twice (e.g.
 *      once in debug() and again in debug_no_newline(). An alternative would be
 *      to duplicate code in debug() and debug_no_newline().
 *
 * format - A printf-style format string.
 * args - printf-style arguments that match the format string.
 */
#define debug(...) debug_no_newline(__VA_ARGS__); debug_no_newline("\r\n");

extern const int MAX_LOG_LINE_LENGTH;

/* Public: Initialize the debug logging framework. This function must be called
 *      before using debug().
 */
void initialize_logging();

/* Public: Like debug() but doesn't add a newline to the end of the message.
 */
void debug_no_newline(const char* format, ...);

#endif // _LOG_H_
