#include "agenda.h"
#include <signal.h>

int send_notification(const char* title, const char* message) {
    // Use macOS osascript to display notification
    char command[512];
    snprintf(command, sizeof(command), 
        "osascript -e 'display notification \"%s\" with title \"%s\" sound name \"default\"'",
        message, title);
    
    int result = system(command);
    return (result == 0) ? 0 : -1;
}

int show_visual_notification(const char* title, const char* message, const char* time_str) {
    // Initialize raylib window
    const int windowWidth = 400;
    const int windowHeight = 250;
    
    InitWindow(windowWidth, windowHeight, "Agenda Notification");
    SetWindowPosition(100, 100);
    SetTargetFPS(60);
    
    // Colors
    Color backgroundColor = (Color){45, 45, 55, 255};
    Color titleColor = (Color){255, 255, 255, 255};
    Color messageColor = (Color){200, 200, 200, 255};
    Color timeColor = (Color){100, 200, 255, 255};
    Color buttonColor = (Color){70, 130, 180, 255};
    Color buttonHoverColor = (Color){100, 149, 237, 255};
    Color accentColor = (Color){255, 165, 0, 255};
    
    // Animation variables
    float slideOffset = windowHeight;
    float targetOffset = 0;
    float animationSpeed = 8.0f;
    
    // Button properties
    Rectangle dismissButton = {windowWidth - 80, windowHeight - 40, 70, 30};
    bool isHovering = false;
    
    // Auto-dismiss timer (30 seconds)
    float autoCloseTimer = 30.0f;
    
    // Main loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // Smooth slide-in animation
        slideOffset += (targetOffset - slideOffset) * animationSpeed * deltaTime;
        
        // Auto-close countdown
        autoCloseTimer -= deltaTime;
        if (autoCloseTimer <= 0) {
            break;
        }
        
        // Check for button hover
        Vector2 mousePos = GetMousePosition();
        isHovering = CheckCollisionPointRec(mousePos, dismissButton);
        
        // Check for button click
        if (isHovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            break;
        }
        
        // Check for ESC key
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }
        
        // Drawing
        BeginDrawing();
        ClearBackground(backgroundColor);
        
        // Apply slide animation offset
        int yOffset = (int)slideOffset;
        
        // Draw notification icon (bell)
        DrawCircle(50, 50 - yOffset, 20, accentColor);
        DrawText("🔔", 42, 35 - yOffset, 20, WHITE);
        
        // Draw title
        DrawText(title, 90, 30 - yOffset, 24, titleColor);
        
        // Draw time
        char timeDisplay[64];
        snprintf(timeDisplay, sizeof(timeDisplay), "⏰ %s", time_str);
        DrawText(timeDisplay, 90, 60 - yOffset, 16, timeColor);
        
        // Draw separator line
        DrawLine(20, 90 - yOffset, windowWidth - 20, 90 - yOffset, accentColor);
        
        // Draw message (word wrapped)
        const char* messageLines[5];
        int lineCount = 0;
        char messageCopy[512];
        strncpy(messageCopy, message, sizeof(messageCopy) - 1);
        messageCopy[sizeof(messageCopy) - 1] = '\0';
        
        // Simple word wrapping
        char* line = strtok(messageCopy, "\n");
        while (line && lineCount < 5) {
            messageLines[lineCount++] = line;
            line = strtok(NULL, "\n");
        }
        
        // If no newlines, try to wrap long text
        if (lineCount == 1 && strlen(messageLines[0]) > 45) {
            // Simple character-based wrapping for demo
            static char line1[46], line2[46];
            strncpy(line1, messageLines[0], 45);
            line1[45] = '\0';
            strcpy(line2, messageLines[0] + 45);
            messageLines[0] = line1;
            messageLines[1] = line2;
            lineCount = 2;
        }
        
        for (int i = 0; i < lineCount && i < 4; i++) {
            DrawText(messageLines[i], 30, 110 + (i * 25) - yOffset, 18, messageColor);
        }
        
        // Draw dismiss button
        Color currentButtonColor = isHovering ? buttonHoverColor : buttonColor;
        DrawRectangleRec(dismissButton, currentButtonColor);
        DrawRectangleLinesEx(dismissButton, 1, WHITE);
        
        const char* buttonText = "OK";
        int textWidth = MeasureText(buttonText, 16);
        DrawText(buttonText, 
                dismissButton.x + (dismissButton.width - textWidth) / 2,
                dismissButton.y + 8, 16, WHITE);
        
        // Draw auto-close countdown
        if (autoCloseTimer < 10) {
            char countdownText[32];
            snprintf(countdownText, sizeof(countdownText), "Auto-close: %.0fs", autoCloseTimer);
            DrawText(countdownText, 20, windowHeight - 20, 12, (Color){150, 150, 150, 255});
        }
        
        // Draw subtle glow effect around the window
        for (int i = 0; i < 3; i++) {
            DrawRectangleLinesEx((Rectangle){i, i - yOffset, windowWidth - 2*i, windowHeight - 2*i}, 
                               1, (Color){accentColor.r, accentColor.g, accentColor.b, 50 - i*15});
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}

static volatile int notification_thread_running = 1;

void* notification_thread(void* arg) {
    (void)arg; // Suppress unused parameter warning
    
    while (notification_thread_running) {
        agenda_item_t* items;
        int count;
        
        if (db_get_pending_notifications(&items, &count) == 0 && count > 0) {
            for (int i = 0; i < count; i++) {
                char title[64];
                char message[512];
                char formatted_time[32];
                
                format_time_for_display(items[i].time, formatted_time);
                snprintf(title, sizeof(title), "📅 Agenda Reminder");
                snprintf(message, sizeof(message), "%s", items[i].description);
                
                printf("Showing notification for: %s\n", items[i].description);
                
                // Fork a process to show the visual notification
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process - show visual notification
                    show_visual_notification(title, message, formatted_time);
                    exit(0);
                } else if (pid > 0) {
                    // Parent process - also send system notification as backup
                    send_notification(title, items[i].description);
                    db_mark_notified(items[i].id);
                } else {
                    fprintf(stderr, "Failed to fork process for notification\n");
                    // Fallback to system notification only
                    if (send_notification(title, items[i].description) == 0) {
                        db_mark_notified(items[i].id);
                    }
                }
            }
            free(items);
        }
        
        // Check every 30 seconds
        sleep(30);
    }
    
    return NULL;
}

void stop_notification_thread(void) {
    notification_thread_running = 0;
}
