#include "color_print.h"

#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

const char *lv_format[] = {
        "%s\n",
        LIGHT_CYAN_TEXT("%s") "\n",
        GREEN_TEXT("%s") "\n",
        LIGHT_YELLOW_TEXT("%s") "\n",
        RED_TEXT("%s") "\n",
};


static char out_put_str[500];
static unsigned int old_tick = 0;
static unsigned int current_tick;

struct timeval tv;

void color_print(uint8_t level, const char *file, int line, const char *func, const char *format, ...) {
    (void)func;
    current_tick = sysTick;
    memset(out_put_str, 0, 500);
    printf(DEFAULT_LOG_FORMAT,
           current_tick/1000,
           current_tick%1000,
           current_tick - old_tick,
           file,
           line
    );
    va_list args;
    va_start(args, format);
    vsprintf(out_put_str, format, args);
    va_end(args);

#if 1
    //带颜色打印
    printf(lv_format[level], out_put_str);
#else
    //不带颜色打印
    printf("%s\n", out_put_str);
#endif
    old_tick = current_tick;
}
