#include "agenda.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <title> <message> <time>\n", argv[0]);
        return 1;
    }
    
    return show_visual_notification(argv[1], argv[2], argv[3]);
}
