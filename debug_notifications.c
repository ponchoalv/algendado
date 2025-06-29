#include "agenda.h"

void print_timestamp(time_t t) {
    struct tm* tm_info = localtime(&t);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s", buffer);
}

int main() {
    printf("=== Agenda Debug Tool ===\n\n");
    
    if (db_init() != 0) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }
    
    // Get all items to see what's in the database
    agenda_item_t* items;
    int count;
    
    printf("1. All items in database:\n");
    if (db_get_items(VIEW_MONTH, &items, &count) == 0) {
        for (int i = 0; i < count; i++) {
            printf("   ID: %d, Date: %s, Time: %s, Description: %s\n", 
                   items[i].id, items[i].date, items[i].time, items[i].description);
            printf("   Datetime: %ld (", items[i].datetime);
            print_timestamp(items[i].datetime);
            printf("), Notified: %d\n\n", items[i].notified);
        }
        free(items);
    }
    
    // Check current time
    time_t now = time(NULL);
    printf("2. Current time: %ld (", now);
    print_timestamp(now);
    printf(")\n\n");
    
    // Check notification window (using the correct logic from database.c)
    time_t target_time = now + (NOTIFICATION_ADVANCE_MINUTES * 60);
    time_t notification_start = target_time - 30; // 30 seconds before the 15-minute mark
    time_t notification_end = target_time + 30;   // 30 seconds after the 15-minute mark
    
    printf("3. Notification window:\n");
    printf("   Start: %ld (", notification_start);
    print_timestamp(notification_start);
    printf(")\n   End:   %ld (", notification_end);
    print_timestamp(notification_end);
    printf(")\n\n");
    
    // Check for pending notifications
    printf("4. Pending notifications:\n");
    if (db_get_pending_notifications(&items, &count) == 0) {
        printf("   Found %d pending notifications\n", count);
        for (int i = 0; i < count; i++) {
            printf("   - %s at %s: %s\n", items[i].date, items[i].time, items[i].description);
        }
        if (items) free(items);
    } else {
        printf("   Error getting pending notifications\n");
    }
    
    db_close();
    return 0;
}
