#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "events.h"
#include "bookings.h"
#include "utils.h"

#define INITIAL_EVENT_CAP 4

Event *events = NULL;
int event_count = 0;
int event_capacity = 0;

int seat_index(const Event *e, int r, int c) {
    return r * e->cols + c;
}

void init_events_system(void) {
    event_capacity = 0;
    event_count = 0;
    events = NULL;
}

void free_event(Event *e) {
    if (!e) return;
    free(e->seats);
    e->seats = NULL;
}

void cleanup_events_system(void) {
    if (!events) return;
    for (int i = 0; i < event_count; ++i) {
        free_event(&events[i]);
    }
    free(events);
    events = NULL;
    event_count = 0;
    event_capacity = 0;
}

int ensure_event_capacity(void) {
    if (event_capacity == 0) {
        event_capacity = INITIAL_EVENT_CAP;
        events = (Event *)calloc(event_capacity, sizeof(Event));
        if (!events) { fprintf(stderr, "Event allocation failed\n"); exit(1); }
    } else if (event_count >= event_capacity) {
        event_capacity *= 2;
        Event *tmp = (Event *)realloc(events, sizeof(Event) * event_capacity);
        if (!tmp) { fprintf(stderr, "Event reallocation failed\n"); exit(1); }
        /* zero-init new tail region */
        memset(tmp + event_count, 0, sizeof(Event) * (event_capacity - event_count));
        events = tmp;
    }
    return 1;
}

static void trim(char *s) {
    size_t n = strlen(s);
    while (n && (s[n-1] == '\r' || s[n-1] == '\n' || isspace((unsigned char)s[n-1]))) s[--n] = 0;
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p)+1);
}

void load_events_from_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return; /* ok if file absent */
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        trim(line);
        if (!line[0]) continue;
        /* Format: name|base|rows|cols|code|percent|date|time */
        char name[100], code[32], date[20], etime[10];
        double base = 0;
        int rows = 0, cols = 0, percent = 0;
        char *tok;
        char *rest = line;

        tok = strtok(rest, "|"); if (!tok) continue; strncpy(name, tok, sizeof(name)-1); name[sizeof(name)-1]=0;
        tok = strtok(NULL, "|"); if (!tok) continue; base = atof(tok);
        tok = strtok(NULL, "|"); if (!tok) continue; rows = atoi(tok);
        tok = strtok(NULL, "|"); if (!tok) continue; cols = atoi(tok);
        tok = strtok(NULL, "|"); if (!tok) continue; strncpy(code, tok, sizeof(code)-1); code[sizeof(code)-1]=0;
        tok = strtok(NULL, "|"); if (!tok) continue; percent = atoi(tok);
        tok = strtok(NULL, "|"); 
        if (tok) { strncpy(date, tok, sizeof(date)-1); date[sizeof(date)-1]=0; } else { strcpy(date, "2025-12-31"); }
        tok = strtok(NULL, "|"); 
        if (tok) { strncpy(etime, tok, sizeof(etime)-1); etime[sizeof(etime)-1]=0; } else { strcpy(etime, "18:00"); }

        ensure_event_capacity();
        Event *e = &events[event_count];
        e->id = event_count;
        strncpy(e->name, name, sizeof(e->name)-1); e->name[sizeof(e->name)-1]=0;
        e->base_price = base;
        e->rows = rows;
        e->cols = cols;
        strncpy(e->discount_code, code, sizeof(e->discount_code)-1); e->discount_code[sizeof(e->discount_code)-1]=0;
        e->discount_percent = percent;
        strncpy(e->event_date, date, sizeof(e->event_date)-1); e->event_date[sizeof(e->event_date)-1]=0;
        strncpy(e->event_time, etime, sizeof(e->event_time)-1); e->event_time[sizeof(e->event_time)-1]=0;
        e->revenue = 0;
        e->total_bookings = 0;
        e->bookings_head = NULL;
        e->wait_queue = create_priority_queue(50);
        size_t nseats = (size_t)rows * (size_t)cols;
        e->seats = (int *)calloc(nseats, sizeof(int));
        if (!e->seats) { fprintf(stderr, "Seats allocation failed\n"); exit(1); }
        event_count++;
    }
    fclose(fp);
}

