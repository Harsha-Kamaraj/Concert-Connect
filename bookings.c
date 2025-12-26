#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "bookings.h"
#include "utils.h"

static int booking_counter = 1;  /* Global counter for unique booking IDs */

/* ============= PRIORITY QUEUE (MIN-HEAP) IMPLEMENTATION ============= */

PriorityQueue* create_priority_queue(int capacity) {
    PriorityQueue *pq = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    if (!pq) return NULL;
    pq->heap = (PriorityQueueNode *)malloc(sizeof(PriorityQueueNode) * capacity);
    if (!pq->heap) { free(pq); return NULL; }
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void free_priority_queue(PriorityQueue *pq) {
    if (!pq) return;
    free(pq->heap);
    free(pq);
}

int pq_is_empty(PriorityQueue *pq) {
    return pq == NULL || pq->size == 0;
}

static void pq_swap(PriorityQueueNode *a, PriorityQueueNode *b) {
    PriorityQueueNode temp = *a;
    *a = *b;
    *b = temp;
}

static void pq_heapify_up(PriorityQueue *pq, int idx) {
    if (idx == 0) return;
    int parent = (idx - 1) / 2;
    
    /* FIFO: earlier join_time = higher priority */
    if (pq->heap[idx].join_time < pq->heap[parent].join_time) {
        pq_swap(&pq->heap[idx], &pq->heap[parent]);
        pq_heapify_up(pq, parent);
    }
}

static void pq_heapify_down(PriorityQueue *pq, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    
    if (left < pq->size && pq->heap[left].join_time < pq->heap[smallest].join_time) {
        smallest = left;
    }
    
    if (right < pq->size && pq->heap[right].join_time < pq->heap[smallest].join_time) {
        smallest = right;
    }
    
    if (smallest != idx) {
        pq_swap(&pq->heap[idx], &pq->heap[smallest]);
        pq_heapify_down(pq, smallest);
    }
}

void pq_insert(PriorityQueue *pq, const char *username, const char *phone, const char *email, int num_seats) {
    if (!pq) return;
    if (pq->size >= pq->capacity) {
        /* Resize */
        pq->capacity *= 2;
        PriorityQueueNode *new_heap = (PriorityQueueNode *)realloc(pq->heap, sizeof(PriorityQueueNode) * pq->capacity);
        if (!new_heap) return;
        pq->heap = new_heap;
    }
    
    PriorityQueueNode *node = &pq->heap[pq->size];
    strncpy(node->username, username, MAX_USERNAME - 1); node->username[MAX_USERNAME - 1] = '\0';
    strncpy(node->phone, phone, MAX_PHONE - 1); node->phone[MAX_PHONE - 1] = '\0';
    strncpy(node->email, email, MAX_EMAIL - 1); node->email[MAX_EMAIL - 1] = '\0';
    node->join_time = time(NULL);
    node->num_seats = num_seats;
    
    pq->size++;
    pq_heapify_up(pq, pq->size - 1);
}

int pq_extract_min(PriorityQueue *pq, char *out_username, char *out_phone, char *out_email, int *out_num_seats) {
    if (pq_is_empty(pq)) return 0;
    
    PriorityQueueNode *root = &pq->heap[0];
    strncpy(out_username, root->username, MAX_USERNAME - 1); out_username[MAX_USERNAME - 1] = '\0';
    strncpy(out_phone, root->phone, MAX_PHONE - 1); out_phone[MAX_PHONE - 1] = '\0';
    strncpy(out_email, root->email, MAX_EMAIL - 1); out_email[MAX_EMAIL - 1] = '\0';
    *out_num_seats = root->num_seats;
    
    pq->heap[0] = pq->heap[pq->size - 1];
    pq->size--;
    pq_heapify_down(pq, 0);
    
    return 1;
}

/* ============= BOOKING ID GENERATION ============= */

void generate_booking_id(char *out_id, int event_idx, int booking_num) {
    time_t now = time(NULL);
    snprintf(out_id, 32, "BK%d-E%d-%ld", booking_num, event_idx, (long)now % 100000);
    booking_counter++;
}

/* ============= REFUND POLICY ============= */

double calculate_refund(double price_paid) {
    /* Always keep 10-15% as per requirement */
    double retention_percent = 12.5;  /* keeping 12.5% (middle of 10-15%) */
    double refund_percent = 100.0 - retention_percent;
    return price_paid * (refund_percent / 100.0);
}

/* ============= EXISTING FUNCTIONS (updated) ============= */

static double apply_discount_event(double base_price, const char *entered, const char *event_code, int event_percent) {
    if (!entered || !entered[0]) return base_price;
    if (strcasecmp(entered, "NA") == 0 || strcasecmp(entered, "X") == 0) return base_price;
    if (event_code && event_code[0] && strcasecmp(entered, event_code) == 0) {
        double disc = (base_price * event_percent) / 100.0;
        if (disc < 0) disc = 0;
        if (disc > base_price) disc = base_price;
        return base_price - disc;
    }
    return base_price;
}

void show_all_bookings_for_event(int event_idx) {
    if (event_idx < 0 || event_idx >= event_count) return;
    Booking *b = events[event_idx].bookings_head;
    printf("\nBookings for %s:\n", events[event_idx].name);
    if (!b) { printf(" (none)\n"); return; }
    while (b) {
        char time_str[26];
        struct tm *tm_info = localtime(&b->timestamp);
        strftime(time_str, 26, "%Y-%m-%d %H:%M", tm_info);
        printf(" - ID: %s | %s | seat %c%d | Rs.%.2f | %s | phone: %s\n", 
               b->booking_id, b->username, 'A' + b->row, b->col + 1, b->price_paid, time_str, b->phone);
        b = b->next;
    }
}

void show_waiting_queue(int event_idx) {
    if (event_idx < 0 || event_idx >= event_count) return;
    PriorityQueue *pq = events[event_idx].wait_queue;
    Event *ev = &events[event_idx];
    
    printf("\n+============================================================+\n");
    printf("|  WAITING QUEUE - %s\n", ev->name);
    printf("+============================================================+\n");
    printf("Event: %s - %s @ %s\n", ev->name, ev->event_date, ev->event_time);
    printf("People waiting: %d\n", pq->size);
    
    if (pq_is_empty(pq)) { 
        printf("\n(No one in waiting queue)\n"); 
        return; 
    }
    
    printf("\nList of people in queue (FIFO order):\n");
    for (int i = 0; i < pq->size; ++i) {
        PriorityQueueNode *node = &pq->heap[i];
        printf(" %d) %s | phone: %s | email: %s | Requested: %d seat%s\n", 
               i + 1, node->username, node->phone, node->email, 
               node->num_seats, node->num_seats > 1 ? "s" : "");
    }
}

void show_all_bookings_admin(void) {
    printf("\n=== All Bookings Across Events ===\n");
    for (int e = 0; e < event_count; ++e) show_all_bookings_for_event(e);
}

void show_full_seatmap_all_events(void) {
    for (int e = 0; e < event_count; ++e) display_seat_map(e);
}

int auto_assign_seat(int event_idx, int *out_row, int *out_col) {
    if (event_idx < 0 || event_idx >= event_count) return 0;
    Event *ev = &events[event_idx];
    for (int r = 0; r < ev->rows; ++r) {
        for (int c = 0; c < ev->cols; ++c) {
            int idx = seat_index(ev, r, c);
            if (!ev->seats[idx]) { *out_row = r; *out_col = c; return 1; }
        }
    }
    return 0;
}

static void enqueue_waiting(int event_idx, const User *user, int num_seats) {
    Event *ev = &events[event_idx];
    pq_insert(ev->wait_queue, user->username, user->phone, user->email, num_seats);
    printf("Added to waiting queue for %s (requested %d seat%s).\n", ev->name, num_seats, num_seats > 1 ? "s" : "");
}

static int dequeue_waiting(int event_idx, char *out_username, char *out_phone, char *out_email, int *out_num_seats) {
    Event *ev = &events[event_idx];
    return pq_extract_min(ev->wait_queue, out_username, out_phone, out_email, out_num_seats);
}

int book_seat_for_user(int event_idx, User *user) {
    printf("\nHow many seats would you like to book? (1-10): ");
    int num_seats = read_int();
    if (num_seats < 1 || num_seats > 10) {
        printf("Invalid number. Please choose 1-10 seats.\n");
        return 0;
    }
    return book_multiple_seats_for_user(event_idx, user, num_seats);
}

int auto_assign_multiple_seats(int event_idx, int num_seats, int rows_out[], int cols_out[]) {
    if (event_idx < 0 || event_idx >= event_count) return 0;
    Event *ev = &events[event_idx];
    
    /* Try to find adjacent seats in the same row first */
    for (int r = 0; r < ev->rows; ++r) {
        int consecutive = 0;
        int start_col = -1;
        for (int c = 0; c < ev->cols; ++c) {
            if (!ev->seats[seat_index(ev, r, c)]) {
                if (consecutive == 0) start_col = c;
                consecutive++;
                if (consecutive == num_seats) {
                    /* Found enough consecutive seats */
                    for (int i = 0; i < num_seats; ++i) {
                        rows_out[i] = r;
                        cols_out[i] = start_col + i;
                    }
                    return 1;
                }
            } else {
                consecutive = 0;
            }
        }
    }
    
    /* If no consecutive seats found, just assign any available */
    int assigned = 0;
    for (int r = 0; r < ev->rows && assigned < num_seats; ++r) {
        for (int c = 0; c < ev->cols && assigned < num_seats; ++c) {
            if (!ev->seats[seat_index(ev, r, c)]) {
                rows_out[assigned] = r;
                cols_out[assigned] = c;
                assigned++;
            }
        }
    }
    return (assigned == num_seats);
}

int book_multiple_seats_for_user(int event_idx, User *user, int num_seats) {
    if (event_idx < 0 || event_idx >= event_count) { printf("Invalid event.\n"); return 0; }
    if (event_count == 0) { printf("No events available at the moment. Please check later.\n"); return 0; }

    display_seat_map(event_idx);
    
    Event *ev = &events[event_idx];
    int rows[10], cols[10];  /* max 10 seats */
    int original_num_seats = num_seats;  /* Track original request */
    int is_partial_booking = 0;  /* Flag for partial booking */
    
    /* Ask user: manual or auto selection */
    printf("\nChoose seats: 1) Manually select  2) Auto-assign\nEnter choice: ");
    int choice = read_int();
    
    if (choice == 1) {
        /* Manual seat selection */
        printf("\nEnter %d seat(s) (format: A1, B3, etc.):\n", num_seats);
        for (int i = 0; i < num_seats; ++i) {
            char seat_input[16];
            printf("Seat %d: ", i + 1);
            read_line(seat_input, sizeof(seat_input));
            
            if (strlen(seat_input) < 2) { 
                printf("Invalid seat input. Booking cancelled.\n"); 
                return 0; 
            }
            
            char rch = toupper((unsigned char)seat_input[0]);
            int cnum = atoi(seat_input + 1);
            int row = rch - 'A';
            int col = cnum - 1;
            
            if (row < 0 || row >= ev->rows || col < 0 || col >= ev->cols) { 
                printf("Seat %s out of range. Booking cancelled.\n", seat_input); 
                return 0; 
            }
            
            if (ev->seats[seat_index(ev, row, col)]) { 
                printf("Seat %s already booked. Booking cancelled.\n", seat_input); 
                return 0; 
            }
            
            /* Check for duplicates */
            for (int j = 0; j < i; ++j) {
                if (rows[j] == row && cols[j] == col) {
                    printf("You already selected seat %s. Booking cancelled.\n", seat_input);
                    return 0;
                }
            }
            
            rows[i] = row;
            cols[i] = col;
        }
    } else if (choice == 2) {
        /* Auto-assign seats */
        int available = get_available_seat_count(event_idx);
        
        if (available == 0) {
            /* No seats available at all */
            printf("No seats available. Join waiting queue for %d seat%s? (y/n): ", 
                   num_seats, num_seats > 1 ? "s" : "");
            char yn[8]; read_line(yn, sizeof(yn));
            if (yn[0] == 'y' || yn[0] == 'Y') { enqueue_waiting(event_idx, user, num_seats); return 2; }
            return 0;
        } else if (available < num_seats) {
            /* Partial availability */
            printf("\nOnly %d seat%s available (you requested %d).\n", 
                   available, available > 1 ? "s" : "", num_seats);
            printf("Options:\n");
            printf("  1) Book %d seat%s now and join waiting queue for remaining %d\n", 
                   available, available > 1 ? "s" : "", num_seats - available);
            printf("  2) Join waiting queue for all %d seats\n", num_seats);
            printf("  3) Cancel booking\n");
            printf("Enter choice (1-3): ");
            int partial_choice = read_int();
            
            if (partial_choice == 1) {
                /* Book available seats, queue for rest */
                if (!auto_assign_multiple_seats(event_idx, available, rows, cols)) {
                    printf("Error assigning seats.\n");
                    return 0;
                }
                /* Continue with partial booking, then add to queue */
                num_seats = available;  /* Update num_seats for the booking process */
                is_partial_booking = 1;  /* Flag that we need to queue remaining seats */
            } else if (partial_choice == 2) {
                /* Queue for all seats */
                enqueue_waiting(event_idx, user, num_seats);
                return 2;
            } else {
                printf("Booking cancelled.\n");
                return 0;
            }
        } else {
            /* Enough seats available */
            if (!auto_assign_multiple_seats(event_idx, num_seats, rows, cols)) {
                printf("Error assigning seats.\n");
                return 0;
            }
        }
    } else {
        printf("Invalid choice.\n");
        return 0;
    }
    
    /* Show selected seats */
    printf("\nSelected seats: ");
    for (int i = 0; i < num_seats; ++i) {
        printf("[%c%d] ", 'A' + rows[i], cols[i] + 1);
    }
    printf("\n");

    char code_entered[64];
    printf("Enter discount code (or NA if none): ");
    read_line(code_entered, sizeof(code_entered));

    double base_price_per_seat = apply_discount_event(ev->base_price, code_entered, ev->discount_code, ev->discount_percent);
    double total_price = base_price_per_seat * num_seats;
    
    printf("\nPrice per seat: Rs.%.2f (%.0f%% full)\n", base_price_per_seat, get_occupancy_percent(event_idx));
    printf("Total for %d seats: Rs.%.2f\n", num_seats, total_price);
    printf("Confirm booking? (y/n): ");
    char yn2[8];
    read_line(yn2, sizeof(yn2));
    if (!(yn2[0] == 'y' || yn2[0] == 'Y')) { printf("Booking cancelled.\n"); return 0; }

    /* Create bookings */
    char booking_id[32];
    generate_booking_id(booking_id, event_idx, booking_counter);
    time_t now = time(NULL);
    
    for (int i = 0; i < num_seats; ++i) {
        ev->seats[seat_index(ev, rows[i], cols[i])] = 1;
        Booking *b = (Booking *)malloc(sizeof(Booking));
        if (!b) { printf("Memory error.\n"); return 0; }
        
        strncpy(b->username, user->username, MAX_USERNAME - 1); b->username[MAX_USERNAME - 1] = '\0';
        strncpy(b->display_name, user->username, MAX_NAME - 1); b->display_name[MAX_NAME - 1] = '\0';
        strncpy(b->phone, user->phone, MAX_PHONE - 1); b->phone[MAX_PHONE - 1] = '\0';
        strncpy(b->email, user->email, MAX_EMAIL - 1); b->email[MAX_EMAIL - 1] = '\0';
        strncpy(b->booking_id, booking_id, 31); b->booking_id[31] = '\0';
        b->event_id = event_idx;
        b->row = rows[i]; b->col = cols[i];
        b->price_paid = base_price_per_seat;
        b->timestamp = now;
        b->num_seats = num_seats;
        b->next = ev->bookings_head;
        ev->bookings_head = b;
    }
    
    ev->revenue += total_price;
    ev->total_bookings++;
    
    printf("\nBooking successful!\n");
    printf("  Booking ID: %s\n", booking_id);
    printf("  Event: %s\n", ev->name);
    printf("  Seats: ");
    for (int i = 0; i < num_seats; ++i) {
        printf("[%c%d] ", 'A' + rows[i], cols[i] + 1);
    }
    printf("\n  Total Paid: Rs.%.2f\n", total_price);
    
    /* If this was a partial booking, add remaining seats to waiting queue */
    if (is_partial_booking) {
        int remaining_seats = original_num_seats - num_seats;
        printf("\n  -> Added to waiting queue for %d more seat%s.\n", 
               remaining_seats, remaining_seats > 1 ? "s" : "");
        enqueue_waiting(event_idx, user, remaining_seats);
    }
    
    return 1;
}

int cancel_seat_by_user(int event_idx, const User *user) {
    if (event_idx < 0 || event_idx >= event_count) { printf("Invalid event.\n"); return 0; }
    Event *ev = &events[event_idx];
    
    /* First, show user's bookings for this event */
    printf("\n=== Your Tickets for %s ===\n", ev->name);
    Booking *b = ev->bookings_head;
    int count = 0;
    Booking *booking_ptrs[100]; /* Store pointers */
    
    while (b) {
        if (strcmp(b->username, user->username) == 0) {
            booking_ptrs[count] = b;
            printf(" %d) Seat: [%c%d] | Booking ID: %s | Paid: Rs.%.2f\n", 
                   count + 1, 'A' + b->row, b->col + 1, b->booking_id, b->price_paid);
            count++;
        }
        b = b->next;
    }
    
    if (count == 0) {
        printf("You have no bookings for this event.\n");
        return 0;
    }
    
    printf("\nHow many tickets do you want to cancel? (1-%d, or 0 to go back): ", count);
    int num_to_cancel = read_int();
    
    if (num_to_cancel == 0) {
        printf("Cancellation aborted.\n");
        return 0;
    }
    
    if (num_to_cancel < 1 || num_to_cancel > count) {
        printf("Invalid number.\n");
        return 0;
    }
    
    /* Ask which specific seats to cancel */
    int choices[100];
    printf("\nEnter the ticket numbers to cancel (space-separated):\n");
    for (int i = 0; i < num_to_cancel; i++) {
        printf("Ticket %d: ", i + 1);
        int ch = read_int();
        if (ch < 1 || ch > count) {
            printf("Invalid ticket number. Cancellation aborted.\n");
            return 0;
        }
        /* Check for duplicates */
        for (int j = 0; j < i; j++) {
            if (choices[j] == ch) {
                printf("You already selected ticket %d. Cancellation aborted.\n", ch);
                return 0;
            }
        }
        choices[i] = ch;
    }
    
    /* Show confirmation */
    printf("\nYou are about to cancel:\n");
    double total_refund = 0.0;
    for (int i = 0; i < num_to_cancel; i++) {
        Booking *sel = booking_ptrs[choices[i] - 1];
        double refund = calculate_refund(sel->price_paid);
        total_refund += refund;
        printf("  - Seat [%c%d] | Refund: Rs.%.2f\n", 'A' + sel->row, sel->col + 1, refund);
    }
    printf("Total refund: Rs.%.2f\n", total_refund);
    printf("\nConfirm cancellation? (y/n): ");
    char confirm[8];
    read_line(confirm, sizeof(confirm));
    if (!(confirm[0] == 'y' || confirm[0] == 'Y')) {
        printf("Cancellation aborted.\n");
        return 0;
    }
    
    /* Process each cancellation */
    printf("\n=== Processing Cancellations ===\n");
    int cancelled_count = 0;
    
    for (int i = 0; i < num_to_cancel; i++) {
        Booking *selected = booking_ptrs[choices[i] - 1];
        Booking *prev = NULL, *cur = ev->bookings_head;
        
        while (cur) {
            if (cur == selected) {
                int r = cur->row, c = cur->col;
                double refund_amount = calculate_refund(cur->price_paid);
                
                printf("\nCancelled: Seat [%c%d] | Booking ID: %s | Refund: Rs.%.2f\n", 
                       'A' + r, c + 1, cur->booking_id, refund_amount);
                
                if (prev) prev->next = cur->next; 
                else ev->bookings_head = cur->next;
                free(cur);
                ev->seats[seat_index(ev, r, c)] = 0;
                ev->revenue -= refund_amount;
                cancelled_count++;
                
                /* Try to assign to waiting list */
                char namebuf[MAX_USERNAME], phonebuf[MAX_PHONE], emailbuf[MAX_EMAIL];
                int requested_seats = 0;
                if (dequeue_waiting(event_idx, namebuf, phonebuf, emailbuf, &requested_seats)) {
                    /* Try to auto-assign the requested number of seats */
                    int temp_rows[10], temp_cols[10];
                    int seats_available = get_available_seat_count(event_idx);
                    int seats_to_book = (requested_seats <= seats_available) ? requested_seats : seats_available;
                    
                    if (seats_to_book > 0 && auto_assign_multiple_seats(event_idx, seats_to_book, temp_rows, temp_cols)) {
                        printf("  -> Assigned %d seat%s to waiting customer: %s (%s)\n", 
                               seats_to_book, seats_to_book > 1 ? "s" : "", namebuf, phonebuf);
                        
                        char new_booking_id[32];
                        generate_booking_id(new_booking_id, event_idx, booking_counter);
                        
                        /* Create bookings for all assigned seats */
                        for (int j = 0; j < seats_to_book; j++) {
                            Booking *b = (Booking *)malloc(sizeof(Booking));
                            if (b) {
                                strncpy(b->username, namebuf, MAX_USERNAME - 1); b->username[MAX_USERNAME - 1] = '\0';
                                strncpy(b->display_name, namebuf, MAX_NAME - 1); b->display_name[MAX_NAME - 1] = '\0';
                                strncpy(b->phone, phonebuf, MAX_PHONE - 1); b->phone[MAX_PHONE - 1] = '\0';
                                strncpy(b->email, emailbuf, MAX_EMAIL - 1); b->email[MAX_EMAIL - 1] = '\0';
                                strncpy(b->booking_id, new_booking_id, 31); b->booking_id[31] = '\0';
                                b->event_id = event_idx;
                                b->row = temp_rows[j]; b->col = temp_cols[j];
                                b->timestamp = time(NULL);
                                b->num_seats = seats_to_book;
                                b->price_paid = ev->base_price;
                                b->next = ev->bookings_head;
                                ev->bookings_head = b;
                                ev->seats[seat_index(ev, temp_rows[j], temp_cols[j])] = 1;
                                ev->revenue += b->price_paid;
                            }
                        }
                        
                        /* If not all seats could be assigned, re-queue for remaining */
                        if (seats_to_book < requested_seats) {
                            printf("  -> Re-queuing %s for remaining %d seat%s\n", 
                                   namebuf, requested_seats - seats_to_book, 
                                   (requested_seats - seats_to_book) > 1 ? "s" : "");
                            pq_insert(ev->wait_queue, namebuf, phonebuf, emailbuf, requested_seats - seats_to_book);
                        }
                    } else {
                        /* Re-queue if seats couldn't be assigned */
                        printf("  -> Re-queuing %s (seats not yet available)\n", namebuf);
                        pq_insert(ev->wait_queue, namebuf, phonebuf, emailbuf, requested_seats);
                    }
                }
                break;
            }
            prev = cur;
            cur = cur->next;
        }
    }
    
    if (cancelled_count > 0) {
        printf("\nSuccessfully cancelled %d ticket(s).\n", cancelled_count);
        return 1;
    }
    return 0;
}

void view_my_bookings(const User *user) {
    printf("\n+============================================================+\n");
    printf("|          MY BOOKINGS - %s\n", user->username);
    printf("+============================================================+\n");
    int found = 0;
    for (int e = 0; e < event_count; ++e) {
        Booking *b = events[e].bookings_head;
        while (b) {
            if (strcmp(b->username, user->username) == 0) {
                char time_str[26];
                struct tm *tm_info = localtime(&b->timestamp);
                strftime(time_str, 26, "%Y-%m-%d %H:%M", tm_info);
                
                printf("\n Booking ID: %s\n", b->booking_id);
                printf("  Event: %s - %s @ %s\n", events[e].name, events[e].event_date, events[e].event_time);
                printf("  Seat: [%c%d]\n", 'A' + b->row, b->col + 1);
                printf("  Price Paid: Rs.%.2f\n", b->price_paid);
                printf("  Booked On: %s\n", time_str);
                printf(" -------------------------------------------------------\n");
                found = 1;
            }
            b = b->next;
        }
    }
    if (!found) printf("\n  (no bookings)\n");
}

/* Search bookings by ID, username, or phone */
void search_bookings_interactive(void) {
    printf("\nSearch bookings by:\n");
    printf("1) Booking ID\n");
    printf("2) Username\n");
    printf("3) Phone number\n");
    printf("Choose: ");
    int choice = read_int();
    
    char search_term[64];
    if (choice == 1) {
        printf("Enter Booking ID: ");
        read_line(search_term, sizeof(search_term));
    } else if (choice == 2) {
        printf("Enter Username: ");
        read_line(search_term, sizeof(search_term));
    } else if (choice == 3) {
        printf("Enter Phone: ");
        read_line(search_term, sizeof(search_term));
    } else {
        printf("Invalid choice.\n");
        return;
    }
    
    printf("\n+============================================================+\n");
    printf("|              SEARCH RESULTS                                |\n");
    printf("+============================================================+\n");
    
    int found = 0;
    for (int e = 0; e < event_count; ++e) {
        Booking *b = events[e].bookings_head;
        while (b) {
            int match = 0;
            if (choice == 1 && strcmp(b->booking_id, search_term) == 0) match = 1;
            else if (choice == 2 && strcmp(b->username, search_term) == 0) match = 1;
            else if (choice == 3 && strcmp(b->phone, search_term) == 0) match = 1;
            
            if (match) {
                char time_str[26];
                struct tm *tm_info = localtime(&b->timestamp);
                strftime(time_str, 26, "%Y-%m-%d %H:%M", tm_info);
                
                printf("\n Booking ID: %s\n", b->booking_id);
                printf("  Customer: %s\n", b->username);
                printf("  Event: %s - %s @ %s\n", events[e].name, events[e].event_date, events[e].event_time);
                printf("  Seat: [%c%d]\n", 'A' + b->row, b->col + 1);
                printf("  Price Paid: Rs.%.2f\n", b->price_paid);
                printf("  Booked On: %s\n", time_str);
                printf("  Contact: %s | %s\n", b->phone, b->email);
                printf(" -------------------------------------------------------\n");
                found = 1;
            }
            b = b->next;
        }
    }
    
    if (!found) {
        printf("\n  No bookings found matching '%s'\n", search_term);
    }
}

/* Cancel booking by ID (admin function) */
int cancel_booking_by_id(const char *booking_id) {
    for (int e = 0; e < event_count; ++e) {
        Event *ev = &events[e];
        Booking *prev = NULL, *cur = ev->bookings_head;
        while (cur) {
            if (strcmp(cur->booking_id, booking_id) == 0) {
                int r = cur->row, c = cur->col;
                double refund_amount = calculate_refund(cur->price_paid);
                
                printf("\nCancelling booking %s...\n", booking_id);
                printf("  Customer: %s\n", cur->username);
                printf("  Seat: %c%d | Refund: Rs.%.2f\n", 'A' + r, c + 1, refund_amount);
                
                if (prev) prev->next = cur->next; else ev->bookings_head = cur->next;
                free(cur);
                ev->seats[seat_index(ev, r, c)] = 0;
                ev->revenue -= refund_amount;
                
                /* Try to assign to waiting queue */
                char namebuf[MAX_USERNAME], phonebuf[MAX_PHONE], emailbuf[MAX_EMAIL];
                int requested_seats = 0;
                if (dequeue_waiting(e, namebuf, phonebuf, emailbuf, &requested_seats)) {
                    printf("-> Allocated to waiting customer: %s (%d seat%s requested)\n", 
                           namebuf, requested_seats, requested_seats > 1 ? "s" : "");
                    /* Note: This simplified version just notifies. Full implementation would auto-book. */
                }
                
                return 1;
            }
            prev = cur;
            cur = cur->next;
        }
    }
    printf("Booking ID '%s' not found.\n", booking_id);
    return 0;
}

/* Save all bookings to file */
void save_bookings_to_file(const char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        printf("Failed to open %s for writing bookings.\n", path);
        return;
    }
    
    for (int e = 0; e < event_count; ++e) {
        Event *ev = &events[e];
        Booking *b = ev->bookings_head;
        while (b) {
            /* Format: event_id|username|display_name|phone|email|row|col|price_paid|booking_id|timestamp|num_seats */
            fprintf(fp, "%d|%s|%s|%s|%s|%d|%d|%.2f|%s|%ld|%d\n",
                    b->event_id, b->username, b->display_name, b->phone, b->email,
                    b->row, b->col, b->price_paid, b->booking_id, (long)b->timestamp, b->num_seats);
            b = b->next;
        }
    }
    fclose(fp);
}

