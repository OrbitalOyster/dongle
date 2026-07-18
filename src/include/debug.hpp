#ifndef DONGLE_DEBUG
#define DONGLE_DEBUG

#define MODE_START "\x1b["
#define MODE_RESET "\x1b[0m"

#define MODE_NORMAL ";0"
#define MODE_BOLD ";1"
#define MODE_DIM ";2"
#define MODE_ITALIC ";3"
#define MODE_UNDERSCORE ";4"
#define MODE_BLINK ";5"
#define MODE_REVERSE ";7"

#define COLOR_BLACK ";30"
#define COLOR_RED ";31"
#define COLOR_GREEN ";32"
#define COLOR_YELLOW ";33"
#define COLOR_BLUE ";34"
#define COLOR_PURPLE ";35"
#define COLOR_CYAN ";36"
#define COLOR_GRAY ";37"

#define COLOR_DEFAULT ";39"

#define COLOR_BRIGHT_BLACK ";90"
#define COLOR_BRIGHT_RED ";91"
#define COLOR_BRIGHT_GREEN ";92"
#define COLOR_BRIGHT_YELLOW ";93"
#define COLOR_BRIGHT_BLUE ";94"
#define COLOR_BRIGHT_PURPLE ";95"
#define COLOR_BRIGHT_CYAN ";96"
#define COLOR_WHITE ";97"

#define TMSG(mode, color, s) MODE_START mode color "m" s MODE_RESET

#define MAX_MSG_SIZE 1024

#define INFO(debug_message)                                                    \
  fprintf(stdout,                                                              \
          TMSG(MODE_BOLD, COLOR_DEFAULT, "[%s:%i] ")                           \
              TMSG(MODE_NORMAL, COLOR_BLUE, "%s") "\n",                        \
          __FILE__, __LINE__, debug_message);

#define INFOF(debug_message, ...)                                              \
  {                                                                            \
    char _debug_b[MAX_MSG_SIZE];                                               \
    snprintf(_debug_b, MAX_MSG_SIZE, debug_message, __VA_ARGS__);              \
    fprintf(stdout,                                                            \
            MODE_START MODE_BOLD COLOR_DEFAULT                                 \
            "m[" __FILE__ ":%i] " MODE_RESET MODE_START MODE_NORMAL COLOR_BLUE \
            "m%s" MODE_RESET "\n",                                             \
            __LINE__, _debug_b);                                               \
  }

#endif
