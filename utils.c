#include "utils.h"
#include <string.h>
#include <stdlib.h>  

void read_line(char *buf, int n) {
    if (!fgets(buf, n, stdin)) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\n")] = '\0';
}

int read_int(void) {
    char s[64];
    read_line(s, sizeof(s));
    return atoi(s);
}

void print_divider(void) {
    printf("------------------------------------------------------------\n");
}

void pause_enter(void) {
    printf("Press Enter to continue...");
    char tmp[4];
    read_line(tmp, sizeof(tmp));
}