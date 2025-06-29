#include "agenda.h"
#include <microhttpd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Global variables
static struct MHD_Daemon* web_daemon = NULL;
static pthread_t notification_thread_id;
static volatile int server_running = 0;

// External function declarations
extern void* notification_thread(void* arg);
extern void stop_notification_thread(void);
extern enum MHD_Result handle_web_request(void* cls, struct MHD_Connection* connection,
                                         const char* url, const char* method,
                                         const char* version, const char* upload_data,
                                         size_t* upload_data_size, void** con_cls);

static void signal_handler(int sig) {
    (void)sig;
    printf("\nShutting down server...\n");
    server_running = 0;
}

int server_start(void) {
    // Check if server is already running
    if (server_is_running()) {
        printf("Server is already running on port %d\n", SERVER_PORT);
        return 0;
    }
    
    // Initialize database
    if (db_init() != 0) {
        fprintf(stderr, "Failed to initialize database\n");
        return -1;
    }
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Start web daemon
    web_daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD,
        SERVER_PORT,
        NULL, NULL,
        &handle_web_request, NULL,
        MHD_OPTION_END
    );
    
    if (!web_daemon) {
        fprintf(stderr, "Failed to start web server on port %d\n", SERVER_PORT);
        return -1;
    }
    
    printf("Agenda server started on port %d\n", SERVER_PORT);
    printf("Web interface: http://localhost:%d\n", SERVER_PORT);
    printf("ICS calendar: http://localhost:%d/calendar.ics\n", SERVER_PORT);
    
    // Start notification thread
    if (pthread_create(&notification_thread_id, NULL, notification_thread, NULL) != 0) {
        fprintf(stderr, "Failed to start notification thread\n");
        MHD_stop_daemon(web_daemon);
        return -1;
    }
    
    server_running = 1;
    
    // Main server loop
    while (server_running) {
        sleep(1);
    }
    
    // Cleanup
    server_stop();
    return 0;
}

void server_stop(void) {
    if (server_running) {
        server_running = 0;
        
        // Stop notification thread
        stop_notification_thread();
        pthread_join(notification_thread_id, NULL);
        
        // Stop web daemon
        if (web_daemon) {
            MHD_stop_daemon(web_daemon);
            web_daemon = NULL;
        }
        
        // Close database
        db_close();
        
        printf("Server stopped\n");
    }
}

int main(void) {
    printf("Starting Algendado Server...\n");
    
    int result = server_start();
    if (result != 0) {
        fprintf(stderr, "Server failed to start\n");
        return 1;
    }
    
    return 0;
}
