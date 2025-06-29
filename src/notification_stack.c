#include "agenda.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <title> <message> <time>\n", argv[0]);
        return 1;
    }
    
    // Add the notification to the stack
    add_notification_to_stack(argv[1], argv[2], argv[3]);
    
    // Show the stacked notifications
    return show_stacked_notifications();
}
