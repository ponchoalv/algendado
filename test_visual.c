#include "agenda.h"

int main() {
    printf("Testing raylib visual notification directly...\n");
    
    // Test the visual notification function directly
    int result = show_visual_notification(
        "🔔 Test Notification",
        "This is a test of the visual notification system!\nClick OK or press ESC to close.",
        "5:15 PM"
    );
    
    printf("Visual notification result: %d\n", result);
    return 0;
}
