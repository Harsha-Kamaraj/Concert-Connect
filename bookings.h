#ifndef BOOKINGS_H
#define BOOKINGS_H

#include "users.h"
#include "events.h"
#include <time.h>

typedef struct Booking {
    char username[MAX_USERNAME];
    char display_name[MAX_NAME];
    char phone[MAX_PHONE];
    char email[MAX_EMAIL];
    int event_id; /* index into events array */
    int row;
    int col;
    double price_paid;
    char booking_id[32];  /* unique booking ID */
    time_t timestamp;     /* when booking was made */
    int num_seats;        /* number of seats in this booking group */
    struct Booking *next;
} Booking;

/* Simple FIFO Queue Node for waiting list */
typedef struct PriorityQueueNode {
    char username[MAX_USERNAME];
    char phone[MAX_PHONE];
    char email[MAX_EMAIL];
    time_t join_time;
    int num_seats;  /* Number of seats wanted */
} PriorityQueueNode;

/* Simple FIFO Queue implementation */
typedef struct PriorityQueue {
    PriorityQueueNode *heap;
    int size;
    int capacity;
} PriorityQueue;

/* Queue operations */
PriorityQueue* create_priority_queue(int capacity);
void pq_insert(PriorityQueue *pq, const char *username, const char *phone, const char *email, int num_seats);
int pq_extract_min(PriorityQueue *pq, char *out_username, char *out_phone, char *out_email, int *out_num_seats);
void free_priority_queue(PriorityQueue *pq);
int pq_is_empty(PriorityQueue *pq);

/* Booking ID generation */
void generate_booking_id(char *out_id, int event_idx, int booking_num);

/* Booking operations */
void show_all_bookings_for_event(int event_idx);
void show_all_bookings_admin(void);
void show_waiting_queue(int event_idx);
int auto_assign_seat(int event_idx, int *out_row, int *out_col);
int auto_assign_multiple_seats(int event_idx, int num_seats, int rows_out[], int cols_out[]);
int book_seat_for_user(int event_idx, User *user);
int book_multiple_seats_for_user(int event_idx, User *user, int num_seats);
int cancel_seat_by_user(int event_idx, const User *user);
int cancel_booking_by_id(const char *booking_id);
void view_my_bookings(const User *user);
void show_full_seatmap_all_events(void);
double calculate_refund(double price_paid);
void search_bookings_interactive(void);

/* Booking persistence */
void save_bookings_to_file(const char *path);
void load_bookings_from_file(const char *path);

#endif /* BOOKINGS_H */