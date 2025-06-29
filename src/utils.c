#include "agenda.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int parse_date_input(const char* input, char* output_date) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    struct tm tm_target = *tm_now;

    if (strcmp(input, "today") == 0) {
        // Use current date
    } else if (strcmp(input, "tomorrow") == 0) {
        tm_target.tm_mday += 1;
        mktime(&tm_target); // Normalize the date
    } else {
        // Parse DD/MM/YYYY format
        int day, month, year;
        if (sscanf(input, "%d/%d/%d", &day, &month, &year) != 3) {
            return -1;
        }
        tm_target.tm_mday = day;
        tm_target.tm_mon = month - 1; // months are 0-based
        tm_target.tm_year = year - 1900; // years since 1900
    }

    strftime(output_date, MAX_DATE_LEN, "%Y-%m-%d", &tm_target);
    return 0;
}

int parse_time_input(const char* input, char* output_time) {
    int hour, minute, second = 0;
    
    // Try HH:MM:SS format first
    if (sscanf(input, "%d:%d:%d", &hour, &minute, &second) == 3) {
        if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59) {
            snprintf(output_time, MAX_TIME_LEN, "%02d:%02d:%02d", hour, minute, second);
            return 0;
        }
    }
    
    // Try HH:MM format
    if (sscanf(input, "%d:%d", &hour, &minute) == 2) {
        if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
            snprintf(output_time, MAX_TIME_LEN, "%02d:%02d:00", hour, minute);
            return 0;
        }
    }
    
    return -1;
}

time_t combine_datetime(const char* date, const char* time) {
    struct tm tm_datetime = {0};
    
    // Parse date (YYYY-MM-DD)
    if (sscanf(date, "%d-%d-%d", &tm_datetime.tm_year, &tm_datetime.tm_mon, &tm_datetime.tm_mday) != 3) {
        return -1;
    }
    tm_datetime.tm_year -= 1900; // years since 1900
    tm_datetime.tm_mon -= 1;     // months are 0-based
    
    // Parse time (HH:MM:SS)
    if (sscanf(time, "%d:%d:%d", &tm_datetime.tm_hour, &tm_datetime.tm_min, &tm_datetime.tm_sec) != 3) {
        return -1;
    }
    
    return mktime(&tm_datetime);
}

void format_date_for_display(const char* date, char* output) {
    struct tm tm_date = {0};
    if (sscanf(date, "%d-%d-%d", &tm_date.tm_year, &tm_date.tm_mon, &tm_date.tm_mday) == 3) {
        tm_date.tm_year -= 1900;
        tm_date.tm_mon -= 1;
        strftime(output, 32, "%A, %B %d, %Y", &tm_date);
    } else {
        strcpy(output, date);
    }
}

void format_time_for_display(const char* time, char* output) {
    struct tm tm_time = {0};
    if (sscanf(time, "%d:%d:%d", &tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec) == 3) {
        strftime(output, 16, "%I:%M %p", &tm_time);
    } else {
        strcpy(output, time);
    }
}

int is_same_day(time_t t1, time_t t2) {
    struct tm* tm1 = localtime(&t1);
    struct tm* tm2 = localtime(&t2);
    
    return (tm1->tm_year == tm2->tm_year &&
            tm1->tm_mon == tm2->tm_mon &&
            tm1->tm_mday == tm2->tm_mday);
}

int is_same_week(time_t t1, time_t t2) {
    struct tm* tm1 = localtime(&t1);
    struct tm* tm2 = localtime(&t2);
    
    // Calculate week number
    int week1 = tm1->tm_yday / 7;
    int week2 = tm2->tm_yday / 7;
    
    return (tm1->tm_year == tm2->tm_year && week1 == week2);
}

int is_same_month(time_t t1, time_t t2) {
    struct tm* tm1 = localtime(&t1);
    struct tm* tm2 = localtime(&t2);
    
    return (tm1->tm_year == tm2->tm_year && tm1->tm_mon == tm2->tm_mon);
}

int server_is_running(void) {
    // Try to connect to the server port to check if it's running
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return 0;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    
    return (result == 0) ? 1 : 0;
}
