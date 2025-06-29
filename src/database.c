#include "agenda.h"

// Global database connection
static sqlite3* db = NULL;

int db_init(void) {
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Create table if it doesn't exist
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS agenda_items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "date TEXT NOT NULL,"
        "time TEXT NOT NULL,"
        "description TEXT NOT NULL,"
        "datetime INTEGER NOT NULL,"
        "notified INTEGER DEFAULT 0"
        ");";

    char* err_msg = NULL;
    rc = sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }

    return 0;
}

int db_add_item(const char* date, const char* time, const char* description) {
    if (!db) {
        if (db_init() != 0) return -1;
    }

    time_t datetime = combine_datetime(date, time);
    if (datetime == -1) {
        fprintf(stderr, "Invalid date/time format\n");
        return -1;
    }

    const char* sql = "INSERT INTO agenda_items (date, time, description, datetime) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, time, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, description, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, datetime);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert item: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

int db_get_items(view_type_t view, agenda_item_t** items, int* count) {
    if (!db) {
        if (db_init() != 0) return -1;
    }

    time_t now = time(NULL);
    time_t start_time, end_time;
    struct tm* tm_now = localtime(&now);

    // Calculate time range based on view type
    switch (view) {
        case VIEW_TODAY:
            tm_now->tm_hour = 0;
            tm_now->tm_min = 0;
            tm_now->tm_sec = 0;
            start_time = mktime(tm_now);
            end_time = start_time + 24 * 60 * 60;
            break;
        case VIEW_WEEK:
            tm_now->tm_hour = 0;
            tm_now->tm_min = 0;
            tm_now->tm_sec = 0;
            tm_now->tm_mday -= tm_now->tm_wday; // Start of week
            start_time = mktime(tm_now);
            end_time = start_time + 7 * 24 * 60 * 60;
            break;
        case VIEW_MONTH:
            tm_now->tm_hour = 0;
            tm_now->tm_min = 0;
            tm_now->tm_sec = 0;
            tm_now->tm_mday = 1; // First day of month
            start_time = mktime(tm_now);
            tm_now->tm_mon++;
            if (tm_now->tm_mon > 11) {
                tm_now->tm_mon = 0;
                tm_now->tm_year++;
            }
            end_time = mktime(tm_now);
            break;
        default:
            return -1;
    }

    const char* sql = "SELECT id, date, time, description, datetime, notified "
                     "FROM agenda_items WHERE datetime >= ? AND datetime < ? "
                     "ORDER BY datetime;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, start_time);
    sqlite3_bind_int64(stmt, 2, end_time);

    // Count items first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }

    sqlite3_reset(stmt);

    if (*count == 0) {
        sqlite3_finalize(stmt);
        *items = NULL;
        return 0;
    }

    // Allocate memory for items
    *items = malloc(*count * sizeof(agenda_item_t));
    if (!*items) {
        sqlite3_finalize(stmt);
        return -1;
    }

    // Fetch items
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < *count) {
        (*items)[i].id = sqlite3_column_int(stmt, 0);
        strncpy((*items)[i].date, (const char*)sqlite3_column_text(stmt, 1), MAX_DATE_LEN - 1);
        strncpy((*items)[i].time, (const char*)sqlite3_column_text(stmt, 2), MAX_TIME_LEN - 1);
        strncpy((*items)[i].description, (const char*)sqlite3_column_text(stmt, 3), MAX_DESCRIPTION_LEN - 1);
        (*items)[i].datetime = sqlite3_column_int64(stmt, 4);
        (*items)[i].notified = sqlite3_column_int(stmt, 5);
        i++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int db_get_pending_notifications(agenda_item_t** items, int* count) {
    if (!db) {
        if (db_init() != 0) return -1;
    }

    time_t now = time(NULL);
    // Look for events that are exactly 15 minutes away (with a 1-minute window for safety)
    time_t target_time = now + (NOTIFICATION_ADVANCE_MINUTES * 60);
    time_t notification_start = target_time - 30; // 30 seconds before the 15-minute mark
    time_t notification_end = target_time + 30;   // 30 seconds after the 15-minute mark

    const char* sql = "SELECT id, date, time, description, datetime, notified "
                     "FROM agenda_items WHERE datetime >= ? AND datetime <= ? AND notified = 0 "
                     "ORDER BY datetime;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, notification_start);
    sqlite3_bind_int64(stmt, 2, notification_end);

    // Count items first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }

    sqlite3_reset(stmt);

    if (*count == 0) {
        sqlite3_finalize(stmt);
        *items = NULL;
        return 0;
    }

    // Allocate memory for items
    *items = malloc(*count * sizeof(agenda_item_t));
    if (!*items) {
        sqlite3_finalize(stmt);
        return -1;
    }

    // Fetch items
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < *count) {
        (*items)[i].id = sqlite3_column_int(stmt, 0);
        strncpy((*items)[i].date, (const char*)sqlite3_column_text(stmt, 1), MAX_DATE_LEN - 1);
        strncpy((*items)[i].time, (const char*)sqlite3_column_text(stmt, 2), MAX_TIME_LEN - 1);
        strncpy((*items)[i].description, (const char*)sqlite3_column_text(stmt, 3), MAX_DESCRIPTION_LEN - 1);
        (*items)[i].datetime = sqlite3_column_int64(stmt, 4);
        (*items)[i].notified = sqlite3_column_int(stmt, 5);
        i++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int db_mark_notified(int id) {
    if (!db) {
        if (db_init() != 0) return -1;
    }

    const char* sql = "UPDATE agenda_items SET notified = 1 WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to update item: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

int db_remove_item(int id) {
    const char* sql = "DELETE FROM agenda_items WHERE id = ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to remove item: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Check if any rows were actually deleted
    if (sqlite3_changes(db) == 0) {
        fprintf(stderr, "No item found with ID %d\n", id);
        return -1;
    }

    return 0;
}

void db_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}
