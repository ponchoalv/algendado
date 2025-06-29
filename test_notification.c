#include "agenda.h"

int main() {
    printf("Testing visual notification...\n");
    
    // Test the visual notification
    show_visual_notification(
        "📅 Test Notification",
        "This is a beautiful visual notification!\nClick OK or press ESC to close.",
        "2:30 PM"
    );
    
    printf("Notification test completed!\n");
    return 0;
}
