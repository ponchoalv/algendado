#include "agenda.h"

static void print_usage(void) {
    printf("Usage:\n");
    printf("  algen add <date> <time> \"<description>\"\n");
    printf("  algen get <period>\n\n");
    printf("Date formats:\n");
    printf("  today, tomorrow, or DD/MM/YYYY\n\n");
    printf("Time formats:\n");
    printf("  HH:MM or HH:MM:SS\n\n");
    printf("Period options:\n");
    printf("  today, week, month\n\n");
    printf("Examples:\n");
    printf("  algen add today 11:15:00 \"finish the project\"\n");
    printf("  algen add tomorrow 14:30 \"meeting with team\"\n");
    printf("  algen add 15/07/2025 09:00 \"doctor appointment\"\n");
    printf("  algen get today\n");
    printf("  algen get week\n");
}

static int start_server_if_needed(void) {
    if (!server_is_running()) {
        printf("Starting agenda server...\n");
        if (fork() == 0) {
            // Child process - start server
            execl("./algen-server", "algen-server", NULL);
            // If execl fails, try from PATH
            execlp("algen-server", "algen-server", NULL);
            exit(1);
        }
        // Give server time to start
        sleep(1);
    }
    return 0;
}

static int handle_add_command(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Error: Insufficient arguments for add command\n");
        print_usage();
        return 1;
    }

    char date[MAX_DATE_LEN];
    char time[MAX_TIME_LEN];

    if (parse_date_input(argv[2], date) != 0) {
        fprintf(stderr, "Error: Invalid date format '%s'\n", argv[2]);
        return 1;
    }

    if (parse_time_input(argv[3], time) != 0) {
        fprintf(stderr, "Error: Invalid time format '%s'\n", argv[3]);
        return 1;
    }

    // Start server if needed
    start_server_if_needed();

    if (db_add_item(date, time, argv[4]) != 0) {
        fprintf(stderr, "Error: Failed to add item to database\n");
        return 1;
    }

    char formatted_date[64];
    char formatted_time[32];
    format_date_for_display(date, formatted_date);
    format_time_for_display(time, formatted_time);

    printf("Added: %s at %s - %s\n", formatted_date, formatted_time, argv[4]);
    return 0;
}

static int handle_get_command(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Error: Insufficient arguments for get command\n");
        print_usage();
        return 1;
    }

    view_type_t view;
    if (strcmp(argv[2], "today") == 0) {
        view = VIEW_TODAY;
    } else if (strcmp(argv[2], "week") == 0) {
        view = VIEW_WEEK;
    } else if (strcmp(argv[2], "month") == 0) {
        view = VIEW_MONTH;
    } else {
        fprintf(stderr, "Error: Invalid period '%s'\n", argv[2]);
        print_usage();
        return 1;
    }

    agenda_item_t* items;
    int count;

    if (db_get_items(view, &items, &count) != 0) {
        fprintf(stderr, "Error: Failed to retrieve items from database\n");
        return 1;
    }

    if (count == 0) {
        printf("No agenda items found for %s.\n", argv[2]);
        return 0;
    }

    printf("Agenda items for %s:\n\n", argv[2]);
    for (int i = 0; i < count; i++) {
        char formatted_date[64];
        char formatted_time[32];
        format_date_for_display(items[i].date, formatted_date);
        format_time_for_display(items[i].time, formatted_time);
        
        printf("%d. %s at %s\n", i + 1, formatted_date, formatted_time);
        printf("   %s\n\n", items[i].description);
    }

    free(items);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    // Initialize database
    if (db_init() != 0) {
        fprintf(stderr, "Error: Failed to initialize database\n");
        return 1;
    }

    int result = 0;
    if (strcmp(argv[1], "add") == 0) {
        result = handle_add_command(argc, argv);
    } else if (strcmp(argv[1], "get") == 0) {
        result = handle_get_command(argc, argv);
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", argv[1]);
        print_usage();
        result = 1;
    }

    db_close();
    return result;
}
