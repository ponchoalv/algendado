#include "agenda.h"

char* generate_ics_calendar(void) {
    agenda_item_t* items;
    int count;
    
    // Get all items for the month
    if (db_get_items(VIEW_MONTH, &items, &count) != 0) {
        return NULL;
    }
    
    // Calculate required buffer size
    size_t buffer_size = 1024 + (count * 512); // Base size + estimated per item
    char* ics_content = malloc(buffer_size);
    if (!ics_content) {
        if (items) free(items);
        return NULL;
    }
    
    // ICS header
    strcpy(ics_content, "BEGIN:VCALENDAR\r\n");
    strcat(ics_content, "VERSION:2.0\r\n");
    strcat(ics_content, "PRODID:-//Algendado//Personal Agenda//EN\r\n");
    strcat(ics_content, "CALSCALE:GREGORIAN\r\n");
    
    // Add events
    for (int i = 0; i < count; i++) {
        char event_block[512];
        char start_datetime[32];
        char uid[64];
        
        // Convert date and time to ICS format (YYYYMMDDTHHMMSS)
        struct tm tm_event = {0};
        sscanf(items[i].date, "%d-%d-%d", &tm_event.tm_year, &tm_event.tm_mon, &tm_event.tm_mday);
        sscanf(items[i].time, "%d:%d:%d", &tm_event.tm_hour, &tm_event.tm_min, &tm_event.tm_sec);
        tm_event.tm_year -= 1900;
        tm_event.tm_mon -= 1;
        
        strftime(start_datetime, sizeof(start_datetime), "%Y%m%dT%H%M%S", &tm_event);
        snprintf(uid, sizeof(uid), "agenda-item-%d@algendado", items[i].id);
        
        snprintf(event_block, sizeof(event_block),
            "BEGIN:VEVENT\r\n"
            "UID:%s\r\n"
            "DTSTAMP:%s\r\n"
            "DTSTART:%s\r\n"
            "SUMMARY:%s\r\n"
            "DESCRIPTION:%s\r\n"
            "END:VEVENT\r\n",
            uid, start_datetime, start_datetime, 
            items[i].description, items[i].description);
        
        strcat(ics_content, event_block);
    }
    
    // ICS footer
    strcat(ics_content, "END:VCALENDAR\r\n");
    
    if (items) free(items);
    return ics_content;
}

char* generate_html_calendar(void) {
    agenda_item_t* items;
    int count;
    
    // Get all items for the month
    if (db_get_items(VIEW_MONTH, &items, &count) != 0) {
        return NULL;
    }
    
    // Calculate required buffer size
    size_t buffer_size = 4096 + (count * 256);
    char* html_content = malloc(buffer_size);
    if (!html_content) {
        if (items) free(items);
        return NULL;
    }
    
    // HTML header
    strcpy(html_content, 
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <title>Personal Agenda</title>\n"
        "    <style>\n"
        "        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }\n"
        "        .container { max-width: 800px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n"
        "        h1 { color: #333; text-align: center; margin-bottom: 30px; }\n"
        "        .agenda-item { background-color: #f9f9f9; border-left: 4px solid #4CAF50; margin: 10px 0; padding: 15px; border-radius: 5px; }\n"
        "        .date-time { font-weight: bold; color: #2196F3; margin-bottom: 5px; }\n"
        "        .description { color: #666; }\n"
        "        .no-items { text-align: center; color: #999; font-style: italic; padding: 40px; }\n"
        "        .header-actions { text-align: center; margin-bottom: 20px; }\n"
        "        .ics-link { display: inline-block; background-color: #4CAF50; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; margin: 5px; }\n"
        "        .ics-link:hover { background-color: #45a049; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <div class=\"container\">\n"
        "        <h1>📅 Personal Agenda</h1>\n"
        "        <div class=\"header-actions\">\n"
        "            <a href=\"/calendar.ics\" class=\"ics-link\">📱 Download ICS Calendar</a>\n"
        "        </div>\n");
    
    if (count == 0) {
        strcat(html_content, "        <div class=\"no-items\">No agenda items found for this month.</div>\n");
    } else {
        for (int i = 0; i < count; i++) {
            char item_html[512];
            char formatted_date[64];
            char formatted_time[32];
            
            format_date_for_display(items[i].date, formatted_date);
            format_time_for_display(items[i].time, formatted_time);
            
            snprintf(item_html, sizeof(item_html),
                "        <div class=\"agenda-item\">\n"
                "            <div class=\"date-time\">%s at %s</div>\n"
                "            <div class=\"description\">%s</div>\n"
                "        </div>\n",
                formatted_date, formatted_time, items[i].description);
            
            strcat(html_content, item_html);
        }
    }
    
    // HTML footer
    strcat(html_content, 
        "    </div>\n"
        "</body>\n"
        "</html>\n");
    
    if (items) free(items);
    return html_content;
}
