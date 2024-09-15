#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

static struct watchman_s
{
    time_t raw_time;
    struct tm *tim_info;
    struct
    {
        bool show_seconds;
        bool show_date;
        bool show_timezone;
        bool show_month;
        bool show_weekday;
        bool am_pm_format;
        bool show_utc;
        bool blink;
    } options;
    volatile bool running;
    bool dots_on_off; 
} watchman_s;

static void show_time(const struct tm *tim);
static void handle_signal(int signal);
static void set_nonblocking_mode(bool enable);
static void print_help(void);

int main(int argc, char *argv[]) 
{
    memset(&watchman_s, 0, sizeof(watchman_s));
    watchman_s.running = true;
    int opt;

    while ((opt = getopt(argc, argv, "hsdmtuwab")) != -1) 
    {
        switch (opt) 
        {
            case 'h':
            default:
                print_help();
                exit(EXIT_SUCCESS);
            case 's':
                watchman_s.options.show_seconds = true;
                break;
            case 'd':
                watchman_s.options.show_date = true;
                break;
            case 't':
                watchman_s.options.show_timezone = true;
                break;
            case 'm':
                watchman_s.options.show_month = true;
                break;
            case 'u':
                watchman_s.options.show_utc = true;
                break;
            case 'w':
                watchman_s.options.show_weekday = true;
                break;
            case 'a':
                watchman_s.options.am_pm_format = true;
                break;
            case 'b':
                watchman_s.options.blink = true;
                break;
        }
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGQUIT, handle_signal);

    set_nonblocking_mode(true);

    while (watchman_s.running) 
    {
        time(&watchman_s.raw_time);
        if (watchman_s.options.show_utc) 
        {
            watchman_s.tim_info = gmtime(&watchman_s.raw_time);
        } 
        else 
        {
            watchman_s.tim_info = localtime(&watchman_s.raw_time);
        }

        show_time(watchman_s.tim_info);

        char ch = getchar();
        if (ch == 'q') 
        {
            break;
        }
        
        usleep(500000);
    }

    set_nonblocking_mode(false);
    printf("\r\033[K");
    exit(EXIT_SUCCESS);
}

static void show_time(const struct tm *tim) 
{
    printf("\r");

    watchman_s.dots_on_off = !watchman_s.dots_on_off;
    if (watchman_s.options.am_pm_format) 
    {
        int hour = tim->tm_hour % 12;
        if (hour == 0) hour = 12;

        if (watchman_s.options.show_seconds) 
        {
            printf(watchman_s.options.blink && watchman_s.dots_on_off ? "[%02d %02d %02d" : "[%02d:%02d:%02d", hour, tim->tm_min, tim->tm_sec);
        } 
        else 
        {
           printf(watchman_s.options.blink && watchman_s.dots_on_off ? "[%02d %02d" : "[%02d:%02d", hour, tim->tm_min);
        }
        printf(tim->tm_hour >= 12 ? " PM]" : " AM]");
    } 
    else 
    {
        if (watchman_s.options.show_seconds) 
        {
            printf(watchman_s.options.blink && watchman_s.dots_on_off ? "[%02d %02d %02d]" : "[%02d:%02d:%02d]", tim->tm_hour, tim->tm_min, tim->tm_sec);
        } 
        else 
        {
           printf(watchman_s.options.blink && watchman_s.dots_on_off ? "[%02d %02d]" : "[%02d:%02d]", tim->tm_hour, tim->tm_min);
        }
    }

    if (watchman_s.options.show_weekday) 
    {
        static const char *const weekday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        printf("[%s]", weekday[tim->tm_wday]);
    }

    if (watchman_s.options.show_month) 
    {
        static const char *const month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        printf("[%s]", month[tim->tm_mon]);
    }

    if (watchman_s.options.show_date) 
    {
        printf("[%04d-%02d-%02d]", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday);
    }

    if (watchman_s.options.show_timezone) 
    {
        extern char *tzname[2];
        char * timezone = watchman_s.options.show_utc ? "UTC" : tzname[tim->tm_isdst > 0];

        printf("[%s]", timezone);
    }

    fflush(stdout);
}

static void handle_signal(int signal) 
{
    if (signal == SIGINT || signal == SIGTERM || signal == SIGQUIT) 
    {
        watchman_s.running = false;
    }
}

static void set_nonblocking_mode(bool enable) 
{
    struct termios tattr;
    tcgetattr(STDIN_FILENO, &tattr);

    if (enable) 
    {
        tattr.c_lflag &= ~(ICANON | ECHO);
        fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
    } 
    else 
    {
        tattr.c_lflag |= (ICANON | ECHO);
        fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

static void print_help(void) 
{
    printf
    (
        "Usage: ./watchman [-hsdtmuwab]\n"
        "Options:\n"
        "  -h          Show this help message\n"
        "  -s          Show seconds\n"
        "  -d          Show date (year-month-day)\n"
        "  -t          Show timezone\n"
        "  -m          Show month name\n"
        "  -u          Show time in UTC\n"
        "  -w          Show weekday name\n"
        "  -a          Use AM/PM format\n"
        "  -b          Enable dots blinking\n"
    );
}
