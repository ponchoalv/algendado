#include "agenda.h"

int main() {
    // Add multiple notifications to the stack to test stacking
    add_notification_to_stack("📅 First Notification", "Meeting with team at 2:00 PM", "14:00");
    add_notification_to_stack("📅 Second Notification", "Doctor appointment at 3:00 PM", "15:00");
    add_notification_to_stack("📅 Third Notification", "Grocery shopping reminder", "16:00");
    add_notification_to_stack("📅 Fourth Notification", "Call mom about dinner plans", "17:00");
    
    printf("Added 4 notifications to stack, showing stacked display...\n");
    
    // Show the stacked notifications
    return show_stacked_notifications();
}