/* Load all bookings from file */
void load_bookings_from_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return; /* OK if file doesn't exist yet */
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        /* Parse: event_id|username|display_name|phone|email|row|col|price_paid|booking_id|timestamp|num_seats */
        int event_id, row, col, num_seats;
        double price_paid;
        long timestamp;
        char username[MAX_USERNAME], display_name[MAX_NAME], phone[MAX_PHONE], email[MAX_EMAIL], booking_id[32];
        
        char *tok;
        tok = strtok(line, "|"); if (!tok) continue; event_id = atoi(tok);
        tok = strtok(NULL, "|"); if (!tok) continue; strncpy(username, tok, MAX_USERNAME-1); username[MAX_USERNAME-1] = '\0';
        tok = strtok(NULL, "|"); if (!tok) continue; strncpy(display_name, tok, MAX_NAME-1); display_name[MAX_NAME-1] = '\0';
        tok = strtok(NULL, "|"); if (!tok) continue; strncpy(phone, tok, MAX_PHONE-1); phone[MAX_PHONE-1] = '\0';
        tok = strtok(NULL, "|"); if (!tok) continue; strncpy(email, tok, MAX_EMAIL-1); email[MAX_EMAIL-1] = '\0';
        tok = strtok(NULL, "|"); if (!tok) continue; row = atoi(tok);
        tok = strtok(NULL, "|"); if (!tok) continue; col = atoi(tok);
        tok = strtok(NULL, "|"); if (!tok) continue; price_paid = atof(tok);
        tok = strtok(NULL, "|"); if (!tok) continue; strncpy(booking_id, tok, 31); booking_id[31] = '\0';
        tok = strtok(NULL, "|"); if (!tok) continue; timestamp = atol(tok);
        tok = strtok(NULL, "|"); if (!tok) continue; num_seats = atoi(tok);
        
        /* Validate event_id */
        if (event_id < 0 || event_id >= event_count) continue;
        
        Event *ev = &events[event_id];
        
        /* Create booking and add to event */
        Booking *b = (Booking *)malloc(sizeof(Booking));
        if (!b) continue;
        
        strncpy(b->username, username, MAX_USERNAME-1); b->username[MAX_USERNAME-1] = '\0';
        strncpy(b->display_name, display_name, MAX_NAME-1); b->display_name[MAX_NAME-1] = '\0';
        strncpy(b->phone, phone, MAX_PHONE-1); b->phone[MAX_PHONE-1] = '\0';
        strncpy(b->email, email, MAX_EMAIL-1); b->email[MAX_EMAIL-1] = '\0';
        strncpy(b->booking_id, booking_id, 31); b->booking_id[31] = '\0';
        b->event_id = event_id;
        b->row = row;
        b->col = col;
        b->price_paid = price_paid;
        b->timestamp = (time_t)timestamp;
        b->num_seats = num_seats;
        b->next = ev->bookings_head;
        ev->bookings_head = b;
        
        /* Mark seat as booked */
        ev->seats[seat_index(ev, row, col)] = 1;
        
        /* Update event stats */
        ev->revenue += price_paid;
        ev->total_bookings++;
    }
    
    fclose(fp);
}