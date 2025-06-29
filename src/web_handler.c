#include "agenda.h"
#include <microhttpd.h>

static struct MHD_Response* create_response(const char* content, const char* content_type) {
    struct MHD_Response* response = MHD_create_response_from_buffer(
        strlen(content), (void*)content, MHD_RESPMEM_MUST_COPY);
    
    if (response) {
        MHD_add_response_header(response, "Content-Type", content_type);
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    }
    
    return response;
}

static enum MHD_Result handle_calendar_request(struct MHD_Connection* connection) {
    char* ics_content = generate_ics_calendar();
    if (!ics_content) {
        const char* error_msg = "Error generating calendar";
        struct MHD_Response* response = create_response(error_msg, "text/plain");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    struct MHD_Response* response = create_response(ics_content, "text/calendar");
    MHD_add_response_header(response, "Content-Disposition", "attachment; filename=\"agenda.ics\"");
    
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    free(ics_content);
    
    return ret;
}

static enum MHD_Result handle_web_interface(struct MHD_Connection* connection) {
    char* html_content = generate_html_calendar();
    if (!html_content) {
        const char* error_msg = "Error generating web interface";
        struct MHD_Response* response = create_response(error_msg, "text/plain");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    struct MHD_Response* response = create_response(html_content, "text/html");
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    free(html_content);
    
    return ret;
}

static enum MHD_Result handle_not_found(struct MHD_Connection* connection) {
    const char* not_found_html = 
        "<!DOCTYPE html>\n"
        "<html><head><title>404 - Not Found</title></head>\n"
        "<body><h1>404 - Page Not Found</h1>\n"
        "<p>The requested page was not found.</p>\n"
        "<p><a href=\"/\">Return to Calendar</a></p></body></html>";
    
    struct MHD_Response* response = create_response(not_found_html, "text/html");
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    
    return ret;
}

enum MHD_Result handle_web_request(void* cls, struct MHD_Connection* connection,
                                  const char* url, const char* method,
                                  const char* version, const char* upload_data,
                                  size_t* upload_data_size, void** con_cls) {
    (void)cls;
    (void)version;
    (void)upload_data;
    (void)upload_data_size;
    (void)con_cls;
    
    // Only handle GET requests
    if (strcmp(method, "GET") != 0) {
        return MHD_NO;
    }
    
    // Route requests
    if (strcmp(url, "/") == 0 || strcmp(url, "/index.html") == 0) {
        return handle_web_interface(connection);
    } else if (strcmp(url, "/calendar.ics") == 0) {
        return handle_calendar_request(connection);
    } else {
        return handle_not_found(connection);
    }
}