void save_events_to_file(const char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp) { printf("Failed to open %s for write\n", path); return; }
    for (int i = 0; i < event_count; ++i) {
        Event *e = &events[i];
        fprintf(fp, "%s|%.2f|%d|%d|%s|%d|%s|%s\n", e->name, e->base_price, e->rows, e->cols, 
                e->discount_code, e->discount_percent, e->event_date, e->event_time);
    }
    fclose(fp);
}

void list_events_brief(void) {
    if (event_count == 0) {
        printf("\nNo events available at the moment. Please check later.\n");
        return;
    }
    printf("\nAvailable Events:\n");
    for (int i = 0; i < event_count; ++i) {
        printf(" %d) %s - %s @ %s\n", i + 1, events[i].name, events[i].event_date, events[i].event_time);
        printf("    Price: Rs.%.2f (%.0f%% full) | Seats: %dx%d | Code: %s - %d%%\n",
            events[i].base_price, get_occupancy_percent(i), events[i].rows, events[i].cols, 
            events[i].discount_code, events[i].discount_percent);
    }
}

int create_event_interactive(void) {
    char name[100], code[32], buf[64];
    int rows, cols, percent;
    double base;

    printf("\n== Create Event ==\n");
    printf("Event name: ");
    read_line(name, sizeof(name));
    if (!name[0]) { printf("Invalid name.\n"); return 0; }

    printf("Base price: ");
    read_line(buf, sizeof(buf));
    base = atof(buf);
    if (base <= 0) { printf("Invalid price.\n"); return 0; }

    printf("Rows: ");
    rows = read_int();
    if (rows <= 0 || rows > 26) { printf("Rows must be 1..26\n"); return 0; }

    printf("Columns: ");
    cols = read_int();
    if (cols <= 0 || cols > 50) { printf("Columns must be 1..50\n"); return 0; }

    printf("Event date (YYYY-MM-DD): ");
    char date[20];
    read_line(date, sizeof(date));
    
    printf("Event time (HH:MM): ");
    char etime[10];
    read_line(etime, sizeof(etime));
    
    printf("Discount code (e.g., ROCK10, or leave empty for none): ");
    read_line(code, sizeof(code));
    if (!code[0]) { strcpy(code, ""); percent = 0; }
    else {
        printf("Discount percent (0-100): ");
        percent = read_int();
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
    }

    ensure_event_capacity();
    Event *e = &events[event_count];
    e->id = event_count;
    strncpy(e->name, name, sizeof(e->name)-1); e->name[sizeof(e->name)-1]=0;
    e->base_price = base;
    e->rows = rows;
    e->cols = cols;
    strncpy(e->discount_code, code, sizeof(e->discount_code)-1); e->discount_code[sizeof(e->discount_code)-1]=0;
    e->discount_percent = percent;
    strncpy(e->event_date, date, sizeof(e->event_date)-1); e->event_date[sizeof(e->event_date)-1]=0;
    strncpy(e->event_time, etime, sizeof(e->event_time)-1); e->event_time[sizeof(e->event_time)-1]=0;
    e->revenue = 0;
    e->total_bookings = 0;
    e->bookings_head = NULL;
    e->wait_queue = create_priority_queue(50);  /* initial capacity 50 */
    size_t nseats = (size_t)rows * (size_t)cols;
    e->seats = (int *)calloc(nseats, sizeof(int));
    if (!e->seats) { fprintf(stderr, "Seats allocation failed\n"); exit(1); }
    event_count++;

    save_events_to_file("events.txt");
    printf("Event created.\n");
    return 1;
}

void delete_event_interactive(void) {
    if (event_count == 0) { printf("No events to delete.\n"); return; }
    list_events_brief();
    printf("Enter event number to delete: ");
    int ev = read_int();
    ev -= 1;
    if (ev < 0 || ev >= event_count) { printf("Invalid event.\n"); return; }

    /* free resources for this event */
    free_event(&events[ev]);
    /* shift remaining */
    for (int i = ev; i < event_count - 1; ++i) {
        events[i] = events[i+1];
        events[i].id = i;
    }
    event_count--;
    save_events_to_file("events.txt");
    printf("Event deleted.\n");
}

