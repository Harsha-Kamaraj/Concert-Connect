#ifndef EVENTS_H
#define EVENTS_H

#include <stdio.h>

struct Booking;
struct QueueNode;

typedef struct Event {
    int id;
    char name[100];
    double base_price;
    int rows;
    int cols;
    char discount_code[32];
    int discount_percent;
    int *seats;
    struct Booking *bookings_head;   // use struct tag here
    struct PriorityQueue *wait_queue;  // replaced linked list with priority queue
    double revenue;
    char event_date[20];  /* format: YYYY-MM-DD */
    char event_time[10];  /* format: HH:MM */
    int total_bookings;   /* for analytics */
} Event;

/* Globals for events */
extern Event *events;
extern int event_count;
extern int event_capacity;

/* Event lifecycle */
void init_events_system(void);
void free_event(Event *e);
void cleanup_events_system(void);

/* Persistence */
void load_events_from_file(const char *path);
void save_events_to_file(const char *path);

/* Admin operations */
int create_event_interactive(void);
void list_events_brief(void);
void delete_event_interactive(void);
void change_ticket_price_interactive(void);
void display_seat_map(int event_idx);
void show_booking_analytics(void);

/* Helpers */
int ensure_event_capacity(void);
int seat_index(const Event *e, int r, int c);
int get_seats_booked(int event_idx);
int get_available_seat_count(int event_idx);
double get_occupancy_percent(int event_idx);

#endif /* EVENTS_H */
