#include "agenda.h"
#include <signal.h>

// Global notification stack
static notification_window_t* notification_stack = NULL;
static pthread_mutex_t notification_mutex = PTHREAD_MUTEX_INITIALIZER;

int send_notification(const char* title, const char* message) {
    // Use macOS osascript to display notification
    char command[512];
    snprintf(command, sizeof(command), 
        "osascript -e 'display notification \"%s\" with title \"%s\" sound name \"default\"'",
        message, title);
    
    int result = system(command);
    return (result == 0) ? 0 : -1;
}

void add_notification_to_stack(const char* title, const char* message, const char* time_str) {
    pthread_mutex_lock(&notification_mutex);
    
    notification_window_t* new_notification = malloc(sizeof(notification_window_t));
    if (!new_notification) {
        pthread_mutex_unlock(&notification_mutex);
        return;
    }
    
    strncpy(new_notification->title, title, sizeof(new_notification->title) - 1);
    strncpy(new_notification->message, message, sizeof(new_notification->message) - 1);
    strncpy(new_notification->time_str, time_str, sizeof(new_notification->time_str) - 1);
    new_notification->title[sizeof(new_notification->title) - 1] = '\0';
    new_notification->message[sizeof(new_notification->message) - 1] = '\0';
    new_notification->time_str[sizeof(new_notification->time_str) - 1] = '\0';
    
    new_notification->slideOffset = -130; // Start off-screen (above)
    new_notification->autoCloseTimer = 30.0f;
    new_notification->position_index = 0;
    new_notification->next = notification_stack;
    
    // Update position indices
    notification_window_t* current = notification_stack;
    int index = 1;
    while (current) {
        current->position_index = index++;
        current = current->next;
    }
    
    notification_stack = new_notification;
    pthread_mutex_unlock(&notification_mutex);
}

static void remove_notification_from_stack(notification_window_t* to_remove) {
    if (!to_remove) return;
    
    if (notification_stack == to_remove) {
        notification_stack = to_remove->next;
    } else {
        notification_window_t* current = notification_stack;
        while (current && current->next != to_remove) {
            current = current->next;
        }
        if (current) {
            current->next = to_remove->next;
        }
    }
    
    // Update position indices
    notification_window_t* current = notification_stack;
    int index = 0;
    while (current) {
        current->position_index = index++;
        current = current->next;
    }
    
    free(to_remove);
}