void change_ticket_price_interactive(void) {
    if (event_count == 0) { printf("No events.\n"); return; }
    list_events_brief();
    printf("Enter event number to modify price: ");
    int ev = read_int(); ev -= 1;
    if (ev < 0 || ev >= event_count) { printf("Invalid event.\n"); return; }
    printf("Current base price for %s = Rs.%.2f\n", events[ev].name, events[ev].base_price);
    printf("Enter new base price (positive): ");
    char buf[64];
    read_line(buf, sizeof(buf));
    double p = atof(buf);
    if (p <= 0) { printf("Invalid price.\n"); return; }
    events[ev].base_price = p;
    save_events_to_file("events.txt");
    printf("Price updated for %s. New base price = Rs.%.2f\n", events[ev].name, events[ev].base_price);
}

void display_seat_map(int event_idx) {
    if (event_idx < 0 || event_idx >= event_count) return;
    Event *e = &events[event_idx];
    double occ = get_occupancy_percent(event_idx);
    
    printf("\n+============================================================+\n");
    printf("|  Seat Map: %s\n", e->name);
    printf("|  Price: Rs.%.2f (%.0f%% full)\n", e->base_price, occ);
    printf("+============================================================+\n");
    printf("\n");
    
    /* Print column numbers */
    printf("     ");
    for (int c = 0; c < e->cols; ++c) {
        printf(" %3d ", c + 1);
    }
    printf("\n");
    
    /* Print seats in [A1] format */
    for (int r = 0; r < e->rows; ++r) {
        printf("  %c  ", 'A' + r);
        for (int c = 0; c < e->cols; ++c) {
            int idx = seat_index(e, r, c);
            if (e->seats[idx]) {
                printf("[XXX]");
            } else {
                printf("[%c%2d]", 'A' + r, c + 1);
            }
        }
        printf("\n");
    }
    printf("\n  Legend: [XXX] = Booked  |  [A 1] = Available\n");
}

int get_seats_booked(int event_idx) {
    if (event_idx < 0 || event_idx >= event_count) return 0;
    Event *e = &events[event_idx];
    int count = 0;
    for (int i = 0; i < e->rows * e->cols; ++i) {
        if (e->seats[i]) count++;
    }
    return count;
}

int get_available_seat_count(int event_idx) {
    if (event_idx < 0 || event_idx >= event_count) return 0;
    Event *e = &events[event_idx];
    int total = e->rows * e->cols;
    int booked = get_seats_booked(event_idx);
    return total - booked;
}

double get_occupancy_percent(int event_idx) {
    if (event_idx < 0 || event_idx >= event_count) return 0.0;
    Event *e = &events[event_idx];
    int total = e->rows * e->cols;
    if (total == 0) return 0.0;
    int booked = get_seats_booked(event_idx);
    return (booked * 100.0) / total;
}



void show_booking_analytics(void) {
    if (event_count == 0) {
        printf("\nNo events available for analytics.\n");
        return;
    }
    
    printf("\n+============================================================+\n");
    printf("|              BOOKING ANALYTICS REPORT                      |\n");
    printf("+============================================================+\n\n");
    
    int most_popular_idx = 0;
    int max_bookings = events[0].total_bookings;
    double total_revenue = 0.0;
    
    for (int i = 0; i < event_count; ++i) {
        Event *e = &events[i];
        int booked = get_seats_booked(i);
        int total = e->rows * e->cols;
        double occ = get_occupancy_percent(i);
        
        printf("Event: %s\n", e->name);
        printf("  Total Bookings: %d\n", e->total_bookings);
        printf("  Seats Booked: %d / %d (%.1f%%)\n", booked, total, occ);
        printf("  Revenue: Rs.%.2f\n", e->revenue);
        printf("  Base Price: Rs.%.2f\n\n", e->base_price);
        
        total_revenue += e->revenue;
        if (e->total_bookings > max_bookings) {
            max_bookings = e->total_bookings;
            most_popular_idx = i;
        }
    }
    
    printf("===========================================================\n");
    printf("Most Popular Event: %s (%d bookings)\n", events[most_popular_idx].name, max_bookings);
    printf("Total Revenue (All Events): Rs.%.2f\n", total_revenue);
    printf("===========================================================\n");
}