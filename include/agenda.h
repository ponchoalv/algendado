#ifndef AGENDA_H
#define AGENDA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <pthread.h>
#include <microhttpd.h>
#include <raylib.h>

// Configuration
#define SERVER_PORT 8080
#define DB_PATH "agenda.db"
#define MAX_DESCRIPTION_LEN 256
#define MAX_DATE_LEN 32
#define MAX_TIME_LEN 16
#define NOTIFICATION_ADVANCE_MINUTES 15

// Structures
typedef struct {
    int id;
    char date[MAX_DATE_LEN];     // YYYY-MM-DD format
    char time[MAX_TIME_LEN];     // HH:MM:SS format
    char description[MAX_DESCRIPTION_LEN];
    time_t datetime;             // Unix timestamp for easy comparison
    int notified;                // 0 = not notified, 1 = notified
} agenda_item_t;

typedef enum {
    VIEW_TODAY,
    VIEW_WEEK,
    VIEW_MONTH
} view_type_t;

// Function prototypes

// Database functions
int db_init(void);
int db_add_item(const char* date, const char* time, const char* description);
int db_get_items(view_type_t view, agenda_item_t** items, int* count);
int db_get_pending_notifications(agenda_item_t** items, int* count);
int db_mark_notified(int id);
void db_close(void);

// Server functions
int server_start(void);
void server_stop(void);
int server_is_running(void);

// Client functions
int parse_date_input(const char* input, char* output_date);
int parse_time_input(const char* input, char* output_time);
time_t combine_datetime(const char* date, const char* time);

// Notification functions
int send_notification(const char* title, const char* message);
int show_visual_notification(const char* title, const char* message, const char* time_str);
void* notification_thread(void* arg);

// Web interface functions
enum MHD_Result handle_web_request(void* cls, struct MHD_Connection* connection,
                      const char* url, const char* method,
                      const char* version, const char* upload_data,
                      size_t* upload_data_size, void** con_cls);

// Calendar functions
char* generate_ics_calendar(void);
char* generate_html_calendar(void);

// Utility functions
void format_date_for_display(const char* date, char* output);
void format_time_for_display(const char* time, char* output);
int is_same_day(time_t t1, time_t t2);
int is_same_week(time_t t1, time_t t2);
int is_same_month(time_t t1, time_t t2);

#endif // AGENDA_H