int show_stacked_notifications(void) {
    if (!notification_stack) return 0;
    
    // Count notifications to calculate window size
    pthread_mutex_lock(&notification_mutex);
    int notification_count = 0;
    notification_window_t* counter = notification_stack;
    while (counter) {
        notification_count++;
        counter = counter->next;
    }
    pthread_mutex_unlock(&notification_mutex);
    
    // Initialize raylib window with dynamic sizing
    const int windowWidth = 420;
    const int notificationHeight = 120; // Smaller individual notification height
    const int stackSpacing = 10;
    const int windowHeight = (notificationHeight + stackSpacing) * notification_count + 20;
    
    InitWindow(windowWidth, windowHeight, "Agenda Notifications");
    
    // Position window at top-right of screen
    int screenWidth = GetMonitorWidth(0);
    SetWindowPosition(screenWidth - windowWidth - 20, 20);
    SetTargetFPS(60);
    
    // Colors
    Color backgroundColor = (Color){45, 45, 55, 255};
    Color titleColor = (Color){255, 255, 255, 255};
    Color messageColor = (Color){200, 200, 200, 255};
    Color timeColor = (Color){100, 200, 255, 255};
    Color buttonColor = (Color){70, 130, 180, 255};
    Color buttonHoverColor = (Color){100, 149, 237, 255};
    Color accentColor = (Color){255, 165, 0, 255};
    Color borderColor = (Color){70, 70, 80, 255};
    
    float animationSpeed = 8.0f;
    
    // Main loop
    while (!WindowShouldClose() && notification_stack) {
        float deltaTime = GetFrameTime();
        
        pthread_mutex_lock(&notification_mutex);
        
        // Update all notifications
        notification_window_t* current = notification_stack;
        
        while (current) {
            // Calculate target position - stack from top to bottom
            float targetY = 10 + (current->position_index * (notificationHeight + stackSpacing));
            
            // Smooth animation to target position
            current->slideOffset += (targetY - current->slideOffset) * animationSpeed * deltaTime;
            
            // Auto-close countdown
            current->autoCloseTimer -= deltaTime;
            
            if (current->autoCloseTimer <= 0) {
                // Remove expired notification
                notification_window_t* to_remove = current;
                current = current->next;
                remove_notification_from_stack(to_remove);
                continue;
            }
            
            current = current->next;
        }
        
        pthread_mutex_unlock(&notification_mutex);
        
        // Handle input
        Vector2 mousePos = GetMousePosition();
        bool clickHandled = false;
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            pthread_mutex_lock(&notification_mutex);
            current = notification_stack;
            while (current && !clickHandled) {
                Rectangle dismissButton = {windowWidth - 80, current->slideOffset + notificationHeight - 35, 70, 25};
                
                if (CheckCollisionPointRec(mousePos, dismissButton)) {
                    remove_notification_from_stack(current);
                    clickHandled = true;
                    break;
                }
                current = current->next;
            }
            pthread_mutex_unlock(&notification_mutex);
        }
        
        // Check for ESC key to close all
        if (IsKeyPressed(KEY_ESCAPE)) {
            pthread_mutex_lock(&notification_mutex);
            while (notification_stack) {
                notification_window_t* to_remove = notification_stack;
                notification_stack = notification_stack->next;
                free(to_remove);
            }
            pthread_mutex_unlock(&notification_mutex);
            break;
        }
        
        // Drawing
        BeginDrawing();
        ClearBackground((Color){0, 0, 0, 0}); // Transparent background
        
        pthread_mutex_lock(&notification_mutex);
        current = notification_stack;
        while (current) {
            float yPos = current->slideOffset;
            
            // Draw notification background with border
            DrawRectangle(0, yPos, windowWidth, notificationHeight, backgroundColor);
            DrawRectangleLinesEx((Rectangle){0, yPos, windowWidth, notificationHeight}, 2, borderColor);
            
            // Draw notification icon (bell shape using raylib primitives)
            DrawCircle(30, yPos + 25, 15, accentColor);
            // Draw bell shape
            DrawCircle(30, yPos + 22, 8, backgroundColor);
            DrawRectangle(26, yPos + 22, 8, 6, backgroundColor);
            DrawCircle(30, yPos + 30, 2, WHITE); // Bell clapper
            
            // Draw title (smaller)
            DrawText(current->title, 55, yPos + 10, 18, titleColor);
            
            // Draw time with clock icon
            char timeDisplay[64];
            snprintf(timeDisplay, sizeof(timeDisplay), "Time: %s", current->time_str);
            // Draw simple clock icon
            DrawCircle(60, yPos + 37, 6, timeColor);
            DrawLine(60, yPos + 37, 60, yPos + 32, WHITE); // Hour hand
            DrawLine(60, yPos + 37, 64, yPos + 37, WHITE); // Minute hand
            DrawText(timeDisplay, 75, yPos + 30, 14, timeColor);
            
            // Draw separator line
            DrawLine(10, yPos + 50, windowWidth - 10, yPos + 50, accentColor);
            
            // Draw message (truncated if too long)
            char displayMessage[80];
            strncpy(displayMessage, current->message, sizeof(displayMessage) - 1);
            displayMessage[sizeof(displayMessage) - 1] = '\0';
            if (strlen(current->message) > 75) {
                strcpy(displayMessage + 72, "...");
            }
            
            DrawText(displayMessage, 10, yPos + 60, 14, messageColor);
            
            // Draw timer bar
            float timerWidth = (current->autoCloseTimer / 30.0f) * (windowWidth - 20);
            DrawRectangle(10, yPos + notificationHeight - 40, timerWidth, 3, accentColor);
            
            // Draw dismiss button
            Rectangle dismissButton = {windowWidth - 80, yPos + notificationHeight - 35, 70, 25};
            bool isHovering = CheckCollisionPointRec(GetMousePosition(), dismissButton);
            Color btnColor = isHovering ? buttonHoverColor : buttonColor;
            
            DrawRectangleRec(dismissButton, btnColor);
            DrawText("Dismiss", dismissButton.x + 8, dismissButton.y + 6, 14, WHITE);
            
            current = current->next;
        }
        pthread_mutex_unlock(&notification_mutex);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
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
    
    printf("Notification thread started\n");
    
    while (notification_thread_running) {
        agenda_item_t* items;
        int count;
        
        if (db_get_pending_notifications(&items, &count) == 0) {
            if (count > 0) {
                printf("Found %d pending notifications\n", count);
                
                bool has_new_notifications = false;
                char last_title[64] = "";
                char last_message[512] = "";
                char last_formatted_time[32] = "";
                
                for (int i = 0; i < count; i++) {
                    char title[64];
                    char message[512];
                    char formatted_time[32];
                    
                    format_time_for_display(items[i].time, formatted_time);
                    snprintf(title, sizeof(title), "Agenda Reminder");
                    snprintf(message, sizeof(message), "%s", items[i].description);
                    
                    printf("Adding notification to stack: %s at %s (ID: %d)\n", 
                           items[i].description, formatted_time, items[i].id);
                    
                    // Add to notification stack
                    add_notification_to_stack(title, message, formatted_time);
                    has_new_notifications = true;
                    
                    // Keep track of last notification for external command
                    strncpy(last_title, title, sizeof(last_title) - 1);
                    strncpy(last_message, message, sizeof(last_message) - 1);
                    strncpy(last_formatted_time, formatted_time, sizeof(last_formatted_time) - 1);
                    last_title[sizeof(last_title) - 1] = '\0';
                    last_message[sizeof(last_message) - 1] = '\0';
                    last_formatted_time[sizeof(last_formatted_time) - 1] = '\0';
                    
                    // Always send system notification as backup
                    send_notification(title, items[i].description);
                    db_mark_notified(items[i].id);
                    printf("Marked item %d as notified\n", items[i].id);
                }
                
                // If we have new notifications, start stacked display
                if (has_new_notifications) {
                    printf("Starting stacked notification display\n");
                    // Use external executable for better display compatibility
                    char command[1024];
                    snprintf(command, sizeof(command), "./algen-stack \"%s\" \"%s\" \"%s\" &", 
                            last_title, last_message, last_formatted_time);
                    
                    int result = system(command);
                    if (result != 0) {
                        printf("Stacked notification executable failed, trying fork method...\n");
                        pid_t pid = fork();
                        if (pid == 0) {
                            // Child process - show stacked notifications
                            show_stacked_notifications();
                            exit(0);
                        }
                    }
                }
                
                free(items);
            }
        }
        
        // Check every 30 seconds
        sleep(30);
    }
    
    printf("Notification thread stopped\n");
    return NULL;
}

void stop_notification_thread(void) {
    notification_thread_running = 0;
}
