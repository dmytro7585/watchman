#define _XOPEN_SOURCE 700  // Для POSIX функцій

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

// Структура для збереження опцій програми
struct program_options 
{
    bool show_seconds;
    bool show_date;
    bool show_timezone;
    bool show_month;
    bool show_weekday;
    bool am_pm_format;
    bool show_utc;
};

// Глобальна змінна для керування циклом виконання програми
volatile sig_atomic_t running = 1;

// Оголошення функцій
static void show_time(const struct tm *tim, const struct program_options *opts);
static void handle_signal(int signal);
static void set_nonblocking_mode(int enable);
static void print_help(void);

int main(int argc, char *argv[]) 
{
    struct program_options options = {false, false, false, false, false, false, false};
    int opt;

    // Обробка аргументів командного рядка
    while ((opt = getopt(argc, argv, "hsdmtuwa")) != -1) 
    {
        switch (opt) {
            case 'h':
                print_help();
                return 0;
            case 's':
                options.show_seconds = true;
                break;
            case 'd':
                options.show_date = true;
                break;
            case 't':
                options.show_timezone = true;
                break;
            case 'm':
                options.show_month = true;
                break;
            case 'u':
                options.show_utc = true;
                break;
            case 'w':
                options.show_weekday = true;
                break;
            case 'a':
                options.am_pm_format = true;
                break;
            default:
                print_help();
                return 1;
        }
    }

    // Налаштування обробників сигналів
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGQUIT, handle_signal);

    // Увімкнення неблокуючого режиму вводу
    set_nonblocking_mode(1);

    // Основний цикл для оновлення часу
    while (running) 
    {
        time_t raw_time;
        struct tm *tim_info;

        // Отримання поточного часу
        time(&raw_time);
        if (options.show_utc) {
            tim_info = gmtime(&raw_time);
        } else {
            tim_info = localtime(&raw_time);
        }

        // Вивід часу
        show_time(tim_info, &options);

        // Перевірка на натискання клавіші 'q' для виходу
        char ch = getchar();
        if (ch == 'q') 
        {
            break;
        }

        // Оновлення часу щосекунди
        sleep(1);
    }

    // Повернення терміналу до звичайного режиму і очищення екрану
    set_nonblocking_mode(0);
    printf("\033[2J\033[H");  // Очищення екрану і переміщення курсора на початок
    return 0;
}

// Функція для відображення часу
static void show_time(const struct tm *tim, const struct program_options *opts) 
{
    // Очищення попереднього виводу
    printf("\r");

    // Вибір формату часу
    if (opts->am_pm_format) 
    {
        // 12-годинний формат з AM/PM
        int hour = tim->tm_hour % 12;

        if (hour == 0) hour = 12;  // 12 години в AM/PM форматі

        if (opts->show_seconds) 
        {
            printf("[%02d:%02d:%02d", hour, tim->tm_min, tim->tm_sec);
        } 
        else 
        {
            printf("[%02d:%02d", hour, tim->tm_min);
        }
        printf(tim->tm_hour >= 12 ? " PM]" : " AM]");
    } 
    else 
    {        
        // 24-годинний формат
        if (opts->show_seconds) 
        {
            printf("[%02d:%02d:%02d]", tim->tm_hour, tim->tm_min, tim->tm_sec);
        } 
        else 
        {
            printf("[%02d:%02d]", tim->tm_hour, tim->tm_min);
        }
    }

    // Виведення дати, місяця, дня тижня та часового поясу
    if (opts->show_weekday) 
    {
        char *weekday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        printf("[%s]", weekday[tim->tm_wday]);
    }

    if (opts->show_month) 
    {
        char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        printf("[%s]", month[tim->tm_mon]);
    }

    if (opts->show_date) 
    {
        printf("[%04d-%02d-%02d]", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday);
    }

    if (opts->show_timezone)
    {
        extern char *tzname[2];
        char *timezone = tzname[tim->tm_isdst > 0];
        printf("[%s]", timezone);
    }

    // Оновлення буферу виводу
    fflush(stdout);
}

// Функція обробки сигналів
static void handle_signal(int signal) 
{
    if (signal == SIGINT || signal == SIGTERM || signal == SIGQUIT) 
    {
        running = 0;  // Завершення циклу
    }
}

// Функція для встановлення неблокуючого режиму для stdin
static void set_nonblocking_mode(int enable) 
{
    struct termios tattr;
    tcgetattr(STDIN_FILENO, &tattr);

    if (enable) 
    {
        // Увімкнути неблокуючий режим
        tattr.c_lflag &= ~(ICANON | ECHO);  // Вимикаємо канонічний режим і ехо
        fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
    }

    else 
    {
        // Відновити стандартний режим
        tattr.c_lflag |= (ICANON | ECHO);
        fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

// Функція для виводу допомоги
static void print_help(void) 
{
    printf("Usage: time_display [options]\n");
    printf("Options:\n");
    printf("  -h          Show this help message\n");
    printf("  -s          Show seconds\n");
    printf("  -d          Show date (year-month-day)\n");
    printf("  -t          Show timezone\n");
    printf("  -m          Show month name\n");
    printf("  -u          Show time in UTC\n");
    printf("  -w          Show weekday name\n");
    printf("  -a          Use AM/PM format\n");
}
